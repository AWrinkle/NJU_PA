#include "nemu.h"
#include <stdlib.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
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
  {"[0-9]",TK_NUM}
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
int N=0;

static bool make_token(char *e)
{
  int position = 0;
  int i;
  regmatch_t pmatch;
  printf("%s",e);
  nr_token = 0;
  int terr=0;
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
        printf("breakpointa");
        strncpy(tokens[terr].str,substr_start,substr_len);
        *(tokens[terr].str+substr_len)='\0';
        switch (rules[i].token_type) 
        {
          case TK_NOTYPE:
             tokens[terr].type=1;
             break;
          case '+':
             tokens[terr].type=2;
             break;
          case TK_EQ:
             tokens[terr].type=3;
             break;
          case '-':
             tokens[terr].type=2;
             break;
          case '*':
             tokens[terr].type=4;
             break;
          case '/':
             tokens[terr].type=4;
             break;
          case TK_NUM:
             tokens[terr].type=5;
             break;
          default: break;
        }
        terr++;
        nr_token++;
        printf("%d",terr);
        break;
       
      }
    }
    if (i == NR_REGEX) 
    {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  //nr_token=terr;
  N=terr;
  printf("return");
  return true;
}

uint32_t eval(int p,int q)
{
  if(p>q)
  {
     //assert(0);
     return 0;
  }
  else if(p==q)
  {
     return atoi(tokens[p].str);
  }
  else
  {
     int op=0;
     int n0=0;
     int n1=0;
     int i;
     for(i=0;i<N;i++)
     {
        if(tokens[i].type==2)
        n0++;
        else if(tokens[i].type==4)
        n1++;
     }
     if(n0!=0)
     {
        int j;
        for(j=N-1;j>-1;j--)
        {
           if(tokens[j].type==2)
           {
              op=j;
              break;
           }
        }
     }
     else if(n1!=0)
     {
        int j;
        for(j=N-1;j>-1;j--)
        {
           if(tokens[j].type==4)
           {
              op=j;
              break;
           }
        }
     }
     int val1=eval(p,op-1);
     int val2=eval(op+1,q);
     switch(tokens[op].str[0])
     {
        case '+':
          return val1+val2;
          break;
        case '-':
          return val1-val2;
          break;
        case '*':
          return val1*val2;
          break;
        case '/':
          return val1/val2;
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
  printf("test");
  int terr=eval(0,N-1);
  printf("%d",terr);
  return 0;
}
