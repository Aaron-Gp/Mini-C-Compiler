%{
#include"common.h"
#define YYSTYPE TreeNode*

TreeNode* root = new TreeNode(0, NODE_PROG);
extern int yylineno;

// max_scope_id 是堆栈下一层结点的最大编号
unsigned char max_scope_id = SCOPE_ID_BASE;
string presentScope = "0";
unsigned int top = 0;

TreeNode* nodePrint = new TreeNode(-1, NODE_VARDEC);

// multimap <标识符名称， 作用域> 变量名列表
extern multimap<string, string> idNameList = {
	{"println_int", "0"}
};
// map <<标识符名称， 作用域>, 结点指针> 变量列表
extern map<pair<string, string>, TreeNode*> idList = {
	{make_pair("println_int", "0"), nodePrint}
};
// 用于检查continue和break是否在循环内部
int inCycle = 0;

extern void InitIOFunctionNode();
int yylex ();
int yyerror(const char*);
int scopeCmp(string preScope, string varScope);
void scopePush();
void scopePop();
%}

%locations

// 关键字
%token RETURN INT VOID IF ELSE WHILE CONTINUE BREAK
// 标识符
%token ID
// 常量
%token INTEGER
// 赋值运算符
%token ASSIGN
// 一元运算符
%token NOT COMPL
// 二元运算符
%token ADD SUB MUL DIV MOD AND OR BAND BOR XOR EQ GE LE NE GT LT
// 括号分号逗号
%token SEMI COMMA LPAREN RPAREN LBRACE RBRACE

// 优先级
%left COMMA
%right ASSIGN
%left OR
%left AND
%left BOR
%left XOR
%left BAND
%left EQ NE
%left GT GE LT LE
%left ADD SUB
%left MUL DIV MOD
%right UNM NOT COMPL
%left LPAREN RPAREN LBRACE RBRACE

%start program

%%
/* 
 * High-Level Definitions
 */

/* 程序 */
program
: extDefList {root->addChild($1);}
;

/* 全局函数定义 */
extDefList
: %empty {$$ = new TreeNode(yylineno, NODE_EXTDEFL);}
| extDefList extDef {$$ = $1; $$->addChild($2);}
;
extDef
: specifier funcDec compSt {
	$$ = new TreeNode(yylineno, NODE_EXTDEF);
	$$->addChild($1);
	$$->addChild($2);
	$$->addChild($3);
  } /* 返回类 函数头 函数体 */
;

/*
 * Specifiers
 */

/* 类型描述符 */
specifier
: type {$$ = new TreeNode(yylineno, NODE_SPEC); $$->addChild($1);} /* 基本类型 */
;
type
: INT {$$ = new TreeNode(yylineno, NODE_TYPE); $$->type = TYPE_INT;}
| VOID {$$ = new TreeNode(yylineno, NODE_TYPE); $$->type = TYPE_VOID;}
;

/*
 * Declarators
 */

/* 单变量声明 */
varDec
: ID {
	$$ = new TreeNode(yylineno, NODE_VARDEC);
	$$->var_name = $1->var_name;
	$$->var_scope = presentScope;
	#ifdef ID_REDUCE_DEBUG
		cout<<"$ reduce declIdentifier : "<<$$->var_name<<", at scope :"<<presentScope<<endl;
	#endif
	if (idList.count(make_pair($$->var_name, $$->var_scope)) != 0) {
		string t = "Redeclared identifier : " + $$->var_name;
		yyerror(t.c_str());
	}
	idNameList.insert(make_pair($$->var_name, $$->var_scope));
	idList[make_pair($$->var_name, $$->var_scope)] = $$;
  }
;

/* 函数声明 */
funcDec
: varDec funcLP varList RPAREN {
	$$ = new TreeNode(yylineno, NODE_FUNCDEC);
	$$->addChild($1);
	$$->addChild($3);
  }
| varDec funcLP RPAREN {
	$$ = new TreeNode(yylineno, NODE_FUNCDEC);
	$$->addChild($1);
  }
;
funcLP
: LPAREN {
	scopePush();
  }
;

/* 形参列表 */
varList
: paramDec {
	$$ = new TreeNode(yylineno, NODE_VARLIST);
	$$->addChild($1);
  }
| varList COMMA paramDec {
	$$ = $1;
	$$->addChild($3);
  }
;
/* 形参声明 */
paramDec
: specifier varDec {
	$$ = new TreeNode(yylineno, NODE_PARAMDEC);
	$$->addChild($1);
	$$->addChild($2);
  }
;

/*
 * Statements
 */

/* 语句块 */
compSt
: LBRACE compList RBRACE {
	$$ = new TreeNode(yylineno, NODE_COMPST);
	$$->addChild($2);
	scopePop();
  }
;
compList
: %empty {
	$$ = new TreeNode(yylineno, NODE_COMPL);
  }
| compList defList stmtList {
	$$ = $1;
	$$->addChild($2);
	$$->addChild($3);
}
;

/* 一系列语句 */
stmtList
: %empty {$$ = new TreeNode(yylineno, NODE_STMTL);}
| stmtList stmt {
	$$ = $1;
	$$->addChild($2);
  }
;
/* 单个语句 */
stmt
: expr SEMI {
	$$ = new TreeNode(yylineno, NODE_STMT);
	$$->stype = STMT_EXPR;
	$$->addChild($1);
  }
| RETURN expr SEMI {
	$$ = new TreeNode(yylineno, NODE_STMT);
	$$->stype = STMT_RETURN;
	$$->addChild($2);
  } /* 返回语句 */
| compSt {
	$$ = $1;
  }
| if LPAREN expr RPAREN stmt {
	$$ = new TreeNode(yylineno, NODE_STMT);
	$$->stype = STMT_IF;
	$$->addChild($3);
	$$->addChild($5);
  }
| if LPAREN expr RPAREN stmt else stmt {
	$$ = new TreeNode(yylineno, NODE_STMT);
	$$->stype = STMT_IF_ELSE;
	$$->addChild($3);
	$$->addChild($5);
	$$->addChild($7);
  }
| while LPAREN expr RPAREN stmt {
	$$ = new TreeNode(yylineno, NODE_STMT);
	$$->stype = STMT_WHILE;
	$$->addChild($3);
	$$->addChild($5);
	inCycle--;
  }
| BREAK SEMI {
	if(!inCycle) yyerror("break statement outside loop");
	$$ = new TreeNode(yylineno, NODE_STMT);
	$$->stype = STMT_BREAK;
  }
| CONTINUE SEMI {
	if(!inCycle) yyerror("continue statement outside loop");
	$$ = new TreeNode(yylineno, NODE_STMT);
	$$->stype = STMT_CONTINUE;
  }
;
if
: IF {scopePush();}
;
else
: ELSE {scopePush();}
;
while
: WHILE {inCycle++; scopePush();}
;

/*
 * Local Definitions
 */

/* 一系列局部变量定义 */
defList
: %empty {$$ = new TreeNode(yylineno, NODE_DEFL);}
| defList def {
	$$ = $1;
	$$->addChild($2);
  }
;
/* 局部变量定义 */
def
: specifier decList SEMI {
	$$ = new TreeNode(yylineno, NODE_DEF);
	$$->addChild($1);
	$$->addChild($2);
  }
;
/* 局部变量列表 */
decList
: dec {$$ = new TreeNode(yylineno, NODE_DECL); $$->addChild($1);}
| decList COMMA dec {
	$$ = $1;
	$$->addChild($3);
  }
;
/* 单局部变量 */
dec
: varDec {$$ = new TreeNode(yylineno, NODE_DEC); $$->addChild($1);}
| varDec ASSIGN expr {
	$$ = new TreeNode(yylineno, NODE_DEC);
	$$->optype = OP_ASSIGN;
	$$->addChild($1);
	$$->addChild($3);
  } /* 定义时初始化 */
;

/*
 * Expressions
 */

expr
: expr ASSIGN expr {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_BINARY;
	$$->optype = OP_ASSIGN;
	$$->addChild($1);
	$$->addChild($3);
	#ifdef ASSIGN_DEBUG
		cerr << "$ reduce ASSIGN at scope : " << presentScope << ", at line " << lineno << endl;
	#endif
  }
| expr AND expr {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_BINARY;
	$$->optype = OP_AND;
	$$->addChild($1);
	$$->addChild($3);
  }
| expr OR expr {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_BINARY;
	$$->optype = OP_OR;
	$$->addChild($1);
	$$->addChild($3);
  }
| expr BAND expr {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_BINARY;
	$$->optype = OP_BAND;
	$$->addChild($1);
	$$->addChild($3);
  }
| expr BOR expr {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_BINARY;
	$$->optype = OP_BOR;
	$$->addChild($1);
	$$->addChild($3);
  }
| expr XOR expr {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_BINARY;
	$$->optype = OP_XOR;
	$$->addChild($1);
	$$->addChild($3);
  }
| expr ADD expr {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_BINARY;
	$$->optype = OP_ADD;
	$$->addChild($1);
	$$->addChild($3);
  }
| expr SUB expr {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_BINARY;
	$$->optype = OP_SUB;
	$$->addChild($1);
	$$->addChild($3);
  }
| expr MUL expr {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_BINARY;
	$$->optype = OP_MUL;
	$$->addChild($1); 
	$$->addChild($3);
  }
| expr DIV expr {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_BINARY;
	$$->optype = OP_DIV;
	$$->addChild($1);
	$$->addChild($3);
  }
| expr MOD expr {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_BINARY;
	$$->optype = OP_MOD;
	$$->addChild($1);
	$$->addChild($3);
  }
| expr EQ expr {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_BINARY;
	$$->optype = OP_EQ;
	$$->addChild($1);
	$$->addChild($3);
  }
| expr NE expr {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_BINARY;
	$$->optype = OP_NE;
	$$->addChild($1);
	$$->addChild($3);
  }
| expr LE expr {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_BINARY;
	$$->optype = OP_LE;
	$$->addChild($1); 
	$$->addChild($3);
  }
| expr LT expr {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_BINARY;
	$$->optype = OP_LT;
	$$->addChild($1);
	$$->addChild($3);
  }
| expr GE expr {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_BINARY;
	$$->optype = OP_GE;
	$$->addChild($1);
	$$->addChild($3);
  }
| expr GT expr {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_BINARY;
	$$->optype = OP_GT;
	$$->addChild($1);
	$$->addChild($3);
  }
| LPAREN expr RPAREN {$$ = $2;}
| SUB expr %prec UNM {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_UNARY;
	$$->optype = OP_UNM;
	$$->addChild($2);
  }
| NOT expr {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_UNARY;
	$$->optype = OP_NOT;
	$$->addChild($2);
  }
| COMPL expr {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_UNARY;
	$$->optype = OP_COMPL;
	$$->addChild($2);
  }
| ID LPAREN args RPAREN {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_FUNCALL;
	$$->addChild($1);
	$$->addChild($3);
	#ifdef FUNCALL_DEBUG
		cerr << "$ reduce function call at scope : " << presentScope << ", at line " << yylineno << endl;
	#endif
  } /* 带参数调用函数 */
| ID LPAREN RPAREN {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_FUNCALL;
	$$->addChild($1);
	#ifdef FUNCALL_DEBUG
		cerr << "$ reduce function call at scope : " << presentScope << ", at line " << yylineno << endl;
	#endif
  } /* 无参数调用函数 */
| ID {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_ID;
	$$->addChild($1);
  }
| INTEGER {
	$$ = new TreeNode(yylineno, NODE_EXPR);
	$$->etype = EXPR_CONST;
	$$->addChild($1);
  }
;

args
: expr {
	$$ = new TreeNode(yylineno, NODE_ARGS);
	$$->addChild($1);
  }
| args COMMA expr {
	$$ = $1;
	$$->addChild($3);
  }
;

%%

int yyerror(const char* s)
{
	PR("Parse error : %s at line %d\n", s, yylineno);
	return -1;
}

/*
 *	作用域比较函数 int scopeCmp (string, string)
 *
 *  输入参数： 
 *    presScope： 当前变量所处的作用域
 *    varScope:   希望进行比较的已声明变量作用域
 *
 *  返回值：
 *    0： 作用域相同，
 *          若为变量声明语句，为变量重定义。
 *   >0： 已声明变量作用域在当前作用域外层，返回作用域距离（堆栈层数）
 *          若为声明语句，不产生冲突，当前变量为新定义变量，
 *          若为使用语句，当前变量为新定义变量。
 *   -1：已声明变量作用域在当前作用域内层，
 *          若为声明语句，不可能出现这种情况，
 *          若为使用语句，不产生冲突。
 *   -2：两个作用域互不包含，任何情况下都不会互相干扰
 */
int scopeCmp(string presScope, string varScope) {
	unsigned int plen = presScope.length(), vlen = varScope.length();
	unsigned int minlen = min(plen, vlen);
	if (presScope.substr(0, minlen) == varScope.substr(0, minlen)) {
		if (plen >= vlen)
			return plen - vlen;
		else
			return -1;
	}
	return -2;
}
void scopePush() {
	presentScope += max_scope_id;
	max_scope_id = SCOPE_ID_BASE;
	top++;
#ifdef SCOPE_DEBUG
	PR("* push -> %s, at line %d\n", presentScope.c_str(), yylineno);
#endif
}

void scopePop() {
	max_scope_id = presentScope[top] + 1;
	presentScope = presentScope.substr(0, presentScope.length() - 1);
	top--;
#ifdef SCOPE_DEBUG
	PR("* pop -> %s, at line %d\n", presentScope.c_str(), yylineno);
#endif
}

void InitIOFunctionNode() {
    nodePrint->var_name = "println_int";
	nodePrint->var_scope = "0";
	nodePrint->type = new Type(COMPOSE_FUNCTION);
	nodePrint->type->retType = TYPE_VOID;
	nodePrint->type->paramType[nodePrint->type->paramNum++] = TYPE_INT;
}