"""Exercise fixed-point ballistics and the bounded solo artillery planner."""
from pathlib import Path
import subprocess,sys,tempfile
root=Path(__file__).resolve().parents[2]
flags=['-std=c++11','-O2','-Wall','-Wextra','-Werror']
if sys.platform=='linux':flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer']
with tempfile.TemporaryDirectory() as tmp:
    binary=Path(tmp)/'artillery'
    subprocess.run(['c++',*flags,'-I',str(root/'src'),str(root/'test/games/test_artillery.cpp'),'-o',str(binary)],check=True,timeout=60)
    subprocess.run([str(binary)],check=True,timeout=30)
