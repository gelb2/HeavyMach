package com.gelb2.heavymach

import android.app.Activity
import android.content.*
import android.graphics.*
import android.hardware.*
import android.media.*
import android.net.Uri
import android.os.*
import android.view.*
import android.widget.*
import org.json.JSONObject
import kotlin.math.roundToInt

class GameActivity : Activity(), SurfaceHolder.Callback, SensorEventListener {
 private lateinit var root:FrameLayout
 private lateinit var surface:SurfaceView
 private lateinit var error:TextView
 private lateinit var recoveryInput:EditText
 private var changingText=false
 private var textRevision=-1L
 private var clipboardRevision=-1L
 private lateinit var sensors:SensorManager
 private lateinit var audio:AudioManager
 private lateinit var focus:AudioFocusRequest
 private var resumed=false
 private var windowFocused=false
 private var audioGranted=false
 private var lastState=JSONObject()
 private var lastIntent:Intent?=null
 private val touchPositions=mutableMapOf<Int,Pair<Float,Float>>()
 private var backCallback:android.window.OnBackInvokedCallback?=null
 private val noisy=object:BroadcastReceiver(){override fun onReceive(c:Context?,i:Intent?){GameEngine.command("interrupt")}}
 override fun onCreate(state:Bundle?){
  super.onCreate(state)
  root=FrameLayout(this);surface=SurfaceView(this);root.addView(surface,FrameLayout.LayoutParams(-1,-1));setContentView(root)
  window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
  if(Build.VERSION.SDK_INT>=28)window.attributes=window.attributes.apply{layoutInDisplayCutoutMode=WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES}
  if(Build.VERSION.SDK_INT>=30){window.setDecorFitsSystemWindows(false);window.insetsController?.apply{hide(WindowInsets.Type.systemBars());systemBarsBehavior=WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE}}
  else @Suppress("DEPRECATION") run{window.decorView.systemUiVisibility=View.SYSTEM_UI_FLAG_FULLSCREEN or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION or View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION or View.SYSTEM_UI_FLAG_LAYOUT_STABLE}
  surface.holder.addCallback(this);surface.isFocusableInTouchMode=true;surface.requestFocus()
  surface.setOnTouchListener{v,e->
   if(e.actionMasked==MotionEvent.ACTION_DOWN&&currentFocus is EditText){(getSystemService(INPUT_METHOD_SERVICE) as android.view.inputmethod.InputMethodManager).hideSoftInputFromWindow(recoveryInput.windowToken,0);recoveryInput.clearFocus();surface.requestFocus()}
   val stateCode=when(e.actionMasked){MotionEvent.ACTION_DOWN,MotionEvent.ACTION_POINTER_DOWN->0;MotionEvent.ACTION_UP,MotionEvent.ACTION_POINTER_UP->2;MotionEvent.ACTION_CANCEL->3;else->1}
   if(e.actionMasked==MotionEvent.ACTION_MOVE||e.actionMasked==MotionEvent.ACTION_CANCEL){
    for(i in 0 until e.pointerCount){val id=e.getPointerId(i);val point=e.getX(i) to e.getY(i)
     if(stateCode==3||touchPositions[id]!=point){GameEngine.touch(id,stateCode,point.first/v.width,point.second/v.height);touchPositions[id]=point}
    }
    if(stateCode==3)touchPositions.clear()
   } else {val i=e.actionIndex;val id=e.getPointerId(i);GameEngine.touch(id,stateCode,e.getX(i)/v.width,e.getY(i)/v.height);if(stateCode==0)touchPositions[id]=e.getX(i) to e.getY(i) else touchPositions.remove(id)}
   true
  }
  root.setOnApplyWindowInsetsListener{_,i->
   if(Build.VERSION.SDK_INT>=30){val s=i.getInsets(WindowInsets.Type.displayCutout() or WindowInsets.Type.systemBars());GameEngine.insets(s.left,s.top,s.right,s.bottom)}
   else if(Build.VERSION.SDK_INT>=28){val c=i.displayCutout;GameEngine.insets(c?.safeInsetLeft?:0,c?.safeInsetTop?:0,c?.safeInsetRight?:0,c?.safeInsetBottom?:0)}
   i
  }
  // Only actual text entry is native. All actions and game panels are GL sprites.
  recoveryInput=EditText(this).apply{
   hint="Paste an HM1- code";setSingleLine(false);maxLines=3
   imeOptions=android.view.inputmethod.EditorInfo.IME_FLAG_NO_EXTRACT_UI
   filters=arrayOf(android.text.InputFilter.LengthFilter(24000))
   inputType=android.text.InputType.TYPE_CLASS_TEXT or android.text.InputType.TYPE_TEXT_FLAG_MULTI_LINE or android.text.InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS
   setTextColor(Color.rgb(241,215,151));setBackgroundColor(0xff171b16.toInt());textSize=12f
   setPadding(dp(6),dp(3),dp(6),dp(3));visibility=View.GONE
   addTextChangedListener(object:android.text.TextWatcher{
    override fun beforeTextChanged(s:CharSequence?,start:Int,count:Int,after:Int){}
    override fun onTextChanged(s:CharSequence?,start:Int,before:Int,count:Int){if(!changingText)GameEngine.text("recovery",s?.toString()?:"")}
    override fun afterTextChanged(e:android.text.Editable?){}
   })
  }
  root.addView(recoveryInput,FrameLayout.LayoutParams(1,1))
  error=TextView(this).apply{setBackgroundColor(0xf0161616.toInt());setTextColor(Color.WHITE);textSize=16f;setPadding(dp(24),dp(24),dp(24),dp(24));visibility=View.GONE};root.addView(error,FrameLayout.LayoutParams(-1,-1))
  sensors=getSystemService(SENSOR_SERVICE) as SensorManager;audio=getSystemService(AUDIO_SERVICE) as AudioManager
  focus=AudioFocusRequest.Builder(AudioManager.AUDIOFOCUS_GAIN).setAudioAttributes(AudioAttributes.Builder().setUsage(AudioAttributes.USAGE_GAME).setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION).build()).setWillPauseWhenDucked(true).setOnAudioFocusChangeListener { value->audioGranted=value==AudioManager.AUDIOFOCUS_GAIN;GameEngine.command("focus",if(audioGranted)1 else 0)}.build()
  if(Build.VERSION.SDK_INT>=33)registerReceiver(noisy,IntentFilter(AudioManager.ACTION_AUDIO_BECOMING_NOISY),RECEIVER_NOT_EXPORTED) else @Suppress("DEPRECATION") registerReceiver(noisy,IntentFilter(AudioManager.ACTION_AUDIO_BECOMING_NOISY))
  if(Build.VERSION.SDK_INT>=33){val callback=android.window.OnBackInvokedCallback { GameEngine.key(0x29,true);GameEngine.key(0x29,false) };backCallback=callback;onBackInvokedDispatcher.registerOnBackInvokedCallback(android.window.OnBackInvokedDispatcher.PRIORITY_DEFAULT,callback)}
  GameEngine.bind(this);GameEngine.initialize(this);lastIntent=intent
 }
 override fun onResume(){super.onResume();resumed=true;sensors.getDefaultSensor(Sensor.TYPE_ACCELEROMETER)?.let{sensors.registerListener(this,it,20_000)};audioGranted=audio.requestAudioFocus(focus)==AudioManager.AUDIOFOCUS_REQUEST_GRANTED;updateActive()}
 override fun onPause(){resumed=false;touchPositions.clear();GameEngine.active(false);sensors.unregisterListener(this);audio.abandonAudioFocusRequest(focus);audioGranted=false;super.onPause()}
 override fun onDestroy(){if(Build.VERSION.SDK_INT>=33)backCallback?.let{onBackInvokedDispatcher.unregisterOnBackInvokedCallback(it)};unregisterReceiver(noisy);super.onDestroy()}
 override fun onWindowFocusChanged(value:Boolean){super.onWindowFocusChanged(value);windowFocused=value;if(!value)touchPositions.clear();updateActive()}
 private fun updateActive(){if(!::audio.isInitialized)return;GameEngine.active(resumed&&windowFocused);GameEngine.command("focus",if(audioGranted)1 else 0)}
 override fun surfaceCreated(h:SurfaceHolder){}
 override fun surfaceChanged(h:SurfaceHolder,format:Int,width:Int,height:Int){
  if(Build.VERSION.SDK_INT>=30)h.surface.setFrameRate(60f,Surface.FRAME_RATE_COMPATIBILITY_FIXED_SOURCE)
  GameEngine.surface(h.surface);root.requestApplyInsets();updateActive();lastIntent?.let{acceptTestIntent(it);lastIntent=null}
 }
 override fun surfaceDestroyed(h:SurfaceHolder){GameEngine.surface(null)}
 override fun onAccuracyChanged(sensor:Sensor?,accuracy:Int){}
 override fun onSensorChanged(e:SensorEvent){
  // Android reports acceleration opposite to CoreMotion gravity. Remap into the
  // displayed landscape axes: original accelY is the negative screen-right axis.
  val rotation=if(Build.VERSION.SDK_INT>=30)display?.rotation?:Surface.ROTATION_0 else @Suppress("DEPRECATION") windowManager.defaultDisplay.rotation
  val x=e.values[0].toDouble();val y=e.values[1].toDouble();val z=e.values[2].toDouble();val g=SensorManager.GRAVITY_EARTH.toDouble()
  val right=when(rotation){Surface.ROTATION_90->-y;Surface.ROTATION_180->-x;Surface.ROTATION_270->y;else->x}
  val up=when(rotation){Surface.ROTATION_90->x;Surface.ROTATION_180->-y;Surface.ROTATION_270->-x;else->y}
  GameEngine.sensor(-up/g,-right/g,-z/g)
 }
 override fun onNewIntent(intent:Intent){super.onNewIntent(intent);setIntent(intent);acceptTestIntent(intent)}
 private fun acceptTestIntent(i:Intent){if(!BuildConfig.CHEAT)return;val cmd=i.getStringExtra("command")?:return;GameEngine.command(cmd,i.getIntExtra("a",0),i.getIntExtra("b",0),i.getIntExtra("c",0),i.getIntExtra("d",0))}
 @Deprecated("Android back callback bridge") override fun onBackPressed(){GameEngine.key(0x29,true);GameEngine.key(0x29,false)}
 override fun dispatchKeyEvent(e:KeyEvent):Boolean{if(currentFocus is EditText)return super.dispatchKeyEvent(e);val code=hid(e.keyCode);if(code!=0){GameEngine.key(code,e.action==KeyEvent.ACTION_DOWN);return true};return super.dispatchKeyEvent(e)}
 private fun hid(k:Int):Int=when(k){in KeyEvent.KEYCODE_A..KeyEvent.KEYCODE_Z->k-KeyEvent.KEYCODE_A+4;in KeyEvent.KEYCODE_1..KeyEvent.KEYCODE_9->k-KeyEvent.KEYCODE_1+30;KeyEvent.KEYCODE_0->39;KeyEvent.KEYCODE_ENTER->40;KeyEvent.KEYCODE_ESCAPE,KeyEvent.KEYCODE_BACK->41;KeyEvent.KEYCODE_DEL->42;KeyEvent.KEYCODE_TAB->43;KeyEvent.KEYCODE_SPACE->44;KeyEvent.KEYCODE_DPAD_RIGHT->79;KeyEvent.KEYCODE_DPAD_LEFT->80;KeyEvent.KEYCODE_DPAD_DOWN->81;KeyEvent.KEYCODE_DPAD_UP->82;KeyEvent.KEYCODE_SHIFT_LEFT->225;KeyEvent.KEYCODE_SHIFT_RIGHT->229;KeyEvent.KEYCODE_F2->59;KeyEvent.KEYCODE_F3->60;KeyEvent.KEYCODE_F6->63;else->0}
 private fun dp(n:Int)=(n*resources.displayMetrics.density).roundToInt()
 internal fun showState(raw:String){
  if(isFinishing||isDestroyed)return
  val s=JSONObject(raw);lastState=s
  val visible=s.optBoolean("textEntry")
  if(!visible&&recoveryInput.visibility==View.VISIBLE){
   (getSystemService(INPUT_METHOD_SERVICE) as android.view.inputmethod.InputMethodManager).hideSoftInputFromWindow(recoveryInput.windowToken,0)
   recoveryInput.clearFocus();surface.requestFocus()
  }
  recoveryInput.visibility=if(visible)View.VISIBLE else View.GONE
  s.optJSONArray("textRect")?.let{r->
   recoveryInput.layoutParams=FrameLayout.LayoutParams((r.getDouble(2)*root.width).toInt().coerceAtLeast(1),(r.getDouble(3)*root.height).toInt().coerceAtLeast(1)).apply{leftMargin=(r.getDouble(0)*root.width).toInt();topMargin=(r.getDouble(1)*root.height).toInt()}
  }
  val revision=s.optLong("textRevision")
  if(revision!=textRevision){textRevision=revision;val text=s.optString("textValue");if(text!=recoveryInput.text.toString()){changingText=true;recoveryInput.setText(text);recoveryInput.setSelection(text.length);changingText=false}}
  val copy=s.optLong("clipboardRevision")
  if(copy!=clipboardRevision){clipboardRevision=copy;if(copy>0)(getSystemService(CLIPBOARD_SERVICE) as ClipboardManager).setPrimaryClip(ClipData.newPlainText("HeavyMach recovery",s.optString("clipboardValue")))}
  val message=s.optString("error");error.text=message;error.visibility=if(message.isEmpty())View.GONE else View.VISIBLE
  if(message.isNotEmpty())error.bringToFront()
  val url=s.optString("url");if(url.isNotEmpty())runCatching{startActivity(Intent(Intent.ACTION_VIEW,Uri.parse(url)))}
 }
}
