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
#include <unistd.h>
#include "sdb.h"
#define nullptr NULL
#define NR_WP 32
#define expr_len 32

typedef struct watchpoint
{
  int NO;
  char content[expr_len];
  uint32_t old_value;
  uint32_t new_value;
  struct watchpoint *next;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;
static WP *dull;
static bool init = false;
void init_wp_pool()
{
  init = true;
  int i;
  for (i = 0; i < NR_WP; i++)
  {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
  dull = (WP *)malloc(sizeof(WP));
  dull->next = head;
  dull->NO = -1;
}
/// @brief 可能会有修改
/// @return WP
WP *new_wp()
{
  if (!init)
  {
    printf("wait for init the wp_pool!\n");
    sleep(1);
    init_wp_pool();
    init = true;
  }
  if (free_ == NULL)
  {
    printf("the wp_pool is empty!\n");
    assert(0);
  }
  else
  {
    WP *new = free_;
    free_ = free_->next;
    new->next = NULL;
    if (head == NULL)
      head = new;
    else
    {
      new->next = head;
      head = new;
    }
    return new;
  }
}

void check_watchpoint(bool *is_stop)
{
  if (head == nullptr)
    return;
  else
  {
    WP *ptr = head;
    while (ptr != nullptr)
    {
      bool success = true;
      ptr->old_value = ptr->new_value;
      ptr->new_value = expr(ptr->content, &success);
      if(!success) {
        printf("you should input correct expr!\n");
        return;
      }
      else if((ptr->new_value != ptr->old_value) && !(ptr->new_value==-1 && ptr->old_value==-2)) {
        printf("wp no:%d  old_value:0x%x  new_value:0x%x  expr:%s",ptr->NO,ptr->old_value,ptr->new_value,ptr->content);
        printf("\n");
        *is_stop = true;
      }
      ptr = ptr->next;
    }
  }
}

void free_wp(int number)
{
  if (number > NR_WP)
  {
    int tmp = NR_WP;
    printf("the number should not be bigger than %d!\n", tmp);
    assert(0);
  }
  if (head == NULL)
  {
    printf("you can not free before use !\n");
    assert(0);
  }
  else
  {
    WP *pre = NULL;
    WP *tmp = head;
    while (tmp != NULL && tmp->NO != number)
    {
      pre = tmp;
      tmp = tmp->next;
    }
    if (tmp != NULL)
    {
      if (tmp == head)
        head = head->next;
      else
        pre->next = tmp->next;
    }
    else
    {
      printf("there is no this WP!\n");
      assert(0);
    }
    tmp->next = NULL;
    if (free_ == NULL)
      free_ = tmp;
    else
    {
      tmp->next = free_;
      free_ = tmp;
    }
  }
}

void add_watchpoint(char *wp_expr)
{
  WP *ptr = new_wp();
  strcpy(ptr->content, wp_expr);
  ptr->content[expr_len - 1] = '\0';
  bool success = true;
  uint32_t value = expr(ptr->content, &success);
  if (!success)
  {
    printf("you should set a right watchpoint!\n");
    free_wp(ptr->NO);
  }
  else
  {
    ptr->old_value = value;
    ptr->new_value = value;
  }
}

void info_watchpoint()
{
  if (head == NULL)
    printf("there is no watchpoint yet ! \n");
  else
  {
    WP *ptr = head;
    while (ptr != nullptr)
    {
      printf("wp no:%d  old_value:%u  new_value:%u  expr:%s", ptr->NO, ptr->old_value, ptr->new_value, ptr->content);
      printf("\n");
      ptr = ptr->next;
    }
  }
}