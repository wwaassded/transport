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
#include <memory/paddr.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <string.h>

static word_t bra[20];
static int bra_len = 0;
enum
{
  TK_NOTYPE = 256,
  TK_PLUS,
  TK_MI,
  TK_MUL,
  TK_EQ,
  TK_TNUMBER,
  TK_HEX,
  TK_DECODE,
  TK_REG,
  /* TODO: Add more token types */

};

static struct rule
{
  const char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE}, // spaces
    {"\\+", '+'},      // plus
    {"/", '/'},
    {"-", '-'},
    {"\\*", '*'},
    {"0x[0-9]{8}", TK_HEX},
    {"[0-9]+", TK_TNUMBER},
    {
        "\\(",
        '(',
    },
    {"\\)", ')'},
    {"\\$[a-z]{0,2}[1-9]{0,1}[0-9]{0,1}", TK_REG},
    {"==", TK_EQ},
};

#define NR_REGEX 11

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++)
  {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0)
    {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token
{
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(char *e)
{
  int position = 0;
  int i;
  regmatch_t pmatch;
  nr_token = 0;
  bra_len = 0;
  u_int32_t fake_stack[100];
  int pos = 0;
  bool flag;
  fake_stack[pos++] = 114514;
  while (e[position] != '\0')
  {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; ++i)
    {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
      {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
        if (i != 0)
        {
          Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
          tokens[nr_token].type = rules[i].token_type;
          position += substr_len;
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          if (substr_len < 32)
            tokens[nr_token].str[substr_len] = '\0';
          else
            tokens[nr_token].str[31] = '\0';
          ++nr_token;
        }

        switch (rules[i].token_type)
        {
        case '(':
        {
          fake_stack[pos++] = nr_token - 1;
          break;
        }
        case ')':
        {
          flag = fake_stack[--pos] != 114514;
          if (!flag)
            return false;
          bra[bra_len++] = fake_stack[pos];
          break;
        }
        }
        break;
      }
      if (i == NR_REGEX)
      {
        printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
        return false;
      }
    }
  }
  if (pos != 1)
    return false;
  return true;
}

u_int32_t eval(Token pToken[], u_int32_t left, u_int32_t right, bool *success)
{
  if (left > right)
  {
    *success = false;
    return -1;
  }
  else if (left == right)
  {
    switch (pToken[left].type)
    {

    case TK_TNUMBER:
    {
      return atoi(pToken[left].str);
    }

    case TK_HEX:
    {
      int i = 0;
      u_int32_t ans = 0;
      for (i = 2; pToken[left].str[i] != '\0'; ++i)
      {
        char tmp = pToken[left].str[i];
        if (tmp >= '0' && tmp <= '9')
          ans = ans * 16 + pToken[left].str[i] - '0';
        else if (tmp >= 'a' && tmp <= 'f')
          ans = ans * 16 + tmp - 'a' + 10;
        else if (tmp >= 'A' && tmp <= 'F')
          ans = ans * 16 + tmp - 'A' + 10;
        else
        {
          printf("format error !\n");
          *success = false;
          return -1;
        }
      }
      return ans;
    }

    case TK_REG:
    {
      return isa_reg_str2val(pToken[left].str + 1, success);
    }
    default:
    {
      *success = false;
      return -1;
    }
    }
  }
  else
  {
    if (pToken[left].type == '(' && pToken[right].type == ')' && bra[bra_len - 1] == left)
    {
      --bra_len;
      return eval(pToken, left + 1, right - 1, success);
    }
    else
    {
      int is_in_bra = 0;
      int op_pos = -100;
      int op_tmp = left;
      for (op_tmp = left; op_tmp <= right; ++op_tmp)
      {
        if (pToken[op_tmp].type == '(')
          ++is_in_bra;
        else if (pToken[op_tmp].type == ')')
          --is_in_bra;
        else if (is_in_bra == 0 && pToken[op_tmp].type != TK_TNUMBER)
        {
          if (pToken[op_tmp].type == '+' || pToken[op_tmp].type == '-')
            op_pos = op_tmp;
          else if (pToken[op_tmp].type == '*' || pToken[op_tmp].type == '/')
          {
            if (op_pos == -100 || pToken[op_pos].type == '*' || pToken[op_pos].type == '/' || pToken[op_pos].type == 114514)
              op_pos = op_tmp;
          }
          else if (pToken[op_tmp].type == 114514)
          {
            if (op_pos == -100 || pToken[op_pos].type == TK_DECODE)
              op_pos = op_tmp;
          }
          else if (pToken[op_tmp].type == TK_DECODE)
          {
            if (op_pos == -100)
              op_pos = op_tmp;
          }
        }
      }
      if (pToken[op_pos].type == 114514)
        return -1 * eval(pToken, op_pos + 1, right, success);
      else if (pToken[op_pos].type == TK_DECODE)
        return paddr_read(eval(pToken, op_pos + 1, right, success), 1);
      int right_number = eval(pToken, op_pos + 1, right, success);
      int left_number = eval(pToken, left, op_pos - 1, success);
      switch (pToken[op_pos].type)
      {
      case '+':
      {
        return left_number + right_number;
      }
      case '-':
      {
        return left_number - right_number;
      }
      case '*':
      {
        return left_number * right_number;
      }
      case '/':
      {
        if (right_number == 0)
        {
          *success = false;
          printf("a number can not devide zero!\n");
          return -1;
        }
        return left_number / right_number;
      }
      default:
        return -1;
      }
    }
  }
}

u_int32_t expr(char *e, bool *success)
{
  if (!make_token(e))
  {
    *success = false;
    return 0;
  }
  else
  {
    int i = 0;
    for (i = 0; i < nr_token; ++i)
    {
      if (tokens[i].type == '-')
      {
        if (i == 0)
          tokens[i].type = 114514;
        else if (tokens[i - 1].type != ')' && tokens[i - 1].type != TK_TNUMBER)
          tokens[i].type = 114514;
      }
      else if (tokens[i].type == '*')
      {
        if (i == 0 || (tokens[i - 1].type != ')' && tokens[i].type != TK_TNUMBER))
        {
          if (i + 1 < nr_token && tokens[i + 1].type == TK_HEX)
            tokens[i].type = TK_DECODE;
          else
          {
            printf("format error!\n");
            *success = false;
            return -1;
          }
        }
      }
    }
    return eval(tokens, 0, nr_token - 1, success);
  }
}
