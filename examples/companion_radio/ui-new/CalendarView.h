#pragma once
// Month-grid layout adapted from MeshCore Solo's calendar view.
#include <cstdio>
#include "../CalendarMath.h"

namespace calendar {
template<class Display>
int height(Display& d, const struct tm& t) {
  return (1 + calmath::weeks(t)) * (d.getLineHeight() + 2) + 3;
}

template<class Display>
void draw(Display& d, int x, int y, int width, const struct tm& t, uint32_t marked = 0) {
  static const char* const days[] = {"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"};
  const int lh = d.getLineHeight(), step = lh + 2, cell = width / 7;
  const int left = x + (width - cell * 7) / 2;
  d.setColor(Display::LIGHT);
  for (int col = 0; col < 7; ++col) d.drawTextCentered(left + col * cell + cell / 2, y, days[col]);
  d.fillRect(left, y + lh + 1, cell * 7, 1);
  const int top = y + step + 3;
  for (int day = 1; day <= calmath::daysIn(t.tm_year, t.tm_mon); ++day) {
    const int index = calmath::firstCol(t) + day - 1;
    const int cx = left + (index % 7) * cell, cy = top + (index / 7) * step;
    char number[3];
    snprintf(number, sizeof(number), "%d", day);
    const bool today = day == t.tm_mday;
    if (today) {
      d.fillRect(cx + 1, cy - 1, cell - 2, lh + 2);
      d.setColor(Display::DARK);
    }
    d.drawTextCentered(cx + cell / 2, cy, number);
    if (marked & (1UL << (day - 1))) {
      const int underline = d.getTextWidth(number);
      d.fillRect(cx + (cell - underline) / 2, cy + lh, underline, 1);
    }
    d.setColor(Display::LIGHT);
  }
}
} // namespace calendar
