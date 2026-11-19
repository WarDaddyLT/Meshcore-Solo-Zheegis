#pragma once
#include <games/multiplayer/SecureChannel.h>
#include <cstring>
using namespace solo_games::multiplayer;
struct TestIdentity {
 uint8_t key[32]={};uint32_t seed;
 explicit TestIdentity(unsigned id):seed(id){Codec::put(key,id,4);for(unsigned i=4;i<32;++i)key[i]=id+i;}
 static void pub(void* context,uint8_t* out){memcpy(out,static_cast<TestIdentity*>(context)->key,32);}
 static void secret(void* context,const uint8_t* peer,uint8_t* out){unsigned a=publicId(static_cast<TestIdentity*>(context)->key),b=publicId(peer);memset(out,0x37,32);Codec::put(out,a<b?a:b,4);Codec::put(out+4,a>b?a:b,4);}
 static bool random(void* context,uint8_t* out,size_t n){auto& state=static_cast<TestIdentity*>(context)->seed;while(n--){state=state*1664525+1013904223;*out++=state>>24;}return true;}
 IdentityCallbacks callbacks(){IdentityCallbacks c;c.context=this;c.publicKey=pub;c.sharedSecret=secret;c.randomBytes=random;return c;}
};
