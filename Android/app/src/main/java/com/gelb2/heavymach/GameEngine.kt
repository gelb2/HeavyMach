package com.gelb2.heavymach
import android.content.Context
import android.os.Handler
import android.os.HandlerThread
import android.os.Looper
import android.view.Surface
import java.lang.ref.WeakReference
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit

/** One process-lifetime game. Activity/Surface recreation does not reconstruct its world. */
internal object GameEngine {
 private val thread = HandlerThread("HeavyMach GL").apply { start() }
 private val queue = Handler(thread.looper)
 private val main = Handler(Looper.getMainLooper())
 private var handle = 0L
 private var initialized = false
 private var running = false
 private var hasSurface = false
 private var due = 0L
 private var uiDue = 0L
 private var sensorDue = 0L
 private var listener = WeakReference<GameActivity>(null)
 private var sample = doubleArrayOf(0.0,0.0,-1.0)
 private var failure = ""
 private var lastUI = ""
 private const val interval = 16_666_667L
 fun bind(activity: GameActivity) { queue.post { listener = WeakReference(activity); lastUI="" } }
 fun initialize(context: Context) { val app=context.applicationContext; queue.post {
  if (!initialized) { initialized=true; try { handle=Native.create(app.assets,app.filesDir.absolutePath) } catch(e:Exception) { failure=e.message ?: e.toString() } }
  publish()
 } }
 private fun publish() {
  val state = if(failure.isNotEmpty()) org.json.JSONObject().put("error",failure).toString() else if(handle!=0L) Native.ui(handle) else "{}"
  if(state!=lastUI){lastUI=state;main.post { listener.get()?.showState(state) }}
 }
 private val frame = object: Runnable { override fun run() {
  if(!running||!hasSurface||handle==0L)return
  val now=System.nanoTime()
  if(now>=sensorDue){Native.sensor(handle,sample[0],sample[1],sample[2]);sensorDue=now+20_000_000L}
  Native.frame(handle,now/1_000_000_000.0)
  if(now>=uiDue){publish();uiDue=now+100_000_000L}
  // One tick per displayed frame. Missed intervals are skipped, never replayed.
  due += interval
  val end=System.nanoTime()
  if(due<end-interval)due=end
  queue.postDelayed(this,((due-end).coerceAtLeast(0)+999_999L)/1_000_000L)
 } }
 fun surface(surface: Surface?) {
  val done=CountDownLatch(1)
  queue.post { try {
   queue.removeCallbacks(frame);hasSurface=surface!=null
   if(handle!=0L)Native.surface(handle,surface)
   if(running&&hasSurface){due=System.nanoTime();queue.post(frame)}
   publish()
  } finally { done.countDown() } }
  // Surface must not be released by Android while EGL still owns its window.
  if(surface==null&&!done.await(5,TimeUnit.SECONDS)) throw IllegalStateException("Render thread did not release Surface")
 }
 fun active(value:Boolean){queue.post { running=value;queue.removeCallbacks(frame);if(handle!=0L)Native.active(handle,value);if(value&&hasSurface){due=System.nanoTime();sensorDue=0;queue.post(frame)};publish() }}
 fun sensor(x:Double,y:Double,z:Double){queue.post { sample=doubleArrayOf(x,y,z) }}
 fun insets(l:Int,t:Int,r:Int,b:Int){queue.post { if(handle!=0L)Native.insets(handle,l,t,r,b) }}
 fun touch(id:Int,state:Int,x:Float,y:Float){queue.post { if(handle!=0L)Native.touch(handle,id,state,x,y) }}
 fun key(code:Int,down:Boolean){queue.post { if(handle!=0L)Native.key(handle,code,down);publish() }}
 fun afterCommands(action:()->Unit){queue.post{action()}}
 fun command(name:String,a:Int=0,b:Int=0,c:Int=0,d:Int=0){queue.post { if(handle!=0L)Native.command(handle,name,a,b,c,d);publish() }}
 fun transfer(action:String,code:String="",source:Int=-1,target:Int=-1,done:(String)->Unit){queue.post {val result=Native.transfer(handle,action,code,source,target);publish();main.post{done(result)}}}
}
