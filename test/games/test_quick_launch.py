"""Exercise the real Tools menu and favourite methods without hardware."""
from pathlib import Path
import re
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
ui = root / 'examples/companion_radio/ui-new'

def function(source, signature):
    start = source.index(signature)
    brace = source.index('{', start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]

prefs = (root/'examples/companion_radio/NodePrefs.h').read_text()
constants = '\n'.join(re.findall(r'  static const uint8_t FAV[^;]+;', prefs))
header = (ui/'UITask.h').read_text()
implementation = (ui/'UITask.cpp').read_text()
inline = '\n'.join(function(header, signature) for signature in (
    '  int findFavouriteSlot(', '  int findFavouriteChannelSlot(',
    '  bool isFavouriteSlotEmpty(', '  void setFavouriteSlot(',
    '  void setFavouriteChannelSlot(', '  void clearFavouriteSlot('))
methods = '\n'.join(function(implementation, signature) for signature in (
    'const char* UITask::favouriteGameLabel(', 'bool UITask::setFavouriteGameSlot(',
    'bool UITask::openFavouriteGame('))
accordion = (ui/'AccordionList.h').read_text().replace('#pragma once', '').replace('#include "icons.h"', '')
tools = (ui/'ToolsScreen.h').read_text().replace('#pragma once', '').replace('#include "AccordionList.h"', '')
icons = sorted(set(re.findall(r'&?(ICON_\w+)', tools)))
navigators = sorted(set(re.findall(r'_task->(\w+)\(\)', tools)) - {'gotoGamesScreen'})
keys = '\n'.join(line for line in (root/'src/helpers/ui/UIScreen.h').read_text().splitlines() if line.startswith('#define KEY_'))
harness = r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#define SOLO_GAMES_ENABLED 1
#define SOLO_GAME_TICTACTOE 1
#define SOLO_GAME_SNAKE 1
#define SOLO_GAME_CHECKERS 1
#define SOLO_GAME_BATTLESHIP 1
#define SOLO_GAME_HOLDEM 1
#define SOLO_GAME_ARTILLERY 1
#define PIN_GPIO1 1
KEYS
struct NodePrefs { CONSTANTS
  uint8_t favourite_contacts[FAVOURITES_COUNT][FAVOURITE_PREFIX_LEN] = {};
  uint8_t favourite_kinds[FAVOURITES_COUNT] = {};
};
struct Mesh { unsigned saves=0; void savePrefs(){++saves;} } the_mesh;
struct UITask {
  NodePrefs* _node_prefs;
  unsigned game=99, alerts=0;
  INLINE
  static const char* favouriteGameLabel(uint8_t);
  bool setFavouriteGameSlot(int,uint8_t);
  bool openFavouriteGame(uint8_t);
  void gotoGamesScreen(uint8_t id){game=id;}
  void showAlert(const char*,int){++alerts;}
  NAVIGATORS
};
METHODS
struct DisplayDriver {
  enum { LIGHT };
  std::vector<std::string> labels;
  int y=0;
  void setTextSize(int){} void setColor(int){}
  void drawCenteredHeader(const char*){}
  int getCharWidth(){return 6;}
  void setCursor(int,int yy){y=yy;assert(y>=0 && y+9<=64);}
  void print(const char* text){labels.emplace_back(text);}
};
struct MiniIcon {};
ICONS
int miniIconScale(DisplayDriver&){return 1;}
void miniIconDraw(DisplayDriver&,int,int,const MiniIcon&){}
void drawRowSelection(DisplayDriver&,int,bool,int){}
template<class F> void drawList(DisplayDriver&,int count,int& sel,int& scroll,F paint) {
  // Four rows are visible on the 128x64 device; scroll to reveal the selection.
  if(sel<scroll)scroll=sel;
  if(sel>=scroll+4)scroll=sel-3;
  for(int i=scroll;i<count && i<scroll+4;++i)paint(i,13+(i-scroll)*12,i==sel,0);
}
struct UIScreen {
  virtual void onShow(){} virtual int render(DisplayDriver&)=0;
  virtual bool handleInput(char)=0;
};
ACCORDION
TOOLS
int main() {
  NodePrefs prefs;UITask task{&prefs};
  uint8_t contact[6]={1,0,0,0,0,0}; // Same bytes as game 1; kind must distinguish them.
  task.setFavouriteSlot(0,contact);task.setFavouriteChannelSlot(1,0);
  assert(task.setFavouriteGameSlot(2,NodePrefs::FAV_GAME_MENU));
  assert(!task.isFavouriteSlotEmpty(2)); // Menu ID zero is a filled slot.
  assert(task.setFavouriteGameSlot(3,NodePrefs::FAV_GAME_TICTACTOE));
  assert(task.findFavouriteSlot(contact)==0 && task.findFavouriteChannelSlot(0)==1);
  assert(task.setFavouriteGameSlot(4,NodePrefs::FAV_GAME_TICTACTOE));
  assert(task.isFavouriteSlotEmpty(3) && !task.isFavouriteSlotEmpty(4));
  assert(!memcmp(prefs.favourite_contacts[0],contact,6));
  assert(prefs.favourite_kinds[1]==NodePrefs::FAV_KIND_CHANNEL);
  assert(task.openFavouriteGame(prefs.favourite_contacts[4][0]) && task.game==1);
  assert(task.openFavouriteGame(0) && task.game==0);
  assert(task.setFavouriteGameSlot(3,NodePrefs::FAV_GAME_TICTACTOE_MESH));
  assert(task.openFavouriteGame(2) && task.game==2);
  assert(task.setFavouriteGameSlot(5,NodePrefs::FAV_GAME_SNAKE));
  assert(task.openFavouriteGame(3) && task.game==3);
  assert(task.setFavouriteGameSlot(5,NodePrefs::FAV_GAME_CHECKERS));
  assert(task.openFavouriteGame(4) && task.game==4);
  assert(task.setFavouriteGameSlot(5,NodePrefs::FAV_GAME_BATTLESHIP));
  assert(task.openFavouriteGame(5) && task.game==5);
  assert(task.setFavouriteGameSlot(5,NodePrefs::FAV_GAME_ARTILLERY));
  assert(task.openFavouriteGame(6) && task.game==6);
  assert(task.setFavouriteGameSlot(5,NodePrefs::FAV_GAME_HOLDEM));
  assert(task.openFavouriteGame(7) && task.game==7);
  assert(!task.openFavouriteGame(255));
  unsigned saves=the_mesh.saves;
  assert(!task.setFavouriteGameSlot(-1,1) && !task.setFavouriteGameSlot(6,1));
  assert(!task.setFavouriteGameSlot(0,255) && the_mesh.saves==saves);
  // Unsupported game IDs stay occupied, preventing contact/channel aliasing.
  prefs.favourite_kinds[5]=NodePrefs::FAV_KIND_GAME;
  prefs.favourite_contacts[5][0]=255;
  assert(!task.isFavouriteSlotEmpty(5));
  NodePrefs reloaded=prefs;task._node_prefs=&reloaded;
  assert(!task.isFavouriteSlotEmpty(2) && !task.isFavouriteSlotEmpty(4));
  task.clearFavouriteSlot(4);assert(task.isFavouriteSlotEmpty(4));
  ToolsScreen screen(&task);screen.onShow();DisplayDriver display;
  screen.render(display);
  bool visible=false;for(auto& label:display.labels)if(label=="Games")visible=true;
  assert(visible); // Games is visible among the folded top-level categories.
  for(int i=0;i<3;++i)screen.handleInput(KEY_DOWN);
  screen.handleInput(KEY_ENTER);screen.handleInput(KEY_DOWN);
  screen.render(display);screen.handleInput(KEY_ENTER);assert(task.game==1);
  screen.handleInput(KEY_DOWN);screen.handleInput(KEY_ENTER);assert(task.game==2);
  screen.handleInput(KEY_DOWN);screen.handleInput(KEY_ENTER);assert(task.game==3);
  screen.handleInput(KEY_DOWN);screen.handleInput(KEY_ENTER);assert(task.game==4);
  screen.handleInput(KEY_DOWN);screen.handleInput(KEY_ENTER);assert(task.game==5);
  screen.handleInput(KEY_DOWN);screen.handleInput(KEY_ENTER);assert(task.game==6);
  screen.handleInput(KEY_DOWN);screen.handleInput(KEY_ENTER);assert(task.game==7);
  puts("Visible Games category, all six game launches, typed pins, duplicate moves and slot safety passed");
}
'''
for marker, value in {
    'KEYS': keys, 'CONSTANTS': constants, 'INLINE': inline, 'METHODS': methods,
    'NAVIGATORS': '\n'.join('void '+name+'(){}' for name in navigators),
    'ICONS': '\n'.join('const MiniIcon '+name+'{};' for name in icons),
    'ACCORDION': accordion, 'TOOLS': tools,
}.items():
    harness = harness.replace(marker, value)
with tempfile.TemporaryDirectory() as tmp:
    source, binary = Path(tmp)/'quick_launch.cpp', Path(tmp)/'quick_launch'
    source.write_text(harness)
    subprocess.run(['c++', '-std=c++17', '-funsigned-char', '-Wall', '-Wextra', '-Werror', str(source), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
