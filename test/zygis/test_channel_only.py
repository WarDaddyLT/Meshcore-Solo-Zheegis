"""Execute actual family-profile storage and live-handoff code with host fakes."""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[2]
def function(file,signature):
 s=(root/file).read_text();a=s.index(signature);start=s.index('{',a);end=start+1;depth=1
 while depth:depth+=(s[end]=='{')-(s[end]=='}');end+=1
 return s[a:end]
pieces=[function('examples/companion_radio/MyMesh.cpp',sig) for sig in ['bool MyMesh::Frame::isChannelMsg(','void MyMesh::addToOfflineQueue(','int MyMesh::getFromOfflineQueue(']]
pieces += [function('examples/companion_radio/DataStore.cpp',sig) for sig in ['void DataStore::loadContacts(','void DataStore::saveContacts(']]
pieces += [function('src/helpers/BaseChatMesh.cpp',sig) for sig in ['void BaseChatMesh::onAdvertRecv(','bool BaseChatMesh::addContact(','bool BaseChatMesh::removeContact(','void BaseChatMesh::scanRecentContacts(']]
harness=r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdio>
#define ZYGIS_CHANNEL_ONLY 1
#define OFFLINE_QUEUE_SIZE 4
#define MAX_ANON_CONTACTS 8
#define ADV_TYPE_NONE 0
#define ADV_TYPE_CHAT 1
#define ADV_TYPE_REPEATER 2
#define MESH_DEBUG_PRINTLN(...) do{}while(0)
#define RESP_CODE_CHANNEL_MSG_RECV 8
#define RESP_CODE_CHANNEL_MSG_RECV_V3 17
#define RESP_CODE_CHANNEL_DATA_RECV 21
struct Serial {bool connected=false;bool isClientConnected(){return connected;}};
struct MyMesh {
 struct Frame {uint8_t len=0,buf[176]={};bool isChannelMsg()const;};
 Serial* _serial;int offline_queue_len=0;Frame offline_queue[4];
 void addToOfflineQueue(const uint8_t*,int);int getFromOfflineQueue(uint8_t*);
};
struct ContactInfo {uint8_t type=0;struct Id {uint8_t value=0;bool matches(Id other)const{return value==other.value;}}id;uint32_t lastmod=0;};
struct DataStoreHost {};struct DataStore {void loadContacts(DataStoreHost*);void saveContacts(DataStoreHost*,bool (*)(const ContactInfo&));};
namespace mesh {struct Identity:ContactInfo::Id {};struct Packet {unsigned hops=0;unsigned getPathHashCount(){return hops;}};}
struct AdvertDataParser {const uint8_t* p;AdvertDataParser(const uint8_t* data,size_t):p(data){}bool isValid()const{return true;}bool hasName()const{return true;}uint8_t getType()const{return p[0];}};
struct ContactVisitor {};
struct BaseChatMesh {
 ContactInfo contacts[8];unsigned changes=0;uint32_t ticks=1;
 BaseChatMesh* getRTCClock(){return this;}uint32_t getCurrentTime(){return ticks++;}
 void populateContactFromAdvert(ContactInfo& c,const mesh::Identity& id,const AdvertDataParser& a,uint32_t){c={};c.type=a.getType();c.id=id;++changes;}
 void onAdvertRecv(mesh::Packet*,const mesh::Identity&,uint32_t,const uint8_t*,size_t);
 bool addContact(const ContactInfo&);bool removeContact(ContactInfo&);void scanRecentContacts(int,ContactVisitor*);
};
FUNCTIONS
int main(){
 Serial link;MyMesh m{};m._serial=&link;uint8_t in[3]={8,1,2},out[176]={};
 for(unsigned i=0;i<1000;++i)m.addToOfflineQueue(in,3);
 assert(m.offline_queue_len==0 && !m.getFromOfflineQueue(out));
 link.connected=true;
 for(unsigned i=0;i<10;++i){in[1]=i;m.addToOfflineQueue(in,3);assert(m.offline_queue_len<=4);}
 for(unsigned i=6;i<10;++i){assert(m.getFromOfflineQueue(out)==3 && out[1]==i);}
 assert(!m.getFromOfflineQueue(out));m.addToOfflineQueue(in,3);link.connected=false;
 assert(!m.getFromOfflineQueue(out) && !m.offline_queue_len);link.connected=true;assert(!m.getFromOfflineQueue(out));
 DataStore store;DataStoreHost host;store.loadContacts(&host);store.saveContacts(&host,nullptr);
 BaseChatMesh peers;mesh::Packet packet;mesh::Identity id;uint8_t advert[1]={ADV_TYPE_CHAT};
 for(unsigned i=0;i<1000;++i){id.value=i;peers.onAdvertRecv(&packet,id,0,advert,1);}assert(!peers.changes);
 advert[0]=ADV_TYPE_REPEATER;packet.hops=2;peers.onAdvertRecv(&packet,id,0,advert,1);assert(!peers.changes);
 packet.hops=0;
 for(unsigned i=0;i<100;++i){id.value=i;peers.onAdvertRecv(&packet,id,0,advert,1);}
 assert(peers.changes==100);unsigned count=0;for(auto& c:peers.contacts)count+=c.type==ADV_TYPE_REPEATER;assert(count==8);
 auto copy=peers.contacts[0];assert(!peers.addContact(copy) && !peers.removeContact(copy));peers.scanRecentContacts(0,nullptr);
 assert(peers.contacts[0].id.matches(copy.id));
 printf("Channel-only: no offline backlog, four live frames, disconnect clears pending data, contact files untouched, eight direct repeater slots and no ordinary add/remove/sort passed\n");
}
'''.replace('FUNCTIONS','\n'.join(pieces))
with tempfile.TemporaryDirectory() as tmp:
 cpp=Path(tmp)/'family.cpp';binary=Path(tmp)/'family';cpp.write_text(harness)
 subprocess.run(['c++','-std=c++17','-O2','-Wall','-Wextra','-Werror',str(cpp),'-o',str(binary)],check=True)
 subprocess.run([str(binary)],check=True)
