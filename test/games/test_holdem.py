"""Exhaustive standard high-hand ranking and heads-up betting regressions."""
from pathlib import Path
import subprocess,sys,tempfile
root=Path(__file__).resolve().parents[2]
flags=['-std=c++11','-O2','-Wall','-Wextra','-Werror']
if sys.platform=='linux':flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer']
with tempfile.TemporaryDirectory() as tmp:
    binary=Path(tmp)/'holdem'
    subprocess.run(['c++',*flags,'-I',str(root/'src'),str(root/'test/games/test_holdem.cpp'),'-o',str(binary)],check=True,timeout=60)
    subprocess.run([str(binary)],check=True,timeout=90)
