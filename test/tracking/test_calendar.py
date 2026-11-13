"""Verify calendar dates and the portrait grid's geometry using its real helpers."""
from pathlib import Path
import subprocess
import tempfile
root = Path(__file__).resolve().parents[2]
harness = r'''
#include <cassert>
#include <ctime>
#include <cstring>
#include <string>
#include <vector>
#include "ui-new/CalendarView.h"
struct Display {
  enum Color { LIGHT, DARK };
  std::vector<std::string> labels;
  int getLineHeight() const { return 9; }
  int getTextWidth(const char* s) const { return int(strlen(s)) * 6; }
  void setColor(Color) {}
  void fillRect(int x, int y, int w, int h) {
    assert(x >= 0 && y >= 0 && w > 0 && h > 0 && x + w <= 122 && y + h <= 250);
  }
  void drawTextCentered(int x, int y, const char* text) {
    assert(x - getTextWidth(text) / 2 >= 0 && x + getTextWidth(text) / 2 <= 122);
    assert(y >= 0 && y + getLineHeight() <= 250);
    labels.emplace_back(text);
  }
};
int main() {
  assert(calmath::daysIn(124, 1) == 29);
  assert(calmath::daysIn(0, 1) == 28);
  assert(calmath::daysIn(100, 1) == 29);
  assert(calmath::daysIn(200, 1) == 28);
  for (int year = 120; year <= 132; ++year) {
    for (int month = 0; month < 12; ++month) {
      std::tm t = {}; t.tm_year = year; t.tm_mon = month; t.tm_mday = 1;
      time_t epoch = timegm(&t); t = *gmtime(&epoch);
      Display d;
      assert(calmath::firstCol(t) == (t.tm_wday + 6) % 7);
      int height = calendar::height(d, t);
      assert(128 + height + 2 <= 250);
      calendar::draw(d, 0, 128, 122, t);
      assert(d.labels.size() == size_t(7 + calmath::daysIn(year, month)));
    }
  }
  std::tm today = {}; today.tm_year = 126; today.tm_mon = 10;
  today.tm_mday = 13; today.tm_wday = 5; today.tm_hour = 7;
  assert(calmath::firstCol(today) == 6 && calmath::weeks(today) == 6);
  assert(calmath::alarmDays(today, false, 0, 8, 0) == 0);
  assert(calmath::alarmDays(today, true, 0, 8, 0) == (1UL << 12));
  today.tm_hour = 9;
  assert(calmath::alarmDays(today, true, 0, 8, 0) == (1UL << 13));
}
'''
with tempfile.TemporaryDirectory() as tmp:
    cpp, binary = Path(tmp) / 'calendar.cpp', Path(tmp) / 'calendar'
    cpp.write_text(harness)
    subprocess.run(['c++', '-std=c++17', '-Wall', '-Wextra', '-Werror', '-I', str(root/'examples/companion_radio'), str(cpp), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
print('Calendar leap years, Monday-first grid, alarm markers and portrait layout bounds passed')
