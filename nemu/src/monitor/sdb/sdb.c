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

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <ctype.h>
#include <memory/paddr.h>
#include <string.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char *rl_gets()
{
  static char *line_read = NULL;

  if (line_read)
  {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read)
  {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_x(char *args)
{
  char *arg = strtok(NULL, " ");
  if (arg == NULL)
  {
    printf("please input the memory address you want to print!\n");
    return 0;
  }
  else
  {
    char *address = strtok(NULL, " ");
    if (address == NULL)
    {
      printf("please input memory address you want to print!\n");
      return 0;
    }
    int len = strlen(address);
    int number = 0;
    int i;
    for (int i = 0; arg[i] != '\0'; ++i)
    {
      if (!isdigit(arg[i]))
      {
        printf("please give me a number!\n");
        return 0;
      }
      int tmp = number;
      number = number * 10 + (arg[i] - '0');
      if (tmp > number)
      {
        printf("integer overflow!\n");
        return 0;
      }
    }
    if (len >= 3 && address[0] == '0' && (address[1] == 'x' || address[1] == 'X'))
    {
      address += 1;
      address += 1;
    }
    unsigned int addre = 0;
    for (i = 0; address != NULL && address[i] != '\0'; ++i)
    {
      if (isdigit(address[i]))
      {
        addre = addre * 16 + (address[i] - '0');
      }
      else if ((address[i] >= 'a' && address[i] <= 'f') || (address[i] >= 'A' && address[i] <= 'B'))
      {
        addre = addre * 16 + (tolower(address[i]) - 'a' + 10);
      }
      else
      {
        printf("you should not input something like that !\n");
        return 0;
      }
    }
    unsigned int j;
    unsigned int jj;
    for (j = 0; j < number; ++j)
    {
      printf("at 0x%x : ", addre);
      for (jj = 0; jj < 4; ++jj)
      {
        word_t nu = paddr_read(addre + jj, 1);
        printf("%02x ", nu);
      }
      printf("\n");
      addre += 4;
    }
  }
  return 0;
}

static int cmd_info(char *args)
{
  char *arg = strtok(NULL, " ");
  if (arg == NULL)
  {
    printf("you was suposed to input r/w after the info command!\n");
    return 0;
  }
  else
  {
    if (strcmp(arg, "r") == 0)
    {
      isa_reg_display();
    }
    else if (strcmp(arg, "w") == 0)
    {
      info_watchpoint();
    }
    else
    {
      printf("you was suposed to input r/w after the info command!\n");
      return 0;
    }
  }
  return 0;
}

static int cmd_si(char *args)
{
  char *arg = strtok(NULL, " ");
  if (arg == NULL)
    cpu_exec(1);
  // si命令之后又跟随其他内容，判断输入是否合法并处理合法的输入
  else
  {
    int i;
    uint64_t number = 0;
    for (i = 0; arg[i] != '\0'; ++i)
    {
      if (!isdigit(arg[i]))
      {
        if (arg[i] == '.' || arg[i] == '-')
          printf("you should input positive integer only !\n");
        else
          printf("you should only input number after the si command!\n");
        return 0;
      }
      else
      {
        uint64_t tmp = number;
        number = number * 10 + (arg[i] - '0');
        if (tmp >= number)
        {
          printf("your input is too big! overflow error!\n");
          return 0;
        }
      }
    }
    cpu_exec(number);
  }
  return 0;
}

static int cmd_c(char *args)
{
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args)
{
  return -1;
}

static int cmd_p(char *args)
{
  bool islegal = true;
  char *arg = strtok(NULL, " ");
  uint32_t number = 23;
  if (arg != NULL)
    number = expr(arg, &islegal);
  else
    islegal = false;
  if (islegal)
  {
    printf("%u\n", number);
    return 0;
  }
  else
    return -23;
}

static int cmd_help(char *args);
static int cmd_d(char *args);

static int cmd_w(char *args) {
  char *arg = strtok(NULL, " ");
  if(arg == NULL) {
    printf("you should input a expr!\n");
    return 0;
  }
  else {
    add_watchpoint(arg);
    return 0;
  }
}

static struct
{
  const char *name;
  const char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display information about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},
    {"si", "run code by single step", cmd_si},
    {"info", "print out the state of register or watchpoint by adding r/w after the info command", cmd_info},
    {"x", "print out the memory", cmd_x},
    {"p", "calculate the expresion", cmd_p},
    {"w", "set a new watchpoint", cmd_w},
    {"d", "delete a watchpoint by giving a number", cmd_d},
    /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args)
{
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL)
  {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++)
    {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else
  {
    for (i = 0; i < NR_CMD; i++)
    {
      if (strcmp(arg, cmd_table[i].name) == 0)
      {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode()
{
  is_batch_mode = true;
}

void sdb_mainloop()
{
  if (is_batch_mode)
  {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;)
  {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL)
    {
      continue;
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end)
    {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++)
    {
      if (strcmp(cmd, cmd_table[i].name) == 0)
      {
        int number = cmd_table[i].handler(args);
        if (number == -1)
          return;
        else if (number == -23)
          printf("you should input right expresion!\n");

        break;
      }
    }

    if (i == NR_CMD)
    {
      printf("Unknown command '%s'\n", cmd);
    }
  }
}

void init_sdb()
{
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}

static int cmd_d(char *args) {
  char *arg = strtok(NULL, " ");
  if(arg == NULL) {
    printf("please give me a number!\n");
    return 0;
  }
  else {
    free_wp(atoi(arg));
    return 0;
  }
}