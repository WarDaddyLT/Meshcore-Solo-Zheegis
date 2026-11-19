"""Exercise private channel tools and game pins without allocating chat history."""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[2]
header=(root/'examples/companion_radio/ui-new/FamilyChannelsScreen.h').read_text().replace('#pragma once','').replace('#include "ChannelsView.h"','')
keys='\n'.join(line for line in (root/'src/helpers/ui/UIScreen.h').read_text().splitlines() if line.startswith('#define KEY_'))
code=r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdio>
#define MAX_GROUP_CHANNELS 4
KEYS
struct ContactInfo {};
struct ChannelDetails {char name[32]={};struct {uint8_t secret[32]={};}channel;};
struct NodePrefs {static const unsigned FAV_GAME_HOLDEM=7;uint8_t loc_share_target_type=0,loc_share_channel_idx=0,bot_channel_enabled=0,bot_channel_idx=0;};
struct Clock {uint32_t getCurrentTimeUnique(){return 123;}};
struct Mesh {unsigned saves=0,sends=0;bool privateEnabled=true;Clock clock;
 bool getChannel(unsigned slot,ChannelDetails& c){if(slot>1)return false;snprintf(c.name,sizeof(c.name),"%s",slot==0?"Public":"Test private");return true;}
 void savePrefs(){++saves;}Clock* getRTCClock(){return &clock;}const char* getNodeName(){return "Tracker01";}
 template<class... Args> bool sendGroupMessage(Args...){++sends;return true;}
}the_mesh;
struct KeyboardWidget {};
struct DisplayDriver {enum{LIGHT};int width(){return 128;}int height(){return 64;}int listStart(){return 14;}int lineStep(){return 12;}
 void setTextSize(int){}void setColor(int){}void drawCenteredHeader(const char*){}void setCursor(int,int){}void print(const char*){}
 void drawSelectionRow(int x,int y,int w,int h,bool){assert(x>=0&&y>=0&&x+w<=128&&y+h<=64);}
 void drawTextEllipsized(int x,int y,int w,const char*,bool){assert(x>=0&&y>=0&&x+w<=128&&y+9<=64);}
};
struct UIScreen {virtual void onShow(){}virtual int render(DisplayDriver&)=0;virtual bool handleInput(char)=0;};
struct UITask {NodePrefs prefs;unsigned home=0,alerts=0,game=99,channel=99,slot=99;
 bool gameChannel(unsigned id,char* name,size_t n,uint8_t*){if(id!=1||!the_mesh.privateEnabled)return false;snprintf(name,n,"Test private");return true;}
 NodePrefs* getNodePrefs(){return &prefs;}void setFavouriteChannelSlot(int s,uint8_t ch){slot=s;channel=ch;}
 bool setFavouriteGameSlot(int s,uint8_t g){slot=s;game=g;return true;}
 const char* favouriteGameLabel(unsigned id){static const char* labels[]={"Games menu","Tic-Tac-Toe CPU","TTT multiplayer","Snake","Checkers","Battleship","Scorched Earth","Texas Holdem"};return labels[id];}
 void gotoHomeScreen(){++home;}void gotoLiveShareScreen(){}void gotoBotScreen(){}void showAlert(const char*,int){++alerts;}
};
struct ChannelsView {static unsigned edits,adds;bool shown=false;ChannelsView(UITask*){}bool active()const{return shown;}void reset(){shown=false;}
 void openEdit(unsigned i,const char*){assert(i==1);++edits;shown=true;}void openAdd(unsigned i){assert(i==2);++adds;shown=true;}
 void openPrivate(){shown=true;}int render(DisplayDriver&){return 300;}bool handleInput(char c){if(c==KEY_CANCEL)shown=false;return true;}
};unsigned ChannelsView::edits=0,ChannelsView::adds=0;
HEADER
int main(){
 UITask task;KeyboardWidget kb;MessagesScreen screen(&task,&kb);DisplayDriver display;
 for(unsigned id=0;id<8;++id){screen.startPickFavourite(3);screen.handleInput(KEY_ENTER);for(unsigned n=0;n<id;++n)screen.handleInput(KEY_DOWN);screen.render(display);screen.handleInput(KEY_ENTER);assert(task.game==id && task.slot==3);}
 screen.startPickFavourite(2);screen.handleInput(KEY_DOWN);screen.handleInput(KEY_ENTER);screen.render(display);screen.handleInput(KEY_ENTER);assert(task.channel==1&&task.slot==2);
 screen.reset();screen.handleInput(KEY_ENTER);assert(ChannelsView::edits==1);screen.handleInput(KEY_CANCEL);screen.handleInput(KEY_DOWN);screen.handleInput(KEY_ENTER);assert(ChannelsView::adds==1);
 screen.startShare("Waypoint: example");screen.handleInput(KEY_ENTER);assert(the_mesh.sends==1);
 screen.startShare("Waypoint: example");the_mesh.privateEnabled=false;screen.handleInput(KEY_ENTER);assert(the_mesh.sends==1 && task.alerts);
 assert(screen.addChannelMsg(1,"unretained")==-1 && !screen.getTotalChannelUnread());
 screen.addDMMsg(0,"unretained");assert(!screen.dmHistCountForContact(nullptr));
 assert(sizeof(MessagesScreen)<=512);printf("Family channels: private-only selection/rekey guard, waypoint sharing, private add/edit, all game pins and no chat history passed; host screen %zu bytes\n",sizeof(MessagesScreen));
}
'''.replace('KEYS',keys).replace('HEADER',header)
with tempfile.TemporaryDirectory() as tmp:
 cpp=Path(tmp)/'channels.cpp';out=Path(tmp)/'channels';cpp.write_text(code)
 subprocess.run(['c++','-std=c++17','-O2','-Wall','-Wextra','-Werror',str(cpp),'-o',str(out)],check=True)
 subprocess.run([str(out)],check=True)
