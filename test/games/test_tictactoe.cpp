#include <games/TicTacToe.h>
#include <cassert>
#include <cstdio>
#include <cstring>

using namespace solo_games;
static bool seen[19683];
static int8_t values[19683];
static unsigned positions = 0, hard_checks = 0, max_batches = 0;

static unsigned key(const TicTacToePosition& p) {
  unsigned k=0,scale=1;
  for(uint8_t i=0;i<9;++i){k+=p.at(i)*scale;scale*=3;}
  return k;
}
static int reference(TicTacToePosition p) {
  uint8_t result=p.result();
  if(result)return result==1?1:result==2?-1:0;
  int8_t& cached=values[key(p)];if(cached!=2)return cached;
  int best=p.turn==1?-2:2;
  for(uint8_t c=0;c<9;++c)if(p.legal(c)) {
    auto next=p;assert(next.play(c));int value=reference(next);
    if(p.turn==1?value>best:value<best)best=value;
  }
  return cached=int8_t(best);
}
static void visit(TicTacToePosition p) {
  unsigned k=key(p);if(seen[k])return;seen[k]=true;++positions;
  if(p.result())return;
  TicTacToeAI ai;ai.begin(p,TicTacToeAI::HARD,1);
  unsigned batches=0;
  while(!ai.step(128)){assert(++batches<10000);}
  if(batches>max_batches)max_batches=batches;
  assert(p.legal(ai.choice()));auto chosen=p;assert(chosen.play(ai.choice()));
  assert(reference(chosen)==reference(p));++hard_checks;
  for(uint8_t c=0;c<9;++c)if(p.legal(c)){auto next=p;assert(next.play(c));visit(next);}
}
int main() {
  memset(values,2,sizeof(values));
  TicTacToePosition p;
  assert(!p.result() && p.turn==1 && !p.play(9) && !p.play(255));
  assert(p.play(4));auto old=p;assert(!p.play(4));assert(p.x==old.x&&p.o==old.o&&p.turn==old.turn);
  const uint16_t lines[]={7,56,448,73,146,292,273,84};
  for(uint16_t mask:lines){p={};p.x=mask;assert(p.result()==1&&!p.play(8));p.x=0;p.o=mask;assert(p.result()==2);}
  p={};p.x=(1<<0)|(1<<2)|(1<<3)|(1<<7)|(1<<8);p.o=511^p.x;assert(p.result()==3);
  for(uint32_t seed=0;seed<200;++seed){p={};p.play(4);TicTacToeAI ai;ai.begin(p,TicTacToeAI::EASY,seed);assert(!ai.busy()&&p.legal(ai.choice()));}
  p={};p.x=3;p.o=24;p.turn=1;TicTacToeAI normal;normal.begin(p,TicTacToeAI::NORMAL,1);assert(normal.choice()==2);
  p={};p.x=3;p.o=16;p.turn=2;normal.begin(p,TicTacToeAI::NORMAL,1);assert(normal.choice()==2);
  p={};visit(p);assert(positions==5478);
  printf("%u reachable positions; Hard AI optimal on all %u nonterminal positions; max %u batches of 128 search steps\n",positions,hard_checks,max_batches);
  printf("Position + CPU search: %zu bytes, no recursion or dynamic allocation\n",sizeof(TicTacToePosition)+sizeof(TicTacToeAI));
}
