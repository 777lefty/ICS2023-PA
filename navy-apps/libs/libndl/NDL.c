#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int canvas_w = 0, canvas_h = 0;

uint32_t NDL_GetTicks() {
  uint32_t time=0;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  time=tv.tv_sec*1000+tv.tv_usec/1000;
  return time;
  //return 0;
}

int NDL_PollEvent(char *buf, int len) {
  int fd=open("/dev/events", 0, 0);
  int ret=read(fd, buf, len);
  //return read(fd, buf, len);
  return ret==0?0:1;
}

void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
  if(*w==0&&*h==0){
    *w=screen_w;
    *h=screen_h;
  }
  canvas_w=*w;
  canvas_h=*h;
  //printf("canvas w is %d, canvas h is %d\n", canvas_w, canvas_h);
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  assert(pixels!=NULL);  
  int fd=open("/dev/fb", 0, 0);
  for(int i=0;i<h;i++){
    size_t open_offset=(screen_w-canvas_w)/2+((screen_h-canvas_h)/2)*screen_w+screen_w*(i+y)+x;
    size_t len=w;     
    lseek(fd, open_offset, SEEK_SET);
    write(fd, pixels+(i*w), len);
    //printf("Draw %d lines\n",i);
  }
  close(fd);
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  char buf[72];
  int fd=open("/proc/dispinfo", 0, 0);
  read(fd, buf, sizeof(buf));
  sscanf(buf, "HEIGHT : %d\nWIDTH : %d\n", &screen_h, &screen_w);
  return 0;
}

void NDL_Quit() {
}
