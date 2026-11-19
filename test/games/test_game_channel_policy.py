"""Games accepts every existing private channel without extra state or storage."""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[2]
source=(root/'examples/companion_radio/MyMesh.cpp').read_text()
def function(signature):
    start=source.index(signature);brace=source.index('{',start);end=brace+1;depth=1
    while depth:
        depth+=(source[end]=='{')-(source[end]=='}');end+=1
    return source[start:end]
harness=r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#define SOLO_GAMES_ENABLED 1
#define PRIVATE_STATUS_REPORTER 1
struct ChannelDetails {struct {uint8_t secret[32];} channel;char name[32];};
namespace mesh {struct Utils {
  // Storage-independent stand-in: exercise policy, not the SHA-256 library.
  static void sha256(uint8_t* out,size_t count,const uint8_t* in,size_t n){
    for(size_t i=0;i<count;++i)out[i]=in[i%n]^uint8_t(i*7);
  }
};}
struct MyMesh {
  ChannelDetails configured[8]={};
  bool getChannel(uint8_t slot,ChannelDetails& out){if(slot>=8)return false;out=configured[slot];return true;}
  const char* privateStatusChannel(const ChannelDetails&);
  bool getGameChannel(uint8_t,ChannelDetails&,uint8_t[16]);
  // No datastore, tracking binding or origin metadata exists in this fixture.
};
METHODS
int main(){
  MyMesh mesh;ChannelDetails out;uint8_t digest[16],old[16];
  for(unsigned slot=0;slot<8;++slot){strcpy(mesh.configured[slot].name,"Test channel");memset(mesh.configured[slot].channel.secret,slot+7,32);}
  assert(mesh.getGameChannel(1,out,digest));memcpy(old,digest,16);
  assert(mesh.getGameChannel(2,out,digest)); // Another private channel is equally eligible.
  static const uint8_t publicKey[16]={0x8b,0x33,0x87,0xe9,0xc5,0xcd,0xea,0x6a,0xc9,0xe5,0xed,0xba,0xa1,0x15,0xcd,0x72};
  memset(mesh.configured[0].channel.secret,0,32);memcpy(mesh.configured[0].channel.secret,publicKey,16);
  strcpy(mesh.configured[0].name,"Renamed public");assert(!mesh.getGameChannel(0,out,digest));
  strcpy(mesh.configured[3].name,"#topic");assert(!mesh.getGameChannel(3,out,digest));
  mesh::Utils::sha256(mesh.configured[4].channel.secret,32,(const uint8_t*)"#Test channel",13);
  assert(!mesh.getGameChannel(4,out,digest));
  mesh::Utils::sha256(mesh.configured[5].channel.secret,32,(const uint8_t*)"Test channel",12);
  assert(!mesh.getGameChannel(5,out,digest));
  memset(mesh.configured[6].channel.secret,0,32);assert(!mesh.getGameChannel(6,out,digest));
  mesh.configured[7].name[0]=0;assert(!mesh.getGameChannel(7,out,digest));
  assert(!mesh.getGameChannel(40,out,digest));
  mesh.configured[1].channel.secret[0]^=1;
  assert(mesh.getGameChannel(1,out,digest) && memcmp(digest,old,16));
  puts("All existing private channels admitted without binding/metadata; public, hashtag, derived and empty channels excluded; rekey changes session binding");
}
'''.replace('METHODS',function('const char* MyMesh::privateStatusChannel(')+'\n'+function('bool MyMesh::getGameChannel('))
with tempfile.TemporaryDirectory() as tmp:
    cpp,binary=Path(tmp)/'policy.cpp',Path(tmp)/'policy';cpp.write_text(harness)
    subprocess.run(['c++','-std=c++17','-Wall','-Wextra','-Werror',str(cpp),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
