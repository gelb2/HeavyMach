heavymach::transfer::Snapshot Runtime::transferSnapshot(){
 heavymach::transfer::Snapshot s;s.game=_save;s.positions=_hudLayout.positions;s.keys=_keyboardControls.bindings;s.aim=int(_preferredAimMode);return s;
}
heavymach::transfer::FileStore Runtime::transferStore(){
 using namespace heavymach::transfer;
 return FileStore(files_+"/transfer",[this]{
  auto b=readLocal(files_+"/HvM.dat");if(!b.empty())_save=heavymach::SaveData::decode(b);
  loadSettings();return transferSnapshot();
 },[this](const Snapshot&s){
  auto bytes=s.game.encode();atomicFile(files_+"/HvM.dat",{reinterpret_cast<const char*>(bytes.data()),bytes.size()});
  std::ostringstream h;h.precision(17);h<<"1\n";for(auto p:s.positions)h<<(p?1:0)<<' '<<(p?p->x:0)<<' '<<(p?p->y:0)<<'\n';atomicFile(files_+"/HUDLayoutV1.txt",h.str());
  std::ostringstream k;k<<s.aim<<'\n';for(auto key:s.keys.value_or(heavymach::KeyboardInput::defaults))k<<key<<' ';k<<'\n';atomicFile(files_+"/InputV1.txt",k.str());
 });
}
std::string Runtime::transfer(const std::string&action,const std::string&code,int source,int target){
 using namespace heavymach::transfer;
 if(!transferAvailable())throw std::invalid_argument("Open save slot selection first.");
 if(action=="export")return encode(transferStore().read());
 auto incoming=decode(code);
 if(action=="check")return summary(incoming);
 if(action!="import")throw std::invalid_argument("Unknown transfer action");
 auto desired=merge(transferSnapshot(),incoming,source,target);
 auto store=transferStore();store.recover();restore(store,desired);loadSettings();
 _save=heavymach::SaveData::decode(readLocal(files_+"/HvM.dat"));requestScreen(3);return "Progress restored.";
}
