#include <proc.h>
#include <elf.h>
#include<fs.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

#define ALIGN(x) ((uintptr_t)(x)&0xfffff000)
#define PAGESZ 4096

size_t ramdisk_read(void *buf, size_t offset, size_t len);
int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);

void *page_allocator(PCB *pcb, void *vaddr, size_t memsz){
  int page_num=(((uintptr_t)vaddr+memsz-1)>>12)-((uintptr_t)vaddr>>12)+1;
  void *start=new_page(page_num);
  for(int i=0;i<page_num;i++){
    map(&pcb->as, (void *)(ALIGN(vaddr)+i*PAGESZ), (void *)(start+PAGESZ*i), 1);
  }
  pcb->max_brk = pcb->max_brk > ((uintptr_t)vaddr & ~0xfff) + page_num * PGSIZE ? pcb->max_brk : ((uintptr_t)vaddr & ~0xfff) + page_num * PGSIZE;
  return start;
}

static uintptr_t loader(PCB *pcb, const char *filename) {
  //TODO();
  //assert(0);
  Elf_Ehdr ehdr;
  //ramdisk_read(&ehdr, 0, sizeof(Elf_Ehdr));
  int fd=fs_open(filename, 0, 0);
  if(fd<0)assert(0);
  fs_read(fd, &ehdr, sizeof(Elf_Ehdr));
  assert(*(uint32_t *)ehdr.e_ident == 0x464c457f);
  Elf_Phdr phdr[ehdr.e_phnum];
  fs_lseek(fd, ehdr.e_phoff, SEEK_SET);
  fs_read(fd, phdr, sizeof(Elf_Phdr)*ehdr.e_phnum);
  //ramdisk_read(phdr, ehdr.e_phoff, sizeof(Elf_Phdr)*ehdr.e_phnum);
  for(int i=0;i<ehdr.e_phnum;i++){
    if(phdr[i].p_type==PT_LOAD){
      void *vaddr=page_allocator(pcb, (void *)phdr[i].p_vaddr, phdr[i].p_memsz);
      fs_lseek(fd, phdr[i].p_offset, SEEK_SET);
      fs_read(fd, vaddr+(phdr[i].p_vaddr&0xfff), phdr[i].p_memsz);
      //ramdisk_read((void*)phdr[i].p_vaddr, phdr[i].p_offset, phdr[i].p_memsz);
      memset((void*)(vaddr+(phdr[i].p_vaddr&0xfff)+phdr[i].p_filesz), 0, phdr[i].p_memsz-phdr[i].p_filesz);
    }
  }
  printf("entry is %d\n", ehdr.e_entry);
  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

void context_kload(PCB *pcb, void (*entry)(void *), void *arg){
  Area stack;
  stack.start = pcb->stack;
  stack.end = pcb->stack + STACK_SIZE;
  pcb->cp = kcontext(stack, entry, arg);
}

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]){
  AddrSpace *as=&pcb->as;
  protect(as);
  /*Area stack;
  stack.start=pcb->stack;
  stack.end=pcb->stack+STACK_SIZE;*/
  //assert(0);
  //uintptr_t entry=loader(pcb, filename);
  //uintptr_t *ustack=(uintptr_t *)heap.end;
  uintptr_t *ustack=(uintptr_t *)(new_page(8)+8*PGSIZE);
  for(int i=1;i<=8;i++){
    //assert(0);
    map(as, as->area.end-i*PAGESZ, ustack-i*PAGESZ, 1);
    //assert(0);
  }
  int argc=0,envc=0;
  //assert(0);
  if(argv){
    while(argv[argc]){
      printf("%s %d\n",argv[argc],argc+1);
      ++argc;
    }
  }
  if(envp){
    while(envp[envc])++envc;
  }
  //assert(0);
  uintptr_t argaddr[argc];
  uintptr_t envaddr[envc];
  for(int i=argc-1;i>=0;i--){
    //assert(0);
    int len=strlen(argv[i])+1;
    //assert(0);
    ustack-=len;
    //assert(0);
    //assert(argaddr[i]!=(uintptr_t)ustack);
    argaddr[i]=(uintptr_t)ustack;
    strncpy((char *)ustack, argv[i], len);
    //assert(strcmp((char *)ustack[0], argv[i])!=0);
    //assert(0);
    //assert(0);
    //assert(0);
  }
  //assert(0);
  for(int i=envc-1;i>=0;i--){
    int len=strlen(envp[i])+1;
    ustack-=len;
    strncpy((char *)ustack, envp[i], len);
    envaddr[i]=(uintptr_t)ustack;
  }
  ustack-=(argc+envc+3);
  ustack[0]=argc;
  for(int i=0;i<argc;i++){
    ustack[i+1]=(uintptr_t)argaddr[i];
    //printf("argaddr[%d] is %p\n", i, argaddr[i]);
  }
  //assert(0);
  ustack[argc+1]=0;
  for(int i=0;i<envc;i++){
    ustack[i+argc+2]=envaddr[i];
  }
  ustack[argc+envc+2]=0;
  uintptr_t entry=loader(pcb, filename);
  Area stack;
  stack.start=pcb->stack;
  stack.end=pcb->stack+STACK_SIZE;
  pcb->cp=ucontext(as, stack, (void *)entry);
  pcb->cp->gpr[10]=(uintptr_t)ustack;
  printf("ustack is at %p\n", (uintptr_t)ustack);
  //assert(0);
}

