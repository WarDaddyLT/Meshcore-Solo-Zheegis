#include "../../examples/companion_radio/TrackerProtocol.h"
#include <cassert>
#include <cstring>
using tracker::Action;
int main() {
  auto parse = [](const char* text) { return tracker::parse(text, "Daubis"); };
  assert(parse("!kur").action == Action::Locate);
  assert(parse("@[Daubis] !kur").action == Action::Locate);
  assert(parse("@[Other] !kur").action == Action::Ignore);
  assert(parse("@[Daubis] !kurious").action == Action::Unknown);
  assert(parse("@[Daubis] !kur Other").action == Action::Unknown);
  assert(parse("@[Daubis] !unknown").action == Action::Unknown);
  assert(parse("@[Daubis] Hello").action == Action::Conversation);
  assert(parse("Hello").action == Action::Ignore);
  assert(parse("@[Other] Hello").action == Action::Ignore);
  assert(parse("@[Daubis] !tikra Come home").action == Action::Alarm);
  assert(!strcmp(parse("@[Daubis] !tikra Come home").text,"Come home"));
  assert(parse("!tikra").action == Action::Alarm);
  assert(parse("@[Daubis] !power down").action == Action::PowerDown);
  assert(parse("@[Daubis] !power up").action == Action::PowerUp);
  assert(parse("!power").action == Action::PowerState);
  assert(parse("!power downwards").action == Action::Unknown);
  assert(parse("@[Daubis] !help").action == Action::Help);
  assert(parse("@[Other] !help").action == Action::Ignore);
  assert(parse("@[Daubis").action == Action::Ignore);
  assert(parse("@[Daubis]oops !kur").action == Action::Ignore);
  assert(parse("@[Daubis] ").action == Action::Ignore);
  assert(parse("@[Daubis]   !kur").action == Action::Locate);
  assert(parse("@[Daubis] !interval 300").action == Action::Interval);
  assert(parse("@[Other] !interval 300").action == Action::Ignore);
  assert(parse("!interval").action == Action::Interval);
  assert(parse("!triangulate").action == Action::Triangulate);
  assert(parse("!traingulate").action == Action::Triangulate);
  assert(parse("@[Other] !triangulate").action == Action::Ignore);
  assert(strlen(tracker::HELP) <= 100);
  assert(strlen(tracker::HELP) + 31 + 2 <= 133);
}
