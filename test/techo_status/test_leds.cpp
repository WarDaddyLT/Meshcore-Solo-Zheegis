#include "../../src/helpers/ui/LedBlinkBurst.h"
#include <cassert>
#include <cstdint>

int main() {
  LedBlinkBurst led;
  assert(!led.isOn(0));
  led.trigger(100);
  assert(led.isOn(100));
  assert(led.isOn(349));
  assert(!led.isOn(350));
  assert(led.isOn(600));
  led.trigger(1000); // A second event must not extend the original burst.
  assert(led.isOn(4600));
  assert(!led.isOn(5099));
  assert(!led.isOn(5100));
  assert(!led.isOn(6000));
  led.trigger(UINT32_MAX - 100);
  assert(led.isOn(UINT32_MAX - 100));
  assert(!led.isOn(149)); // 250ms across millis() wrap
  assert(!led.isOn(4899)); // 5000ms across wrap
  led.trigger(5000);
  assert(led.isOn(5000));
}
