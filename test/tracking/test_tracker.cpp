#include "../../examples/companion_radio/TrackerProtocol.h"
#include <cassert>
#include <cstring>
using tracker::Action;
int main() {
  auto parse = [](const char* text) { return tracker::parse(text, "Tracker01"); };
  assert(parse("!kur").action == Action::Locate);
  assert(parse("@[Tracker01] !kur").action == Action::Locate);
  assert(parse("@[Other] !kur").action == Action::Ignore);
  assert(parse("@[Tracker01] !kurious").action == Action::Unknown);
  assert(parse("@[Tracker01] !kur Other").action == Action::Unknown);
  assert(parse("@[Tracker01] !unknown").action == Action::Unknown);
  assert(parse("@[Tracker01] Hello").action == Action::Conversation);
  assert(parse("Hello").action == Action::Ignore);
  assert(parse("@[Other] Hello").action == Action::Ignore);
  assert(parse("@[Tracker01] !tikra Come home").action == Action::Alarm);
  assert(!strcmp(parse("@[Tracker01] !tikra Come home").text,"Come home"));
  assert(parse("!tikra").action == Action::Alarm);
  assert(parse("@[Tracker01] !power down").action == Action::PowerDown);
  assert(parse("@[Tracker01] !power up").action == Action::PowerUp);
  assert(parse("!power").action == Action::PowerState);
  assert(parse("!power downwards").action == Action::Unknown);
  assert(parse("@[Tracker01] !help").action == Action::Help);
  assert(parse("@[Other] !help").action == Action::Ignore);
  assert(parse("@[Tracker01").action == Action::Ignore);
  assert(parse("@[Tracker01]oops !kur").action == Action::Ignore);
  assert(parse("@[Tracker01] ").action == Action::Ignore);
  assert(parse("@[Tracker01]   !kur").action == Action::Locate);
  assert(parse("@[Tracker01] !interval 300").action == Action::Interval);
  assert(parse("@[Other] !interval 300").action == Action::Ignore);
  assert(parse("!interval").action == Action::Interval);
  assert(parse("!triangulate").action == Action::Triangulate);
  assert(parse("!traingulate").action == Action::Triangulate);
  assert(parse("@[Other] !triangulate").action == Action::Ignore);
  assert(strlen(tracker::HELP) <= 100);
  assert(strlen(tracker::HELP) + 31 + 2 <= 133);
}
