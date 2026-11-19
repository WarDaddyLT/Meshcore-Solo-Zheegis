"""Check the requested 8x8 draughts rules and bounded CPU implementation."""
from pathlib import Path
import subprocess,sys,tempfile
root=Path(__file__).resolve().parents[2]
flags=['-std=c++11','-O2','-Wall','-Wextra','-Werror']
if sys.platform=='linux':flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer']
with tempfile.TemporaryDirectory() as tmp:
    binary=Path(tmp)/'checkers'
    subprocess.run(['c++',*flags,'-I',str(root/'src'),str(root/'test/games/test_checkers.cpp'),'-o',str(binary)],check=True,timeout=60)
    subprocess.run([str(binary)],check=True,timeout=30)
