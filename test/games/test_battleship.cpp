#include <games/Battleship.h>
#include <cassert>
#include <cstdio>
using solo_games::Battleship;
namespace solo_games {
struct BattleshipFixture {
  static void changeHiddenFleet(Battleship& game){
    uint8_t copy[100];for(unsigned i=0;i<100;++i)copy[i]=game._fleet[0][i];
    for(unsigned i=0;i<100;++i)game._fleet[0][(i+17)%100]=copy[i];
  }
};
}
static void fleet(const Battleship& game,unsigned side){
  unsigned counts[6]={};for(unsigned i=0;i<100;++i){unsigned id=game.shipAt(side,i);assert(id<=5);++counts[id];}
  assert(counts[0]==83);for(unsigned i=0;i<5;++i)assert(counts[i+1]==Battleship::shipLength(i));
}
int main(){
  Battleship game;
  for(unsigned seed=0;seed<1000;++seed){game.reset(seed);fleet(game,1);assert(game.autoPlace(0));fleet(game,0);}
  game.reset(1);assert(!game.start());
  assert(!game.place(0,1,0,false) && !game.place(0,0,8,false) && !game.place(0,0,70,true));
  assert(game.place(0,0,0,false));assert(!game.place(0,1,3,false));
  assert(game.place(0,1,9,true));assert(game.place(0,2,20,false));
  assert(game.place(0,3,40,false));assert(game.place(0,4,60,false));fleet(game,0);
  assert(game.start() && !game.clearFleet(0) && !game.autoPlace(0));
  auto first=game.fire(0,99);assert(first!=Battleship::INVALID && game.turn()==1);
  assert(game.fire(0,98)==Battleship::INVALID && game.turn()==1);
  assert(game.fire(1,99)==Battleship::MISS && game.turn()==0);
  assert(game.fire(0,99)==Battleship::INVALID && game.fire(0,255)==Battleship::INVALID && game.turn()==0);
  game.reset(7);game.autoPlace(0);game.start();game.fire(0,99);
  auto other=game;solo_games::BattleshipFixture::changeHiddenFleet(other);
  assert(game.chooseShot(1)==other.chooseShot(1)); // CPU cannot peek at hidden cells.
  // Every ship sinks only after all its cells are hit; a hit still changes turn.
  game.reset(10);game.autoPlace(0);game.start();unsigned sunk=0;
  for(unsigned cell=0;cell<100;++cell)if(game.shipAt(1,cell)) {
    auto shot=game.fire(0,cell);assert(shot==Battleship::HIT || shot==Battleship::SUNK || shot==Battleship::WIN);
    sunk+=shot==Battleship::SUNK || shot==Battleship::WIN;
    if(!game.result()) {
      unsigned miss=0;while(game.shipAt(0,miss) || game.shotAt(1,miss))++miss;
      assert(miss<100 && game.fire(1,miss)==Battleship::MISS);
    }
  }
  assert(game.result()==1 && sunk==5 && game.fire(0,0)==Battleship::INVALID);
  for(unsigned seed=1;seed<=200;++seed) {
    game.reset(seed);game.autoPlace(0);game.start();
    unsigned shots=0;
    while(!game.result() && shots<200) {
      unsigned attacker=game.turn(),cell=game.chooseShot(attacker);assert(cell<100 && !game.shotAt(attacker,cell));
      assert(game.fire(attacker,cell)!=Battleship::INVALID);++shots;
    }
    assert(game.result() && shots<=199);
  }
  printf("Battleship: 1000 fleets, placement bounds/overlap/rotation, turn and shot validation, sunk/win, fair CPU and 200 complete games passed; state %zu bytes\n",sizeof(Battleship));
}
