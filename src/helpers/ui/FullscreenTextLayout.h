#pragma once
#include <stddef.h>
#include <string.h>

namespace fullscreen {
inline bool nextLine(const char*& text, size_t columns, char* line) {
  if (!*text || !columns) return false;
  size_t count = 0;
  while (text[count] && text[count] != '\n' && count < columns) ++count;
  if (count == columns && text[count] && text[count] != '\n' && text[count] != ' ') {
    size_t space = count;
    while (space && text[space - 1] != ' ') --space;
    if (space) count = space - 1;
  }
  memcpy(line, text, count);
  line[count] = 0;
  text += count;
  if (*text == '\n') ++text;
  else while (*text == ' ') ++text;
  return true;
}
inline size_t lineCount(const char* text, size_t columns) {
  size_t lines = 0;
  char line[161];
  while (nextLine(text, columns, line)) ++lines;
  return lines;
}
inline unsigned fontSize(const char* text, int width, int height) {
  for (unsigned size = 8; size > 1; --size) {
    int columns = width / (6 * size);
    if (columns > 0 && lineCount(text, columns) * 8 * size <= unsigned(height)) return size;
  }
  return 1;
}
}
