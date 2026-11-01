#!/usr/bin/env python3
import argparse,pathlib,subprocess,shutil
p=argparse.ArgumentParser();p.add_argument('--emxx',required=True);p.add_argument('--out',required=True);a=p.parse_args()
r=pathlib.Path(__file__).resolve().parent;o=pathlib.Path(a.out);o.mkdir(parents=True,exist_ok=True)
functions=['malloc','free','hm_st_error','hm_st_begin','hm_st_hud','hm_st_key','hm_st_encode','hm_st_decode','hm_st_game','hm_st_aim','hm_st_key_at','hm_st_hud_at','hm_st_summary','hm_st_restore','hm_st_step','hm_st_before','hm_st_after','hm_st_ack','hm_st_verify']
subprocess.run([a.emxx,'-std=c++20','-O2','-fwasm-exceptions',str(r/'SaveTransfer.cpp'),str(r/'Bridge.cpp'),str(r/'../../Sources/Core/SaveData.cpp'),'-sMODULARIZE=1','-sEXPORT_ES6=1','-sENVIRONMENT=web,node','-sALLOW_MEMORY_GROWTH=1','-sEXPORTED_FUNCTIONS='+','.join('_'+f for f in functions),'-sEXPORTED_RUNTIME_METHODS=UTF8ToString,stringToUTF8,lengthBytesUTF8,HEAPU8','-o',str(o/'codec.js')],check=True)
for f in (r/'web').glob('*.js'):shutil.copyfile(f,o/f.name)
