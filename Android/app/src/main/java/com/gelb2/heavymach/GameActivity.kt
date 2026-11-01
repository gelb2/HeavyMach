package com.gelb2.heavymach

import android.app.Activity
import android.app.AlertDialog
import android.content.*
import android.graphics.*
import android.graphics.drawable.BitmapDrawable
import android.graphics.drawable.StateListDrawable
import android.hardware.*
import android.media.*
import android.net.Uri
import android.os.*
import android.view.*
import android.widget.*
import org.json.JSONObject
import kotlin.math.roundToInt

class GameActivity : Activity(), SurfaceHolder.Callback, SensorEventListener {
 private lateinit var transferButton:Button
 private lateinit var root:FrameLayout
 private lateinit var surface:SurfaceView
 private lateinit var tools:LinearLayout
 private lateinit var toggle:Button
 private lateinit var fallback:Button
 private lateinit var error:TextView
 private lateinit var cheats:Button
 private lateinit var directions:LinearLayout
 private lateinit var keys:LinearLayout
 private lateinit var sensors:SensorManager
 private lateinit var audio:AudioManager
 private lateinit var focus:AudioFocusRequest
 private var resumed=false
 private var windowFocused=false
 private var audioGranted=false
 private var lastState=JSONObject()
 private val buttons=mutableMapOf<String,Bitmap>()
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
  tools=LinearLayout(this).apply{orientation=LinearLayout.VERTICAL;visibility=View.GONE;setPadding(0,0,0,0)}
  val label=TextView(this).apply{id=100;setTextColor(Color.rgb(244,198,102));setBackgroundColor(0xee151718.toInt());gravity=Gravity.CENTER;textSize=12f;setPadding(4,4,4,4)};tools.addView(label,LinearLayout.LayoutParams(-1,dp(44)))
  for((caption,command) in listOf("PREVIEW" to "preview","SAMPLE SCENE" to "sample","RESET LAYOUT" to "reset","CANCEL" to "cancel","APPLY" to "apply")){val b=metalButton(caption){GameEngine.command(command)};b.tag=command;tools.addView(b,LinearLayout.LayoutParams(-1,dp(40)))}
  val scroll=ScrollView(this).apply{isFillViewport=false;addView(tools);visibility=View.GONE};root.addView(scroll,FrameLayout.LayoutParams(dp(168),-2,Gravity.TOP or Gravity.RIGHT).apply{topMargin=dp(48);rightMargin=dp(8);bottomMargin=dp(4)})
  toggle=metalButton("TOOLS"){GameEngine.command("tools")};root.addView(toggle,FrameLayout.LayoutParams(dp(168),dp(44),Gravity.TOP or Gravity.RIGHT).apply{rightMargin=dp(8);topMargin=dp(4)});toggle.visibility=View.GONE
  fallback=metalButton("SET GAME UI"){GameEngine.command("editor")};root.addView(fallback,FrameLayout.LayoutParams(dp(168),dp(44),Gravity.RIGHT or Gravity.CENTER_VERTICAL).apply{rightMargin=dp(8)});fallback.visibility=View.GONE
  error=TextView(this).apply{setBackgroundColor(0xf0161616.toInt());setTextColor(Color.WHITE);textSize=16f;setPadding(dp(24),dp(24),dp(24),dp(24));visibility=View.GONE};root.addView(error,FrameLayout.LayoutParams(-1,-1))
  cheats=metalButton("TEST"){showCheats()};root.addView(cheats,FrameLayout.LayoutParams(dp(76),dp(40),Gravity.TOP or Gravity.RIGHT));cheats.visibility=if(BuildConfig.CHEAT)View.VISIBLE else View.GONE
  directions=LinearLayout(this).apply{orientation=LinearLayout.HORIZONTAL;visibility=View.GONE}
  for((label,code) in listOf("◀" to 0x04,"▶" to 0x07,"JUMP" to 0x2c,"FIRE" to 0x09)){val b=metalButton(label){};b.setSingleLine(true);b.setAutoSizeTextTypeUniformWithConfiguration(9,16,1,android.util.TypedValue.COMPLEX_UNIT_SP);b.setOnTouchListener{_,e->if(lastState.optBoolean("editor")){val loc=IntArray(2);root.getLocationOnScreen(loc);val state=when(e.actionMasked){MotionEvent.ACTION_DOWN->0;MotionEvent.ACTION_UP->2;MotionEvent.ACTION_CANCEL->3;else->1};GameEngine.touch(1000+code,state,(e.rawX-loc[0])/root.width,(e.rawY-loc[1])/root.height);true}else{when(e.actionMasked){MotionEvent.ACTION_DOWN->GameEngine.key(code,true);MotionEvent.ACTION_UP,MotionEvent.ACTION_CANCEL->GameEngine.key(code,false)};true}};directions.addView(b,LinearLayout.LayoutParams(0,-1,1f))};root.addView(directions,FrameLayout.LayoutParams(dp(210),dp(42),Gravity.BOTTOM or Gravity.LEFT))
  keys=LinearLayout(this).apply{orientation=LinearLayout.VERTICAL;setBackgroundColor(0xf0181b1e.toInt());visibility=View.GONE};val keyScroll=ScrollView(this).apply{addView(keys);visibility=View.GONE};root.addView(keyScroll,FrameLayout.LayoutParams(-1,-1))
  transferButton=metalButton("TRANSFER PROGRESS"){showTransfer()};root.addView(transferButton,FrameLayout.LayoutParams(dp(168),dp(44),Gravity.TOP or Gravity.RIGHT).apply{rightMargin=dp(8);topMargin=dp(8)})
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
 private fun hid(k:Int):Int=when(k){in KeyEvent.KEYCODE_A..KeyEvent.KEYCODE_Z->k-KeyEvent.KEYCODE_A+4;in KeyEvent.KEYCODE_1..KeyEvent.KEYCODE_9->k-KeyEvent.KEYCODE_1+30;KeyEvent.KEYCODE_0->39;KeyEvent.KEYCODE_ENTER->40;KeyEvent.KEYCODE_ESCAPE,KeyEvent.KEYCODE_BACK->41;KeyEvent.KEYCODE_DEL->42;KeyEvent.KEYCODE_TAB->43;KeyEvent.KEYCODE_SPACE->44;KeyEvent.KEYCODE_DPAD_RIGHT->79;KeyEvent.KEYCODE_DPAD_LEFT->80;KeyEvent.KEYCODE_DPAD_DOWN->81;KeyEvent.KEYCODE_DPAD_UP->82;KeyEvent.KEYCODE_SHIFT_LEFT->225;KeyEvent.KEYCODE_SHIFT_RIGHT->229;KeyEvent.KEYCODE_F2->59;else->0}
 private fun dp(n:Int)=(n*resources.displayMetrics.density).roundToInt()
 private fun metalButton(text:String,click:()->Unit)=Button(this).apply{this.text=text;contentDescription=text;isAllCaps=false;textSize=16f;typeface=Typeface.create("sans-serif-condensed",Typeface.BOLD);setTextColor(Color.rgb(255,196,69));setShadowLayer(1.5f,1f,1f,Color.BLACK);setPadding(dp(5),0,dp(5),0);minimumWidth=0;minimumHeight=0;minWidth=0;minHeight=0;background=StateListDrawable().apply{addState(intArrayOf(android.R.attr.state_pressed),BitmapDrawable(resources,toolBitmap(true)));addState(intArrayOf(),BitmapDrawable(resources,toolBitmap(false)))};setOnClickListener{click()}}
 private fun toolBitmap(pressed:Boolean):Bitmap=buttons.getOrPut(pressed.toString()){
  val sheet=assets.open("UI/Common_Btn_OK.png").use{BitmapFactory.decodeStream(it)};val h=sheet.height/2;val y=if(pressed)h else 0;val out=Bitmap.createBitmap(336,88,Bitmap.Config.ARGB_8888);val canvas=Canvas(out);val paint=Paint(Paint.FILTER_BITMAP_FLAG)
  canvas.drawBitmap(sheet,Rect(6,y,8,y+h),Rect(16,0,320,88),paint);canvas.drawBitmap(sheet,Rect(0,y,8,y+h),Rect(0,0,16,88),paint);canvas.drawBitmap(sheet,Rect(sheet.width-8,y,sheet.width,y+h),Rect(320,0,336,88),paint);sheet.recycle();out
 }
 internal fun showState(raw:String){
  if(isFinishing||isDestroyed)return;val s=JSONObject(raw);lastState=s
  transferButton.visibility=if(s.optBoolean("transferAvailable"))View.VISIBLE else View.GONE
  val editing=s.optBoolean("editor");toggle.visibility=if(editing)View.VISIBLE else View.GONE;tools.visibility=if(editing&&s.optBoolean("tools"))View.VISIBLE else View.GONE;(tools.parent as View).visibility=tools.visibility
  tools.findViewById<TextView>(100).text="HUD LAYOUT · ${if(s.optBoolean("sample"))"SAMPLE" else "PAUSED"}\n${s.optString("selection")} · Drag to move."
  for(i in 0 until tools.childCount){val v=tools.getChildAt(i);if(v.tag=="sample"&&(v as Button).text!=(if(s.optBoolean("sample"))"CURRENT SCENE" else "SAMPLE SCENE"))v.text=if(s.optBoolean("sample"))"CURRENT SCENE" else "SAMPLE SCENE";if(v.tag=="apply")v.isEnabled=s.optBoolean("valid",true)}
  fallback.visibility=View.GONE // Edit transferred layouts from the pause menu.
  cheats.visibility=if(BuildConfig.CHEAT&&!editing)View.VISIBLE else View.GONE
  directions.visibility=if(BuildConfig.CHEAT&&(editing||s.optBoolean("game")))View.VISIBLE else View.GONE
  if(BuildConfig.CHEAT){s.optJSONArray("cheatRect")?.let{r->directions.layoutParams=FrameLayout.LayoutParams((r.getDouble(2)*root.width).toInt().coerceAtLeast(1),(r.getDouble(3)*root.height).toInt().coerceAtLeast(1)).apply{leftMargin=(r.getDouble(0)*root.width).toInt();topMargin=(r.getDouble(1)*root.height).toInt()}};directions.isEnabled=true}
  val message=s.optString("error");error.text=message;error.visibility=if(message.isEmpty())View.GONE else View.VISIBLE;if(message.isNotEmpty())error.bringToFront()
  val notice=s.optString("notice");if(notice.isNotEmpty())Toast.makeText(this,notice,Toast.LENGTH_LONG).show()
  val url=s.optString("url");if(url.isNotEmpty())runCatching{startActivity(Intent(Intent.ACTION_VIEW,Uri.parse(url)))}
  keys.visibility=if(s.optBoolean("keys"))View.VISIBLE else View.GONE;(keys.parent as View).visibility=keys.visibility;if(s.optBoolean("keys"))showKeys(s)
 }
 private fun showKeys(s:JSONObject){keys.removeAllViews();val title=TextView(this).apply{text="KEY BINDINGS\nSelect an action, then press a key. Esc closes. Backspace resets.";setTextColor(Color.WHITE);textSize=18f};keys.addView(title)
  val names=listOf("Move left","Move right","Aim left","Aim right","Aim up","Aim down","Fire","Jump","Previous weapon","Next weapon","Fine movement / aim","Confirm","Pause / back","Next button","Key settings","Next target");val bindings=s.getJSONArray("bindings")
  names.forEachIndexed{i,name->keys.addView(metalButton("${if(s.optInt("keySelection")==i)"▶ " else ""}$name — ${keyName(bindings.getInt(i))}${if(s.optBoolean("capture")&&s.optInt("keySelection")==i)" · PRESS KEY" else ""}"){GameEngine.command("keySelect",i)},LinearLayout.LayoutParams(-1,dp(44)))}
  keys.addView(metalButton("RESET KEYS"){GameEngine.command("keysReset")});keys.addView(metalButton("CLOSE"){GameEngine.command("keys")})
 }
 private fun keyName(k:Int)=when(k){in 4..29->('A'.code+k-4).toChar().toString();40->"ENTER";41->"ESC";43->"TAB";44->"SPACE";59->"F2";79->"RIGHT";80->"LEFT";81->"DOWN";82->"UP";225->"SHIFT";229->"RIGHT SHIFT";else->"HID $k"}
 private fun showCheats(){if(!BuildConfig.CHEAT)return;GameEngine.command("interrupt");val items=arrayOf("START STAGE","INVINCIBLE / UNLIMITED AMMO ON","CHEATS OFF","ADD EXPERIENCE","RESUME","KEY SETTINGS","SAVE DIAGNOSTICS","RESUME SAVED GAME")
  AlertDialog.Builder(this).setTitle("HM VALIDATION").setItems(items){_,which->when(which){0->stageDialog();1->GameEngine.command("cheats",1);2->GameEngine.command("cheats",0);3->GameEngine.command("exp",1);4->GameEngine.command("resume");5->GameEngine.command("keys");6->GameEngine.command("dump");7->GameEngine.command("saved")}}.show()
 }
 private fun stageDialog(){val column=LinearLayout(this).apply{orientation=LinearLayout.VERTICAL;setPadding(dp(20),0,dp(20),0)};val fields=listOf("Mission (1–7)" to "1","Stage (1–5)" to "1","Level (1–100)" to "1","Weapon index (0+)" to "0","Body grade (1–7)" to "1","Turret grade (1–7)" to "1","Gun grade (1–7)" to "1","Before boss (0: off, 1+: boss number)" to "0","Near level up (0 / 1)" to "0").map{(label,value)->EditText(this).apply{hint=label;setText(value);inputType=android.text.InputType.TYPE_CLASS_NUMBER;column.addView(this)}}
  AlertDialog.Builder(this).setTitle("START STAGE · ALL WEAPONS").setView(ScrollView(this).apply{addView(column)}).setPositiveButton("START"){_,_->GameEngine.command("fixtureSettings",(fields[4].text.toString().toIntOrNull()?:1)-1,(fields[5].text.toString().toIntOrNull()?:1)-1,(fields[6].text.toString().toIntOrNull()?:1)-1,((fields[7].text.toString().toIntOrNull()?:0) shl 1) or ((fields[8].text.toString().toIntOrNull()?:0) and 1));GameEngine.command("stage",(fields[0].text.toString().toIntOrNull()?:1)-1,(fields[1].text.toString().toIntOrNull()?:1)-1,(fields[2].text.toString().toIntOrNull()?:1)-1,fields[3].text.toString().toIntOrNull()?:0)}.setNegativeButton("CANCEL",null).show()
 }
 private fun showTransfer(){
  GameEngine.command("interrupt")
  val box=LinearLayout(this).apply{orientation=LinearLayout.VERTICAL;setPadding(dp(16),dp(8),dp(16),dp(8))}
  val code=EditText(this).apply{hint="Paste an HM1- code";setSingleLine(false);maxLines=5;inputType=android.text.InputType.TYPE_CLASS_TEXT or android.text.InputType.TYPE_TEXT_FLAG_MULTI_LINE;setSelectAllOnFocus(true)}
  val info=TextView(this);box.addView(code);box.addView(info)
  val source=Spinner(this);val target=Spinner(this);val labels=arrayOf("All three slots","Slot 1","Slot 2","Slot 3");for(v in arrayOf(source,target)){v.adapter=ArrayAdapter(this,android.R.layout.simple_spinner_dropdown_item,labels);box.addView(v)}
  box.addView(metalButton("CREATE CODE"){GameEngine.transfer("export"){value->if(value.startsWith("ERROR:"))info.text=value else {code.setText(value);info.text="Copy this code to another device."}}})
  box.addView(metalButton("COPY"){(getSystemService(CLIPBOARD_SERVICE) as ClipboardManager).setPrimaryClip(ClipData.newPlainText("HeavyMach recovery",code.text));info.text="Code copied."})
  val dialog=AlertDialog.Builder(this).setTitle("TRANSFER PROGRESS").setView(ScrollView(this).apply{addView(box)}).setNegativeButton("CLOSE",null).create()
  box.addView(metalButton("RESTORE"){val value=code.text.toString();GameEngine.transfer("check",value){summary->if(summary.startsWith("ERROR:")){info.text=summary}else AlertDialog.Builder(this).setTitle("RESTORE PROGRESS?").setMessage(summary).setNegativeButton("CANCEL",null).setPositiveButton("RESTORE"){_,_->GameEngine.transfer("import",value,source.selectedItemPosition-1,target.selectedItemPosition-1){info.text=it;if(!it.startsWith("ERROR:"))dialog.dismiss()}}.show()}})
  dialog.show()
 }
}
