#include <cassert>
#include <cstdio>
#include <cstring>
#include <games/veylith/Progress.h>
#include <vector>
// KEY_DEFINITIONS
static uint32_t ticks = 100;
uint32_t millis() {
  return ticks;
}
uint32_t micros() {
  static uint32_t n = 0;
  return ++n;
}
class DisplayDriver {
  int x = 0, y = 0;

public:
  enum Color { DARK, LIGHT };
  void setTextSize(int n) { assert(n == 1); }
  void setColor(Color) {}
  void drawCenteredHeader(const char *s) { assert(strlen(s) * 6 <= 128); }
  void setCursor(int a, int b) {
    x = a;
    y = b;
  }
  void print(const char *s) {
    if (x < 0 || y < 0 || x + int(strlen(s)) * 6 > 128 || y + 9 > 64) {
      fprintf(stderr, "Overflow at %d,%d: %s\n", x, y, s);
      assert(false);
    }
  }
  void fillRect(int a, int b, int w, int h) {
    assert(a >= 0 && b >= 0 && w >= 0 && h >= 0 && a + w <= 128 && b + h <= 64);
  }
  void drawRect(int a, int b, int w, int h) { fillRect(a, b, w, h); }
  void drawXbm(int a, int b, const uint8_t *bytes, int w, int h) {
    assert(bytes);
    fillRect(a, b, w, h);
  }
};
struct UITask {
  std::vector<uint8_t> saved;
  unsigned writes = 0, backups = 0;
  bool fail = false;
  int loadVeylith(uint8_t *p, size_t n) {
    if (saved.empty()) return -1;
    if (saved.size() != n) return -2;
    memcpy(p, saved.data(), n);
    return n;
  }
  bool saveVeylith(const uint8_t *p, size_t n) {
    if (fail) return false;
    saved.assign(p, p + n);
    ++writes;
    return true;
  }
  bool backupVeylith() {
    ++backups;
    return !fail;
  }
  void showAlert(const char *, int) {}
  void playGameSound(bool) {}
};
#include "../../examples/companion_radio/ui-new/VeylithScreen.h"
static unsigned state(const VeylithScreen &screen, unsigned field) {
  uint8_t data[44];
  assert(screen.diagnostics(data, sizeof(data), 0) == 44);
  auto p = data + 4 * field;
  return unsigned(p[0]) | unsigned(p[1]) << 8 | unsigned(p[2]) << 16 | unsigned(p[3]) << 24;
}
static DisplayDriver display;
static void key(VeylithScreen &s, int k) {
  assert(s.handleInput(char(k)));
  s.render(display);
}
static void row(VeylithScreen &s, unsigned n) {
  while (state(s, 7) != n)
    key(s, KEY_DOWN);
}
static void page(VeylithScreen &s, unsigned view) {
  if (state(s, 5) != view) {
    fprintf(stderr, "Expected view %u, got %u row %u\n", view, state(s, 5), state(s, 7));
    assert(false);
  }
  s.render(display);
}
int main() {
  UITask task;
  veylith::Progress profile;
  profile.reset();
  for (auto &t : profile.family) {
    t.wallet = 1000000000;
    t.lifetime = 1000000000;
  }
  task.saved.resize(veylith::SaveCodec::SIZE);
  assert(veylith::SaveCodec::encode(profile, task.saved.data(), task.saved.size()));
  {
    VeylithScreen game(&task, 1);
    page(game, 0);
    key(game, KEY_CONTEXT_MENU);
    page(game, 20);
    key(game, KEY_CANCEL);
    key(game, KEY_ENTER);
    page(game, 1);
    row(game, 1);
    key(game, KEY_ENTER);
    page(game, 0);
    row(game, 1);
    key(game, KEY_ENTER);
    page(game, 2);
    for (unsigned n = 0; n < 105; ++n) {
      key(game, KEY_ENTER);
      page(game, 3);
      key(game, KEY_RIGHT);
      key(game, KEY_ENTER);
      page(game, 21);
      key(game, KEY_ENTER);
      row(game, (n + 1) % 6);
    }
    assert(state(game, 3) == 36);
    key(game, KEY_CANCEL);
    row(game, 2);
    key(game, KEY_ENTER);
    page(game, 4);
    key(game, KEY_ENTER);
    page(game, 5);
    for (unsigned i = 0; i < 40; ++i) {
      key(game, KEY_ENTER);
      page(game, 6);
      key(game, KEY_CANCEL);
      row(game, i % 35);
    }
    key(game, KEY_CANCEL);
    key(game, KEY_CANCEL);
    page(game, 0);
    // Failed save cannot spend XP, change the live team or leave the Trainer.
    unsigned writes = task.writes;
    task.fail = true;
    row(game, 3);
    key(game, KEY_ENTER);
    page(game, 0);
    assert(task.writes == writes);
    task.fail = false;
    key(game, KEY_ENTER);
    page(game, 7);
    key(game, KEY_RIGHT);
    page(game, 13);
    key(game, KEY_ENTER);
    key(game, KEY_DOWN);
    key(game, KEY_LEFT);
    key(game, KEY_CANCEL);
    page(game, 7);
    key(game, KEY_ENTER);
    page(game, 8);
    for (unsigned n = 0; n < 5000 && state(game, 5) != 15; ++n) {
      unsigned view = state(game, 5);
      if (view == 8) {
        key(game, KEY_ENTER);
        row(game, 1);
        key(game, KEY_ENTER);
      } else if (view == 14) {
        ticks += 250;
        game.poll();
        game.render(display);
      } else if (view == 12) {
        bool done = false;
        for (unsigned i = 0; i < 3 && state(game, 5) == 12; ++i) {
          row(game, i);
          key(game, KEY_ENTER);
          done = true;
        }
        assert(done);
      } else
        assert(false);
    }
    page(game, 15);
    assert(state(game, 2));
    unsigned earnedWrites = task.writes;
    for (unsigned i = 0; i < 10; ++i)
      game.poll();
    assert(task.writes == earnedWrites);
    key(game, KEY_ENTER);
    page(game, 0);
  }
  // Inspect every form and all 940 affiliated cards, including locked gates,
  // without mutating a loadout, stat or wallet.
  for (unsigned f = 0; f < 20; ++f) {
    UITask cards;
    veylith::Progress notes;
    notes.reset();
    notes.selected[0] = f;
    notes.selected[1] = (f + 1) % 20;
    notes.selected[2] = (f + 2) % 20;
    cards.saved.resize(veylith::SaveCodec::SIZE);
    assert(veylith::SaveCodec::encode(notes, cards.saved.data(), cards.saved.size()));
    VeylithScreen card(&cards, f + 100);
    key(card, KEY_ENTER);
    row(card, f);
    key(card, KEY_RIGHT);
    page(card, 22);
    for (unsigned stage = 0; stage < 3; ++stage) {
      key(card, KEY_ENTER);
      key(card, KEY_ENTER);
      key(card, KEY_RIGHT);
    }
    key(card, KEY_CANCEL);
    key(card, KEY_CANCEL);
    row(card, 2);
    key(card, KEY_ENTER);
    key(card, KEY_ENTER);
    unsigned count = 0;
    for (unsigned id = 1; id <= 80; ++id)
      count += notes.family[f].eligible(f, id, false);
    for (unsigned i = 0; i < count; ++i) {
      row(card, i);
      key(card, KEY_ENTER);
      page(card, 6);
      for (unsigned pageIndex = 0; pageIndex < 4; ++pageIndex)
        key(card, KEY_RIGHT);
      key(card, KEY_CANCEL);
    }
    assert(cards.writes == 0);
  }
  // Random navigation keeps every rendered text/bitmap within the OLED bounds.
  for (unsigned seed = 1; seed <= 40; ++seed) {
    VeylithScreen game(&task, seed);
    unsigned rng = seed;
    for (unsigned n = 0; n < 2500 && !game.wantsExit(); ++n) {
      rng = rng * 1664525 + 1013904223;
      ticks += 250;
      game.poll();
      game.render(display);
      const int keys[] = { KEY_UP,    KEY_DOWN,  KEY_LEFT,   KEY_RIGHT,
                           KEY_ENTER, KEY_ENTER, KEY_CANCEL, KEY_CONTEXT_MENU };
      key(game, keys[rng % 8]);
    }
  }
  // Corrupt saves require an explicit yes, and are backed up before reset.
  task.saved[8] ^= 1;
  VeylithScreen bad(&task, 2);
  page(bad, 17);
  key(bad, KEY_ENTER);
  page(bad, 18);
  key(bad, KEY_ENTER);
  page(bad, 17);
  assert(task.backups == 0);
  key(bad, KEY_ENTER);
  key(bad, KEY_RIGHT);
  key(bad, KEY_ENTER);
  page(bad, 0);
  assert(task.backups == 1);
  printf("Arena UI: team, 105 stat purchases/evolutions, move library, CPU match/reward, save "
         "failure/recovery and bounded 128x64 rendering passed; screen %zu bytes\n",
         sizeof(VeylithScreen));
}
