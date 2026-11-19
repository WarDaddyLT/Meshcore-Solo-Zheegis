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
int main() {
  using Board = solo_games::Checkers;
  UITask a(10), b(20);
  ArenaOnlineScreen host(&a, CHECKERS), guest(&b, CHECKERS);
  bool dropCommit = true;
  auto pump = [&](unsigned steps) {
    for (unsigned i = 0; i < steps; ++i) {
      ticks += 50;
      host.poll();
      guest.poll();
      host.render(display);
      guest.render(display);
      auto deliver = [&](UITask& from, ArenaOnlineScreen& to) {
        for (const auto& text : from.sent) {
          Packet packet;
          assert(Codec::decode(text.c_str(), packet) && packet.game == CHECKERS);
          if (packet.type == COMMIT && dropCommit) {
            dropCommit = false;
            continue;
          }
          to.receiveText(1, text.c_str(), from.name.c_str());
          to.receiveText(1, text.c_str(), from.name.c_str());
        }
        from.sent.clear();
      };
      deliver(a, guest);
      deliver(b, host);
    }
  };
  key(guest, KEY_DOWN);
  key(guest, KEY_ENTER);
  key(host, KEY_ENTER);
  key(host, KEY_ENTER);
  pump(100);
  assert(state(guest, 5) == 2);
  key(guest, KEY_ENTER);
  pump(200);
  assert(state(host, 1) == Session::PLAYING && state(guest, 1) == Session::PLAYING);

  auto select = [](ArenaOnlineScreen& screen, unsigned square) {
    int row = Board::row(square), col = Board::col(square);
    if (state(screen, 2) == 1) { row = 7 - row; col = 7 - col; }
    while (state(screen, 7) / 8 < unsigned(row)) key(screen, KEY_DOWN);
    while (state(screen, 7) / 8 > unsigned(row)) key(screen, KEY_UP);
    while (state(screen, 7) % 8 < unsigned(col)) key(screen, KEY_RIGHT);
    while (state(screen, 7) % 8 > unsigned(col)) key(screen, KEY_LEFT);
    key(screen, KEY_ENTER);
  };
  Board board;
  board.reset();
  assert(state(host, 9) == board.stateHash() && state(guest, 9) == board.stateHash());
  auto before = state(guest, 8);
  select(guest, 8);
  select(guest, 12);
  pump(10);
  assert(state(guest, 8) == before && state(host, 8) == before);

  unsigned turns = 0, captureChains = 0;
  while (!board.result() && turns < 160) {
    bool firstPlayer = board.turn() == 1;
    auto& screen = firstPlayer ? host : guest;
    Board::Move move;
    assert(board.legalMoveAt(0, move));
    if (board.forced() == Board::NO_SQUARE) select(screen, move.from);
    select(screen, move.to);
    assert(board.play(move.from, move.to));
    if (board.forced() != Board::NO_SQUARE) ++captureChains;
    ++turns;
    pump(turns == 1 ? 250 : 20);
    assert(state(host, 8) == turns && state(guest, 8) == turns);
    assert(state(host, 9) == board.stateHash() && state(guest, 9) == board.stateHash());
  }

  assert(!dropCommit && board.result() && captureChains);
  assert(state(host, 1) == Session::FINISHED && state(guest, 1) == Session::FINISHED);
  key(guest, KEY_ENTER);
  pump(100);
  board.reset();
  assert(state(host, 3) == 1 && state(guest, 3) == 1);
  assert(state(host, 9) == board.stateHash() && state(guest, 9) == board.stateHash());
  key(host, KEY_CANCEL);
  pump(40);
  assert(state(guest, 5) == 5);
  puts("LoRa Checkers screens: host/join, turn validation, captures, retransmission, rematch and exit passed.");
}
