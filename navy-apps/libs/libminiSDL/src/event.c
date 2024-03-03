#include <NDL.h>
#include <SDL.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

uint8_t key_state[83]={0};

int SDL_PushEvent(SDL_Event *ev) {
  
	assert(0);
return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  char EvEntry[20];
  if(NDL_PollEvent(EvEntry, sizeof(EvEntry))==0){
    ev->type=SDL_KEYUP;
    return 0;
  }
  else{
    ev->type=(EvEntry[1]=='d')?SDL_KEYDOWN:SDL_KEYUP;
    for(int i=0;i<sizeof(keyname)/sizeof(char *);i++){
      if(strcmp(keyname[i], &EvEntry[3])==0){
        ev->key.keysym.sym=i;
	key_state[i]=(ev->type == SDL_KEYDOWN)? 1: 0;
        return 1;         
      }
    }
  }
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  char EvEntry[20];
  while(1){
    if(NDL_PollEvent(EvEntry, sizeof(EvEntry))==0){
      event->type=SDL_KEYUP;
    }
    else{
      event->type=(EvEntry[1]=='d')?SDL_KEYDOWN:SDL_KEYUP;
      for(int i=0;i<sizeof(keyname)/sizeof(char *);i++){
        if(strcmp(keyname[i], &EvEntry[3])==0){
          event->key.keysym.sym=i;
	  key_state[i]=(event->type == SDL_KEYDOWN)? 1: 0;
          return 1;
        }
      }
    }
  }
  return 0;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
assert(0);

      	return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  //assert(0);
  if(numkeys)*numkeys=83;
  return key_state;
}
