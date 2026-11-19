"""Verify standard five-ship fleets and the solo CPU without hardware."""
from pathlib import Path
import subprocess,sys,tempfile
root=Path(__file__).resolve().parents[2]
flags=['-std=c++11','-O2','-Wall','-Wextra','-Werror']
if sys.platform=='linux':flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer']
with tempfile.TemporaryDirectory() as tmp:
    binary=Path(tmp)/'battleship'
    subprocess.run(['c++',*flags,'-I',str(root/'src'),str(root/'test/games/test_battleship.cpp'),'-o',str(binary)],check=True,timeout=60)
    subprocess.run([str(binary)],check=True,timeout=30)
