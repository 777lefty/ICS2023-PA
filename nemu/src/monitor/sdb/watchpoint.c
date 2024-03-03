/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "sdb.h"
#include "watchpoint.h"
#define NR_WP 32

//typedef struct watchpoint {
//  int NO;
//  struct watchpoint *next;
//  char str[32];
//  word_t val;
//  bool work;
//  /* TODO: Add more members if necessary *//*

//} WP;

WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

WP* new_wp(){
  if(free_==NULL)assert(free_!=NULL);
  if(head==NULL){
    head=free_;
    free_=free_->next;
    head->next=NULL;
    head->work=true;
   // printf("success! %d\n",head==&wp_pool[31]?1:0);
   // printf("%p %p\n",head,wp_pool+31);
    return head;
  }
  else{
    WP *tmp=head;
    while(tmp->next!=NULL){tmp=tmp->next;}
    tmp->next=free_;
    free_=free_->next;
    tmp->next->next=NULL;
    tmp->next->work=true;
    return tmp->next;
  }
}

void free_wp(WP *wp){
  if(wp->work==false)assert(wp->work!=false);
  if(free_==NULL){
    wp->work=false;
    if(head==wp){
      head=wp->next;
      wp->next=NULL;
      free_=wp;
    }
    else{
      WP *tmp=head;
      while(tmp->next!=wp)tmp=tmp->next;
      tmp->next=wp->next;
      wp->next=NULL;
      free_=wp;
    }
  }
  else{
    wp->work=false;
    WP *tmp=free_;
    while(tmp->next!=NULL)tmp=tmp->next;
    if(head==wp){
      head=wp->next;
      wp->next=NULL;
      tmp->next=wp;
    }
    else{
      WP *mid=head;
      while(mid->next==wp)mid=mid->next;
      mid->next=wp->next;
      wp->next=NULL;
      tmp->next=wp;
    }
  }
}

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].work = false;
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

