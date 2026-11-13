"""Exercise the production app-CLI reply encoder, including request-prefix handling."""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[2]
source=(root/'examples/companion_radio/MyMesh.cpp').read_text()
method=source[source.index('void MyMesh::handleAppCLI('):source.index('static bool statusNumber(')]
harness=r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#define MAX_FRAME_SIZE 176
#define RESP_CODE_CLI_REPLY 29
struct Serial {
 std::vector<uint8_t> reply;
 void writeFrame(const uint8_t* p,size_t n) { reply.assign(p,p+n); }
} serial;
struct MyMesh {
 uint8_t cmd_frame[176],out_frame[176]; Serial* _serial=&serial;
 std::string seen;
 bool handleStatusCommand(const char* command,char* reply) {
  seen=command;
  if(seen=="status show") { strcpy(reply,"Tracking:off"); return true; }
  return false;
 }
 void handleAppCLI(size_t len);
 void send(const std::string& command) {
  assert(command.size()<=175);cmd_frame[0]=66;
  memcpy(cmd_frame+1,command.data(),command.size());handleAppCLI(command.size()+1);
 }
};
METHOD
int main() {
 MyMesh m;
 m.send("status show"); assert(m.seen=="status show");
 assert(serial.reply[0]==29);
 assert(std::string(serial.reply.begin()+1,serial.reply.end())=="Tracking:off");
 m.send("  a1| status show"); assert(m.seen=="status show");
 assert(std::string(serial.reply.begin()+1,serial.reply.end())=="a1|Tracking:off");
 m.send("a2|unknown");
 assert(std::string(serial.reply.begin()+1,serial.reply.end()).find("a2|Unknown tracker command")==0);
 m.send(std::string(175,'x')); assert(serial.reply.size()<=176);
}
'''.replace('METHOD',method)
with tempfile.TemporaryDirectory() as tmp:
 cpp,binary=Path(tmp)/'app.cpp',Path(tmp)/'app'
 cpp.write_text(harness)
 subprocess.run(['c++','-std=c++17','-Wall','-Wextra','-Werror',str(cpp),'-o',str(binary)],check=True)
 subprocess.run([str(binary)],check=True)
print('Mobile app CLI reply code, command prefixes, unknown commands and frame bounds passed')
