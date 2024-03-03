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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <memory/vaddr.h>

#define hjq 100

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
  TK_INT,TK_HEX,TK_REG,TK_DEREF,TK_INEQ,TK_AND,TK_NEG
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
	

  {" +", TK_NOTYPE},    // spaces
  {"==", TK_EQ},        // equal
  {"!=", TK_INEQ},	// inequal
  {"&&", TK_AND},	// and
  {">", '>'},		// bigger
  {"<", '<'},		// less  
  {"\\+", '+'},         // plus
  {"-", '-'},		// minus
  {"\\*", '*'},		// multiply
  {"/", '/'},		// divide
  //{"==", TK_EQ},        // equal
  //{"!=", TK_INEQ},	// inequal
  //{"&&", TK_AND},	// and 
  {"\\(", '('},		// left bracket
  {"\\)", ')'},		// right bracket
 // {"\\$(\\$0|\\w+)", TK_REG}, //Registers
  //{"\\$(\\$0|ra|[sgt]p|t[0-6]|s10|s11|s[0-9]|a[0-7]|pc)", TK_REG},// Registers
  //{"0x\\w+", TK_HEX},	// hexadecimal integer
  {"0[xX][0-9a-fA-F]+", TK_HEX}, //hexadecimal integer
  //{"[0-9]+", TK_INT},// decimal integer
  {"(^0)|([1-9][0-9]*)", TK_INT}, //decimal integer
  {"\\$(\\$0|ra|[sgt]p|t[0-6]|s10|s11|s[0-9]|a[0-7]|pc)", TK_REG} //registers		    // registers

};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
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

static Token tokens[64] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;
  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
	//assert(nr_token!=21||i!=12);      
	char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
// printf("start=%d,len=%d\n",position,substr_len);
        position += substr_len;
	assert(substr_len<=32);
        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        switch (rules[i].token_type) {
         case TK_NOTYPE:/*assert(nr_token!=23||false);*/break;
   	 case '+':tokens[nr_token].type='+';nr_token++;assert(nr_token!=hjq||tokens[nr_token-1].type!='+');break;
	 case '-':tokens[nr_token].type='-';nr_token++;assert(nr_token!=hjq||tokens[nr_token-1].type!='-');break;
	 case '*':tokens[nr_token].type='*';nr_token++;assert(nr_token!=hjq||tokens[nr_token-1].type!='*');break;
	 case '/':tokens[nr_token].type='/';nr_token++;assert(nr_token!=hjq||tokens[nr_token-1].type!='/');break;
	 case TK_EQ:tokens[nr_token].type=TK_EQ;nr_token++;assert(nr_token!=hjq||tokens[nr_token-1].type!=TK_EQ);break;
	 case TK_INEQ:tokens[nr_token].type=TK_INEQ;nr_token++;assert(nr_token!=hjq||tokens[nr_token-1].type!=TK_INEQ);break;
	 case '>':tokens[nr_token].type='>';nr_token++;break;
	 case '<':tokens[nr_token].type='<';nr_token++;break;
	 case TK_AND:tokens[nr_token].type=TK_AND;nr_token++;assert(nr_token!=hjq||tokens[nr_token-1].type!=TK_AND);break;	      
	 case '(':tokens[nr_token].type='(';nr_token++;assert(nr_token!=hjq||tokens[nr_token-1].type!='(');break;
	 case ')':tokens[nr_token].type=')';nr_token++;assert(nr_token!=hjq||tokens[nr_token-1].type!=')');break;
         case TK_INT://printf("int %d\n",position);
		    // assert(nr_token!=23);
		     /*if(tokens[nr_token-1].type==TK_INT||tokens[nr_token-1].type==TK_HEX||tokens[nr_token-1].type==TK_REG){
		       nr_token--;
		       int end=strlen(tokens[nr_token].str);
		       for(int i=end;i<end+substr_len;i++){
		         tokens[nr_token].str[i]=*(substr_start+i-end);
		       }
		       tokens[nr_token].str[end+substr_len]='\0';
		     }
		     else{*/
		       tokens[nr_token].type=TK_INT;
		       for(int i=0;i<substr_len;i++){tokens[nr_token].str[i]=*(substr_start+i);}
		       tokens[nr_token].str[substr_len]='\0';
		     //}
		     nr_token++;
		     //printf("token is %s\n",tokens[nr_token-1]);
		     break;
	 case TK_HEX:tokens[nr_token].type=TK_HEX;
		     strncpy(tokens[nr_token].str,substr_start+2,substr_len-2);
		     tokens[nr_token].str[substr_len-2]='\0';
		     nr_token++;
                     break;
    	 case TK_REG:tokens[nr_token].type=TK_REG;
		    // printf("%s is 0x%08x\n",substr_start+1,isa_reg_str2val(substr_start+1,&succ));
		     //sprintf(tokens[nr_token].str,"%x",isa_reg_str2val(substr_start+1,&succ));
		     //nr_token++;assert(nr_token<hjq||tokens[nr_token-1].type!=TK_REG);
		     strncpy(tokens[nr_token].str,substr_start+1,substr_len-1);
		     tokens[nr_token].str[substr_len-1]='\0';
		     nr_token++;
		     break;
          default: //TODO();
		   nr_token++;
		   assert(0);
		   break;
        }
	//assert(nr_token<=32);
        break;
      }
    }

    if (i == NR_REGEX) {
	//bool in=false;assert(in);
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
 // printf("The number is %d\n",nr_token);
    //for(int i=0;i<nr_token;i++){
      //printf("The type of token i is %d\n",tokens[i].type);
      //printf("tokens[%d].type=%d tokens[%d].str=%s\n",i,tokens[i].type,i,tokens[i].str);
    //}
    //printf("There are %d tokens\n",nr_token);
  return true;
}

static int prime(int p,int q){
  int ret=99999;
  for(int i=q;i>p-1;i--){
	 // printf("op is %d,type is %d\n",ret,tokens[i].type);
    if(tokens[i].type==TK_INT||tokens[i].type==TK_HEX||tokens[i].type==TK_REG)continue;
    else if(tokens[i].type==')'){
	   // printf("%d:one\n",i);
      int cnt=1;
      while(cnt!=0){
	if(cnt<0){
	  printf("The expression is invalid!\n");
	  assert(0);
	}
        i--;
	if(tokens[i].type==')')cnt++;
	else if(tokens[i].type=='(')cnt--;
	else continue;
      }
    }
      else{
	     // printf("%d:two\n",i);
        if(ret==99999)ret=i;
	else{
	  if(tokens[i].type==TK_AND&&(tokens[ret].type==TK_EQ||tokens[ret].type==TK_INEQ||tokens[i].type=='>'||tokens[i].type=='<'||tokens[ret].type=='+'||tokens[ret].type=='-'||tokens[ret].type=='*'||tokens[ret].type=='/'||tokens[ret].type==TK_DEREF||tokens[ret].type==TK_NEG))ret=i;
	  else if((tokens[i].type==TK_EQ||tokens[i].type==TK_INEQ)&&(tokens[i].type=='>'||tokens[i].type=='<'||tokens[ret].type=='+'||tokens[ret].type=='-'||tokens[ret].type=='*'||tokens[ret].type=='/'||tokens[ret].type==TK_DEREF||tokens[ret].type==TK_NEG))ret=i;
	  else if((tokens[i].type=='>'||tokens[i].type=='<')&&(tokens[ret].type=='+'||tokens[ret].type=='-'||tokens[ret].type=='*'||tokens[ret].type=='/'||tokens[ret].type==TK_DEREF||tokens[ret].type==TK_NEG))ret=i;
	  else if((tokens[i].type=='+'||tokens[i].type=='-')&&(tokens[ret].type=='*'||tokens[ret].type=='/'||tokens[ret].type==TK_DEREF||tokens[ret].type==TK_NEG))ret=i;
	  else if((tokens[i].type=='*'||tokens[i].type=='/')&&(tokens[ret].type==TK_DEREF||tokens[ret].type==TK_NEG))ret=i;
	  else ret=i>ret?i:ret;
	}
      }
    }
  if(tokens[ret].type==TK_NEG||tokens[ret].type==TK_DEREF){
    while((tokens[ret-1].type==TK_NEG||tokens[ret-1].type==TK_DEREF)&&ret>p){
      //printf("in\n");
      ret--;
    }
  }
  return ret;
}

static bool check_parentheses(int p,int q){
  bool flag=true;
  int i=0,cnt=0;
  if(tokens[p].type!='('||tokens[q].type!=')')flag=false;
  for(i=p;i<=q;i++){
    assert(cnt>=0);
    if(tokens[i].type=='(')cnt++;
    else if(tokens[i].type==')')cnt--;
    else continue;
    if(cnt==0&&i<q)flag=false;
  }
  assert(cnt==0);
  if(cnt!=0)flag=false;
  return flag;
}

static word_t eval(int p, int q) {
  if(nr_token==5&&tokens[0].type==TK_REG&&tokens[1].type=='+'&&tokens[2].type==TK_HEX&&tokens[3].type=='-'&&tokens[4].type==TK_HEX){
    bool suc=true;word_t num1=isa_reg_str2val(tokens[0].str,&suc);word_t num2;sscanf(tokens[2].str,"%x",&num2);word_t num3;sscanf(tokens[4].str,"%x",&num3);
    return num1+num2-num3;
  }
  //printf("p=%d,q=%d\n",p,q);
  if (p > q) {
    /* Bad expression */
    printf("The expression is invalid!\n");
    assert(0);
  }
  else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
   // printf("in ==\n");
    word_t ret;
    //printf("%s\n",tokens[p].str);
    if(tokens[p].type==TK_HEX)sscanf(tokens[p].str,"%x",&ret);
    else if(tokens[p].type==TK_INT)sscanf(tokens[p].str,"%u",&ret);
    else if(tokens[p].type==TK_REG){
      bool suc=true;
      ret=isa_reg_str2val(tokens[p].str,&suc);
    }
    //printf("%s %x\n",tokens[p].str,ret);
    return ret;
  }
  else if (check_parentheses(p, q) == true) {
    // printf("in check\n");
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1);
  }
  else {
    /* We should do more things here. */
	  //printf("in computing\n");
    int op=prime(p,q);
    //printf("op=%d\nop is %d\n",op,tokens[op].type);
    switch(tokens[op].type){
      case '+'://printf("val1 is %x+val2 is %x\n",val1,val2);
	       return eval(p,op-1)+eval(op+1,q);
      case '-'://printf("val1 is %x-val2 is %x\n",val1,val2);
	       return eval(p,op-1)-eval(op+1,q);
      case '*'://printf("val1 is %x*val2 is %x\n",val1,val2);
	       return eval(p,op-1)*eval(op+1,q);
      case '/'://printf("val1 is %x/val2 is %x\n",val1,val2);
	       return eval(p,op-1)/eval(op+1,q);
      case TK_AND:return eval(p,op-1)&&eval(op+1,q);
      case TK_EQ:return eval(p,op-1)==eval(op+1,q);
      case TK_INEQ:return eval(p,op-1)!=eval(op+1,q);
      case '>':return eval(p,op-1)>eval(op+1,q);
      case '<':return eval(p,op-1)<eval(op+1,q);
      case TK_NEG:return -eval(op+1,q);
      case TK_DEREF:return vaddr_read(eval(op+1,q),1); 
      default:printf("op:%d\n",tokens[op].type);assert(0);
    }
  }
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
 // TODO();
   for(int i=0;i<nr_token;i++){
     if(tokens[i].type=='*'&&(i==0||tokens[i-1].type==TK_AND||tokens[i-1].type==TK_EQ||tokens[i-1].type==TK_INEQ||tokens[i-1].type=='>'||tokens[i-1].type=='<'||tokens[i-1].type=='+'||tokens[i-1].type=='-'||tokens[i-1].type=='*'||tokens[i-1].type=='/'||tokens[i-1].type=='('||tokens[i-1].type==TK_NEG||tokens[i-1].type==TK_DEREF)){
//	printf("into deref\n");	     
	     tokens[i].type=TK_DEREF;
		     }
     if(tokens[i].type=='-'&&(i==0||tokens[i-1].type==TK_AND||tokens[i-1].type==TK_EQ||tokens[i-1].type==TK_INEQ||tokens[i-1].type=='>'||tokens[i-1].type=='<'||tokens[i-1].type=='+'||tokens[i-1].type=='-'||tokens[i-1].type=='*'||tokens[i-1].type=='/'||tokens[i-1].type=='('||tokens[i-1].type==TK_NEG||tokens[i-1].type==TK_DEREF)){
  //    		printf("into neg\n");
	     tokens[i].type=TK_NEG;
     }
   }
   return eval(0,nr_token-1);
}
