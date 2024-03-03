#include <common.h>
#define NR_WP 32
typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char str[32];
  word_t val;
  bool work;
} WP;
extern WP wp_pool[NR_WP];
WP* new_wp();
void free_wp(WP *wp);
