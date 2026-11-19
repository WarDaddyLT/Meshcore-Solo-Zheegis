"""Native checks for the GDD's deterministic game foundations."""
from pathlib import Path
import subprocess,sys,tempfile
root=Path(__file__).resolve().parents[2]
flags=['-std=c++11','-DZYGIS_GAME_DIAGNOSTICS=1','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation']
if sys.platform=='linux':flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer']
with tempfile.TemporaryDirectory() as tmp:
 for test in sorted((root/'test/veylith').glob('test_*.cpp')):
  binary=Path(tmp)/test.stem
  sources=[root/'src/games/veylith/Content.cpp']
  if test.stem in ('test_battle','test_screen'):sources+=[root/'src/games/veylith/Battle.cpp']
  if test.stem in ('test_assets','test_screen'):
   sources+=[root/'src/games/veylith/Assets.cpp']
  if test.stem=='test_screen':
   keys='\n'.join(line for line in (root/'src/helpers/ui/UIScreen.h').read_text().splitlines() if line.startswith('#define KEY_'))
   generated=Path(tmp)/'screen.cpp'
   generated.write_text(test.read_text().replace('// KEY_DEFINITIONS',keys).replace('../../examples/',str(root/'examples')+'/'))
   test=generated
  subprocess.run(['c++',*flags,'-I',str(root/'src'),str(test),*map(str,sources),'-o',str(binary)],check=True,timeout=60)
  subprocess.run([str(binary)],check=True,timeout=120)
