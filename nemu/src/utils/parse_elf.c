#include<common.h>
#include<elf.h>
#include<fcntl.h>
#include<unistd.h>

#ifdef CONFIG_FTRACE_COND
typedef struct{
  char name[32];
  paddr_t addr;
  unsigned char info;
  Elf32_Xword size;
}Entry;

Entry *symbol_tab=NULL;
int symbol_tab_size=0;
int call_depth=0;
#endif

#ifdef CONFIG_FTRACE
static void read_elf_header(int fd, Elf32_Ehdr *eh){
  assert(lseek(fd,0,SEEK_SET)==0);
  assert(read(fd,(void *)eh,sizeof(Elf32_Ehdr))==sizeof(Elf32_Ehdr));
}

static void read_section_headers(int fd,Elf32_Ehdr eh,Elf32_Shdr *sh){
  assert(lseek(fd,(off_t)eh.e_shoff,SEEK_SET)==(off_t)eh.e_shoff);
  for(int i=0;i<eh.e_shnum;i++){
    assert(read(fd,(void *)&sh[i],eh.e_shentsize)==eh.e_shentsize);
  }
}

static void read_section(int fd,Elf32_Shdr sh, void *buf){
  assert(buf!=NULL);
  assert(lseek(fd,(off_t)sh.sh_offset,SEEK_SET)==(off_t)sh.sh_offset);
  assert(read(fd,buf,sh.sh_size)==sh.sh_size);
}

static void read_symbol_table(int fd, Elf32_Ehdr eh, Elf32_Shdr *sh, int symbol_index){
  Elf32_Sym sym_tab[sh[symbol_index].sh_size];
  read_section(fd,sh[symbol_index],sym_tab);
  int str_index=sh[symbol_index].sh_link;
  char str_tab[sh[str_index].sh_size];
  read_section(fd,sh[str_index],str_tab);
  int sym_cnt=(sh[symbol_index].sh_size/sizeof(Elf32_Sym));
  symbol_tab_size=sym_cnt;
  symbol_tab=malloc(sym_cnt*sizeof(Entry));
  for(int i=0;i<sym_cnt;i++){
    symbol_tab[i].addr=sym_tab[i].st_value;
    symbol_tab[i].info=sym_tab[i].st_info;
    symbol_tab[i].size=sym_tab[i].st_size;
    memset(symbol_tab[i].name, 0, 32);
    strncpy(symbol_tab[i].name, str_tab+sym_tab[i].st_name, 31);
  }
}

static void read_symbols(int fd, Elf32_Ehdr eh, Elf32_Shdr *sh){
  for(int i=0;i<eh.e_shnum;i++){
    if((sh[i].sh_type==SHT_SYMTAB)||(sh[i].sh_type==SHT_DYNSYM)){
      read_symbol_table(fd, eh, sh, i);
    }
  }
}

void parse_elf(const char *elf_file){
  if(elf_file==NULL)return;
  int fd=open(elf_file, O_RDONLY|O_SYNC);
  assert(fd>=0);
  Elf32_Ehdr eh;
  read_elf_header(fd, &eh);
  Elf32_Shdr sh[eh.e_shentsize*eh.e_shnum];
  read_section_headers(fd, eh, sh);
  read_symbols(fd, eh, sh);
  close(fd);
}

int find_func(paddr_t target, bool call){
  int i;
  for(i=0;i<symbol_tab_size;i++){
    if(ELF32_ST_TYPE(symbol_tab[i].info)==STT_FUNC){
      if(call){
        if(symbol_tab[i].addr==target)break;
      }
      else{
        if(symbol_tab[i].addr<=target && target<symbol_tab[i].addr+symbol_tab[i].size)break;
      }
    }
  }
  return i<symbol_tab_size?i:-1;
}

void trace_call(paddr_t pc, paddr_t target){
  if(symbol_tab==NULL)return;
  call_depth++;
  int i=find_func(target, true);
  printf("0x%08x:",pc);
  for(int j=0;j<call_depth;j++){
    printf(" ");
  }
  printf("call [%s@0x%08x]\n",symbol_tab[i].name, target);
#ifdef CONFIG_FTRACE_COND
  if(FTRACE_COND){for(int j=0;j<call_depth;j++){log_write(" ");}}
  if(FTRACE_COND){log_write("call [%s@0x%08x]\n",symbol_tab[i].name, target);}
#endif
}

void trace_ret(paddr_t pc, paddr_t target){
  if(symbol_tab==NULL)return;
  int i=find_func(target, false);
  printf("0x%08x:", pc);
  for(int j=0;j<call_depth;j++){
    printf(" ");
  }
  printf("ret [%s]\n",symbol_tab[i].name);
#ifdef CONFIG_FTRACE_COND
  if(FTRACE_COND){for(int j=0;j<call_depth;j++){log_write(" ");}}
  if(FTRACE_COND){log_write("ret [%s]\n",symbol_tab[i].name);}
#endif
  call_depth--;
}
#endif
