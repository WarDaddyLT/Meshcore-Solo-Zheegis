"""Exhaustively verify the actual offline rules and cooperative CPU search."""
from pathlib import Path
import subprocess,sys,tempfile
root=Path(__file__).resolve().parents[2]
flags=['-DZYGIS_GAME_DIAGNOSTICS=1','-DSOLO_GAME_HOLDEM=1','-DSOLO_GAME_ARTILLERY=1','-DSOLO_GAME_BATTLESHIP=1','-DSOLO_GAME_CHECKERS=1','-DSOLO_GAME_SNAKE=1','-std=c++17','-O2','-Wall','-Wextra','-Werror']
if sys.platform=='linux':flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer']
with tempfile.TemporaryDirectory() as tmp:
    binary=Path(tmp)/'tictactoe'
    subprocess.run(['c++',*flags,'-I',str(root/'src'),str(root/'test/games/test_tictactoe.cpp'),'-o',str(binary)],check=True,timeout=60)
    subprocess.run([str(binary)],check=True,timeout=30)

    binary=Path(tmp)/'mesh'
    subprocess.run(['c++',*flags,'-I',str(root/'src'),str(root/'test/games/test_tictactoe_mesh.cpp'),'-o',str(binary)],check=True,timeout=60)
    subprocess.run([str(binary)],check=True,timeout=30)

    keys=(root/'src/helpers/ui/UIScreen.h').read_text()
    keys='\n'.join(line for line in keys.splitlines() if line.startswith('#define KEY_'))
    ui=Path(tmp)/'screen.cpp'
    ui.write_text((root/'test/games/test_games_screen.cpp').read_text().replace('// KEY_DEFINITIONS',keys).replace('"../../examples/companion_radio/ui-new/GamesScreen.h"','"'+str(root/'examples/companion_radio/ui-new/GamesScreen.h')+'"'))
    binary=Path(tmp)/'screen'
    subprocess.run(['c++',*flags,'-I',str(root/'src'),str(ui),'-o',str(binary)],check=True,timeout=60)
    subprocess.run([str(binary)],check=True,timeout=30)

    # Compile and exercise the same screen without diagnostic members/counters.
    production=Path(tmp)/'production.cpp'
    production.write_text(ui.read_text().split('static uint32_t state(')[0]+r'''
int main() {
  UITask task;GamesScreen game(&task);DisplayDriver display;
  using Open=void (GamesScreen::*)();
  Open entries[]={&GamesScreen::openTicTacToe,&GamesScreen::openSnake,
    &GamesScreen::openCheckers,&GamesScreen::openBattleship,
    &GamesScreen::openArtillery,&GamesScreen::openHoldem};
  for(auto open:entries)for(unsigned cycle=0;cycle<100;++cycle) {
    game.onShow();(game.*open)();game.render(display);
    for(int key:{KEY_DOWN,KEY_ENTER,KEY_LEFT,KEY_RIGHT,KEY_UP,KEY_ENTER,
                 KEY_CONTEXT_MENU,KEY_CANCEL,KEY_ENTER,KEY_CANCEL}) {
      assert(game.handleInput(char(key)));
      testMillis+=1000;game.poll();game.render(display);
    }
    game.onShow();
  }
  assert(task.sent.empty());
  printf("Production: six screens without diagnostic state, 128x64 rendering and 100 cycles each passed; controller %zu bytes\n",sizeof(GamesScreen));
}
''')
    binary=Path(tmp)/'production'
    production_flags=[f for f in flags if not f.startswith('-DZYGIS_GAME_DIAGNOSTICS')]
    subprocess.run(['c++',*production_flags,'-I',str(root/'src'),str(production),'-o',str(binary)],check=True,timeout=60)
    subprocess.run([str(binary)],check=True,timeout=30)
