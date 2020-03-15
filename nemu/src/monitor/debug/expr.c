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
  TK_NOTYPE = 256, TK_EQ, TK_NUM,TK_HEX,TK_REG,TK_NEQ,TK_AND,TK_OR

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
  {"0x[1-9A-Fa-f][0-9A-Fa-f]*",TK_HEX},
  {"0|[1-9][0-9]*",TK_NUM},
  {"\\$(eax|ecx|edx|ebx|esp|ebp|esi|edi|eip|ax|cx|dx|bx|sp|bp|si|di|al|cl|dl|bl|ah|ch|dh|bh)",TK_REG},
  {"==", TK_EQ},         // equal
  {"!=",TK_NEQ},
  {"&&",TK_AND},
  {"\\|\\|",TK_OR},
  {"!",'!'},
  {"\\+", '+'},         // plus 第一次转义\,第二次转义+
  {"-",'-'},            
  {"\\*",'*'},
  {"\\/",'/'},
  {"\\(",'('},
  {"\\)",')'}
  
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
          if(rules[i].token_type==TK_HEX)
          {
            strncpy(tokens[nr_token].str,substr_start+2,substr_len);
            *(tokens[nr_token].str+substr_len)='\0';
          }
          else if(rules[i].token_type==TK_REG)
          {
            strncpy(tokens[nr_token].str,substr_start+1,substr_len);
            *(tokens[nr_token].str+substr_len)='\0';
          }
          else
          {
            strncpy(tokens[nr_token].str,substr_start,substr_len);
            *(tokens[nr_token].str+substr_len)='\0';
          }
        switch (rules[i].token_type) 
        {
          case TK_REG:
          case TK_HEX:
          case TK_AND:
          case TK_OR:
          case TK_EQ:
          case TK_NEQ:
          case '+':
          case '-':
          case '*':
          case '/':
          case '!':
          case TK_NUM:   
          case '(':
          case ')':
          tokens[nr_token].type=rules[i].token_type;
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
      int l_bracket=0;//初始左括号数量，出现右括号会使该值减一
      if(p<q-1)
      {
         int i;
         for(i=p+1;i<q;i++)
         {
            if(tokens[i].type=='(')
            l_bracket++;
            if(tokens[i].type==')')
            l_bracket--;
            if(l_bracket<0)
            return false;
         }
         if(l_bracket!=0)
           return false;
         return true; 
      }
   }
   return false;
}
int dominabtOp(int p,int q)
{
  int op=-1;
  int n0=0; //&& ||
  int n1=0; //== !=
  int n2=0; //+ -
  int n3=0; //* /
  int n4=0; //- * !
  int i;
  int r_bracket=0;
  for(i=p;i<=q;i++)
  {
     if(tokens[i].type=='(')
        r_bracket++;
     if(tokens[i].type==')')
        r_bracket--;
     if(r_bracket<0)
     {
        Bad=Bad_Expression;
        return 0;
     }
     if((tokens[i].type==TK_OR||tokens[i].type==TK_AND)&&r_bracket==0)
        n0++;
     if((tokens[i].type==TK_NEQ||tokens[i].type==TK_EQ)&&r_bracket==0)
        n1++;
     if((tokens[i].type=='+'||tokens[i].type=='-')&&r_bracket==0)
        n2++;
     if((tokens[i].type=='*'||tokens[i].type=='/')&&r_bracket==0)
        n3++;
     if(tokens[i].type=='!'&&r_bracket==0)
        n4++;
  }
  if(n4!=0)
  {
     int j;
     int r_bracket=0;//用来表示右括号数量，遇到左括号减一，遇到右括号加一
     for(j=q;j>p-1;j--)
     {
        if(tokens[j].type=='(')
        r_bracket--;
        if(tokens[j].type==')')
        r_bracket++; 
        if(tokens[j].type=='!'&&r_bracket==0)
        {
           op=j;
           break;
        }
     }
   }
   if(n3!=0) //* /
   {
     int j;
     int r_bracket=0;//用来表示右括号数量，遇到左括号减一，遇到右括号加一
     for(j=q;j>p-1;j--)
     {
        if(tokens[j].type=='(')
        r_bracket--;
        if(tokens[j].type==')')
        r_bracket++; 
        if((tokens[j].type=='*'||tokens[j].type=='/')&&r_bracket==0)
        {
           if(j>p&&tokens[j].type=='*')
           {
              if(tokens[j-1].type!=TK_NUM&&tokens[j-1].type!=')')
              continue;
           }
           op=j;
           break;
        }
      }
    }
    if(n2!=0) //+ -
    {
      int j;
      int r_bracket=0;//用来表示右括号数量，遇到左括号减一，遇到右括号加一
      for(j=q;j>p-1;j--)
      {
         if(tokens[j].type=='(')
         r_bracket--;
         if(tokens[j].type==')')
         r_bracket++; 
         if((tokens[j].type=='+'||tokens[j].type=='-')&&r_bracket==0)
         {
            if(j>p&&tokens[j].type=='-')
            {
               if(tokens[j-1].type!=TK_NUM&&tokens[j-1].type!=')')
               continue;
            }
            op=j;
            break;
         }
       }
     }
     if(n1!=0)
     {
       int j;
       int r_bracket=0;//用来表示右括号数量，遇到左括号减一，遇到右括号加一
       for(j=q;j>p-1;j--)
       {
          if(tokens[j].type=='(')
          r_bracket--;
          if(tokens[j].type==')')
          r_bracket++; 
          if((tokens[j].type==TK_EQ||tokens[j].type==TK_NEQ)&&r_bracket==0)
          {
             op=j;
             break;
          }
       }
     }
     if(n0!=0)
     {
       int j;
       int r_bracket=0;//用来表示右括号数量，遇到左括号减一，遇到右括号加一
       for(j=q;j>p-1;j--)
       {
          if(tokens[j].type=='(')
          r_bracket--;
          if(tokens[j].type==')')
          r_bracket++; 
          if((tokens[j].type==TK_OR||tokens[j].type==TK_AND)&&r_bracket==0)
          {
             op=j;
             break;
          }
       }
     }
     return op;
}

int32_t eval(int p,int q)
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
     int op;
     op=dominabtOp(p,q);
     switch(tokens[op].type)
     {
        case '+':
          return eval(p,op-1)+eval(op+1,q);
          break;
        case '-':
          if(op==p)
          {
            return -eval(p+1,q);
          }
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
