#include <cstdint>
#include <cstring>
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
// KEY_DEFINITIONS
uint32_t testMillis=100;
uint32_t millis() { return testMillis; }
uint32_t micros() { static uint32_t t=1;return ++t; }
class DisplayDriver {
  int x=0,y=0;
public:
  enum Color { DARK,LIGHT };
  unsigned shapes=0;bool sawHost=false,sawRematch=false;
  void setTextSize(int n) { assert(n==1); }
  void setColor(Color) {}
  void drawCenteredHeader(const char* text) { assert(strlen(text)*6<=128); }
  void setCursor(int a,int b) {x=a;y=b;}
  void print(const char* text) { if(strstr(text,"Host One"))sawHost=true;if(!strcmp(text,">Play again"))sawRematch=true;assert(x>=0&&y>=0&&x+int(strlen(text))*6<=128&&y+9<=64); }
  void fillRect(int x,int y,int w,int h) { assert(x>=0&&y>=0&&w>=0&&h>=0&&x+w<=128&&y+h<=64);++shapes; }
  void drawRect(int x,int y,int w,int h) {fillRect(x,y,w,h);}
};
class UIScreen {
public:
  virtual int render(DisplayDriver&)=0;
  virtual bool handleInput(char) {return false;}
  virtual void poll() {}
  virtual void onShow() {}
};
struct UITask {
  unsigned tools=0;bool eligible=false;uint32_t id=11;
  std::vector<std::string> sent;
  void gotoToolsScreen(){++tools;}void playGameSound(bool){} void showAlert(const char*,int){}
  bool gameChannel(uint8_t slot,char* name,size_t size,uint8_t digest[16]) {
    if(!eligible || slot!=1)return false;
    snprintf(name,size,"Test channel");memset(digest,1,16);return true;
  }
  uint32_t gamePlayerId()const{return id;}
  uint64_t gameSessionId()const{return 123;}
  bool sendGameText(uint8_t slot,const char* text,const uint8_t digest[16]) {
    assert(slot==1 && digest[0]==1);sent.emplace_back(text);return true;
  }
};
namespace gfx { void drawLine(DisplayDriver& d,int x,int y,int a,int b) {d.fillRect(x,y,1,1);d.fillRect(a,b,1,1);} }
#include "../../examples/companion_radio/ui-new/GamesScreen.h"

static uint32_t state(GamesScreen& g,unsigned field) {
  uint8_t data[44];assert(g.diagnostics(data,sizeof(data))==44);
  auto p=data+field*4;return uint32_t(p[0])|uint32_t(p[1])<<8|uint32_t(p[2])<<16|uint32_t(p[3])<<24;
}
static void key(GamesScreen& g,int k){assert(g.handleInput(char(k)));}
static void cpu(GamesScreen& g) {
  for(unsigned n=0;n<10000;++n) {
    solo_games::TicTacToePosition p;p.x=state(g,1);p.o=state(g,2);p.turn=state(g,3);
    if(p.result()||p.turn==state(g,4))return;
    g.poll();
  }
  assert(false);
}
static void select(GamesScreen& g,unsigned cell) {
  while(state(g,7)%3>cell%3)key(g,KEY_LEFT);
  while(state(g,7)%3<cell%3)key(g,KEY_RIGHT);
  while(state(g,7)/3>cell/3)key(g,KEY_UP);
  while(state(g,7)/3<cell/3)key(g,KEY_DOWN);
}
int main() {
  UITask task;GamesScreen g(&task);DisplayDriver display;
  g.onShow();g.openTicTacToe();assert(state(g,6)==1);g.render(display);
  for(unsigned difficulty=0;difficulty<3;++difficulty)for(unsigned human=1;human<=2;++human) {
    g.onShow();g.render(display);key(g,KEY_ENTER);
    while(state(g,5)!=difficulty)key(g,KEY_RIGHT);
    key(g,KEY_DOWN);if(state(g,4)!=human)key(g,KEY_ENTER);
    key(g,KEY_DOWN);key(g,KEY_ENTER);assert(state(g,6)==2);
    for(unsigned moves=0;moves<10;++moves) {
      cpu(g);g.render(display);
      solo_games::TicTacToePosition p;p.x=state(g,1);p.o=state(g,2);p.turn=state(g,3);
      if(p.result()){if(difficulty==2)assert(p.result()!=human);break;}
      unsigned chosen=0;while(!p.legal(chosen))++chosen;
      select(g,chosen);key(g,KEY_ENTER);
      // Occupied-cell Enter cannot place an extra human mark or skip a turn.
      auto x=state(g,1),o=state(g,2);solo_games::TicTacToePosition after;after.x=x;after.o=o;
      if(!after.result()){key(g,KEY_ENTER);assert(x==state(g,1)&&o==state(g,2));}
    }
    key(g,KEY_CANCEL);assert(state(g,6)==3);g.render(display);
    key(g,KEY_ENTER);assert(state(g,6)==2);
    key(g,KEY_CANCEL);key(g,KEY_DOWN);key(g,KEY_ENTER);assert(state(g,6)==2);
  }
  for(unsigned i=0;i<100;++i){g.onShow();g.render(display);key(g,KEY_CANCEL);}
  assert(task.tools==100);
  g.openMultiplayer();assert(state(g,0)>sizeof(GamesScreen) && state(g,0)<=1536);
  g.render(display);key(g,KEY_ENTER);g.render(display);
  assert(task.sent.empty()); // No eligible channels => no host or radio traffic.
  key(g,KEY_CANCEL);key(g,KEY_CANCEL);assert(state(g,6)==0 && state(g,0)==sizeof(GamesScreen));
  // Drive two real screens through Host -> accept -> ready -> Start -> play.
  task.eligible=true;UITask peer;peer.eligible=true;peer.id=22;GamesScreen h(&peer);
  auto pump=[&]() {
    for(unsigned n=0;n<130;++n){testMillis+=100;g.poll();h.poll();
      for(auto& text:task.sent)h.receiveText(1,text.c_str(),"Host One");task.sent.clear();
      for(auto& text:peer.sent)g.receiveText(1,text.c_str(),"Guest Two");peer.sent.clear();
    }
    g.render(display);h.render(display);
  };
  g.onShow();g.openMultiplayer();h.onShow();h.openMultiplayer();
  key(h,KEY_DOWN);key(h,KEY_ENTER); // Find lobby
  key(g,KEY_ENTER);g.render(display);key(g,KEY_ENTER);pump();
  assert(display.sawHost); // Names are visible in the discovery list.
  // Discovery uses a bounded table; find the populated row via Enter.
  for(unsigned i=0;i<4 && state(h,6)==6;++i){key(h,KEY_ENTER);if(state(h,6)==6)key(h,KEY_DOWN);}
  pump();assert(state(g,6)==8 && state(h,6)==8);
  select(g,0);key(g,KEY_ENTER);pump();assert(state(h,1)==1);
  select(h,4);key(h,KEY_ENTER);pump();assert(state(g,2)==16);
  select(g,1);key(g,KEY_ENTER);pump();select(h,8);key(h,KEY_ENTER);pump();select(g,2);key(g,KEY_ENTER);pump();
  assert(display.sawRematch);key(h,KEY_ENTER);pump();
  assert(state(g,6)==8 && state(h,6)==8 && !state(g,1) && !state(h,2));
  key(g,KEY_CANCEL);g.render(display);key(g,KEY_DOWN);key(g,KEY_ENTER);pump();
  assert(state(h,6)==10);h.render(display);
  g.onShow();h.onShow();assert(state(g,6)==0 && state(h,6)==0);
  // Snake is allocated only while selected; pause/resume never catches up
  // elapsed time, and leaving releases the separate snake allocation.
  task.sent.clear();peer.sent.clear();
  for(unsigned cycle=0;cycle<100;++cycle) {
    g.onShow();g.openSnake();assert(state(g,6)==11 && state(g,0)>sizeof(GamesScreen));g.render(display);
    key(g,KEY_DOWN);key(g,KEY_ENTER);assert(state(g,5)==1);g.render(display);
    auto head=state(g,1);key(g,KEY_CANCEL);assert(state(g,5)==2);
    testMillis+=5000;g.poll();assert(state(g,1)==head);g.render(display);
    key(g,KEY_ENTER);g.poll();assert(state(g,1)==head);
    testMillis+=190;g.poll();g.render(display);
    key(g,KEY_CANCEL);key(g,KEY_DOWN);key(g,KEY_DOWN);key(g,KEY_ENTER);
    assert(state(g,6)==0 && state(g,0)==sizeof(GamesScreen));
  }
  assert(task.sent.empty() && peer.sent.empty());
  for(unsigned level=0;level<2;++level) {
    g.onShow();g.openArtillery();assert(state(g,6)==14);g.render(display);
    if(level)key(g,KEY_RIGHT);
    key(g,KEY_DOWN);key(g,KEY_ENTER);assert(state(g,5)==1);
    key(g,KEY_DOWN);key(g,KEY_DOWN);key(g,KEY_ENTER);assert((state(g,10)&255)==1);
    key(g,KEY_DOWN);key(g,KEY_ENTER);assert(state(g,5)==2);
    key(g,KEY_CANCEL);auto humanHp=state(g,2),cpuHp=state(g,3);
    for(unsigned i=0;i<100;++i){testMillis+=50;g.poll();}
    assert(state(g,2)==humanHp && state(g,3)==cpuHp);g.render(display);key(g,KEY_ENTER);
    for(unsigned i=0;state(g,5)!=4 && i<100000;++i) {
      if(state(g,5)==1 && state(g,1)==0) {
        while(state(g,7)!=3)key(g,KEY_DOWN);
        key(g,KEY_ENTER);
      }
      testMillis+=40;g.poll();g.render(display);
    }
    assert(state(g,5)==4);key(g,KEY_ENTER);assert(state(g,5)==0 && state(g,2)==100 && state(g,3)==100);
    key(g,KEY_CANCEL);assert(state(g,0)==sizeof(GamesScreen));
  }
  for(unsigned i=0;i<100;++i){g.onShow();g.openArtillery();g.render(display);key(g,KEY_CANCEL);assert(state(g,0)==sizeof(GamesScreen));}
  assert(task.sent.empty() && peer.sent.empty());
  // Poker actions and CPU turns use only the currently visible screen.
  unsigned completed=0,showdowns=0;
  for(unsigned match=0;match<4;++match) {
    g.onShow();g.openHoldem();assert(state(g,6)==15);g.render(display);
    key(g,KEY_ENTER);assert(state(g,5)==1);
    key(g,KEY_CANCEL);auto chips=state(g,3),other=state(g,4),pot=state(g,8);
    testMillis+=10000;g.poll();assert(state(g,3)==chips&&state(g,4)==other&&state(g,8)==pot);g.render(display);key(g,KEY_ENTER);
    for(unsigned n=0;n<5000;++n) {
      assert(state(g,3)+state(g,4)+state(g,8)==1000);
      if(state(g,5)==3) {
        ++completed;if(state(g,1)==5)++showdowns;g.render(display);
        bool busted=!state(g,3)||!state(g,4);key(g,KEY_ENTER);
        if(busted){assert(state(g,5)==0);break;}
      } else if(state(g,2)==0) {
        if(n==0){key(g,KEY_DOWN);key(g,KEY_RIGHT);} // Minimum legal raise + adjustment.
        key(g,KEY_ENTER);
      }
      testMillis+=601;g.poll();g.render(display);
      assert(n<4999);
    }
    key(g,KEY_CANCEL);assert(state(g,0)==sizeof(GamesScreen));
  }
  assert(completed>4 && showdowns);
  g.onShow();g.openHoldem();key(g,KEY_ENTER);key(g,KEY_UP);key(g,KEY_ENTER);
  assert(state(g,5)==3);g.render(display);key(g,KEY_CANCEL);assert(state(g,0)==sizeof(GamesScreen));
  g.onShow();g.openHoldem();key(g,KEY_ENTER);key(g,KEY_DOWN);key(g,KEY_CONTEXT_MENU);key(g,KEY_ENTER);
  for(unsigned i=0;state(g,5)!=3&&i<100;++i){testMillis+=601;g.poll();g.render(display);}
  assert(state(g,5)==3);
  for(unsigned i=0;i<100;++i){g.onShow();g.openHoldem();g.render(display);key(g,KEY_CANCEL);assert(state(g,0)==sizeof(GamesScreen));}
  assert(task.sent.empty()&&peer.sent.empty());
  // Manual placement includes rotation and out-of-bounds rejection.
  g.onShow();g.openBattleship();assert(state(g,6)==13);g.render(display);
  key(g,KEY_ENTER);assert(state(g,5)==1);key(g,KEY_CONTEXT_MENU);assert(state(g,4)==1);
  auto selectFleet=[&](unsigned target) {
    while(state(g,7)%10>target%10)key(g,KEY_LEFT);
    while(state(g,7)%10<target%10)key(g,KEY_RIGHT);
    while(state(g,7)/10>target/10)key(g,KEY_UP);
    while(state(g,7)/10<target/10)key(g,KEY_DOWN);
  };
  selectFleet(70);key(g,KEY_ENTER);assert(state(g,2)==0);g.render(display);
  selectFleet(0);key(g,KEY_ENTER);assert(state(g,2)==1);key(g,KEY_CONTEXT_MENU);
  for(unsigned cell:{1U,51U,61U,71U}){selectFleet(cell);key(g,KEY_ENTER);g.render(display);}
  assert(state(g,2)==5 && state(g,5)==0);key(g,KEY_ENTER);assert(state(g,5)==2);
  key(g,KEY_CONTEXT_MENU);assert(state(g,10)==1);g.render(display);
  key(g,KEY_ENTER);assert(state(g,10)==0 && state(g,8)==0);
  key(g,KEY_ENTER);assert(state(g,8)==1 && state(g,1)==1);
  key(g,KEY_CANCEL);auto cpuShots=state(g,9);testMillis+=1000;g.poll();assert(state(g,9)==cpuShots);g.render(display);
  key(g,KEY_ENTER);g.poll();assert(state(g,1)==0);
  key(g,KEY_ENTER);assert(state(g,8)==1); // Repeated shot cannot consume a turn.
  for(unsigned cell=1;state(g,5)==2 && cell<100;++cell) {
    selectFleet(cell);key(g,KEY_ENTER);testMillis+=600;g.poll();g.render(display);
  }
  assert(state(g,5)==4 && state(g,3));key(g,KEY_ENTER);assert(state(g,5)==0 && !state(g,2));
  key(g,KEY_CANCEL);assert(state(g,6)==0 && state(g,0)==sizeof(GamesScreen));
  for(unsigned i=0;i<100;++i) {
    g.onShow();g.openBattleship();key(g,KEY_DOWN);key(g,KEY_ENTER);assert(state(g,2)==5);g.render(display);
    key(g,KEY_CANCEL);assert(state(g,0)==sizeof(GamesScreen));
  }
  assert(task.sent.empty() && peer.sent.empty());
  for(unsigned level=0;level<2;++level) {
    g.onShow();g.openCheckers();assert(state(g,6)==12);g.render(display);
    if(!level)key(g,KEY_LEFT);
    key(g,KEY_DOWN);key(g,KEY_ENTER);assert(state(g,5)==1 && state(g,3)==1);g.render(display);
    key(g,KEY_ENTER);key(g,KEY_UP);key(g,KEY_RIGHT);key(g,KEY_ENTER);
    assert(state(g,3)==2);auto human=state(g,1),enemy=state(g,2);
    key(g,KEY_CANCEL);assert(state(g,5)==2);g.render(display);
    for(unsigned i=0;i<100;++i)g.poll();assert(state(g,1)==human && state(g,2)==enemy);
    key(g,KEY_ENTER);
    for(unsigned i=0;state(g,3)==2 && i<30000;++i)g.poll();
    assert(state(g,3)==1);g.render(display);
    key(g,KEY_CANCEL);key(g,KEY_DOWN);key(g,KEY_DOWN);key(g,KEY_ENTER);
    assert(state(g,6)==0 && state(g,0)==sizeof(GamesScreen));
  }
  for(unsigned i=0;i<100;++i){g.onShow();g.openCheckers();g.render(display);key(g,KEY_CANCEL);assert(state(g,0)==sizeof(GamesScreen));}
  assert(task.sent.empty() && peer.sent.empty());
  printf("Six single-player screens, CPU matches, pause/restart, 128x64 bounds and 100 allocation cycles per game passed; controller %zu bytes\n",sizeof(GamesScreen));
}
