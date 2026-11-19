#include <cassert>
#include <cstdio>
#include <cstring>
#include <games/veylith/Progress.h>
using namespace veylith;
static bool fitsFourLines(const char *p) {
  for (unsigned line = 0; line < 4 && *p; ++line) {
    unsigned n = strlen(p);
    if (n > 21) {
      n = 21;
      while (n > 8 && p[n] != ' ')
        --n;
    }
    p += n;
    while (*p == ' ')
      ++p;
  }
  return !*p;
}
int main() {
  Progress profile;
  profile.reset();
  unsigned checked = 0;
  bool order[81] = {};
  unsigned priorRole = 0, priorLevel = 0;
  for (auto id : moveOrder) {
    assert(id >= 1 && id <= 80 && !order[id]);
    order[id] = true;
    auto &m = move(id);
    assert(m.role >= priorRole);
    if (m.role == priorRole) assert(m.level >= priorLevel);
    priorRole = m.role;
    priorLevel = m.level;
  }
  for (unsigned f = 0; f < 20; ++f) {
    auto start = profile.family[f];
    unsigned available = 0;
    for (unsigned id = 1; id <= 80; ++id)
      available += start.eligible(f, id);
    assert(available == 6);
    for (unsigned stage = 0; stage < 3; ++stage) {
      auto desc = families[f].descriptions[stage];
      assert(desc && fitsFourLines(desc));
    }
    for (unsigned id = 1; id <= 80; ++id) {
      auto &m = move(id);
      assert(fitsFourLines(m.description) && fitsFourLines(m.effectText));
      if (!start.eligible(f, id, false)) continue;
      Training t = start;
      unsigned level = m.level, stage = level >= 35 ? 2 : level >= 15 ? 1 : 0;
      const unsigned bonus[3][6] = { { 0, 0, 0, 0, 0, 0 }, { 14, 4, 4, 4, 4, 3 }, { 30, 9, 9, 9, 9, 7 } };
      unsigned gated = 0;
      for (unsigned gate = 0; gate < 2; ++gate)
        if (m.requiredValue[gate]) {
          unsigned stat = m.requiredStat[gate], base = families[f].base[stat] + bonus[stage][stat];
          unsigned need = m.requiredValue[gate] > base ? m.requiredValue[gate] - base : 0;
          t.rank[stat] = (need + (stat == HP ? 2 : 0)) / (stat == HP ? 3 : 1);
          assert(t.rank[stat] <= 80);
          gated |= 1U << stat;
        }
      unsigned budget = 3 * (level - 1);
      assert(t.total() <= budget);
      while (t.total() < budget) {
        unsigned stat = 0;
        while (stat < 6 && ((gated & (1U << stat)) || t.rank[stat] >= 80))
          ++stat;
        assert(stat < 6);
        ++t.rank[stat];
      }
      assert(t.level() == level && t.eligible(f, id));
      ++checked;
      for (unsigned gate = 0; gate < 2; ++gate)
        if (m.requiredValue[gate] && t.rank[m.requiredStat[gate]]) {
          auto below = t;
          --below.rank[m.requiredStat[gate]];
          unsigned other = 0;
          while (other < 6 && ((gated & (1U << other)) || below.rank[other] >= 80))
            ++other;
          assert(other < 6);
          ++below.rank[other];
          assert(below.level() == t.level() && !below.eligible(f, id));
        }
    }
  }
  // Same level, different training => different move pools. Neither level nor
  // raw offensive stats can bypass the other gate or the element restriction.
  auto hpOnly = profile.family[0];
  hpOnly.rank[HP] = 9;
  assert(hpOnly.level() == 4 && !hpOnly.eligible(0, 2));
  auto fighter = hpOnly;
  fighter.rank[HP] = 7;
  fighter.rank[ATK] = 2;
  assert(fighter.level() == 4 && fighter.eligible(0, 2));
  auto tooYoung = profile.family[0];
  tooYoung.rank[ATK] = 3;
  assert(tooYoung.value(0, ATK) >= 16 && tooYoung.level() == 2 && !tooYoung.eligible(0, 2));
  assert(!fighter.eligible(0, 18));
  assert(fighter.equip(0, 2, 2) && !fighter.equip(0, 3, 2) && !fighter.equip(0, 4, 41));
  assert(sizeof(fighter.equipped) == 4);
  printf("Move requirements: %u affiliated family/move minimum-level builds feasible; exact stat boundaries, "
         "AND gates, six starting choices, four distinct slots and all descriptions fit OLED cards.\n",
         checked);
}
