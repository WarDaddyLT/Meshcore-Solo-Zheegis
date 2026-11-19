#include <cassert>
#include <cstdio>
#include <games/veylith/Assets.h>
#include <games/veylith/Bitmap.h>
using namespace veylith;
int main() {
  static_assert(sizeof(creatureArt) == 60 * 128, "Store one bitmap per form");
  static_assert(sizeof(creatureArt) + sizeof(miniArt) + sizeof(trainerArt) + sizeof(iconArt) +
                        sizeof(progressArt) ==
                    10280,
                "Front-only art budget");
  for (unsigned f = 0; f < 20; ++f)
    for (unsigned stage = 0; stage < 3; ++stage) {
      auto bits = sprite(f, stage);
      assert(bits == creatureArt + (f * 3 + stage) * 128);
      bool pixels[32][32] = {};
      unsigned runs = 0;
      mirrorRuns(bits, 32, 32, [&](unsigned x, unsigned y, unsigned n) {
        assert(n && x + n <= 32 && y < 32);
        ++runs;
        for (unsigned i = 0; i < n; ++i) {
          assert(!pixels[y][x + i]);
          pixels[y][x + i] = true;
        }
      });
      assert(runs);
      for (unsigned y = 0; y < 32; ++y)
        for (unsigned x = 0; x < 32; ++x) {
          unsigned originalX = 31 - x;
          assert(pixels[y][x] == bool(bits[y * 4 + originalX / 8] & (0x80U >> (originalX % 8))));
        }
    }
  // Non-byte-aligned widths and a run touching either edge.
  const uint8_t fiveWide[] = { 0x90 };
  unsigned mask = 0;
  mirrorRuns(fiveWide, 5, 1, [&](unsigned x, unsigned y, unsigned n) {
    assert(!y);
    for (unsigned i = 0; i < n; ++i)
      mask |= 1U << (x + i);
  });
  assert(mask == 0x12);
  puts("Assets: all 60 runtime mirrors are pixel-exact; 10,280 flash bytes, zero flip scratch.");
}
