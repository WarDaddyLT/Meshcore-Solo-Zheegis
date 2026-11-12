"""Host checks of the firmware's actual privacy gate and interval parser."""
import hashlib
import pathlib
import subprocess
import tempfile

root = pathlib.Path(__file__).resolve().parents[2]
source = (root / 'examples/companion_radio/MyMesh.cpp').read_text()
privacy = source[source.index('const char* MyMesh::privateStatusChannel'):source.index('const char* MyMesh::verifyTrackerChannel')]
verify = source[source.index('const char* MyMesh::verifyTrackerChannel'):source.index('bool MyMesh::trackerReply')]
parser = source[source.index('static bool statusNumber'):source.index('bool MyMesh::handleStatusCommand')]
fixtures = ''
for name in ('team', '#team'):
    digest = ','.join(str(x) for x in hashlib.sha256(name.encode()).digest())
    fixtures += f'if (!strcmp((const char*)data, "{name}")) {{ const uint8_t hash[] = {{{digest}}}; memcpy(out, hash, 32); return; }}\n'
harness = r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cmath>
using std::isfinite;
struct ChannelDetails { char name[32]; struct { uint8_t secret[32]; } channel; };
namespace mesh { struct Utils { static void sha256(uint8_t* out, int, const uint8_t* data, int) {
FIXTURES
assert(false);
} }; }
struct Location { bool fresh = true; bool hasFreshFix(uint32_t) { return fresh; } };
struct Sensors { bool position_valid = true; double node_lat = 54.0, node_lon = 25.0; bool detected = true; Location location; Location* getLocationProvider() { return &location; } bool isGPSDetected() { return detected; } } sensors;
struct Clock { uint32_t now = 1800000000; uint32_t getCurrentTime() { return now; } } clock_state;
struct MyMesh {
  struct { uint8_t status_channel = 1; uint32_t status_interval = 60; bool gps_enabled = true; uint8_t status_key[32]; char status_name[32]; } _prefs;
 decltype(_prefs)& tracker_prefs = _prefs;
  ChannelDetails current;
  bool getChannel(int slot, ChannelDetails& ch) { ch = current; return slot == 1; }
  Clock* getRTCClock() { return &clock_state; }
  const char* privateStatusChannel(const ChannelDetails&);
  const char* verifyTrackerChannel(ChannelDetails&);
  const char* verifyStatus(ChannelDetails&);
};
PRIVACY
VERIFY
PARSER
int main() {
  MyMesh mesh;
  ChannelDetails ch = {};
  assert(mesh.privateStatusChannel(ch));
  strcpy(ch.name, "team");
  assert(mesh.privateStatusChannel(ch));
  const uint8_t pub[] = {0x8b,0x33,0x87,0xe9,0xc5,0xcd,0xea,0x6a,0xc9,0xe5,0xed,0xba,0xa1,0x15,0xcd,0x72};
  memcpy(ch.channel.secret, pub, 16);
  assert(mesh.privateStatusChannel(ch)); // renamed Public
  uint8_t hash[32];
  mesh::Utils::sha256(hash,32,(const uint8_t*)"#team",5);
  memcpy(ch.channel.secret,hash,16);
  assert(mesh.privateStatusChannel(ch)); // hashtag with '#' stripped
  mesh::Utils::sha256(hash,32,(const uint8_t*)"team",4);
  memcpy(ch.channel.secret,hash,16);
  assert(mesh.privateStatusChannel(ch));
  memset(ch.channel.secret,42,32);
  assert(!mesh.privateStatusChannel(ch)); // independent shared key
  strcpy(ch.name,"#team");
  assert(mesh.privateStatusChannel(ch));
  strcpy(ch.name,"team");
  mesh.current = ch;
  memcpy(mesh.tracker_prefs.status_key,ch.channel.secret,32);
  memcpy(mesh.tracker_prefs.status_name,ch.name,32);
  assert(!mesh.verifyStatus(ch));
  mesh.tracker_prefs.status_channel = 255;
  assert(mesh.verifyStatus(ch));
  mesh.tracker_prefs.status_channel = 1;
  mesh.current.channel.secret[0] ^= 1;
  assert(mesh.verifyStatus(ch));
  mesh.current.channel.secret[0] ^= 1;
  strcpy(mesh.current.name,"#team");
  assert(mesh.verifyStatus(ch));
  strcpy(mesh.current.name,"team");
  mesh.tracker_prefs.status_interval = 0;
  assert(!mesh.verifyStatus(ch)); // Manual requests work with the scheduled timer disabled.
  mesh.tracker_prefs.status_interval = 60;
  mesh._prefs.gps_enabled = false;
  assert(!mesh.verifyStatus(ch)); // Manual/cached position works with GPS off.
  mesh._prefs.gps_enabled = true;
  sensors.detected = false;
  assert(!mesh.verifyStatus(ch));
  sensors.detected = true;
  sensors.location.fresh = false;
  assert(!mesh.verifyStatus(ch)); // No live fix required.
  sensors.position_valid = false;
  assert(!mesh.verifyStatus(ch)); // Missing position produces an explicit no-fix/no-position status.
  sensors.position_valid = true;
  sensors.node_lat = 91;
  assert(mesh.verifyStatus(ch));
  sensors.node_lat = NAN;
  assert(mesh.verifyStatus(ch));
  sensors.node_lat = 0;
  sensors.node_lon = 0;
  assert(!mesh.verifyStatus(ch)); // Explicit manual (0,0) is valid.
  sensors.location.fresh = true;
  clock_state.now = 0;
  assert(!mesh.verifyStatus(ch)); // Emergency responses do not wait on time synchronization.
  clock_state.now = 1800000000;
  assert(!mesh.verifyStatus(ch));
  uint32_t n;
  assert(statusNumber("0",86400,n) && n == 0);
  assert(statusNumber("86400",86400,n) && n == 86400);
  for (auto bad : {"", "-1", "+1", "1s", "1.5", "86401", "4294967296", " 60"}) assert(!statusNumber(bad,86400,n));
  puts("Privacy gate, verification conditions, and interval parser passed");
}
'''.replace('#include <cassert>', '#include <cassert>\n#include <initializer_list>').replace('FIXTURES', fixtures).replace('PRIVACY', privacy).replace('PARSER', parser).replace('VERIFY', verify)
with tempfile.TemporaryDirectory() as tmp:
    cpp = pathlib.Path(tmp) / 'status.cpp'
    cpp.write_text(harness)
    binary = pathlib.Path(tmp) / 'status'
    subprocess.run(['c++', '-std=c++11', '-Wall', '-Wextra', '-Werror', str(cpp), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)

with tempfile.TemporaryDirectory() as tmp:
    binary = pathlib.Path(tmp) / 'leds'
    subprocess.run(['c++', '-std=c++11', '-Wall', '-Wextra', '-Werror', str(root / 'test/techo_status/test_leds.cpp'), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
    print('LED burst timing and millis wrap checks passed')

with tempfile.TemporaryDirectory() as tmp:
    binary = pathlib.Path(tmp) / 'tracker'
    subprocess.run(['c++', '-std=c++11', '-Wall', '-Wextra', '-Werror', str(root / 'test/techo_status/test_tracker.cpp'), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
    print('Tracker targeting, command/chirp routing, and single-packet help checks passed')

with tempfile.TemporaryDirectory() as tmp:
    binary = pathlib.Path(tmp) / 'fullscreen'
    subprocess.run(['c++', '-std=c++11', '-Wall', '-Wextra', '-Werror', str(root / 'test/techo_status/test_fullscreen.cpp'), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
    print('Fullscreen largest-fitting font and long-message wrapping checks passed')
