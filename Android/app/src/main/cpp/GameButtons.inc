// Game controls use the original QobButton state and sprite-region renderer.
// Android supplies only the text editor and OS clipboard service.
namespace {
constexpr int hudTool=21000, transferEntry=22000, transferAction=22100;
constexpr int keyChoice=23000, cheatEntry=24000, cheatAction=24100, stageField=24200, cheatDirection=25000;
const char* const keyLabels[]={"MOVE LEFT","MOVE RIGHT","AIM LEFT","AIM RIGHT","AIM UP","AIM DOWN","FIRE","JUMP","PREVIOUS WEAPON","NEXT WEAPON","FINE MOVEMENT","CONFIRM","PAUSE / BACK","NEXT BUTTON","KEY SETTINGS","NEXT TARGET"};
std::string displayKey(unsigned code){if(code>=4&&code<=29)return std::string(1,char('A'+code-4));switch(code){case 40:return "ENTER";case 41:return "ESC";case 43:return "TAB";case 44:return "SPACE";case 59:return "F2";case 79:return "RIGHT";case 80:return "LEFT";case 81:return "DOWN";case 82:return "UP";case 225:return "SHIFT";case 229:return "RSHIFT";default:return std::to_string(code);}}
}
heavymach::gameui::Rect Runtime::recoveryTextRect()const{
 double w=std::min(440.,_menuSafeRect.size.width-24),h=std::min(300.,_menuSafeRect.size.height-12);
 return {CGRectGetMidX(_menuSafeRect)-w/2+12,CGRectGetMidY(_menuSafeRect)+h/2-91,w-24,48};
}
void Runtime::setRecoveryCode(std::string value){recoveryCode_=std::move(value);++textRevision_;}
void Runtime::inputText(const std::string& name,const std::string& value){
 if(name=="recovery"&&panel_==Panel::transfer){if(value.size()>24000){uiStatus_="RECOVERY CODE TOO LONG";return;}recoveryCode_=value;}
}
void Runtime::closeGamePanel(){panel_=Panel::none;gameToolsFocus_=false;gameButtons_.cancel();uiStatus_.clear();}
void Runtime::drawGameButtons(){
 using heavymach::gameui::Rect;
 gameButtons_.beginFrame(_time);
 if(_loading.state()<6||!error.empty())return;
 _media->presentationOffset={};_media->setCameraOffset({});_media->setDrawingOffset({});_media->setDrawingScale(1);_media->resetClip();_media->resourceScope="shared";
 double l=CGRectGetMinX(_menuSafeRect),r=CGRectGetMaxX(_menuSafeRect),t=CGRectGetMaxY(_menuSafeRect),b=CGRectGetMinY(_menuSafeRect);
 const double pw=std::min(464.,r-l-12),ph=std::min(304.,t-b-8),px=(l+r-pw)/2,py=(b+t-ph)/2;
 auto button=[&](int id,std::string label,double x,double y,double w,double h=28,bool enabled=true){gameButtons_.add(id,std::move(label),{x,y,w,h},enabled);};
 auto title=[&](std::string text){gameButtons_.text(std::move(text),{px+10,py+ph-30,pw-20,20},15,true);};
 if(panel_!=Panel::none||_keyboardSettingsOpen)gameButtons_.panel({px,py,pw,ph});
 if(panel_==Panel::transfer||panel_==Panel::confirmTransfer){
  title(panel_==Panel::confirmTransfer?"RESTORE PROGRESS?":"TRANSFER PROGRESS");
  if(panel_==Panel::confirmTransfer){
   gameButtons_.text(uiStatus_,{px+16,py+65,pw-32,ph-107},12,false);
   button(transferAction+8,"RESTORE",px+16,py+18,pw/2-24);
   button(transferAction+9,"CANCEL",px+pw/2+8,py+18,pw/2-24);
  }else{
   const double cell=(pw-40)/3,y=py+ph-126;
   button(transferAction,"CREATE CODE",px+12,y,cell);
   button(transferAction+1,"COPY",px+20+cell,y,cell,28,!recoveryCode_.empty());
   button(transferAction+2,"CHECK CODE",px+28+2*cell,y,cell,28,!recoveryCode_.empty());
   auto slot=[](int n){return n<0?std::string("ALL"):std::to_string(n+1);};
   button(transferAction+5,"FROM "+slot(transferSource_),px+12,y-34,(pw-32)/2);
   button(transferAction+6,"TO "+slot(transferTarget_),px+20+(pw-32)/2,y-34,(pw-32)/2);
   button(transferAction+7,transferSettings_?"HUD AND CONTROLS: YES":"HUD AND CONTROLS: NO",px+12,y-68,pw-24);
   gameButtons_.text(uiStatus_,{px+14,py+52,pw-28,44},10,false);
   button(transferAction+3,"RESTORE",px+12,py+14,(pw-32)/2,28,!recoveryCode_.empty());
   button(transferAction+4,"CLOSE",px+20+(pw-32)/2,py+14,(pw-32)/2);
  }
 }else if(_keyboardSettingsOpen){
  title(_capturingKey?"PRESS A KEY - ESC CANCELS":"KEY BINDINGS");
  double gap=5,cw=(pw-30)/2,ch=std::min(25.,(ph-80)/8-2);
  for(unsigned i=0;i<heavymach::KeyboardInput::count;++i){double x=px+10+(i/8)*(cw+10),y=py+ph-36-(i%8+1)*(ch+2);std::string name=std::string(i==_settingsSelection?"> ":"")+keyLabels[i]+": "+displayKey(_keyboardControls.bindings[i]);button(keyChoice+i,name,x,y,cw,ch);}
  button(keyChoice+100,"RESET KEYS",px+10,py+10,(pw-30)/2);button(keyChoice+101,"CLOSE",px+20+(pw-30)/2,py+10,(pw-30)/2);(void)gap;
 }else if(panel_==Panel::cheats){
  title("HM VALIDATION");
  const char* names[]={"START STAGE", "CHEATS ON", "CHEATS OFF", "ADD EXPERIENCE", "RESUME", "KEY SETTINGS", "SAVE DIAGNOSTICS", "RESUME SAVED GAME"};
  double cw=(pw-36)/2;
  for(int i=0;i<8;++i)button(cheatAction+i,names[i],px+12+(i%2)*(cw+12),py+ph-75-(i/2)*43,cw,34);
  button(cheatAction+8,"CLOSE",px+12,py+14,pw-24);
 }else if(panel_==Panel::stage){
  title("START STAGE - ALL WEAPONS");
  const char* names[]={"MISSION","STAGE","LEVEL","WEAPON INDEX","BODY GRADE","TURRET GRADE","GUN GRADE","BEFORE BOSS","NEAR LEVEL UP"};
  double row=std::min(24.,(ph-77)/9);
  for(int i=0;i<9;++i){double y=py+ph-36-(i+1)*row;gameButtons_.text(std::string(names[i])+" "+std::to_string(stageFields_[i]),{px+16,y,pw-116,row},11,false);button(stageField+i*2,"-",px+pw-94,y,34,row-2);button(stageField+i*2+1,"+",px+pw-52,y,34,row-2);}
  button(stageField+100,"START",px+12,py+10,(pw-32)/2);button(stageField+101,"CANCEL",px+20+(pw-32)/2,py+10,(pw-32)/2);
 }else if(_hudEditing){
  const double w=std::min(166.,(r-l)*.36),h=30,x=r-w-6,y=t-h-4;
  button(hudTool+5,"TOOLS",x,y,w,h);
  if(!_hudHideGuides){
   gameButtons_.panel({x,y-224,w,218});
   static const char* names[]={"MENU","LEVEL / EXP","JUMP","WEAPON / AMMO","GAUGES","AIM CONTROLS","PASSIVES","WEAPON PREVIEW","CHEAT CONTROLS","UPPER TRIM"};
   gameButtons_.text(uiStatus_.empty()?names[_hudSelected]:uiStatus_,{x+4,y-39,w-8,24},10,true);
   const int order[]={3,4,0,1,2};const char* labels[]={"RESET LAYOUT","CANCEL","APPLY","PREVIEW",_hudExample?"CURRENT SCENE":"SAMPLE SCENE"};
   for(int row=0;row<5;++row){int i=order[row];button(hudTool+i,labels[i],x,y-62-row*32,w,h,i!=2||heavymach::hud::valid(currentHUDLayout(),HM_VALIDATION));}
  }
 }else{
  if(transferAvailable())button(transferEntry,"TRANSFER PROGRESS",(l+r-178)/2,b+4,178,30);
#if HM_VALIDATION
  button(cheatEntry,"TEST",r-72,t-36,66,30);
#endif
 }
#if HM_VALIDATION
 if(panel_==Panel::none&&!_keyboardSettingsOpen&&(_hudEditing||(_session&&!_showMenu&&!worldInputBlocked()))){
  auto q=currentHUDLayout().group(heavymach::hud::cheat).visualBounds;
  const char* labels[]={"LEFT","RIGHT","JUMP","FIRE"};
  for(int i=0;i<4;++i)button(cheatDirection+i,labels[i],q.x+i*q.width/4,q.y,q.width/4-1,q.height,!_hudEditing);
 }
#endif
 if(!notice.empty()){gameNotice_=notice;gameNoticeUntil_=realTime_+4;notice.clear();}
 if(realTime_<gameNoticeUntil_&&panel_==Panel::none)gameButtons_.text(gameNotice_,{l+10,b+52,r-l-20,32},11,true);
 if(_hudEditing)gameButtons_.text(gameToolsFocus_?"TAB: BUTTON  ENTER: SELECT  ESC: BACK":"F6: BUTTON KEYS",{l+8,b+4,r-l-16,12},8,false);
 gameButtons_.draw([&](const char* name,Rect src,Rect dst){_media->drawRegion(name,CGRectMake(src.x,src.y,src.width,src.height),CGRectMake(dst.x,dst.y,dst.width,dst.height));});
}
bool Runtime::routeGameButtons(CGPoint p,int state,int token){
 auto event=gameButtons_.touch(token,p.x,p.y,state);
 if(event){
#if HM_VALIDATION
 if(event->id>=cheatDirection&&event->id<cheatDirection+4&&event->state==1){auto bounds=gameButtons_.rect(event->id);gameUIAction(event->id,bounds&&heavymach::gameui::contains(*bounds,p.x,p.y)?0:3);return true;}
#endif
 gameUIAction(event->id,event->state);return true;}
 return panel_!=Panel::none||_keyboardSettingsOpen;
}
void Runtime::gameUIAction(int id,int state){
#if HM_VALIDATION
 if(id>=cheatDirection&&id<cheatDirection+4){const heavymach::KeyAction actions[]={heavymach::KeyAction::left,heavymach::KeyAction::right,heavymach::KeyAction::jump,heavymach::KeyAction::fire};auto code=_keyboardControls.bindings[unsigned(actions[id-cheatDirection])];if(state==0&&_loading.state()==6)key(code,true);else if(state>=2)key(code,false);return;}
#endif
 if(state!=2||_loading.state()<6)return;
 _media->sound("UI_Click");
 if(id>=hudTool&&id<hudTool+6){uiStatus_.clear();const char* names[]={"reset","cancel","apply","preview","sample","tools"};try{command(names[id-hudTool]);}catch(const std::exception&e){uiStatus_=e.what();}return;}
 if(id==transferEntry){if(!transferAvailable())return;cancelGameInput();panel_=Panel::transfer;uiStatus_="CREATE A CODE OR PASTE ONE TO RESTORE.";return;}
 if(id>=transferAction&&id<=transferAction+9){
  try{
   switch(id-transferAction){
    case 0:setRecoveryCode(transfer("export",""));uiStatus_="CODE READY. COPY IT TO ANOTHER DEVICE.";break;
    case 1:clipboardValue_=recoveryCode_;++clipboardRevision_;uiStatus_="CODE COPIED.";break;
    case 2:uiStatus_=transfer("check",recoveryCode_);break;
    case 3:uiStatus_=transfer("check",recoveryCode_)+"\nCURRENT SAVE WILL BE BACKED UP.";panel_=Panel::confirmTransfer;gameButtons_.cancel();break;
    case 4:closeGamePanel();break;
    case 5:transferSource_=(transferSource_+2)%4-1;break;
    case 6:transferTarget_=(transferTarget_+2)%4-1;break;
    case 7:transferSettings_=!transferSettings_;break;
    case 8:{
     if(!transferAvailable())throw std::invalid_argument("Open save slot selection first.");
     using namespace heavymach::transfer;auto incoming=decode(recoveryCode_);auto desired=merge(transferSnapshot(),incoming,transferSource_,transferTarget_,transferSettings_);
     auto store=transferStore();store.recover();restore(store,desired);loadSettings();_save=heavymach::SaveData::decode(readLocal(files_+"/HvM.dat"));closeGamePanel();requestScreen(3);notice="PROGRESS RESTORED.";break;
    }
    case 9:panel_=Panel::transfer;gameButtons_.cancel();break;
   }
  }catch(const std::exception&e){uiStatus_=e.what();if(panel_==Panel::confirmTransfer)panel_=Panel::transfer;}
  return;
 }
 if(id>=keyChoice&&id<keyChoice+int(heavymach::KeyboardInput::count)){command("keySelect",id-keyChoice);return;}
 if(id==keyChoice+100){command("keysReset");return;}if(id==keyChoice+101){command("keys");return;}
#if HM_VALIDATION
 if(id==cheatEntry){pauseForInterruption();panel_=Panel::cheats;return;}
 if(id>=cheatAction&&id<=cheatAction+8){
  switch(id-cheatAction){case 0:panel_=Panel::stage;break;case 1:command("cheats",1);closeGamePanel();break;case 2:command("cheats",0);closeGamePanel();break;case 3:command("exp",1);closeGamePanel();break;case 4:closeGamePanel();command("resume");break;case 5:closeGamePanel();command("keys");break;case 6:command("dump");closeGamePanel();break;case 7:closeGamePanel();command("saved");break;case 8:closeGamePanel();break;}gameButtons_.cancel();return;
 }
 if(id>=stageField&&id<stageField+18){int i=(id-stageField)/2,delta=(id-stageField)%2?1:-1;const int low[]={1,1,1,0,1,1,1,0,0};const int high[]={7,5,100,int(_data.weapons.size())-1,7,7,7,7,1};stageFields_[i]=std::clamp(stageFields_[i]+delta,low[i],high[i]);return;}
 if(id==stageField+100){auto f=stageFields_;closeGamePanel();command("fixtureSettings",f[4]-1,f[5]-1,f[6]-1,(f[7]<<1)|(f[8]&1));command("stage",f[0]-1,f[1]-1,f[2]-1,f[3]);return;}
 if(id==stageField+101){panel_=Panel::cheats;gameButtons_.cancel();return;}
#endif
}
