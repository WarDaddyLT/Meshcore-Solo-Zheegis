#include <cassert>
#include <cstdio>
#include <cstring>
#include <games/veylith/Progress.h>
using namespace veylith;
int main() {
  bool pairs[5][5] = {};
  for (unsigned i = 0; i < 20; ++i) {
    auto &f = families[i];
    assert(f.primary < 5 && f.secondary < 5 && f.primary != f.secondary && !pairs[f.primary][f.secondary]);
    pairs[f.primary][f.secondary] = true;
    for (unsigned s = 0; s < 3; ++s) {
      assert(f.names[s] && strlen(f.names[s]) >= 4);
      for (unsigned j = 0; j <= i; ++j)
        for (unsigned t = 0; t < 3; ++t)
          if (i != j || s != t) assert(strcmp(f.names[s], families[j].names[t]));
    }
  }
  unsigned roles[8] = {};
  for (unsigned i = 1; i <= 80; ++i) {
    auto &m = move(i);
    assert(m.name && m.role < 8 && m.target < 6 && m.element < 6 && m.energy <= 12 && m.level >= 1 &&
           m.level <= 99);
    ++roles[m.role];
    for (unsigned j = 1; j < i; ++j)
      assert(strcmp(m.name, move(j).name));
  }
  for (auto count : roles)
    assert(count);
  for (unsigned attack = 0; attack < 5; ++attack) {
    unsigned strong = 0, weak = 0;
    for (unsigned d = 0; d < 5; ++d) {
      auto e = effectiveness(attack, d, d);
      strong += e > 10000;
      weak += e < 10000;
    }
    assert(strong == 2 && weak == 2);
    for (unsigned a = 0; a < 5; ++a)
      for (unsigned b = 0; b < 5; ++b)
        if (a != b) {
          auto value = effectiveness(attack, a, b);
          assert(value >= 7000 && value <= 14400 && value == effectiveness(attack, b, a));
        }
  }
  assert(effectiveness(EMBER, THORN, STONE) == 14400 && effectiveness(EMBER, TIDE, SPARK) == 7000);
  Progress p;
  p.reset();
  assert(p.valid());
  for (unsigned f = 0; f < 20; ++f) {
    assert(p.family[f].level() == 1 && p.family[f].stage() == 0);
    for (auto id : p.family[f].equipped)
      assert(p.family[f].eligible(f, id));
  }
  auto &t = p.family[0];
  assert(!t.buy(HP));
  t.wallet = 100000;
  t.lifetime = 100000;
  assert(t.cost(HP) == 30);
  assert(t.buy(HP) && t.wallet == 99970 && t.value(0, HP) == 40 && t.level() == 1 && t.total() % 3 == 1);
  assert(t.buy(ATK) && t.total() % 3 == 2 && t.level() == 1);
  assert(t.buy(DEF) && t.level() == 2 && t.total() % 3 == 0);
  while (t.total() < 42)
    assert(t.buy(t.total() % 6));
  assert(t.level() == 15 && t.stage() == 1);
  while (t.total() < 102)
    assert(t.buy(t.total() % 6));
  assert(t.level() == 35 && t.stage() == 2);
  t.wallet = 1000000;
  while (t.total() < 294)
    assert(t.buy(t.total() % 6));
  assert(t.level() == 99 && !t.buy(HP));
  unsigned oldWallet = t.wallet;
  uint8_t lineup[] = { 0, 6, 16 };
  auto id = p.reserveResult();
  assert(id && p.award(id, 1, 99, lineup, 99));
  assert(t.wallet == oldWallet && !p.award(id, 1, 99, lineup, 99));
  Progress loss;
  loss.reset();
  id = loss.reserveResult();
  assert(loss.award(id, 2, 10, lineup, 1));
  for (auto f : lineup)
    assert(loss.family[f].wallet == 63);
  assert(loss.family[2].wallet == 0);
  Progress win;
  win.reset();
  id = win.reserveResult();
  assert(win.award(id, 1, 10, lineup, 1));
  assert(win.family[0].wallet == 140);
  auto wallet = win.family[0].wallet;
  assert(win.family[0].equip(0, 2, 41));
  assert(win.family[0].wallet == wallet && !win.family[0].equip(0, 2, 80) && !win.family[0].equip(0, 2, 9));
  assert(!win.select(0, 6) && win.select(0, 1) && win.valid());
  uint8_t bytes[SaveCodec::SIZE];
  assert(SaveCodec::encode(win, bytes, sizeof(bytes)));
  Progress restored;
  assert(SaveCodec::decode(bytes, sizeof(bytes), restored));
  assert(restored.valid() && restored.family[0].wallet == 140 && restored.selected[0] == 1);
  uint8_t canonical[SaveCodec::SIZE];
  memcpy(canonical, bytes, sizeof(bytes));
  for (unsigned i = 0; i < sizeof(bytes); ++i) {
    memcpy(bytes, canonical, sizeof(bytes));
    bytes[i] ^= 1;
    assert(!SaveCodec::decode(bytes, sizeof(bytes), restored));
  }
  for (unsigned n = 0; n < sizeof(bytes); ++n)
    assert(!SaveCodec::decode(canonical, n, restored));
  printf(
      "Veylith content/progression: 20 families, 60 forms, 80 moves, type wheel, XP/training/evolution/caps, "
      "free loadouts and %zu-byte corruption-safe codec passed; progress %zu bytes\n",
      SaveCodec::SIZE, sizeof(Progress));
}
