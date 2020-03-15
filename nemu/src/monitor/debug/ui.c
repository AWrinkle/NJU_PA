#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"
#include "cpu/reg.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

//调试器添加的功能
static int cmd_si(char *args)
{
   uint64_t n=1;
   //如果args非空，将其转化为数字
   if(args!=NULL)
   {
      n=atoi(args);
      //args非数字或n=0时
      if(n==0)
      {
        printf("excute no instruction\n");
        return 0;
      }
   }
   cpu_exec(n);
   return 0;
}

static int cmd_info(char *args)
{
   if(args==NULL)
   {
      printf("input more specific instruction\n");
      return 0;  
   }
   else
   {
      if(strlen(args)==1&&args[0]=='r')
      {
         int i;
         //32
         for(i=0;i<8;i++)
         {
           printf("%s  0x%x\n",regsl[i],reg_l(i));
         }
         //eip
         printf("eip  0x%x\n",cpu.eip);
         //16
         for(i=0;i<8;i++)
         {
           printf("%s   0x%x\n",regsw[i],reg_w(i));
         }
         //8
         for(i=0;i<8;i++)
         {
           printf("%s   0x%x\n",regsb[i],reg_b(i));
         }
      }
      /*
      else if(strlen(args)==1&&args[0]=='w')
      {
         
      }
      */
      return 0;
   }
}

static int cmd_p(char *args)
{
   bool a;
   int terr=expr(args,&a);
   if(a)
   printf("%d\n",terr);
   return 0;
}
static int cmd_x(char *args)
{
   int nLen=0;
   vaddr_t addr;
   int nRet=sscanf(args,"%d 0x%x",&nLen,&addr);
   if(nRet<=0)
   {
      printf("args error\n");
      return 0;
   }
   int terr=addr;
   char terr_c[30];
   sprintf(terr_c,"%d",terr);
   printf("%s",terr_c);
   printf("Memory:");
   for(int i=0;i<nLen;i++)
   {
      if(i%4==0)
        printf("\n0x%x: 0x%02x",addr+i,vaddr_read(addr+i,1));
      else
        printf(" 0x%02x",vaddr_read(addr+i,1));
   }
   printf("\n");
   return 0;
}
static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  {"si","pause after excuting N instruction", cmd_si },
  {"info","print information about register or watchpoint",cmd_info},
  {"p","calculate the result of expression",cmd_p},
  {"x","Scanning memory",cmd_x},
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  //先判断是不是批处理模式
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();//读一条string，返回首字符指针
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");//以空格对字符串进行分割，返回第一个分割的子字符串
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    //检测分割后是否有两个以上的字符串
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
