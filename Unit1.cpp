/*** PL0 COMPILER WITH CODE GENERATION ***/
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "Unit1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------

const  AL    =  10;  /* LENGTH OF IDENTIFIERS */
const  NORW  =  14+5;  /* # OF RESERVED WORDS */
const  TXMAX = 100;  /* LENGTH OF IDENTIFIER TABLE */
const  NMAX  =  14;  /* MAX NUMBER OF DEGITS IN NUMBERS */
const  AMAX  =2047;  /* MAXIMUM ADDRESS */
const  LEVMAX=   3;  /* MAX DEPTH OF BLOCK NESTING */
const  CXMAX = 200;  /* SIZE OF CODE ARRAY */

//符号
typedef enum { NUL,     IDENT,    NUMBER,    PLUS,     MINUS,   TIMES,
	      SLASH,    ODDSYM,   EQL,       NEQ,      LSS,     LEQ,
              GTR,      GEQ,      LPAREN,    RPAREN,   COMMA,   SEMICOLON,
              PERIOD,   BECOMES,  BEGINSYM,  ENDSYM,  IFSYM,    THENSYM,
	      WHILESYM, WRITESYM, READSYM,   DOSYM,   CALLSYM,	CONSTSYM,
              VARSYM,   PROCSYM,  PROGSYM,   ELSESYM, FORSYM,	TOSYM,
              DOWNTOSYM,RETURNSYM,PLUSBECOMES,MINUSBECOMES,PLUSPLUS, MINUSMINUS,
              NOTESYM
        } SYMBOL;
char *SYMOUT[] = {"NUL", "IDENT", "NUMBER", "PLUS", "MINUS", "TIMES",
	    "SLASH", "ODDSYM", "EQL", "NEQ", "LSS", "LEQ", "GTR", "GEQ",
	    "LPAREN", "RPAREN", "COMMA", "SEMICOLON", "PERIOD",
	    "BECOMES", "BEGINSYM", "ENDSYM", "IFSYM", "THENSYM",
	    "WHILESYM", "WRITESYM", "READSYM", "DOSYM", "CALLSYM",
	    "CONSTSYM", "VARSYM", "PROCSYM", "PROGSYM", "ELSESYM",
            "FORSYM",  "TOSYM", "DOWNTOSYM", "RETURNSYM", "PLUSBECOMES",
            "MINUSBECOMES",	"PLUSPLUS", "MINUSMINUS", "NOTESYM"};


typedef  int *SYMSET; // SET OF SYMBOL;
typedef  char ALFA[11];

typedef  enum { CONSTANT, VARIABLE, PROCEDUR } OBJECTS ;
typedef  enum { LIT, OPR, LOD, STO, CAL, INI, JMP, JPC } FCT;
typedef struct {
	 FCT F;     /*FUNCTION CODE*/
	 int L; 	/*0..LEVMAX  LEVEL*/
	 int A;     /*0..AMAX    DISPLACEMENT ADDR*/
} INSTRUCTION;
	  /* LIT O A -- LOAD CONSTANT A             */
	  /* OPR 0 A -- EXECUTE OPR A               */
	  /* LOD L A -- LOAD VARIABLE L,A           */
	  /* STO L A -- STORE VARIABLE L,A          */
	  /* CAL L A -- CALL PROCEDURE A AT LEVEL L */
	  /* INI 0 A -- INCREMET T-REGISTER BY A    */
	  /* JMP 0 A -- JUMP TO A                   */
	  /* JPC 0 A -- JUMP CONDITIONAL TO A       */
char   CH;  /*LAST CHAR READ*/
SYMBOL SYM; /*LAST SYMBOL READ*/
ALFA   ID;  /*LAST IDENTIFIER READ*/
int    NUM; /*LAST NUMBER READ*/
int    CC;  /*CHARACTER COUNT*/
int    LL;  /*LINE LENGTH*/
int    CX;  /*CODE ALLOCATION INDEX*/
char   LINE[81];
INSTRUCTION  CODE[CXMAX];
ALFA    KWORD[NORW+1];
SYMBOL  WSYM[NORW+1];
SYMBOL  SSYM['^'+1];
ALFA    MNEMONIC[9];
SYMSET  DECLBEGSYS, STATBEGSYS, FACBEGSYS;

struct {
  ALFA NAME;
  OBJECTS KIND;
  union {
    int VAL;   /*CONSTANT*/
    struct { int LEVEL,ADR,SIZE; } vp;  /*VARIABLE,PROCEDUR:*/
  };
} TABLE[TXMAX];

FILE *FIN,*FOUT;
int ERR;

void EXPRESSION(SYMSET FSYS, int LEV, int &TX);
void TERM(SYMSET FSYS, int LEV, int &TX);
//---------------------------------------------------------------------------
int SymIn(SYMBOL SYM, SYMSET S1) {
  return S1[SYM];
}
//---------------------------------------------------------------------------
SYMSET SymSetUnion(SYMSET S1, SYMSET S2) {
  SYMSET S=(SYMSET)malloc(sizeof(int)*43);
  for (int i=0; i<43; i++)
	if (S1[i] || S2[i]) S[i]=1;
	else S[i]=0;
  return S;
}
//---------------------------------------------------------------------------
SYMSET SymSetAdd(SYMBOL SY, SYMSET S) {
  SYMSET S1;
  S1=(SYMSET)malloc(sizeof(int)*43);
  for (int i=0; i<43; i++) S1[i]=S[i];
  S1[SY]=1;
  return S1;
}
//---------------------------------------------------------------------------
SYMSET SymSetNew(SYMBOL a) {
  SYMSET S; int i,k;
  S=(SYMSET)malloc(sizeof(int)*43);
  for (i=0; i<43; i++) S[i]=0;
  S[a]=1;
  return S;
}
//---------------------------------------------------------------------------
SYMSET SymSetNew(SYMBOL a, SYMBOL b) {
  SYMSET S; int i,k;
  S=(SYMSET)malloc(sizeof(int)*43);
  for (i=0; i<43; i++) S[i]=0;
  S[a]=1;  S[b]=1;
  return S;
}
//---------------------------------------------------------------------------
SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c) {
  SYMSET S; int i,k;
  S=(SYMSET)malloc(sizeof(int)*43);
  for (i=0; i<43; i++) S[i]=0;
  S[a]=1;  S[b]=1; S[c]=1;
  return S;
}
//---------------------------------------------------------------------------
SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c, SYMBOL d) {
  SYMSET S; int i,k;
  S=(SYMSET)malloc(sizeof(int)*43);
  for (i=0; i<43; i++) S[i]=0;
  S[a]=1;  S[b]=1; S[c]=1; S[d]=1;
  return S;
}
//---------------------------------------------------------------------------
SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c, SYMBOL d,SYMBOL e) {
  SYMSET S; int i,k;
  S=(SYMSET)malloc(sizeof(int)*43);
  for (i=0; i<43; i++) S[i]=0;
  S[a]=1;  S[b]=1; S[c]=1; S[d]=1; S[e]=1;
  return S;
}
//---------------------------------------------------------------------------
SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c, SYMBOL d,SYMBOL e, SYMBOL f) {
  SYMSET S; int i,k;
  S=(SYMSET)malloc(sizeof(int)*43);
  for (i=0; i<43; i++) S[i]=0;
  S[a]=1;  S[b]=1; S[c]=1; S[d]=1; S[e]=1; S[f]=1;
  return S;
}
//---------------------------------------------------------------------------
SYMSET SymSetNULL() {
  SYMSET S; int i,n,k;
  S=(SYMSET)malloc(sizeof(int)*43);
  for (i=0; i<43; i++) S[i]=0;
  return S;
}
//---------------------------------------------------------------------------
void Error(int n) {
  String s = "***"+AnsiString::StringOfChar(' ', CC-1)+"^";
  Form1->printls(s.c_str(),n);   fprintf(FOUT,"%s%d\n", s.c_str(), n);
  ERR++;
} /*Error*/
//---------------------------------------------------------------------------
void GetCh() {
  if (CC==LL) {
    if (feof(FIN)) {
	  Form1->printfs("PROGRAM INCOMPLETE");
	  fprintf(FOUT,"PROGRAM INCOMPLETE\n");
	  fclose(FOUT);
	  exit(0);
	}
	LL=0; CC=0;
	CH=' ';
	while (!feof(FIN) && CH!=10)
      { CH=fgetc(FIN);  LINE[LL++]=CH; }
	LINE[LL-1]=' ';  LINE[LL]=0;
    String s=IntToStr(CX);
    while(s.Length()<3) s=" "+s;
    s=s+" "+LINE;
	Form1->printfs(s.c_str());
    fprintf(FOUT,"%s\n",s);
  }
  CH=LINE[CC++];
} /*GetCh()*/
//---------------------------------------------------------------------------
void GetSym() {
  int i,J,K;   ALFA  A;
  while (CH<=' ') GetCh();
  if (CH>='A' && CH<='Z') { /*ID OR RESERVED WORD*/
    K=0;
	do {
	  if (K<AL) A[K++]=CH;
	  GetCh();
	}while((CH>='A' && CH<='Z')||(CH>='0' && CH<='9'));
	A[K]='\0';
	strcpy(ID,A); i=1; J=NORW;
	do {
	  K=(i+J) / 2;
	  if (strcmp(ID,KWORD[K])<=0) J=K-1;
	  if (strcmp(ID,KWORD[K])>=0) i=K+1;
	}while(i<=J);
	if (i-1 > J) SYM=WSYM[K];
	else SYM=IDENT;
  }
  else if (CH>='0' && CH<='9') { /*NUMBER*/
      K=0; NUM=0; SYM=NUMBER;
	  do {
	    NUM=10*NUM+(CH-'0');
		K++; GetCh();
      }while(CH>='0' && CH<='9');
	  if (K>NMAX) Error(30);
    }
  else if (CH==':') {
        GetCh();
        if (CH=='=') { SYM=BECOMES; GetCh(); }
        else SYM=NUL;
        }
  else if (CH=='<') {  /* THE FOLLOWING TWO CHECK WERE ADDED BECAUSE ASCII DOES NOT HAVE A SINGLE CHARACTER FOR <= OR >= */
        GetCh();
        if (CH=='>') { SYM=NEQ; GetCh(); }
        else if (CH=='=') { SYM=LEQ; GetCh(); }
        else SYM=LSS;
        }
  else if (CH=='>') {
        GetCh();
        if (CH=='=') { SYM=GEQ; GetCh(); }
        else SYM=GTR;
        }

  else if(CH=='+'){
        GetCh();
        if (CH=='=') { SYM=PLUSBECOMES; GetCh(); }
        else if (CH=='+') { SYM=PLUSPLUS; GetCh(); }
        else {SYM=PLUS;}
        }
  else if(CH=='-'){
        GetCh();
        if (CH=='=') { SYM=MINUSBECOMES; GetCh(); }
        else if (CH=='-') { SYM=MINUSMINUS; GetCh(); }
        else {SYM=MINUS;}
        }
  else if(CH=='/'){
        GetCh();
        if (CH=='/') {
                while(CC!=LL)GetCh();
        }
        else if(CH=='*'){
                SYM=NUL;
                A[0]='/';
                A[1]='*';
                K=2;//已经读入两个字符
                do{
                        GetCh();//获取下一个字符
                        if(CH=='*'){//判断是否是结束的开始标记*
                                GetCh();//获取下一个字符
                                if(CH=='/'){//判断是否构成*/注释结束标志
                                        A[K++]='*';
                                        A[K++]='/';
                                        break;//跳出循环
                                }else{
                                        A[K++]='*';
                                        A[K++]=CH;//将下一个字符添加进去
                                }
                        }else{//未试别到结束的开始标记*
                                A[K++]=CH;
                        }
                }while(true);//如果后面还有符号就继续读取下一个字符
                Form1->printfs(A);//将注释的内容输出展示
                GetCh();//获取下一个字符
                GetSym();
        }
        else {SYM=SLASH;}
  }


  else { SYM=SSYM[CH]; GetCh(); }
} /*GetSym()*/
//---------------------------------------------------------------------------
void GEN(FCT X, int Y, int Z) {
  if (CX>CXMAX) {
    Form1->printfs("PROGRAM TOO LONG");
	fprintf(FOUT,"PROGRAM TOO LONG\n");
	fclose(FOUT);
    exit(0);
  }
  CODE[CX].F=X; CODE[CX].L=Y; CODE[CX].A=Z;
  CX++;
} /*GEN*/
//---------------------------------------------------------------------------
void TEST(SYMSET S1, SYMSET S2, int N) {
  if (!SymIn(SYM,S1)) {
    Error(N);
	while (!SymIn(SYM,SymSetUnion(S1,S2))) GetSym();
  }
} /*TEST*/
//---------------------------------------------------------------------------
void ENTER(OBJECTS K, int LEV, int &TX, int &DX) { /*ENTER OBJECT INTO TABLE*/
  TX++;
  strcpy(TABLE[TX].NAME,ID); TABLE[TX].KIND=K;
  switch (K) {
	case CONSTANT:
	       if (NUM>AMAX) { Error(31); NUM=0; }
	       TABLE[TX].VAL=NUM;
	       break;
    case VARIABLE:
	       TABLE[TX].vp.LEVEL=LEV; TABLE[TX].vp.ADR=DX; DX++;
	       break;
	case PROCEDUR:
	       TABLE[TX].vp.LEVEL=LEV;
	       break;
  }
} /*ENTER*/
//---------------------------------------------------------------------------
int POSITION(ALFA ID, int TX) { /*FIND IDENTIFIER IN TABLE*/
  int i=TX;
  strcpy(TABLE[0].NAME,ID);
  while (strcmp(TABLE[i].NAME,ID)!=0) i--;
  return i;
} /*POSITION*/
//---------------------------------------------------------------------------
void ConstDeclaration(int LEV,int &TX,int &DX) {
  if (SYM==IDENT) {
    GetSym();
    if (SYM==EQL||SYM==BECOMES) {
	  if (SYM==BECOMES) Error(1);
	  GetSym();
	  if (SYM==NUMBER) { ENTER(CONSTANT,LEV,TX,DX); GetSym(); }
	  else Error(2);
    }
    else Error(3);
  }
  else Error(4);
} /*ConstDeclaration()*/
//---------------------------------------------------------------------------
void VarDeclaration(int LEV,int &TX,int &DX) {
  if (SYM==IDENT) { ENTER(VARIABLE,LEV,TX,DX); GetSym(); }
  else Error(4);
} /*VarDeclaration()*/
//---------------------------------------------------------------------------
void ListCode(int CX0) {  /*LIST CODE GENERATED FOR THIS Block*/
  if (Form1->ListSwitch->ItemIndex==0)
    for (int i=CX0; i<CX; i++) {
      String s=IntToStr(i);
      while(s.Length()<3)s=" "+s;
      s=s+" "+MNEMONIC[CODE[i].F]+" "+IntToStr(CODE[i].L)+" "+IntToStr(CODE[i].A);
	  Form1->printfs(s.c_str());
	  fprintf(FOUT,"%3d%5s%4d%4d\n",i,MNEMONIC[CODE[i].F],CODE[i].L,CODE[i].A);
    }
} /*ListCode()*/;
//---------------------------------------------------------------------------
void FACTOR(SYMSET FSYS, int LEV, int &TX) {
  int i;
  TEST(FACBEGSYS,FSYS,24);
  while (SymIn(SYM,FACBEGSYS)) {
	if (SYM==IDENT) {
	  i=POSITION(ID,TX);
	  if (i==0) Error(11);
	  else
		switch (TABLE[i].KIND) {
		  case CONSTANT: GEN(LIT,0,TABLE[i].VAL); break;
		  case VARIABLE: GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR); break;
		  case PROCEDUR: Error(21); break;
		}
	  GetSym();
	}
	else
	  if (SYM==NUMBER) {
		if (NUM>AMAX) { Error(31); NUM=0; }
		GEN(LIT,0,NUM); GetSym();
	  }
	  else
		if (SYM==LPAREN) {
		  GetSym(); EXPRESSION(SymSetAdd(RPAREN,FSYS),LEV,TX);
		  if (SYM==RPAREN) GetSym();
		  else Error(22);
		}
                else if(SYM==ELSESYM){
                     break;//因子分析的过程中，将ELSESYM也当成其开始因子，没有对其操作会进入无限循环
                }
                else if(SYM==TOSYM){ //分析为To跳出
                     break;
                }
                else if(SYM==DOWNTOSYM){  //分析为DownTo跳出
                     break;
                }else if(SYM==DOSYM){  //分析为Do跳出
                     break;

                }
	  TEST(FSYS,FACBEGSYS,23);
  }
}/*FACTOR*/
//---------------------------------------------------------------------------
void TERM(SYMSET FSYS, int LEV, int &TX) {  /*TERM*/
  SYMBOL MULOP;
  FACTOR(SymSetUnion(FSYS,SymSetNew(TIMES,SLASH)), LEV,TX);
  while (SYM==TIMES || SYM==SLASH) {
	MULOP=SYM;  GetSym();
	FACTOR(SymSetUnion(FSYS,SymSetNew(TIMES,SLASH)),LEV,TX);
	if (MULOP==TIMES) GEN(OPR,0,4);
	else GEN(OPR,0,5);
  }
} /*TERM*/;
//---------------------------------------------------------------------------
void EXPRESSION(SYMSET FSYS, int LEV, int &TX) {
  SYMBOL ADDOP;
  int i; // 新增 int i
  if (SYM==PLUS || SYM==MINUS) {
    ADDOP=SYM; GetSym();
    TERM(SymSetUnion(FSYS,SymSetNew(PLUS,MINUS)),LEV,TX);
    if (ADDOP==MINUS) GEN(OPR,0,1);
  }
  // ↓↓↓ 新增部分 ↓↓↓
  else if(SYM==PLUSPLUS){     // ++i
      GetSym();
      if(SYM==IDENT){
          i=POSITION(ID,TX);
          if(i==0) Error(11);
          else if(TABLE[i].KIND!=VARIABLE){
              Error(12);
              i=0;
          }
          if(i!=0) GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
          GEN(LIT,0,1);
          GEN(OPR,0,2);
          if(i!=0){
              GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
              GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
          }
          GetSym();
      }
      else Error(45);
  }
  else if(SYM==MINUSMINUS){     // --i
      GetSym();
      if(SYM==IDENT){
          i=POSITION(ID,TX);
          if(i==0) Error(11);
          else if(TABLE[i].KIND!=VARIABLE){
              Error(12);
              i=0;
          }
          if(i!=0) GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
          GEN(LIT,0,1);
          GEN(OPR,0,3);
          if(i!=0){
              GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
              GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
          }
          GetSym();
      }
      else Error(45);
  }
  // ↑↑↑ 新增部分 ↑↑↑// ↓↓↓ 新增部分 ↓↓↓
  else if(SYM==PLUSPLUS){     // ++i
      GetSym();
      if(SYM==IDENT){
          i=POSITION(ID,TX);
          if(i==0) Error(11);
          else if(TABLE[i].KIND!=VARIABLE){
              Error(12);
              i=0;
          }
          if(i!=0) GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
          GEN(LIT,0,1);
          GEN(OPR,0,2);
          if(i!=0){
              GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
              GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
          }
          GetSym();
      }
      else Error(45);
  }
  else if(SYM==MINUSMINUS){     // --i
      GetSym();
      if(SYM==IDENT){
          i=POSITION(ID,TX);
          if(i==0) Error(11);
          else if(TABLE[i].KIND!=VARIABLE){
              Error(12);
              i=0;
          }
          if(i!=0) GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
          GEN(LIT,0,1);
          GEN(OPR,0,3);
          if(i!=0){
              GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
              GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
          }
          GetSym();
      }
      else Error(45);
  }
  // ↑↑↑ 新增部分 ↑↑↑
  else TERM(SymSetUnion(FSYS,SymSetNew(PLUS,MINUS)),LEV,TX);
  while (SYM==PLUS || SYM==MINUS) {
    ADDOP=SYM; GetSym();
    TERM(SymSetUnion(FSYS,SymSetNew(PLUS,MINUS)),LEV,TX);
    if (ADDOP==PLUS) GEN(OPR,0,2);
    else GEN(OPR,0,3);
  }
} /*EXPRESSION*/
//---------------------------------------------------------------------------
void CONDITION(SYMSET FSYS,int LEV,int &TX) {
  SYMBOL RELOP;
  if (SYM==ODDSYM) { GetSym(); EXPRESSION(FSYS,LEV,TX); GEN(OPR,0,6); }
  else {
	EXPRESSION(SymSetUnion(SymSetNew(EQL,NEQ,LSS,LEQ,GTR,GEQ),FSYS),LEV,TX);
	if (!SymIn(SYM,SymSetNew(EQL,NEQ,LSS,LEQ,GTR,GEQ))) Error(20);
	else {
	  RELOP=SYM; GetSym(); EXPRESSION(FSYS,LEV,TX);
	  switch (RELOP) {
	    case EQL: GEN(OPR,0,8);  break;
	    case NEQ: GEN(OPR,0,9);  break;
	    case LSS: GEN(OPR,0,10); break;
	    case GEQ: GEN(OPR,0,11); break;
	    case GTR: GEN(OPR,0,12); break;
	    case LEQ: GEN(OPR,0,13); break;
	  }
	}
  }
} /*CONDITION*/
//---------------------------------------------------------------------------
void STATEMENT(SYMSET FSYS,int LEV,int &TX) {   /*STATEMENT*/
  int i,CX1,CX2;
  switch (SYM) {
	case IDENT:
		i=POSITION(ID,TX);
		if (i==0) Error(11);
		else
		  if (TABLE[i].KIND!=VARIABLE) { /*ASSIGNMENT TO NON-VARIABLE*/
			Error(12); i=0;
		  }
                GetSym();
		if (SYM==BECOMES){
                        GetSym();
			EXPRESSION(FSYS,LEV,TX);
			if (i!=0) GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
		}
		else if (SYM==PLUSBECOMES){ //codes for +=
                        GetSym(); //获取到右边的表达式
			if (i!=0){ //若变量存在
                                GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR); //将该变量取出到栈顶
                                EXPRESSION(FSYS,LEV,TX); //对表达式进行处理，其结果在栈顶
                                GEN(OPR,0,2); //将栈顶的两个元素做加法
                                GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR); //将栈顶元素作为变量的值进行保存
                        }
		}
		else if(SYM==MINUSBECOMES){ //codes for -=
                        GetSym(); //获取到右边的表达式
			if (i!=0){ //若变量存在
                                GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR); //将该变量取出到栈顶
                                EXPRESSION(FSYS,LEV,TX); //对右边表达式进行处理，其结果在栈顶
                                GEN(OPR,0,3); //将栈顶的两个元素做加法
                                GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR); //将栈顶元素作为变量的值进行保存
                        }
		}
                else if (SYM==PLUSPLUS){
                        if(i!=0){
                                GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);//将变量A入栈
                                GEN(LIT,0,1);//设置自增量1
                                GEN(OPR,0,2);//将栈顶两元素相加，结果放置栈顶（即A+1）
                                GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);//保存自增后的值到A（完成A++操作）
                        }
                        GetSym();//加上这个使得能够读取自增符号后面的分号
                }
                else if (SYM==MINUSMINUS){
                        if(i!=0){
                                GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);//将变量A入栈
                                GEN(LIT,0,1);//设置自减量1
                                GEN(OPR,0,3);//将栈顶两元素减，结果放置栈顶（即A-1）
                                GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);//保存自增后的值到A（完成A--操作）
                        }
                        GetSym();//加上这个使得能够读取自减符号后面的分号
                }
		else Error(13);
		break;
	case READSYM:
		GetSym();
		if (SYM!=LPAREN) Error(34);
		else
		  do {
			GetSym();
			if (SYM==IDENT) i=POSITION(ID,TX);
			else i=0;
			if (i==0) Error(35);
			else {
			  GEN(OPR,0,16);
			  GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
			}
			GetSym();
		  }while(SYM==COMMA);
		if (SYM!=RPAREN) {
		  Error(33);
		  while (!SymIn(SYM,FSYS)) GetSym();
		}
		else GetSym();
		break; /* READSYM */
	case WRITESYM:
		GetSym();
		if (SYM==LPAREN) {
		  do {
			GetSym();
			EXPRESSION(SymSetUnion(SymSetNew(RPAREN,COMMA),FSYS),LEV,TX);
			GEN(OPR,0,14);
		  }while(SYM==COMMA);
		  if (SYM!=RPAREN) Error(33);
		  else GetSym();
		}
		GEN(OPR,0,15);
		break; /*WRITESYM*/
	case CALLSYM:
		GetSym();
		if (SYM!=IDENT) Error(14);
		else {
		  i=POSITION(ID,TX);
		  if (i==0) Error(11);
		  else
			if (TABLE[i].KIND==PROCEDUR)
			  GEN(CAL,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
			else Error(15);
		  GetSym();
		}
		break;
	case IFSYM:
		GetSym();
		CONDITION(SymSetUnion(SymSetNew(THENSYM,DOSYM),FSYS),LEV,TX);/*调用条件处理，处理if的内容=> if（xxx）即处理xxx的部分*/
		if (SYM==THENSYM) GetSym();
		else Error(16);
		CX1=CX;  /*保存当前指令地址*/
                GEN(JPC,0,0); /*生成条件跳转指令，跳转地址未知．暂时写0*/
		STATEMENT(FSYS,LEV,TX);  /*处理then后面的语句*/
                if(SYM!=ELSESYM){CODE[CX1].A=CX;} /*如果是if then语句 直接设置条件跳转地址*/
                else {    /*如果是if then else语句*/
                        GetSym();
                        CX2=CX;
                        GEN(JMP,0,0);  /*生成无条件跳转语句，跳转的目的地址将是else语句块生成指令的尾地址*/
                        CODE[CX1].A=CX; /*CX1的目的跳转地址（如果if为false，跳转到这（注意：这里已经是跳到了jump指令的下一条，即jump指令不会执行！）*/
                        STATEMENT(FSYS,LEV,TX); /*处理else语句块*/
                        CODE[CX2].A=CX; /*CX2的目的”无条件“跳转地址（只有if为true才能够执行到jump，也才能够直接跳过else语句块的执行）*/
                        }
		break;
	case BEGINSYM:
		GetSym();
		STATEMENT(SymSetUnion(SymSetNew(SEMICOLON,ENDSYM),FSYS),LEV,TX);
		while (SymIn(SYM, SymSetAdd(SEMICOLON,STATBEGSYS))) {
		  if (SYM==SEMICOLON) GetSym();
		  else Error(10);
		  STATEMENT(SymSetUnion(SymSetNew(SEMICOLON,ENDSYM),FSYS),LEV,TX);
		}
		if (SYM==ENDSYM) GetSym();
		else Error(17);
		break;
	case WHILESYM:
		CX1=CX; GetSym(); CONDITION(SymSetAdd(DOSYM,FSYS),LEV,TX);
		CX2=CX; GEN(JPC,0,0);
		if (SYM==DOSYM) GetSym();
		else Error(18);
		STATEMENT(FSYS,LEV,TX);
		GEN(JMP,0,CX1);
		CODE[CX2].A=CX;
		break;
       case ELSESYM:
                GetSym();
                Form1->printfs("----ELSESYM----");
                break;
       case FORSYM:
                GetSym();//获取赋值语句表达式
                i=POSITION(ID,TX);//获取变量位置
		if (i==0) {
                        Form1->printfs("following for must be an ident.");
                        Error(11);
                }
		else if (TABLE[i].KIND!=VARIABLE) { /*ASSIGNMENT TO NON-VARIABLE*/
			Error(12);
                }
                STATEMENT(SymSetUnion(SymSetNew(TOSYM,DOWNTOSYM),FSYS),LEV,TX);//处理表达式，以TO或者DOWNTO结尾
                if(SYM==TOSYM){//判断后跟符号是否为TO
                        GetSym();//获取TO后面的表达式
                        CX1=CX;//记录每趟循环结束后回到的判断位置的地址
                        GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);//将赋值语句的值（A）放到栈顶
                        EXPRESSION(SymSetUnion(SymSetNew(TOSYM,DOWNTOSYM,DOSYM),FSYS),LEV,TX);//对TO后面的表达式进行处理，其表达式的结果放置栈顶
                        GEN(OPR,0,10);//比较次栈顶是否小于栈顶（A<B），AB出栈，结果入栈顶
                        if(SYM==DOSYM){//判断后跟符号是否为DO
                                CX2=CX;//记录DO执行开始时候的地址
                                GEN(JPC,0,0);//设置条件跳转
                                GetSym();//获取DO后面的表达式
                                STATEMENT(FSYS,LEV,TX);//处理DO后面编译操作
                                GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);//将赋值语句的值（A）放到栈顶
                                GEN(LIT,0,1);//设置步长为1
                                GEN(OPR,0,2);//将栈顶两元素相加，结果放置栈顶（即A+1）
                                GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);//修改A的值（完成A++操作）
                                GEN(JMP,0,CX1);//无条件跳转至AB比较的地方（此时AB还未加载进进栈）
                                CODE[CX2].A=CX;//确定JPC跳转的目的地址（若JPC判断为假，则循环结束）
                        }else Error(18);//缺少DO报错代号
                }
                else if(SYM==DOWNTOSYM){//判断后跟符号是否为DOWNTO
                        GetSym();//获取DOWNTO后面的表达式
                        CX1=CX;//记录每趟循环结束后回到的判断位置的地址
                        EXPRESSION(SymSetUnion(SymSetNew(TOSYM,DOWNTOSYM,DOSYM),FSYS),LEV,TX);//对DOWNTO后面的表达式进行处理，其表达式的结果放置栈顶
                        if(SYM==DOSYM){//判断后跟符号是否为DO
                                GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);//将赋值语句的值（A）放到栈顶
                                GEN(OPR,0,10);//比较次栈顶是否小于栈顶（A<B），AB出栈，结果入栈顶
                                CX2=CX;//记录DO执行开始时候的地址
                                GEN(JPC,0,0);//设置条件跳转
                                GetSym();//获取DO后面的表达式
                                STATEMENT(FSYS,LEV,TX);//处理DO后面编译操作
                                GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);//将赋值语句的值（A）放到栈顶
                                GEN(LIT,0,1);//设置步长为1
                                GEN(OPR,0,3);//将栈顶两元素相加，结果放置栈顶（即A-1）
                                GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);//修改A的值（完成A--操作）
                                GEN(JMP,0,CX1);//无条件跳转至AB比较的地方（此时AB还未加载进进栈）
                                CODE[CX2].A=CX;//确定JPC跳转的目的地址（若JPC判断为假，则循环结束）
                        }else Error(18);//缺少DO报错代号
                }else Error(36);//缺少TO或者DOWNTO
                break;

      // ++i
       case PLUSPLUS:
                GetSym();
                Form1->printfs("----PLUSPLUS----");
                if(SYM==IDENT){
                    i=POSITION(ID,TX);
                    if(i==0) Error(11);
                    else if(TABLE[i].KIND!=VARIABLE){
                        Error(12);
                        i=0;
                    }
                    if(i!=0) GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
                    GEN(LIT,0,1);
                    GEN(OPR,0,2);
                    if(i!=0) GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
                    GetSym();
                }
                else Error(45);
                break;
       // --i
       case MINUSMINUS:
                GetSym();
                Form1->printfs("----MINUSMINUS----");
                if(SYM==IDENT){
                    i=POSITION(ID,TX);
                    if(i==0) Error(11);
                    else if(TABLE[i].KIND!=VARIABLE){
                        Error(12);
                        i=0;
                    }
                    if(i!=0) GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
                    GEN(LIT,0,1);
                    GEN(OPR,0,3);
                    if(i!=0) GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
                    GetSym();
                }
                else Error(45);
                break;
       case TOSYM:
                GetSym();
                Form1->printfs("----TOSYM----");
                break;

       case DOWNTOSYM:
                GetSym();
                Form1->printfs("----DOWNTOSYM----");
                break;

       case RETURNSYM:
                GetSym();
                Form1->printfs("----RETURNSYM----");
                break;

       case PLUSBECOMES:
                GetSym();
                Form1->printfs("----PLUSBECOMES----");
                break;

       case MINUSBECOMES:
                GetSym();
                Form1->printfs("----MINUSBECOMES----");
                break;

  }
  TEST(FSYS,SymSetNULL(),19);
} /*STATEMENT*/
//---------------------------------------------------------------------------
void Block(int LEV, int TX, SYMSET FSYS) {
  int DX=3;    /*DATA ALLOCATION INDEX*/
  int TX0=TX;  /*INITIAL TABLE INDEX*/
  int CX0=CX;  /*INITIAL CODE INDEX*/
  TABLE[TX].vp.ADR=CX; GEN(JMP,0,0);
  if (LEV>LEVMAX) Error(32);
  do {
    if (SYM==CONSTSYM) {
      GetSym();
      do {
        ConstDeclaration(LEV,TX,DX);
        while (SYM==COMMA) {
          GetSym();  ConstDeclaration(LEV,TX,DX);
        }
        if (SYM==SEMICOLON) GetSym();
        else Error(5);
      }while(SYM==IDENT);
    }
    if (SYM==VARSYM) {
      GetSym();
      do {
        VarDeclaration(LEV,TX,DX);
        while (SYM==COMMA) { GetSym(); VarDeclaration(LEV,TX,DX); }
	    if (SYM==SEMICOLON) GetSym();
	    else Error(5);
      }while(SYM==IDENT);
    }
    while ( SYM==PROCSYM) {
      GetSym();
	  if (SYM==IDENT) { ENTER(PROCEDUR,LEV,TX,DX); GetSym(); }
	  else Error(4);
	  if (SYM==SEMICOLON) GetSym();
	  else Error(5);
	  Block(LEV+1,TX,SymSetAdd(SEMICOLON,FSYS));
	  if (SYM==SEMICOLON) {
	    GetSym();
	    TEST(SymSetUnion(SymSetNew(IDENT,PROCSYM),STATBEGSYS),FSYS,6);
	  }
	  else Error(5);
    }
    TEST(SymSetAdd(IDENT,STATBEGSYS), DECLBEGSYS,7);
  }while(SymIn(SYM,DECLBEGSYS));
  CODE[TABLE[TX0].vp.ADR].A=CX;
  TABLE[TX0].vp.ADR=CX;   /*START ADDR OF CODE*/
  TABLE[TX0].vp.SIZE=DX;  /*SIZE OF DATA SEGMENT*/
  GEN(INI,0,DX);
  STATEMENT(SymSetUnion(SymSetNew(SEMICOLON,ENDSYM),FSYS),LEV,TX);
  GEN(OPR,0,0);  /*RETURN*/
  TEST(FSYS,SymSetNULL(),8);
  ListCode(CX0);
} /*Block*/
//---------------------------------------------------------------------------
int BASE(int L,int B,int S[]) {
  int B1=B; /*FIND BASE L LEVELS DOWN*/
  while (L>0) { B1=S[B1]; L=L-1; }
  return B1;
} /*BASE*/
//---------------------------------------------------------------------------
void Interpret() {
  const STACKSIZE = 500;
  int P,B,T; 		/*PROGRAM BASE TOPSTACK REGISTERS*/
  INSTRUCTION I;
  int S[STACKSIZE];  	/*DATASTORE*/
  Form1->printfs("~~~ RUN PL0 ~~~");
  fprintf(FOUT,"~~~ RUN PL0 ~~~\n");
  T=0; B=1; P=0;
  S[1]=0; S[2]=0; S[3]=0;
  do {
    I=CODE[P]; P=P+1;
    switch (I.F) {
      case LIT: T++; S[T]=I.A; break;
	  case OPR:
	    switch (I.A) { /*OPERATOR*/
	      case 0: /*RETURN*/ T=B-1; P=S[T+3]; B=S[T+2]; break;
	      case 1: S[T]=-S[T];  break;
	      case 2: T--; S[T]=S[T]+S[T+1];   break;
	      case 3: T--; S[T]=S[T]-S[T+1];   break;
	      case 4: T--; S[T]=S[T]*S[T+1];   break;
	      case 5: T--; S[T]=S[T] / S[T+1]; break;
	      case 6: S[T]=(S[T]%2!=0);        break;
	      case 8: T--; S[T]=S[T]==S[T+1];  break;
	      case 9: T--; S[T]=S[T]!=S[T+1];  break;
	      case 10: T--; S[T]=S[T]<S[T+1];   break;
	      case 11: T--; S[T]=S[T]>=S[T+1];  break;
	      case 12: T--; S[T]=S[T]>S[T+1];   break;
	      case 13: T--; S[T]=S[T]<=S[T+1];  break;
	      case 14: Form1->printls("",S[T]); fprintf(FOUT,"%d\n",S[T]); T--;
                   break;
	      case 15: /*Form1->printfs(""); fprintf(FOUT,"\n"); */ break;
	      case 16: T++;  S[T]=InputBox("输入","请键盘输入：", 0).ToInt();
                   Form1->printls("? ",S[T]); fprintf(FOUT,"? %d\n",S[T]);
		           break;
	    }
	    break;
      case LOD: T++; S[T]=S[BASE(I.L,B,S)+I.A]; break;
      case STO: S[BASE(I.L,B,S)+I.A]=S[T]; T--; break;
	  case CAL: /*GENERAT NEW Block MARK*/
	      S[T+1]=BASE(I.L,B,S); S[T+2]=B; S[T+3]=P;
	      B=T+1; P=I.A; break;
	  case INI: T=T+I.A;  break;
	  case JMP: P=I.A; break;
      case JPC: if (S[T]==0) P=I.A;  T--;  break;
    } /*switch*/
  }while(P!=0);
  Form1->printfs("~~~ END PL0 ~~~");
  fprintf(FOUT,"~~~ END PL0 ~~~\n");
} /*Interpret*/
//---------------------------------------------------------------------------
void __fastcall TForm1::ButtonRunClick(TObject *Sender) {
  for (CH=' '; CH<='^'; CH++) SSYM[CH]=NUL;
  strcpy(KWORD[ 1],"BEGIN");    strcpy(KWORD[ 2],"CALL");
  strcpy(KWORD[ 3],"CONST");    strcpy(KWORD[ 4],"DO");
  strcpy(KWORD[ 5],"DOWNTO");    strcpy(KWORD[ 6],"ELSE");
  strcpy(KWORD[ 7],"END");      strcpy(KWORD[ 8],"FOR");
  strcpy(KWORD[ 9],"IF");       strcpy(KWORD[10],"ODD");
  strcpy(KWORD[11],"PROCEDURE");strcpy(KWORD[12],"PROGRAM");
  strcpy(KWORD[13],"READ");     strcpy(KWORD[14],"RETURN");
  strcpy(KWORD[15],"THEN");     strcpy(KWORD[16],"TO");
  strcpy(KWORD[17],"VAR");      strcpy(KWORD[18],"WHILE");
  strcpy(KWORD[19],"WRITE");

  WSYM[ 1]=BEGINSYM;   WSYM[ 2]=CALLSYM;
  WSYM[ 3]=CONSTSYM;   WSYM[ 4]=DOSYM;
  WSYM[ 5]=DOWNTOSYM;  WSYM[ 6]=ELSESYM;
  WSYM[ 7]=ENDSYM;     WSYM[ 8]=FORSYM;
  WSYM[ 9]=IFSYM;      WSYM[10]=ODDSYM;
  WSYM[11]=PROCSYM;    WSYM[12]=PROGSYM;
  WSYM[13]=READSYM;    WSYM[14]=RETURNSYM;
  WSYM[15]=THENSYM;    WSYM[16]=TOSYM;
  WSYM[17]=VARSYM;     WSYM[18]=WHILESYM;
  WSYM[19]=WRITESYM;

  SSYM['+']=PLUS;      SSYM['-']=MINUS;
  SSYM['*']=TIMES;     SSYM['/']=SLASH;
  SSYM['(']=LPAREN;    SSYM[')']=RPAREN;
  SSYM['=']=EQL;       SSYM[',']=COMMA;
  SSYM['.']=PERIOD;    //SSYM['#']=NEQ;
  SSYM[';']=SEMICOLON;

  strcpy(MNEMONIC[LIT],"LIT");   strcpy(MNEMONIC[OPR],"OPR");
  strcpy(MNEMONIC[LOD],"LOD");   strcpy(MNEMONIC[STO],"STO");
  strcpy(MNEMONIC[CAL],"CAL");   strcpy(MNEMONIC[INI],"INI");
  strcpy(MNEMONIC[JMP],"JMP");   strcpy(MNEMONIC[JPC],"JPC");

  DECLBEGSYS=(int*)malloc(sizeof(int)*33);
  STATBEGSYS=(int*)malloc(sizeof(int)*33);
  FACBEGSYS =(int*)malloc(sizeof(int)*33);
  for(int j=0; j<33; j++) {
	DECLBEGSYS[j]=0;  STATBEGSYS[j]=0;  FACBEGSYS[j] =0;
  }
  DECLBEGSYS[CONSTSYM]=1;
  DECLBEGSYS[VARSYM]=1;
  DECLBEGSYS[PROCSYM]=1;
  STATBEGSYS[BEGINSYM]=1;
  STATBEGSYS[CALLSYM]=1;
  STATBEGSYS[IFSYM]=1;
  STATBEGSYS[WHILESYM]=1;
  STATBEGSYS[WRITESYM]=1;
  FACBEGSYS[IDENT] =1;
  FACBEGSYS[NUMBER]=1;
  FACBEGSYS[LPAREN]=1;

  if ((FIN=fopen((Form1->EditName->Text+".PL0").c_str(),"r"))!=0) {
	FOUT=fopen((Form1->EditName->Text+".COD").c_str(),"w");
    Form1->printfs("=== COMPILE PL0 ===");
    fprintf(FOUT,"=== COMPILE PL0 ===\n");
	ERR=0;
	CC=0; CX=0; LL=0; CH=' '; GetSym();
	if (SYM!=PROGSYM) Error(0);
	else {
	  GetSym();
	  if (SYM!=IDENT) Error(0);
	  else {
		GetSym();
		if (SYM!=SEMICOLON) Error(5);
		else GetSym();
	  }
	}
	Block(0,0,SymSetAdd(PERIOD,SymSetUnion(DECLBEGSYS,STATBEGSYS)));
	if (SYM!=PERIOD) Error(9);
	if (ERR==0) Interpret();
	else {
	  Form1->printfs("ERROR IN PL/0 PROGRAM");
	  fprintf(FOUT,"ERROR IN PL/0 PROGRAM");
	}
	fprintf(FOUT,"\n"); fclose(FOUT);
  }
}
//---------------------------------------------------------------------------




