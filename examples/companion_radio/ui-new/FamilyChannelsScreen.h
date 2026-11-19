#pragma once
#include "ChannelsView.h"

// Same navigation entry points as Solo MessagesScreen, with no chat history,
// DM retries or room state. Only channel editing, sharing and typed pins live
// here, and the entire screen is freed on exit.
class MessagesScreen : public UIScreen {
  UITask* _task;
  ChannelsView _editor;
  enum Mode:uint8_t { CHANNELS, PIN_KIND, PIN_GAME };
  enum Purpose:uint8_t { EDIT, PIN, SHARE, LOCATION, BOT };
  Mode _mode=CHANNELS;
  Purpose _purpose=EDIT;
  uint8_t _row=0,_channels[MAX_GROUP_CHANNELS]={},_count=0;
  int _pin_slot=-1;
  char _share[161]={};
  static void text(DisplayDriver& d,int x,int y,const char* s){d.setCursor(x,y);d.print(s);}
  void rebuild() {
    _count=0;for(unsigned i=0;i<MAX_GROUP_CHANNELS;++i){char name[2];uint8_t digest[16];if(_task->gameChannel(i,name,sizeof(name),digest))_channels[_count++]=i;}
    if(_row>_count)_row=0;
  }
  void unsupported(){_task->showAlert("Use a private channel",1600);}
  unsigned gameCount()const{
#ifdef SOLO_GAME_VEYLITH
    return NodePrefs::FAV_GAME_VEYLITH+1;
#else
    return NodePrefs::FAV_GAME_HOLDEM+1;
#endif
  }
  void choose(uint8_t channel) {
    char name[2];uint8_t digest[16];
    if(!_task->gameChannel(channel,name,sizeof(name),digest)){_task->showAlert("Private channel required",1600);rebuild();return;}
    auto prefs=_task->getNodePrefs();
    if(_purpose==PIN){_task->setFavouriteChannelSlot(_pin_slot,channel);the_mesh.savePrefs();_task->gotoHomeScreen();}
    else if(_purpose==LOCATION){prefs->loc_share_target_type=0;prefs->loc_share_channel_idx=channel;the_mesh.savePrefs();_task->gotoLiveShareScreen();}
    else if(_purpose==BOT){prefs->bot_channel_enabled=1;prefs->bot_channel_idx=channel;the_mesh.savePrefs();_task->gotoBotScreen();}
    else if(_purpose==SHARE) {
      ChannelDetails details;
      if(the_mesh.getChannel(channel,details) && the_mesh.sendGroupMessage(the_mesh.getRTCClock()->getCurrentTimeUnique(),details.channel,the_mesh.getNodeName(),_share,strlen(_share))) {
        _task->showAlert("Shared to channel",1200);_task->gotoHomeScreen();
      } else _task->showAlert("Unable to queue share",1600);
    } else {
      ChannelDetails details;if(the_mesh.getChannel(channel,details))_editor.openEdit(channel,details.name);
    }
  }
public:
  MessagesScreen(UITask* task,KeyboardWidget*):_task(task),_editor(task){reset();}
  void reset(){_editor.reset();_mode=CHANNELS;_purpose=EDIT;_row=0;_pin_slot=-1;_share[0]=0;rebuild();}
  void onShow()override{rebuild();}
  bool navActive()const{return _editor.active();}
  void enterChannel(uint8_t channel){reset();ChannelDetails details;char name[2];uint8_t digest[16];if(_task->gameChannel(channel,name,sizeof(name),digest) && the_mesh.getChannel(channel,details))_editor.openEdit(channel,details.name);else unsupported();}
  void enterDM(const ContactInfo&){reset();unsupported();}
  void enterRoom(const ContactInfo&){reset();unsupported();}
  void startShare(const char* value){reset();_purpose=SHARE;strncpy(_share,value,sizeof(_share)-1);}
  void startPickTarget(){reset();_purpose=LOCATION;}
  void startPickFavourite(int slot){reset();_pin_slot=slot;_mode=PIN_KIND;}
  void startPickBotChannel(){reset();_purpose=BOT;}
  void startPickBotRoom(){reset();unsupported();}
  template<class... Args> int addChannelMsg(Args...){return -1;}
  template<class... Args> void armChannelRelay(Args...){}
  template<class... Args> void markChannelRelayed(Args...){}
  template<class... Args> void addDMMsg(Args...){}
  template<class... Args> void markDmDelivered(Args...){}
  template<class... Args> void onRoomLoginResult(Args...){}
  int getTotalChannelUnread()const{return 0;}
  uint8_t chUnread(uint8_t)const{return 0;}
  int dmHistCountForContact(const uint8_t*)const{return 0;}
  void clearAllChannelUnread(){}
  void tickDmResends(){}
  int render(DisplayDriver& display)override {
    if(_editor.active())return _editor.render(display);
    display.setTextSize(1);display.setColor(DisplayDriver::LIGHT);
    display.drawCenteredHeader(_mode==PIN_KIND?"PIN SHORTCUT":_mode==PIN_GAME?"PIN GAME":_purpose==SHARE?"SHARE TO CHANNEL":"PRIVATE CHANNELS");
    if(_mode==PIN_KIND){text(display,2,18,_row==0?"> Game":"  Game");text(display,2,35,_row==1?"> Channel":"  Channel");return 300;}
    unsigned rows=(display.height()-display.listStart())/display.lineStep();if(!rows)rows=1;
    unsigned total=_mode==PIN_GAME?gameCount():_count+(_purpose==EDIT?1:0),first=(_row/rows)*rows;
    if(!total)text(display,2,display.listStart(),"No private channels");
    for(unsigned i=first;i<total && i<first+rows;++i) {
      char label[32];
      if(_mode==PIN_GAME)snprintf(label,sizeof(label),"%s",_task->favouriteGameLabel(i));
      else if(i==_count)snprintf(label,sizeof(label),"+ Add private");
      else {uint8_t digest[16];_task->gameChannel(_channels[i],label,sizeof(label),digest);}
      int y=display.listStart()+(i-first)*display.lineStep();
      display.drawSelectionRow(0,y-1,display.width(),display.lineStep()-1,_row==i);
      display.drawTextEllipsized(2,y,display.width()-4,label,_row==i);
    }
    return 300;
  }
  bool handleInput(char c)override {
    uint8_t key=uint8_t(c);
    if(_editor.active()){bool result=_editor.handleInput(c);if(!_editor.active())rebuild();return result;}
    if(key==KEY_CANCEL){if(_mode==PIN_GAME){_mode=PIN_KIND;_row=0;}else _task->gotoHomeScreen();return true;}
    unsigned count=_mode==PIN_KIND?2:_mode==PIN_GAME?gameCount():_count+(_purpose==EDIT?1:0);
    if(count && key==KEY_UP)_row=(_row+count-1)%count;
    if(count && key==KEY_DOWN)_row=(_row+1)%count;
    if(key!=KEY_ENTER || !count)return true;
    if(_mode==PIN_KIND){if(_row==0)_mode=PIN_GAME;else{_mode=CHANNELS;_purpose=PIN;}_row=0;}
    else if(_mode==PIN_GAME){if(_task->setFavouriteGameSlot(_pin_slot,_row))_task->gotoHomeScreen();}
    else if(_row<_count)choose(_channels[_row]);
    else {
      for(unsigned i=0;i<MAX_GROUP_CHANNELS;++i){ChannelDetails details;if(!the_mesh.getChannel(i,details) || !details.name[0]){_editor.openAdd(i);_editor.openPrivate();return true;}}
      _task->showAlert("Channel slots full",1600);
    }
    return true;
  }
};
static_assert(sizeof(MessagesScreen)<=512,"Channel tools must not carry chat history");
