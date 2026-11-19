#include <cassert>
#include <chrono>
#include <cstdio>
#include <games/veylith/Battle.h>
using namespace veylith;
namespace veylith {
struct BattleFixture {
  static Monster &member(Battle &b, unsigned side, unsigned i = 0) { return b._team[side][i]; }
  static void equip(Battle &b, unsigned side, unsigned id) {
    auto &m = member(b, side);
    m.moves[0] = id;
    m.cooldown[0] = 0;
    m.energy = 12;
  }
  static void round(Battle &b, unsigned round) { b._round = round; }
};
} // namespace veylith
static Action skill(unsigned id, unsigned target = 0) {
  Action a;
  a.move = id;
  a.target = target;
  return a;
}
static Build builds[3];
static Battle fresh() {
  Battle b;
  b.begin(builds, builds, 7);
  return b;
}
int main() {
  Progress p;
  p.reset();
  for (unsigned i = 0; i < 3; ++i) {
    builds[i].family = p.selected[i];
    builds[i].training = p.family[p.selected[i]];
  }
  auto b = fresh();
  assert(b.roundNumber() == 1 && b.member(0, 0).energy == 8 && b.member(0, 1).energy == 7);
  auto before = b.member(1, 0).hp;
  assert(b.resolve(skill(41), skill(43)));
  assert(b.member(1, 0).hp < before && b.roundNumber() == 2);
  // Every content effect reaches its legal execution path with deterministic state.
  for (unsigned id = 1; id <= 80; ++id) {
    b = fresh();
    BattleFixture::equip(b, 0, id);
    auto &user = BattleFixture::member(b, 0);
    auto &ally = BattleFixture::member(b, 0, 1);
    user.stats[SPD] = 200;
    user.hp = user.stats[HP] - 10;
    ally.hp = ally.stats[HP] - 12;
    ally.status = BURN;
    ally.statusTurns = 3;
    auto &effect = move(id);
    unsigned target = (effect.target == ALLY || effect.effect == FLASH_RELAY) ? 1 : 0;
    if (effect.target == FAINT || effect.effect == MASS_REVIVE) {
      ally.hp = 0;
      target = 1;
    }
    if (id == 56) {
      assert(!b.legal(0, skill(id)));
      continue;
    }
    assert(b.legal(0, skill(id, target)));
    assert(b.resolve(skill(id, target), skill(43)));
    for (unsigned s = 0; s < 2; ++s)
      for (unsigned m = 0; m < 3; ++m) {
        auto &mon = b.member(s, m);
        assert(mon.hp <= mon.stats[HP] && mon.energy <= 12 && mon.shield <= mon.stats[HP] * 35 / 100);
      }
    if (effect.effect == REVIVE_ONE) {
      assert(b.member(0, 1).hp && b.token(0, 0));
      assert(!b.legal(0, skill(id, 1)));
    }
    if (effect.effect == MASS_REVIVE) assert(b.member(0, 1).hp && b.token(0, 1));
    if (effect.effect == ANTHEM) assert(b.token(0, 2));
    if (effect.effect == ENERGY_RESTORE) assert(b.member(0, 0).energy == 12);
    if (effect.effect == CLEAN_ONE && effect.target == ALLY) assert(!b.member(0, 1).status);
    if (effect.effect == FLASH_RELAY) assert(b.active(0) == 1);
  }
  // Cooldowns count active activations, not bench rounds; free Strike is never lost.
  b = fresh();
  BattleFixture::equip(b, 0, 3);
  assert(b.resolve(skill(3), skill(43)));
  assert(!b.legal(0, skill(3)) && b.legal(0, skill(41)));
  assert(b.resolve(skill(41), skill(43)));
  assert(b.member(0, 0).cooldown[0] == 0);
  // Silence applied before a queued support move can make it unaffordable, without underflow.
  b = fresh();
  BattleFixture::equip(b, 0, 63);
  BattleFixture::member(b, 0).stats[SPD] = 200;
  BattleFixture::member(b, 1).energy = 4;
  auto hp = b.member(1, 0).hp;
  assert(b.resolve(skill(63), skill(42)));
  assert(b.member(1, 0).energy == 6 && b.member(1, 0).hp == hp);
  // Anchor revives once and cancels the defeated creature's queued attack.
  b = fresh();
  BattleFixture::equip(b, 1, 56);
  auto &victim = BattleFixture::member(b, 1);
  victim.hp = 1;
  victim.stats[SPD] = 1;
  assert(b.resolve(skill(41), skill(41)));
  assert(b.member(1, 0).hp > 0 && (b.member(1, 0).flags & 1));
  assert(b.member(0, 0).hp == b.member(0, 0).stats[HP]);
  // Root blocks voluntary switches; forced KO replacements remain free.
  b = fresh();
  Action switching;
  switching.switching = true;
  switching.target = 1;
  BattleFixture::member(b, 0).status = ROOT;
  assert(!b.legal(0, switching));
  BattleFixture::member(b, 0).hp = 0;
  assert(b.forceSwitch(0, 1) && b.active(0) == 1 && b.roundNumber() == 1);
  // Round 60 hard cap terminates a support-only draw, without unbounded loops.
  b = fresh();
  for (unsigned side = 0; side < 2; ++side)
    for (unsigned i = 0; i < 3; ++i) {
      auto &m = BattleFixture::member(b, side, i);
      m.stats[HP] = m.hp = 1000;
    }
  for (unsigned n = 0; n < 60 && !b.result(); ++n) {
    assert(b.resolve(skill(43), skill(43)));
  }
  assert(b.result() == 3 && b.roundNumber() == 60);
  unsigned wins = 0, losses = 0, draws = 0, maxRounds = 0;
  long long maxChoiceNs = 0;
  for (unsigned seed = 1; seed <= 10000; ++seed) {
    Progress profile;
    profile.reset();
    unsigned level = 1 + seed % 99;
    for (auto f : profile.selected)
      for (unsigned n = 0; n < 3 * (level - 1); ++n)
        ++profile.family[f].rank[n % 6];
    Build first[3], second[3];
    Battle::generateCPU(profile, seed, first);
    Battle::generateCPU(profile, seed + 10000, second);
    for (unsigned i = 0; i < 3; ++i) {
      auto &t = first[i].training;
      assert(t.level() + 2 >= level && t.level() <= level + 2);
      assert(t.total() == 3 * (t.level() - 1));
      for (unsigned slot = 0; slot < 4; ++slot) {
        auto id = t.equipped[slot];
        assert(t.eligible(first[i].family, id));
        for (unsigned prior = 0; prior < slot; ++prior)
          assert(t.equipped[prior] != id);
      }
      for (unsigned j = 0; j < i; ++j)
        assert(first[i].family != first[j].family);
    }
    Battle replay;
    b.begin(first, second, seed);
    replay.begin(first, second, seed);
    for (unsigned step = 0; step < 200 && !b.result(); ++step) {
      for (unsigned side = 0; side < 2; ++side)
        if (b.needsSwitch(side)) {
          for (unsigned i = 0; i < 3; ++i)
            if (b.member(side, i).hp) {
              assert(b.forceSwitch(side, i) && replay.forceSwitch(side, i));
              break;
            }
        }
      auto start = std::chrono::steady_clock::now();
      auto a = b.chooseCPU(0, seed + step, seed % 3), c = b.chooseCPU(1, seed + step + 1, (seed + 1) % 3);
      auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start)
                    .count();
      if (ns > maxChoiceNs) maxChoiceNs = ns;
      assert(b.legal(0, a) && b.legal(1, c));
      assert(b.resolve(a, c) && replay.resolve(a, c));
      assert(b.stateHash() == replay.stateHash());
      for (unsigned side = 0; side < 2; ++side)
        for (unsigned i = 0; i < 3; ++i) {
          auto &m = b.member(side, i);
          assert(m.hp <= m.stats[HP] && m.energy <= 12 && m.shield <= m.stats[HP] * 35 / 100);
        }
    }
    assert(b.result() && b.roundNumber() <= 60);
    wins += b.result() == 1;
    losses += b.result() == 2;
    draws += b.result() == 3;
    if (b.roundNumber() > maxRounds) maxRounds = b.roundNumber();
  }
  printf("Veylith battle: all 80 execution paths, cooldowns, silence, Anchor, forced switch, cap and 10,000 "
         "deterministic CPU matches passed. W/L/D %u/%u/%u; max rounds %u; host max pair choice %lld ns; "
         "Battle %zu bytes\n",
         wins, losses, draws, maxRounds, maxChoiceNs, sizeof(Battle));
}
