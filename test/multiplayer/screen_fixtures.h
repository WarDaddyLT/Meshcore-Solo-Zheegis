#pragma once
static uint32_t ticks=100;
uint32_t millis(){return ticks;}
uint32_t micros(){static uint32_t n=0;return ++n;}
class DisplayDriver {
 int x=0,y=0;
public:
 enum Color{DARK,LIGHT};std::string header;
 void setTextSize(int n){assert(n==1);}void setColor(Color){}
 void drawCenteredHeader(const char* s){assert(strlen(s)*6<=128);header=s;}
 void setCursor(int a,int b){x=a;y=b;}
 void print(const char* s){if(x<0||y<0||x+int(strlen(s))*6>128||y+9>64){fprintf(stderr,"Overflow %d,%d: %s\n",x,y,s);assert(false);}}
 void fillRect(int a,int b,int w,int h){assert(a>=0&&b>=0&&w>=0&&h>=0&&a+w<=128&&b+h<=64);}
 void drawRect(int a,int b,int w,int h){fillRect(a,b,w,h);}
 void drawXbm(int a,int b,const uint8_t* bits,int w,int h){assert(bits);fillRect(a,b,w,h);}
};
namespace UIColor {
constexpr auto primary_txt = DisplayDriver::LIGHT;
constexpr auto window_bkg = DisplayDriver::DARK;
}
struct UITask {
 TestIdentity identity;std::string name;std::vector<std::string> sent;std::vector<uint8_t> saved;std::vector<unsigned> sounds;
 bool muted=false,soundBusy=false,saveFail=false;
 explicit UITask(unsigned id):identity(id),name("Tracker"+std::to_string(id)){}
 void gamePublicKey(uint8_t* p)const{memcpy(p,identity.key,32);}
 void gameSharedSecret(const uint8_t* key,uint8_t* out)const{TestIdentity::secret(const_cast<TestIdentity*>(&identity),key,out);}
 bool gameRandomBytes(uint8_t* out,size_t n){return TestIdentity::random(&identity,out,n);}
 const char* gameNodeName()const{return name.c_str();}
 bool gameChannel(uint8_t slot, char* name, size_t capacity, uint8_t digest[16]) {
   if (slot != 1) return false;
   if (capacity) {
     constexpr char label[] = "Private";
     size_t length = capacity < sizeof(label) ? capacity - 1 : sizeof(label) - 1;
     memcpy(name, label, length);
     name[length] = '\0';
   }
   memset(digest, 5, 16);
   return true;
 }
 bool sendGameText(uint8_t slot,const char* text,const uint8_t digest[16]){assert(slot==1&&digest[0]==5&&strlen(text)<=112);sent.emplace_back(text);return true;}
 int loadVeylith(uint8_t* out,size_t size){if(saved.empty())return -1;if(saved.size()!=size)return -2;memcpy(out,saved.data(),size);return size;}
 bool saveVeylith(const uint8_t* in,size_t size){if(saveFail)return false;saved.assign(in,in+size);return true;}
 bool backupVeylith(){return true;}void showAlert(const char*,int){}void playGameSound(bool){}
 bool playArenaSound(uint8_t cue){if(muted)return true;if(soundBusy)return false;sounds.push_back(cue);return true;}
};
