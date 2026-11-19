"""Pending settings must be saved when an on-demand screen is destroyed."""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[2]
def function(path,signature):
 s=(root/path).read_text();a=s.index(signature);brace=s.index('{',a);b=brace+1;depth=1
 while depth:depth+=(s[b]=='{')-(s[b]=='}');b+=1
 return s[a:b]
destructor=function('examples/companion_radio/ui-new/SettingsScreen.h','  ~SettingsScreen()')
method=function('examples/companion_radio/ui-new/UITask.cpp','bool UITask::savePrefsIfDirty(')
code='''#include <cassert>
struct Mesh{unsigned writes=0;void savePrefs(){++writes;}}the_mesh;
struct UITask{bool savePrefsIfDirty(bool&);};
METHOD
struct SettingsScreen{UITask* _task;bool _dirty;DESTRUCTOR};
int main(){UITask task;
 {SettingsScreen screen{&task,false};}assert(!the_mesh.writes);
 {SettingsScreen screen{&task,true};}assert(the_mesh.writes==1);
 {SettingsScreen screen{&task,true};task.savePrefsIfDirty(screen._dirty);}assert(the_mesh.writes==2);
}
'''.replace('METHOD',method).replace('DESTRUCTOR',destructor)
with tempfile.TemporaryDirectory() as tmp:
 cpp=Path(tmp)/'settings.cpp';out=Path(tmp)/'settings';cpp.write_text(code)
 subprocess.run(['c++','-std=c++17','-Wall','-Wextra','-Werror',str(cpp),'-o',str(out)],check=True)
 subprocess.run([str(out)],check=True)
print('Settings lifetime: dirty unload saves once; clean and already-saved unloads do not rewrite preferences')
