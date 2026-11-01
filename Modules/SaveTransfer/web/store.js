export const defaultKeys=[4,7,80,79,82,81,9,44,20,8,225,40,41,43,59,21];
export function portable(s){
 const file=name=>s.records.find(r=>r.path==='/persist/'+name)?.contents;
 const hud=file('HUDLayoutV1-web.json');const positions=hud?JSON.parse(new TextDecoder().decode(new Uint8Array(hud))).positions:[];
 const mapping=s.settings['HeavyMach.KeyboardBindingsV1'];const keys=mapping?mapping.split(',').map(Number):defaultKeys;
 // The old 15-key mapping predates Next Target. Match the game's deterministic migration.
 if(keys.length===15){let k=21;if(keys.includes(k))for(k=4;keys.includes(k)||k===227||k===231;k++);keys.push(k);}
 return {game:new Uint8Array(file('HvM.dat')??3624),positions,keys,aim:Number(s.settings['HeavyMach.AimModeV1']??1)};
}
export function stored(s){const now=Date.now();const record=(path,contents)=>({path,timestamp:now,mode:path==='/persist'?16895:33206,contents});return {format:1,records:[record('/persist',null),record('/persist/HvM.dat',Array.from(s.game)),record('/persist/HUDLayoutV1-web.json',Array.from(new TextEncoder().encode(JSON.stringify({version:1,positions:s.positions}))))],settings:{'HeavyMach.KeyboardBindingsV1':s.keys.join(','),'HeavyMach.AimModeV1':String(s.aim)}};}
