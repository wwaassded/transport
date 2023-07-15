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

typedef struct watchpoint
{
  int NO;
  char content[32];
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

void free_wp(int number)
{
  if(number > NR_WP) {
    int tmp = NR_WP;
    printf("the number should not be bigger than %d!\n",tmp);
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

void info_watchpoint()
{
  if (head == NULL)
    printf("there is no watchpoint yet ! \n");
  else
  {
    WP *ptr = head;
    while (ptr != nullptr)
    {
      printf("wp no:%d expr:%s", ptr->NO, ptr->content);
      printf("\n");
      ptr = ptr->next;
    }
  }
}