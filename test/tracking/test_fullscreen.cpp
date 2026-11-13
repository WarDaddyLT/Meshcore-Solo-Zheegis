#include "../../src/helpers/ui/FullscreenTextLayout.h"
#include <cassert>
#include <initializer_list>
#include <cstring>

int main() {
  const char* short_text = "Come home";
  for (auto dimensions : {0,1}) {
    int width = dimensions ? 196 : 118;
    int height = dimensions ? 184 : 234;
    unsigned size = fullscreen::fontSize(short_text, width, height);
    assert(size > 1);
    size_t lines = fullscreen::lineCount(short_text, width / (6 * size));
    assert(lines * 8 * size <= unsigned(height));
    if (size < 8) {
      unsigned larger=size+1;
      int cols=width/(6*larger);
      assert(cols==0 || fullscreen::lineCount(short_text,cols)*8*larger>unsigned(height));
    }
    char long_text[161]; memset(long_text,'A',160); long_text[160]=0;
    size = fullscreen::fontSize(long_text,width,height);
    assert(fullscreen::lineCount(long_text,width/(6*size))*8*size<=unsigned(height));
  }
  const char* text="one two\nthree";
  char line[161];
  assert(fullscreen::nextLine(text,5,line) && !strcmp(line,"one"));
  assert(fullscreen::nextLine(text,5,line) && !strcmp(line,"two"));
  assert(fullscreen::nextLine(text,5,line) && !strcmp(line,"three"));
  assert(!fullscreen::nextLine(text,5,line));
}
