#include "nemu.h"
#include <stdlib.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#define Bad_Expression INT32_MAX-1
//枚举类型第一个值确定后，后续依次递增
enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUM

  /* TODO: Add more token types */

};
//单个字符就用ASCII码
static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces 闭包
  {"\\+", '+'},         // plus 第一次转义\,第二次转义+
  {"==", TK_EQ},         // equal
  {"-",'-'},            
  {"\\*",'*'},
  {"\\/",'/'},
  {"\\(",'('},
  {"\\)",')'},
  {"[0-9]+",TK_NUM}
  
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )//正则表达式个数

static regex_t re[NR_REGEX];
//regex_t是一个结构体数据结构，用来存放编译后的正则表达式

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
//初始化正则表达式
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    //编译正则表达式，编译成功返回0
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;
int Bad=0; 

static bool make_token(char *e)
{
  int position = 0;
  int i;
  regmatch_t pmatch;
  nr_token = 0;
  while (e[position] != '\0') 
  {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++)
    {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so==0)
      {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         * 末尾加\0
         */
        if(substr_len>32)
           assert(0);
        if(rules[i].token_type!=TK_NOTYPE)
        {
        strncpy(tokens[nr_token].str,substr_start,substr_len);
        *(tokens[nr_token].str+substr_len)='\0';
        switch (rules[i].token_type) 
        {
          case TK_NOTYPE:
             tokens[nr_token].type=1;
             break;
          case '+':
             tokens[nr_token].type=2;
             break;
          case TK_EQ:
             tokens[nr_token].type=3;
             break;
          case '-':
             tokens[nr_token].type=2;
             break;
          case '*':
             tokens[nr_token].type=4;
             break;
          case '/':
             tokens[nr_token].type=4;
             break;
          case TK_NUM:
             tokens[nr_token].type=5;
             break;
          case '(':
             tokens[nr_token].type=6;
             break;
          case ')':
             tokens[nr_token].type=7;
             break;
          default: break;
        }
        nr_token++;
        }
        break;
      }
    }
    if (i == NR_REGEX) 
    {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  return true;
}

bool check_parentheses(int p, int q)
{
   if(tokens[p].type==6&&tokens[q].type==7)
   {
      int ori=0;//初始左括号数量，出现右括号会使该值减一
      if(p<q-1)
      {
         int i;
         for(i=p+1;i<q;i++)
         {
            if(tokens[i].type==6)
            ori++;
            if(tokens[i].type==7)
            ori--;
            if(ori<0)
            return false;
         }
         if(ori!=0)
           return false;
         return true; 
      }
   }
   return false;
}

uint32_t eval(int p,int q)
{
  if(p>q)
  {
     Bad=Bad_Expression;
     return 0;
  }
  else if(p==q)
  {
     return atoi(tokens[p].str);
  }
  else if(check_parentheses(p,q))
  {
     return eval(p+1,q-1);
  }
  else
  {
     int op=0;
     int n0=0;
     int n1=0;
     int i;
     int ori=0;//用来表示左括号数量，遇到右括号减一，遇到左括号加一
     for(i=p;i<=q;i++)
     { 
        if(tokens[i].type==6)
        ori++;
        if(tokens[i].type==7)
        ori--;
        if(ori<0)
        {
          Bad=Bad_Expression;
          return 0;
        }
        if(tokens[i].type==2&&ori==0)
        n0++;
        else if(tokens[i].type==4&&ori==0)
        n1++;
     }
     if(n0!=0)
     {
        int j;
        int ori1=0;//用来表示右括号数量，遇到左括号减一，遇到右括号加一
        for(j=q;j>p-1;j--)
        {
           if(tokens[j].type==6)
           ori1--;
           if(tokens[j].type==7)
           ori1++; 
           if(tokens[j].type==2&&ori1==0)
           {
              op=j;
              break;
           }
        }
     }
     else if(n1!=0)
     {
        int j;
        int ori1=0;//用来表示右括号数量，遇到左括号减一，遇到右括号加一
        for(j=q;j>p-1;j--)
        {
           if(tokens[j].type==6)
           ori1--;
           if(tokens[j].type==7)
           ori1++; 
           if(tokens[j].type==4&&ori1==0)
           {
              op=j;
              break;
           }
        }
     }
     switch(tokens[op].str[0])
     {
        case '+':
          return eval(p,op-1)+eval(op+1,q);
          break;
        case '-':
          return eval(p,op-1)-eval(op+1,q);
          break;
        case '*':
          return eval(p,op-1)*eval(op+1,q);
          break;
        case '/':
          return eval(p,op-1)/eval(op+1,q);
          break;
        default:
          return 0;
     }
  }
}
uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  //TODO();
  int terr=eval(0,nr_token-1);
  if(Bad==Bad_Expression)
  {
    *success = false;
    printf("Bad Expression\n");
    return 0;
  }
  printf("%d\n",terr);
  return 0;
}
