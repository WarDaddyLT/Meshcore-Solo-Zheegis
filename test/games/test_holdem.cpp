#include <games/Holdem.h>
#include <cassert>
#include <cstdio>
using solo_games::Holdem;
using solo_games::PokerHand;
namespace solo_games {
struct HoldemFixture {
  static void stacks(Holdem& game,unsigned a,unsigned b){game._chips[0]=a;game._chips[1]=b;}
  static void hidden(Holdem& game){
    uint8_t old=game._hole[0][0];game._hole[0][0]=game._deck[game._deck_pos+2];game._deck[game._deck_pos+2]=old;
  }
  static void tie(Holdem& game){
    game.reset(1);game._chips[0]=495;game._chips[1]=496;game._pot=9;game._dealer=0;game._board_count=5;
    for(unsigned i=0;i<5;++i)game._board[i]=8+i; // T-A clubs: board plays for both.
    game._hole[0][0]=13;game._hole[0][1]=14;game._hole[1][0]=26;game._hole[1][1]=27;game.showdown();
  }
};
}
static uint8_t card(unsigned rank,unsigned suit){return uint8_t(rank-2+13*suit);}
static uint32_t rank5(uint8_t a,uint8_t b,uint8_t c,uint8_t d,uint8_t e){uint8_t cards[]={a,b,c,d,e};return PokerHand::rank(cards,5);}
static void invariant(const Holdem& game){
  assert(game.chips(0)+game.chips(1)+game.pot()==1000);
  assert(game.boardCount()==0 || game.boardCount()==3 || game.boardCount()==4 || game.boardCount()==5);
  uint64_t seen=0;
  for(unsigned side=0;side<2;++side)for(unsigned i=0;i<2;++i){unsigned c=game.hole(side,i);assert(c<52 && !((seen>>c)&1));seen|=uint64_t(1)<<c;}
  for(unsigned i=0;i<game.boardCount();++i){unsigned c=game.board(i);assert(c<52 && !((seen>>c)&1));seen|=uint64_t(1)<<c;}
}
int main(){
  uint32_t categories[9]={};uint8_t cards[5];
  for(unsigned a=0;a<48;++a)for(unsigned b=a+1;b<49;++b)for(unsigned c=b+1;c<50;++c)for(unsigned d=c+1;d<51;++d)for(unsigned e=d+1;e<52;++e){
    cards[0]=a;cards[1]=b;cards[2]=c;cards[3]=d;cards[4]=e;
    uint32_t score=PokerHand::rank(cards,5);assert(score && (score>>20)<9);++categories[score>>20];
  }
  const uint32_t expected[]={1302540,1098240,123552,54912,10200,5108,3744,624,40};
  for(unsigned i=0;i<9;++i)assert(categories[i]==expected[i]);
  auto wheel=rank5(card(14,0),card(2,1),card(3,2),card(4,3),card(5,0));
  auto six=rank5(card(6,0),card(2,1),card(3,2),card(4,3),card(5,0));assert((wheel>>20)==PokerHand::STRAIGHT && six>wheel);
  auto kickerA=rank5(card(12,0),card(12,1),card(14,2),card(9,3),card(6,0));
  auto kickerK=rank5(card(12,0),card(12,1),card(13,2),card(9,3),card(6,0));assert(kickerA>kickerK);
  uint8_t seven[]={card(13,0),card(13,1),card(13,2),card(12,0),card(12,1),card(12,2),card(2,0)};
  assert(PokerHand::rank(seven,7)==rank5(card(13,0),card(13,1),card(13,2),card(12,0),card(12,1)));
  uint8_t bad[]={0,0,2,3,4};assert(!PokerHand::rank(bad,5));bad[1]=52;assert(!PokerHand::rank(bad,5));assert(!PokerHand::rank(bad,4));
  Holdem game;game.reset(1);assert(game.deal());invariant(game);
  assert(game.dealer()==0 && game.active()==0 && game.toCall(0)==5 && game.pot()==15);
  assert(!game.act(1,Holdem::CHECK_CALL));assert(game.act(0,Holdem::CHECK_CALL));
  assert(game.active()==1 && game.phase()==Holdem::PREFLOP);assert(game.act(1,Holdem::CHECK_CALL));
  assert(game.phase()==Holdem::FLOP && game.active()==1 && game.boardCount()==3);
  while(game.phase()!=Holdem::HAND_OVER){assert(game.act(game.active(),Holdem::CHECK_CALL));invariant(game);}
  assert(game.shown() && game.boardCount()==5 && game.deal() && game.dealer()==1 && game.active()==1);
  game.reset(2);game.deal();assert(game.minRaiseTo(0)==20);
  assert(!game.act(0,Holdem::RAISE,19) && !game.act(0,Holdem::RAISE,501));
  assert(game.act(0,Holdem::RAISE,20) && game.toCall(1)==10 && game.minRaiseTo(1)==30);
  assert(game.act(1,Holdem::CHECK_CALL) && game.phase()==Holdem::FLOP && game.pot()==40);invariant(game);
  game.reset(3);game.deal();assert(game.act(0,Holdem::FOLD) && game.winner()==2 && !game.shown());invariant(game);
  game.reset(4);game.deal();assert(game.act(0,Holdem::RAISE,500) && !game.maxRaiseTo(1));
  assert(game.act(1,Holdem::CHECK_CALL) && game.phase()==Holdem::HAND_OVER && game.wonPot()==1000);invariant(game);
  game.reset(5);solo_games::HoldemFixture::stacks(game,3,997);assert(game.deal() && game.phase()==Holdem::HAND_OVER && game.wonPot()==6);invariant(game);
  game.reset(6);solo_games::HoldemFixture::stacks(game,993,7);game.deal();
  assert(game.active()==0 && game.toCall(0)==2 && !game.maxRaiseTo(0));
  assert(game.act(0,Holdem::CHECK_CALL) && game.wonPot()==14);invariant(game);
  game.reset(7);solo_games::HoldemFixture::stacks(game,15,985);game.deal();
  assert(game.minRaiseTo(0)==15 && game.act(0,Holdem::RAISE,15) && !game.maxRaiseTo(1));
  assert(game.act(1,Holdem::CHECK_CALL) && game.wonPot()==30);invariant(game);
  solo_games::HoldemFixture::tie(game);assert(game.winner()==3 && game.chips(0)==499 && game.chips(1)==501);invariant(game);
  unsigned played=0;
  for(unsigned seed=1;seed<=100;++seed){
    game.reset(seed);
    for(unsigned hand=0;hand<100 && !game.busted();++hand){
      assert(game.deal());invariant(game);unsigned actions=0;
      while(game.phase()!=Holdem::HAND_OVER){
        unsigned actor=game.active();auto clone=game;
        if(actor==1)solo_games::HoldemFixture::hidden(clone);
        auto choice=game.chooseAction(actor),same=clone.chooseAction(actor);
        assert(choice.action==same.action && choice.target==same.target);
        assert(game.act(actor,choice.action,choice.target));invariant(game);assert(++actions<1000);
      }
      ++played;
    }
  }
  printf("Holdem: all 2,598,960 five-card hands, seven-card selection/kickers, betting streets, heads-up order, all-ins/refunds, ties, hidden-card CPU and %u complete hands passed; state %zu bytes\n",played,sizeof(Holdem));
}
