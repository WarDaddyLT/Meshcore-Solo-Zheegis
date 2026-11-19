#include <games/TicTacToeMesh.h>
#include <cassert>
#include <cstdio>
#include <cstring>
using namespace solo_games;
static uint32_t now=100;
static bool transfer(TicTacToeMesh& from,TicTacToeMesh& to,bool drop=false) {
  GameEvent out;if(!from.outgoing(now,out))return false;
  char text[53];assert(GameCodec::encode(out,text,sizeof(text)));
  GameEvent decoded;assert(GameCodec::decode(text,decoded));assert(GameCodec::equal(out,decoded));
  from.sent(now);
  if(!drop){to.receive(from.channel(),decoded,now,"Host One");to.receive(from.channel(),decoded,now,"Host One");}
  return true;
}
static void settle(TicTacToeMesh& a,TicTacToeMesh& b) {
  for(unsigned i=0;i<120;++i){now+=100;transfer(a,b);transfer(b,a);}
}
static void setup(TicTacToeMesh& a,TicTacToeMesh& b) {
  assert(a.host(1,0x123456789ABCDEFULL,11,now));
  assert(transfer(a,b));
  unsigned lobby=0;while(!b.lobby(lobby,now))++lobby;
  assert(lobby<TicTacToeMesh::LOBBY_COUNT);
  assert(!strcmp(b.lobby(lobby,now)->name,"Host One"));
  assert(b.join(lobby,22,now));
  now+=1000;assert(transfer(b,a));assert(a.candidate()==22);
  assert(!b.move(0));settle(a,b);
  assert(a.phase()==TicTacToeMesh::PLAYING && b.phase()==TicTacToeMesh::PLAYING);
}
int main() {
  GameEvent e;e.type=GameEvent::OPEN;e.session=123;e.host=e.actor=11;
  char wire[54];assert(!GameCodec::encode(e,wire,52));assert(GameCodec::encode(e,wire,sizeof(wire)));
  char canonical[54];strcpy(canonical,wire);
  for(unsigned i=0;i<52;++i){strcpy(wire,canonical);wire[i]='!';GameEvent bad;assert(!GameCodec::decode(wire,bad));}
  for(unsigned i=0;i<52;++i){strcpy(wire,canonical);wire[i]=0;GameEvent bad;assert(!GameCodec::decode(wire,bad));}
  for(unsigned i=4;i<52;++i){strcpy(wire,canonical);wire[i]=wire[i]=='A'?'B':'A';GameEvent bad;assert(!GameCodec::decode(wire,bad));}
  strcpy(wire,canonical);wire[52]='A';wire[53]=0;assert(!GameCodec::decode(wire,e));
  GameEvent boundary=e;boundary.round=255;boundary.seq=15;
  assert(GameCodec::encode(boundary,wire,sizeof(wire)) && GameCodec::decode(wire,e));
  assert(e.round==255 && e.seq==15);
  TicTacToeMesh a,b;setup(a,b);
  assert(!b.move(4)); // no out-of-turn proposal
  assert(a.move(0));now+=2000;
  assert(transfer(a,b,true)); // commit loss: guest must not invent the move
  assert(b.position().x==0 && a.busy());
  settle(a,b);assert(a.position().x==b.position().x && !a.busy());
  assert(b.move(4));assert(b.position().o==0);settle(a,b);
  assert(a.move(1));settle(a,b);assert(b.move(8));settle(a,b);
  assert(a.move(2));settle(a,b);
  assert(a.position().result()==1 && b.position().result()==1);
  assert(a.phase()==TicTacToeMesh::FINISHED && b.phase()==TicTacToeMesh::FINISHED);
  assert(!a.move(3) && !b.move(3));
  // Either player can start another round; delayed old-round moves cannot apply.
  assert(b.rematch());settle(a,b);
  assert(a.phase()==TicTacToeMesh::PLAYING && b.phase()==TicTacToeMesh::PLAYING);
  assert(!a.position().x && !b.position().o);
  GameEvent stale;stale.type=GameEvent::MOVE;stale.session=0x123456789ABCDEFULL;
  stale.host=stale.actor=11;stale.peer=22;stale.seq=1;stale.cell=0;stale.x=1;stale.aux=1;
  b.receive(1,stale,now);assert(!b.position().x);
  for(unsigned round=1;round<=12;++round) {
    assert(a.move(0));settle(a,b);assert(b.move(4));settle(a,b);
    assert(a.move(1));settle(a,b);assert(b.move(8));settle(a,b);assert(a.move(2));settle(a,b);
    assert(a.phase()==TicTacToeMesh::FINISHED && b.phase()==TicTacToeMesh::FINISHED);
    assert((round%2?a:b).rematch());settle(a,b);
    assert(!a.position().x && !b.position().o);
  }
  // Announcements repeat at one second, remain discoverable for late arrivals,
  // and startup retries recover automatically even after prolonged packet loss.
  TicTacToeMesh host,late;assert(host.host(1,9,55,now));
  for(unsigned i=0;i<5;++i){assert(transfer(host,late));assert(!host.outgoing(now,stale));now+=1000;}
  assert(late.lobby(0,now));assert(late.join(0,66,now));
  for(unsigned i=0;i<10;++i){transfer(late,host,true);now+=1000;}
  for(unsigned i=0;i<100;++i){now+=100;transfer(host,late);transfer(late,host);}
  assert(host.phase()==TicTacToeMesh::PLAYING && late.phase()==TicTacToeMesh::PLAYING);
  for(unsigned i=0;i<30;++i){now+=1000;assert(!transfer(host,late) && !transfer(late,host));}
  // Forged board, another channel/session, and conflicting duplicate do not apply.
  GameEvent forged;forged.type=GameEvent::MOVE;forged.session=0x123456789ABCDEFULL;
  forged.host=forged.actor=11;forged.peer=22;forged.seq=4;forged.cell=6;forged.x=64;forged.aux=1;
  auto before=b.position();b.receive(2,forged,now);assert(b.position().x==before.x);
  b.receive(1,forged,now);assert(b.position().x==before.x);
  // Exhausted retries stop emitting until an explicit user retry.
  TicTacToeMesh c,d;setup(c,d);assert(c.move(0));
  unsigned transmissions=0;
  for(unsigned i=0;i<600;++i){now+=100;if(transfer(c,d,true))++transmissions;}
  assert(transmissions==3 && c.stalled() && d.position().x==0);
  c.retry(now);settle(c,d);assert(d.position().x==1);
  GameEvent leave;assert(d.leaveEvent(leave));c.receive(1,leave,now);assert(c.phase()==TicTacToeMesh::CLOSED);
  // Discovery is bounded, expiring and deduplicated; session reset clears it.
  TicTacToeMesh scan;
  for(unsigned i=0;i<30;++i){GameEvent open;open.type=GameEvent::OPEN;open.session=i+1;open.host=open.actor=i+100;
    scan.receive(1,open,now);scan.receive(1,open,now);}
  unsigned found=0;for(unsigned i=0;i<TicTacToeMesh::LOBBY_COUNT;++i)found+=scan.lobby(i,now)!=nullptr;
  assert(found==TicTacToeMesh::LOBBY_COUNT);
  for(unsigned i=0;i<TicTacToeMesh::LOBBY_COUNT;++i)assert(!scan.lobby(i,now+5001));
  scan.reset();for(unsigned i=0;i<TicTacToeMesh::LOBBY_COUNT;++i)assert(!scan.lobby(i,now));
  // New sessions work at high millis values and across the unsigned wrap.
  now=0xFFFFFF00U;TicTacToeMesh rolloverA,rolloverB;setup(rolloverA,rolloverB);
  assert(rolloverA.move(4));settle(rolloverA,rolloverB);assert(rolloverB.position().x==16);
  printf("Two-peer automatic lobby/join/start, named hosts and rematches, loss/duplicates, illegal moves, bounded retries and expiry passed; runtime %zu bytes; event 52 text bytes\n",sizeof(TicTacToeMesh));
}
