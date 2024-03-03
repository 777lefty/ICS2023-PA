#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static int height;
static int width;

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  //return 0;
  //yield();
  for(int i=0;i<len;i++){
    putch(((char*)buf)[i]);
  }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  //yield();
  AM_INPUT_KEYBRD_T kbd=io_read(AM_INPUT_KEYBRD);
  int32_t keycode=kbd.keycode;
  bool keydown=kbd.keydown;
  //int i=0;
  if(keycode==AM_KEY_NONE)return 0;
  size_t length=0;
  //printf("%s\n",keydown?"down":"up");
  if(keydown){
//	  i=1;
    length=snprintf(buf, len, "kd %s", keyname[keycode]);
    ((char *)buf)[length]='\0';
  }
  else{
//	  printf("i is %d\n",i);
    length=snprintf(buf, len, "ku %s", keyname[keycode]);
    ((char *)buf)[length]='\0';
  }  
  return length;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  AM_GPU_CONFIG_T cfg=io_read(AM_GPU_CONFIG);
  height=cfg.height;
  width=cfg.width;
  size_t length=snprintf(buf, len, "HEIGHT : %d\nWIDTH : %d\n", height, width);  
  return length;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  assert(buf!=NULL);
  //yield();
  int x,y,w,h;
  x=offset%width;
  y=offset/width;
  w=len;
  h=1;
  io_write(AM_GPU_FBDRAW, x, y, (void *)buf, w, h, true);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
