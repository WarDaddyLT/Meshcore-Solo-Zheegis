"""Compile multiplayer state machines against the actual vetted crypto library."""
from pathlib import Path
import os,subprocess,sys,tempfile
root=Path(__file__).resolve().parents[2]
crypto=root/'.pio/libdeps/WioTrackerL1_zygis_multiplayer/Crypto'
if not crypto.is_dir():
 candidates=list((root/'.pio/libdeps').glob('*/Crypto'))
 if candidates:crypto=candidates[0]
assert crypto.is_dir(),'Install the firmware dependencies with pio pkg install -e WioTrackerL1_zygis_multiplayer'
modules=['Crypto','ChaCha','ChaChaPoly','Poly1305','AuthenticatedCipher','Cipher','HKDF','Hash','SHA256']
flags=['-std=c++17','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-Wno-unused-parameter']
if sys.platform=='linux':flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer']
with tempfile.TemporaryDirectory() as tmp:
 for test in sorted((root/'test/multiplayer').glob('test_*.cpp')):
  binary=Path(tmp)/test.stem
  extra=[root/'src/games/veylith/Content.cpp',root/'src/games/veylith/Battle.cpp'] if test.stem in ('test_arena','test_arena_screen','test_checkers_online') else []
  if test.stem in ('test_arena_screen', 'test_checkers_online'):
   extra+=[root/'src/games/veylith/Assets.cpp']
   keys='\n'.join(line for line in (root/'src/helpers/ui/UIScreen.h').read_text().splitlines() if line.startswith('#define KEY_'))
   generated=Path(tmp)/'arena_screen.cpp'
   generated.write_text(test.read_text().replace('// KEY_DEFINITIONS',keys).replace('../../examples/',str(root/'examples')+'/'))
   test=generated
  subprocess.run(['c++',*flags,'-I',str(root/'src'),'-I',str(crypto),'-I',str(root/'test/multiplayer'),str(test),*map(str,extra),*[str(crypto/(n+'.cpp')) for n in modules],'-o',str(binary)],check=True,timeout=90)
  subprocess.run([str(binary)],check=True,timeout=180)
