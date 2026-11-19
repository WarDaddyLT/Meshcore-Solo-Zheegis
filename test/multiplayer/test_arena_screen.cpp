#define SOLO_GAMES_MULTIPLAYER 1
#define ZYGIS_GAME_DIAGNOSTICS 1
#include "fixtures.h"
#include <games/veylith/Progress.h>
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
// KEY_DEFINITIONS
#include "screen_fixtures.h"
#include "../../examples/companion_radio/ui-new/ArenaOnlineScreen.h"
static DisplayDriver display;
static unsigned state(const ArenaOnlineScreen& s,unsigned field){uint8_t bytes[44];assert(s.diagnostics(bytes,sizeof(bytes),0)==44);return Codec::get(bytes+field*4,4);}
static void key(ArenaOnlineScreen& s,int k){assert(s.handleInput(char(k)));s.render(display);}
int main(){
 UITask a(10),b(20);ArenaOnlineScreen host(&a),guest(&b);
 auto pump=[&](unsigned steps=1){for(unsigned i=0;i<steps;++i){ticks+=50;host.poll();guest.poll();host.render(display);guest.render(display);
   auto first=a.sent;a.sent.clear();for(auto& text:first)guest.receiveText(1,text.c_str(),a.name.c_str());
   auto second=b.sent;b.sent.clear();for(auto& text:second)host.receiveText(1,text.c_str(),b.name.c_str());
 }};
 key(guest,KEY_DOWN);key(guest,KEY_ENTER);key(host,KEY_ENTER);key(host,KEY_ENTER);pump(100);
 assert(state(guest,5)==2);key(guest,KEY_ENTER);pump(200);assert(state(host,5)==4&&state(guest,5)==4&&state(host,7)==0&&state(guest,7)==0);
 for(auto* screen:{&host,&guest}){key(*screen,KEY_DOWN);key(*screen,KEY_DOWN);key(*screen,KEY_DOWN);key(*screen,KEY_ENTER);}
 pump(200);assert(state(host,9)&&state(host,9)==state(guest,9));
 for(unsigned step=0;step<15000;++step){for(auto* screen:{&host,&guest}){
   unsigned view=state(*screen,7);
   if(view==8){key(*screen,KEY_ENTER);key(*screen,KEY_DOWN);key(*screen,KEY_ENTER);}
   else if(view==12){for(unsigned i=0;i<3&&state(*screen,7)==12;++i){key(*screen,KEY_ENTER);if(state(*screen,7)==12)key(*screen,KEY_DOWN);}}
  }
  pump();if(state(host,7)==15&&state(guest,7)==15)break;
 }
 assert(state(host,7)==15&&state(guest,7)==15&&state(host,10)&&state(guest,10));
 host.render(display);auto outcome=display.header;guest.render(display);assert((outcome=="VICTORY"&&display.header=="DEFEAT")||(outcome=="DEFEAT"&&display.header=="VICTORY")||(outcome=="DRAW"&&display.header=="DRAW"));
 veylith::Progress pa,pb;assert(veylith::SaveCodec::decode(a.saved.data(),a.saved.size(),pa)&&veylith::SaveCodec::decode(b.saved.data(),b.saved.size(),pb));assert(pa.family[0].wallet>=30&&pb.family[0].wallet>=30);auto xpA=pa.family[0].wallet,xpB=pb.family[0].wallet;
 pump(30);key(guest,KEY_ENTER);pump(200);assert(state(host,3)==1&&state(guest,3)==1&&state(host,7)==0&&state(guest,7)==0);
 assert(veylith::SaveCodec::decode(a.saved.data(),a.saved.size(),pa)&&veylith::SaveCodec::decode(b.saved.data(),b.saved.size(),pb));assert(pa.family[0].wallet==xpA&&pb.family[0].wallet==xpB);
 assert(!a.sounds.empty()&&!b.sounds.empty());
 key(host,KEY_CANCEL);pump(30);assert(state(host,5)==0&&state(guest,5)==5);
 puts("Arena screens: named Host/Find -> automatic join -> two Trainers -> simultaneous battle -> correct per-player result/XP -> guest-requested rematch -> restored Trainer, nonblocking sounds and 128x64 bounds passed.");
}
