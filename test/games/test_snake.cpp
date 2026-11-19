#include <games/Snake.h>
#include <cassert>
#include <cstdio>
#include <vector>
using solo_games::Snake;
namespace solo_games {
struct SnakeFixture {
  static void load(Snake& game,const std::vector<uint16_t>& body,Snake::Direction direction,uint16_t food) {
    game.reset(1,1,0);game._tail=0;game._length=body.size();
    for(auto& byte:game._occupied)byte=0;
    for(unsigned i=0;i<body.size();++i){game._body[i]=body[i];game.setOccupied(body[i],true);}
    game._direction=game._queued=direction;game._turn_queued=false;game._food=food;
  }
  static void food(Snake& game,uint16_t cell){game._food=cell;}
};
}
static void invariant(const Snake& game) {
  assert(game.length()>=3 && game.length()<=Snake::CAPACITY);
  bool seen[Snake::CAPACITY]={};
  for(unsigned i=0;i<game.length();++i){auto cell=game.body(i);assert(cell<Snake::CAPACITY && !seen[cell]);seen[cell]=true;}
  assert(game.food()==Snake::NO_FOOD || (game.food()<Snake::CAPACITY && !seen[game.food()]));
}
static Snake::Direction direction(uint16_t from,uint16_t to) {
  if(to==from+1)return Snake::RIGHT;
  if(from==to+1)return Snake::LEFT;
  return to>from?Snake::DOWN:Snake::UP;
}
int main() {
  Snake game;
  for(unsigned seed=0;seed<1000;++seed){game.reset(seed,1,0);invariant(game);}
  game.reset(1,1,0);auto head=game.head();
  assert(!game.steer(Snake::LEFT));assert(game.steer(Snake::UP));assert(!game.steer(Snake::LEFT));
  assert(game.tick(189)==Snake::NONE && game.head()==head);
  assert(game.tick(190)==Snake::MOVED && game.head()==head-Snake::WIDTH);
  assert(game.steer(Snake::LEFT));assert(game.tick(380)==Snake::MOVED);
  game.reset(1,1,0);head=game.head();auto tail=game.body(0);
  solo_games::SnakeFixture::food(game,head+1);
  assert(game.tick(190)==Snake::ATE && game.length()==4 && game.score()==1 && game.body(0)==tail);invariant(game);
  solo_games::SnakeFixture::load(game,{27,28,29},Snake::RIGHT,50);
  assert(game.tick(190)==Snake::LOST && game.finished() && !game.won());
  assert(game.tick(9999)==Snake::NONE);
  // Moving into a tail that vacates on this tick is legal.
  solo_games::SnakeFixture::load(game,{31,32,62,61},Snake::LEFT,100);
  assert(game.steer(Snake::UP));assert(game.tick(190)==Snake::MOVED && game.head()==31);invariant(game);
  // A non-tail occupied square is a collision.
  solo_games::SnakeFixture::load(game,{0,1,31,32,2},Snake::UP,100);
  assert(game.steer(Snake::LEFT));assert(game.tick(190)==Snake::LOST);
  // Ring wrap, all four directions and complete-board growth follow an
  // independent Hamiltonian cycle, without touching the model's algorithm.
  std::vector<uint16_t> cycle;
  for(unsigned y=0;y<Snake::HEIGHT;++y){
    if(!(y&1)){for(unsigned x=1;x<Snake::WIDTH;++x)cycle.push_back(y*Snake::WIDTH+x);}
    else for(int x=Snake::WIDTH-1;x>=1;--x)cycle.push_back(y*Snake::WIDTH+x);
  }
  for(int y=Snake::HEIGHT-1;y>=0;--y)cycle.push_back(y*Snake::WIDTH);
  assert(cycle.size()==Snake::CAPACITY);
  solo_games::SnakeFixture::load(game,{cycle[0],cycle[1],cycle[2]},Snake::RIGHT,Snake::NO_FOOD);
  uint32_t now=0;
  for(unsigned i=3;i<Snake::CAPACITY*4;++i){
    auto next=cycle[i%cycle.size()];assert(game.steer(direction(game.head(),next)));
    now+=game.interval();assert(game.tick(now)==Snake::MOVED && game.head()==next);invariant(game);
  }
  std::vector<uint16_t> full(cycle.begin(),cycle.end()-2);
  solo_games::SnakeFixture::load(game,full,direction(cycle[full.size()-2],cycle[full.size()-1]),cycle[full.size()]);
  now=1000;
  for(unsigned i=full.size();i<cycle.size();++i){
    solo_games::SnakeFixture::food(game,cycle[i]);assert(game.steer(direction(game.head(),cycle[i])));
    now+=game.interval();assert(game.tick(now)==(i+1==cycle.size()?Snake::WON:Snake::ATE));invariant(game);
  }
  assert(game.won() && game.length()==Snake::CAPACITY && game.food()==Snake::NO_FOOD && game.interval()==70);
  // Missed time is not replayed as a blocking movement burst; rollover is safe.
  game.reset(2,1,0);head=game.head();assert(game.tick(100000)!=Snake::NONE);
  assert(game.head()==head+1 && game.tick(100000)==Snake::NONE);
  now=0xFFFFFFA0U;game.reset(3,1,now);head=game.head();
  assert(game.tick(now+189)==Snake::NONE && game.tick(now+190)==Snake::MOVED && game.head()==head+1);
  printf("Snake: steering, growth, collisions, tail vacancy, ring wrap, full board, speed floor and clock rollover passed; state %zu bytes\n",sizeof(Snake));
}
