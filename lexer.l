%{
#include"common.h"
#include"parser.hpp"

extern TreeNode *nodePrint;
extern void InitIOFunctionNode();
extern void init_scanner();
extern void verbose(char* token, char* text);
extern void lex_error(char* msg);
%}

%option yylineno

/* 空白符 */
space     [ \t\r\a]+

/* 关键字 */
int       int
return    return
void      void
if        if
else      else
while     while
continue  continue
break     break

/* 标识符 */
id        [A-Za-z_][0-9A-Za-z_]*

/* 常量 */
integer   0|[1-9][0-9]*

/* 赋值运算 */
assign    =

/* 一元运算 */
not       \!
compl     \~

/* 二元运算 */
add       \+
sub       \-
mul       \*
div       \/
mod       \%
lt        \<
le        \<=
gt        \>
ge        \>=
eq        ==
ne        !=
band      \&
bor       \|
xor       \^
and       \&\&
or        \|\|

/* 逗号分号括号 */
semi      ;
comma     ,
lb        \{
rb        \}
lp        \(
rp        \)

%%
[\n] {PR("\n");}
{space} {/* ignore all spaces */}

{int} {verbose("INT", yytext); return INT;}
{return} {verbose("RETURN", yytext); return RETURN;}
{void} {verbose("VOID", yytext); return VOID;}
{if} {verbose("IF", yytext); return IF;}
{else} {verbose("ELSE", yytext); return ELSE;}
{while} {verbose("WHILE", yytext); return WHILE;}
{continue} {verbose("CONTINUE", yytext); return CONTINUE;}
{break} {verbose("BREAK", yytext); return BREAK;}
"println_int" {
    verbose("ID", yytext);
    TreeNode *node = new TreeNode(-1, NODE_ID);
    node->var_name = string(yytext);
    yylval = node;
    return ID;
}

{id} {
    verbose("ID", yytext);
    TreeNode* node = new TreeNode(yylineno, NODE_ID);
    node->var_name = string(yytext);
    yylval = node;
#ifdef ID_TOKEN_DEBUG
    PR("get IDENTIFIER: %s", yytext);
#endif
    return ID;
}

{integer} {
    verbose("INTEGER", yytext);
    TreeNode* node = new TreeNode(yylineno, NODE_CONST);
    node->type = TYPE_INT;
    node->int_val = atoi(yytext);
    yylval = node;
    return INTEGER;
}

{assign} {verbose("ASSIGN", yytext); return ASSIGN;}

{not} {verbose("NOT", yytext); return NOT;}
{compl} {verbose("COMPL", yytext); return COMPL;}

{add} {verbose("ADD", yytext); return ADD;}
{sub} {verbose("SUB", yytext); return SUB;}
{mul} {verbose("MUL", yytext); return MUL;}
{div} {verbose("DIV", yytext); return DIV;}
{mod} {verbose("MOD", yytext); return MOD;}
{lt} {verbose("LT", yytext); return LT;}
{le} {verbose("LE", yytext); return LE;}
{gt} {verbose("GT", yytext); return GT;}
{ge} {verbose("GE", yytext); return GE;}
{eq} {verbose("EQ", yytext); return EQ;}
{ne} {verbose("NE", yytext); return NE;}
{band} {verbose("BAND", yytext); return BAND;}
{bor} {verbose("BOR", yytext); return BOR;}
{xor} {verbose("XOR", yytext); return XOR;}
{and} {verbose("AND", yytext); return AND;}
{or} {verbose("OR", yytext); return OR;}

{semi} {verbose("SEMI", yytext); return SEMI;}
{comma} {verbose("COMMA", yytext); return COMMA;}
{lb} {verbose("LBRACE", yytext); return LBRACE;}
{rb} {verbose("RBRACE", yytext); return RBRACE;}
{lp} {verbose("LPAREN", yytext); return LPAREN;}
{rp} {verbose("RPAREN", yytext); return RPAREN;}

. {lex_error("Unrecognized character");}
%%

void init_scanner(){
    PR("%-20s%s\n", "TOKEN-TYPE", "TOKEN-VALUE");
    PR("-----------------------------------\n");
}

#ifdef VERBOSE
void verbose(char* token, char* text){
    PR("%-20s%s\n", token, text);
}
#else
void verbose(char* token, char* text){
    return;
}
#endif

void lex_error(char *msg){
    PR("\nError at line %-3d: %s\n\n", yylineno, msg);
}

int yywrap(void){
    return 1;
}
