package com.gelb2.heavymach
import android.content.res.AssetManager
import android.view.Surface
internal object Native {
 init { System.loadLibrary("heavymach") }
 external fun create(assets: AssetManager, files: String): Long
 external fun surface(handle: Long, surface: Surface?)
 external fun active(handle: Long, value: Boolean)
 external fun insets(handle: Long, left: Int, top: Int, right: Int, bottom: Int)
 external fun frame(handle: Long, seconds: Double)
 external fun sensor(handle: Long, x: Double, y: Double, z: Double)
 external fun touch(handle: Long, id: Int, state: Int, x: Float, y: Float)
 external fun key(handle: Long, code: Int, down: Boolean)
 external fun command(handle: Long, name: String, a: Int, b: Int, c: Int, d: Int)
 external fun text(handle:Long,name:String,value:String)
 external fun ui(handle: Long): String
 external fun transfer(handle:Long,action:String,code:String,source:Int,target:Int):String
}
