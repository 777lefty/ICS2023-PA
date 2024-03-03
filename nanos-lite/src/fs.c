#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
#include "files.h"
  {"/dev/events", 0, 0, events_read},
  {"/proc/dispinfo", 0, 0, dispinfo_read},
  {"/dev/fb", 0, 0, invalid_read, fb_write}
};

int files_num=sizeof(file_table)/sizeof(Finfo);

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(void *buf, size_t offset, size_t len);

int fs_open(const char *pathname, int flags, int mode){
  for(int i=0;i<files_num;i++){
    if(strcmp(pathname, file_table[i].name)==0){
      file_table[i].open_offset=0;
      return i;
    }
  }
  printf("The required file is \"%s\"\n", pathname);
  return -1;
  panic("No such file");
}

size_t fs_read(int fd, void *buf, size_t len){
  Finfo cur_file=file_table[fd];
  if(cur_file.read)return cur_file.read(buf, cur_file.open_offset, len);
  size_t bound=len<cur_file.size-cur_file.open_offset?len:cur_file.size-cur_file.open_offset;
  ramdisk_read(buf, cur_file.disk_offset+cur_file.open_offset, bound);
  file_table[fd].open_offset+=bound;
  return bound; 
}

size_t fs_write(int fd, const void *buf, size_t len){
  Finfo cur_file=file_table[fd];
  if(cur_file.write)return cur_file.write(buf, cur_file.open_offset, len);
  size_t bound=len<cur_file.size-cur_file.open_offset?len:cur_file.size-cur_file.open_offset;
  ramdisk_write((void*)buf, cur_file.disk_offset+cur_file.open_offset, bound);
  file_table[fd].open_offset+=bound;
  return bound;
}

size_t fs_lseek(int fd, size_t offset, int whence){
  size_t tmp;
  switch(whence){
    case SEEK_SET:tmp=offset;break;
    case SEEK_CUR:tmp=file_table[fd].open_offset+offset;break;
    case SEEK_END:tmp=file_table[fd].size+offset;break;
    default:panic("No such whence");
  }
  tmp=tmp>file_table[fd].size?file_table[fd].size:tmp;
  file_table[fd].open_offset=tmp;
  return tmp;
}

int fs_close(int fd){
  return 0;
}

void init_fs() {
  // TODO: initialize the size of /dev/fb
  int fd=fs_open("/dev/fb", 0, 0);
  file_table[fd].size=io_read(AM_GPU_CONFIG).vmemsz;
}
