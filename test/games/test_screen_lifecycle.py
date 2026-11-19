"""Exercise the actual deferred allocation/collection code under screen churn."""
from pathlib import Path
import re
import subprocess
import tempfile

root=Path(__file__).resolve().parents[2]
source=(root/'examples/companion_radio/ui-new/UITask.cpp').read_text()

def function(signature):
    start=source.index(signature);brace=source.index('{',start);end=brace+1;depth=1
    while depth:
        depth+=(source[end]=='{')-(source[end]=='}');end+=1
    return source[start:end]

factory=function('bool UITask::ensureScreen(')
collector=function('void UITask::collectInactiveScreens(')
types=sorted(set(re.findall(r'new \(std::nothrow\) (\w+)',factory))|{'SplashScreen','HomeScreen'})
slots=sorted(set(re.findall(r'&screen == &(\w+)',factory))|{'home','splash'})
harness=r'''
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <new>
#include <cstdio>
#define SOLO_UI_ON_DEMAND 1
#define SOLO_GAMES_ENABLED 1
#define SOLO_GAME_TICTACTOE 1
#define PIN_GPIO1 1
bool failNext=false;
unsigned live=0,destroyed=0;
struct UIScreen {};
template<unsigned ID> struct FakeScreen:UIScreen {
  template<class... Args> FakeScreen(Args...){++live;}
  ~FakeScreen(){assert(live);--live;++destroyed;}
  static void* operator new(size_t size,const std::nothrow_t&) noexcept {
    if(failNext){failNext=false;return nullptr;}return malloc(size);
  }
  static void operator delete(void* p) noexcept {free(p);}
};
TYPES
struct UITask {
  SLOTS
  UIScreen* curr=nullptr;
  bool _companion_mode=false,connected=false;
  int _node_prefs=0,_kb=0,_trail=0,_room_unread=0;
  uint8_t _dm_unread_table[16]={};
  unsigned alerts=0;
  bool isBLEConnected(){return connected;}
  void gotoHomeScreen(){curr=home;}
  void showAlert(const char*,int){++alerts;}
  bool ensureScreen(UIScreen*&);
  void collectInactiveScreens();
};
FACTORY
COLLECTOR
int main() {
  UITask task;
  // The home/status shell is resident; every optional screen starts absent.
  HomeScreen home(0);task.home=&home;task.curr=&home;
  UIScreen** slots[]={SLOT_POINTERS};
  for(unsigned round=0;round<100;++round)for(auto slot:slots) {
    assert(task.ensureScreen(*slot));task.curr=*slot;
    task.collectInactiveScreens();assert(*slot && live==2);
    task.curr=&home;
    assert(*slot); // Navigation must not delete a callback's object mid-call.
    task.collectInactiveScreens();assert(!*slot && live==1);
  }
  failNext=true;assert(!task.ensureScreen(task.messages_screen));
  assert(task.alerts==1 && !task.messages_screen && live==1);
  task.connected=true;task.collectInactiveScreens();
  assert(!task.messages_screen); // BLE alone does not allocate local chat UI.
  task.connected=false;task.collectInactiveScreens();
  assert(task.ensureScreen(task.games_screen));task.curr=task.games_screen;
  task.connected=true;task.collectInactiveScreens();
  assert(task.curr==task.home && !task.games_screen && live==1);
  printf("%u deferred screen destructions, 100 full screen cycles, allocation failure and BLE priority passed\n",destroyed);
}
'''
for marker,value in {
    'TYPES':'\n'.join(f'using {name}=FakeScreen<{i}>;' for i,name in enumerate(types)),
    'SLOTS':'\n'.join(f'UIScreen* {slot}=nullptr;' for slot in slots),
    'FACTORY':factory,'COLLECTOR':collector,
    'SLOT_POINTERS':','.join('&task.'+slot for slot in slots if slot not in {'home','splash'}),
}.items():harness=harness.replace(marker,value)
with tempfile.TemporaryDirectory() as tmp:
    cpp,binary=Path(tmp)/'lifecycle.cpp',Path(tmp)/'lifecycle'
    cpp.write_text(harness)
    subprocess.run(['c++','-std=c++17','-Wall','-Wextra','-Werror',str(cpp),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
