#define abs(x) ((x)>0?(x):-(x))
#include <games/Artillery.h>
#undef abs
#include <cassert>
#include <cstdio>
using solo_games::Artillery;
namespace solo_games {
struct ArtilleryFixture {
  static void flat(Artillery& game,int wind=0){game.reset(1);for(auto& y:game._terrain)y=50;game._wind=wind;}
  static void restoreHealthAndTurn(Artillery& game){game._health[0]=game._health[1]=100;game._turn=0;game._result=0;}
};
}
static void finish(Artillery& game){unsigned steps=0;while(game.flying()){game.step();assert(++steps<=256);}for(unsigned x=0;x<128;++x)assert(game.terrain(x)<=63);}
int main(){
  Artillery a,b;
  for(unsigned seed=0;seed<1000;++seed){
    a.reset(seed);b.reset(seed);assert(a.wind()==b.wind());
    for(unsigned x=0;x<128;++x)assert(a.terrain(x)==b.terrain(x) && a.terrain(x)>=40 && a.terrain(x)<=60);
  }
  a.reset(5);assert(!a.fire(1,45,85,0) && !a.fire(0,9,85,0) && !a.fire(0,45,101,0) && !a.fire(0,46,85,0));
  assert(a.fire(0,45,85,0) && !a.fire(0,45,85,0));b=a;finish(a);finish(b);
  assert(a.turn()==1 && a.impactX()==b.impactX() && a.impactY()==b.impactY() && a.health(0)==b.health(0) && a.health(1)==b.health(1));
  for(unsigned x=0;x<128;++x)assert(a.terrain(x)==b.terrain(x));
  solo_games::ArtilleryFixture::flat(a);assert(a.fire(0,80,20,0));finish(a);assert(a.health(0)<100 && a.health(1)==100);
  bool crater=false;for(unsigned x=0;x<128;++x){assert(a.terrain(x)>=50);crater|=a.terrain(x)>50;}assert(crater);
  for(unsigned shot=0;shot<3;++shot){
    solo_games::ArtilleryFixture::restoreHealthAndTurn(a);assert(a.fire(0,80,20,1));finish(a);
  }
  solo_games::ArtilleryFixture::restoreHealthAndTurn(a);assert(!a.heavy(0) && !a.fire(0,45,85,1) && a.fire(0,45,85,0));finish(a);
  solo_games::ArtilleryFixture::flat(a,-2);solo_games::ArtilleryFixture::flat(b,2);
  a.fire(0,45,70,0);b.fire(0,45,70,0);finish(a);finish(b);assert(a.impactX()<b.impactX());
  solo_games::ArtilleryFixture::flat(a);solo_games::ArtilleryAI ai;ai.begin(a,true);
  unsigned candidates=0;while(!ai.step(1))assert(++candidates<=510);
  assert(a.fire(0,ai.angle(),ai.power(),ai.weapon()));finish(a);assert(a.health(1)<100 && a.health(0)==100);
  for(unsigned seed=1;seed<=20;++seed){
    a.reset(seed);
    while(!a.result()) {
      ai.begin(a,seed&1);unsigned count=0;while(!ai.step(1))assert(++count<=510);
      assert(a.fire(a.turn(),ai.angle(),ai.power(),ai.weapon()));finish(a);assert(a.shots()<=40);
    }
    assert(a.result()>=1 && a.result()<=3);
  }
  printf("Artillery: deterministic terrain/trajectories, bounds, crater/fall/self damage, ammo, wind, bounded planning and 20 complete CPU games passed; rules+planner %zu bytes\n",sizeof(Artillery)+sizeof(solo_games::ArtilleryAI));
}
