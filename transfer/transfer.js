import create from './codec.js';
const size=3624;
export async function codec(){
 const m=await create();
 const text=p=>{if(!p)throw Error(m.UTF8ToString(m._hm_st_error()));return m.UTF8ToString(p);};
 const ok=n=>{if(!n)throw Error(m.UTF8ToString(m._hm_st_error()));};
 function input(s){if(s.game.length!==size)throw Error('Invalid game save size');const p=m._malloc(size);try{m.HEAPU8.set(s.game,p);ok(m._hm_st_begin(p,size,s.aim??-1));}finally{m._free(p);}for(const [i,v]of(s.positions??[]).entries())if(v)ok(m._hm_st_hud(i,v[0],v[1]));for(const [i,k]of(s.keys??[]).entries())ok(m._hm_st_key(i,k));}
 function parse(code){const n=m.lengthBytesUTF8(code)+1,p=m._malloc(n);try{m.stringToUTF8(code,p,n);ok(m._hm_st_decode(p));}finally{m._free(p);}return output();}
 function output(){const p=m._hm_st_game();return {game:m.HEAPU8.slice(p,p+size),aim:m._hm_st_aim(),keys:m._hm_st_key_at(0)<0?null:Array.from({length:16},(_,i)=>m._hm_st_key_at(i)),positions:Array.from({length:10},(_,i)=>{const x=m._hm_st_hud_at(i,0);return x<0?null:[x,m._hm_st_hud_at(i,1)];}),summary:text(m._hm_st_summary())};}
 return {encode(s){input(s);return text(m._hm_st_encode());},decode:parse,
 async restore(store,code,{source=-1,target=-1,settings=true}={}){
  input(await store.read());parse(code);ok(m._hm_st_restore(source,target,Number(settings)));
  // The module owns ordering. No other operation may run concurrently on this codec instance.
  const before=text(m._hm_st_before()),after=text(m._hm_st_after()),desired=output();
  while(m._hm_st_step()!==5){switch(m._hm_st_step()){
   case 0:await store.preserve(before);break;
   case 1:await store.pending(after);break;
   case 2:await store.apply(desired);break;
   case 3:input(await store.read());ok(m._hm_st_verify());continue;
   case 4:await store.finish();break;
   default:throw Error('Invalid restore state');
  }ok(m._hm_st_ack());}
 },async resume(store,code){const s=parse(code);await store.apply(s);const actual=await store.read();input(actual);if(text(m._hm_st_encode())!==this.encode(s))throw Error('Restored data does not match');await store.finish();}};
}
