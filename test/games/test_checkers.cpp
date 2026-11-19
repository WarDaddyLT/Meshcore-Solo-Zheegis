#include <games/Checkers.h>
#include <cassert>
#include <cstdio>
using solo_games::Checkers;
namespace solo_games {
struct CheckersFixture {
  static void clear(Checkers& p,int turn=1){p.reset();for(auto& piece:p._board)piece=0;p._turn=turn;}
  static void put(Checkers& p,int row,int col,int piece){int i=Checkers::square(row,col);assert(i>=0);p._board[i]=piece;}
  static void quiet(Checkers& p,unsigned value){p._quiet=value;}
};
}
static unsigned sq(int r,int c){int i=Checkers::square(r,c);assert(i>=0);return unsigned(i);}
int main(){
  Checkers p;Checkers::Move moves[Checkers::MAX_MOVES];
  p.reset();assert(p.legalMoves(moves)==7 && p.turn()==1 && !p.result());
  for(unsigned i=0;i<32;++i)assert(Checkers::square(Checkers::row(i),Checkers::col(i))==int(i));
  assert(Checkers::square(-1,0)==-1 && Checkers::square(1,8)==-1 && Checkers::square(0,0)==-1);
  assert(!p.play(255,0));
  solo_games::CheckersFixture::clear(p);
  solo_games::CheckersFixture::put(p,4,3,1);solo_games::CheckersFixture::put(p,0,7,-2);
  assert(!p.play(sq(4,3),sq(5,2)) && p.turn()==1);
  assert(p.play(sq(4,3),sq(3,2)) && p.turn()==-1);
  // A capture elsewhere forbids quiet moves and locks a multi-jump to one man.
  solo_games::CheckersFixture::clear(p);
  solo_games::CheckersFixture::put(p,5,0,1);solo_games::CheckersFixture::put(p,5,4,1);
  solo_games::CheckersFixture::put(p,4,1,-1);solo_games::CheckersFixture::put(p,2,3,-1);
  solo_games::CheckersFixture::put(p,0,7,-2);
  assert(p.mustCapture() && !p.play(sq(5,4),sq(4,3)));
  assert(p.play(sq(5,0),sq(3,2)) && p.turn()==1 && p.forced()==sq(3,2));
  assert(!p.play(sq(5,4),sq(4,3)) && !p.play(sq(3,2),sq(2,1)));
  assert(p.play(sq(3,2),sq(1,4)) && p.turn()==-1 && p.forced()==255);
  // Crowning is immediate; the new flying king continues the same capture turn.
  solo_games::CheckersFixture::clear(p);
  solo_games::CheckersFixture::put(p,2,1,1);solo_games::CheckersFixture::put(p,1,2,-1);
  solo_games::CheckersFixture::put(p,2,5,-1);solo_games::CheckersFixture::quiet(p,79);
  assert(p.play(sq(2,1),sq(0,3)) && p.at(sq(0,3))==2 && p.turn()==1 && p.forced()==sq(0,3));
  assert(p.play(sq(0,3),sq(4,7)) && p.at(sq(4,7))==2 && p.result()==1 && !p.captured());
  solo_games::CheckersFixture::clear(p);
  solo_games::CheckersFixture::put(p,3,2,2);solo_games::CheckersFixture::put(p,0,7,-2);
  assert(p.play(sq(3,2),sq(5,4))); // A king can slide several empty diagonal squares.
  // Men can capture backward, but cannot take an ordinary backward step.
  solo_games::CheckersFixture::clear(p);
  solo_games::CheckersFixture::put(p,3,2,1);solo_games::CheckersFixture::put(p,4,3,-1);
  solo_games::CheckersFixture::put(p,0,7,-2);
  assert(!p.play(sq(3,2),sq(2,1)) && p.play(sq(3,2),sq(5,4)));
  // Flying capture may land anywhere past one enemy, never past two blockers.
  solo_games::CheckersFixture::clear(p);
  solo_games::CheckersFixture::put(p,4,3,2);solo_games::CheckersFixture::put(p,2,5,-1);
  auto captures=p.legalMoves(moves);assert(captures==2);
  assert(p.play(sq(4,3),sq(0,7)) && p.result()==1);
  solo_games::CheckersFixture::clear(p);
  solo_games::CheckersFixture::put(p,4,3,2);solo_games::CheckersFixture::put(p,2,5,-1);
  solo_games::CheckersFixture::put(p,1,6,-1);assert(!p.play(sq(4,3),sq(0,7)));
  // Already captured pieces remain blockers until the whole turn ends.
  solo_games::CheckersFixture::clear(p);
  solo_games::CheckersFixture::put(p,4,3,2);solo_games::CheckersFixture::put(p,2,5,-1);
  solo_games::CheckersFixture::put(p,5,2,-1);
  assert(p.play(sq(4,3),sq(1,6)) && p.turn()==-1 && p.forced()==255);
  assert(!p.at(sq(2,5)) && p.at(sq(5,2))==-1);

  // No moves loses, regardless of whether a piece remains.
  solo_games::CheckersFixture::clear(p);
  solo_games::CheckersFixture::put(p,5,2,1);solo_games::CheckersFixture::put(p,7,0,-1);
  assert(p.play(sq(5,2),sq(4,1)) && p.result()==1);
  solo_games::CheckersFixture::clear(p);
  solo_games::CheckersFixture::put(p,3,2,1);solo_games::CheckersFixture::put(p,2,3,-1);
  assert(p.play(sq(3,2),sq(1,4)) && p.result()==1);
  // 80 completed quiet turns draw; captures/promotions reset this counter.
  solo_games::CheckersFixture::clear(p);
  solo_games::CheckersFixture::put(p,7,0,2);solo_games::CheckersFixture::put(p,0,7,-2);
  solo_games::CheckersFixture::quiet(p,79);
  assert(p.play(sq(7,0),sq(6,1)) && p.result()==3);
  unsigned completed=0;
  for(unsigned game=0;game<40;++game){
    p.reset();
    for(unsigned turn=0;!p.result() && turn<4200;++turn){
      auto count=p.legalMoves(moves);assert(count);
      solo_games::CheckersAI ai;ai.begin(p,game&1,game*977+turn+1);
      unsigned steps=0;while(!ai.step(1))assert(++steps<=unsigned(Checkers::MAX_MOVES)*(Checkers::MAX_MOVES+2));
      auto move=ai.choice();bool legal=false;
      for(unsigned i=0;i<count;++i)if(moves[i].from==move.from && moves[i].to==move.to)legal=true;
      assert(legal && p.play(move.from,move.to));
      unsigned human=0,cpu=0;for(unsigned i=0;i<32;++i){human+=p.at(i)>0;cpu+=p.at(i)<0;}
      assert(human<=12 && cpu<=12);
    }
    assert(p.result());++completed;
  }
  printf("Checkers: backward/multiple captures, immediate crowning, flying kings, no-move loss, draw and %u complete CPU games passed; rules+AI %zu bytes\n",completed,sizeof(Checkers)+sizeof(solo_games::CheckersAI));
}
