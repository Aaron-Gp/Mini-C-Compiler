#ifndef AST_H
#define AST_H

#include"common.h"

#define MAX_PARAM 16
#define MAX_SCOPE_STACK 32
#define SCOPE_ID_BASE '1'

enum NodeType {
    NODE_NONE,

    NODE_PROG,
    NODE_EXTDEFL,
    NODE_EXTDEF,

    NODE_SPEC,
    NODE_TYPE,

    NODE_VARDEC,
    NODE_FUNCDEC,
    NODE_VARLIST,
    NODE_PARAMDEC,

    NODE_COMPST,
    NODE_COMPL,
    NODE_STMTL,
    NODE_STMT,

    NODE_DEFL,
    NODE_DEF,
    NODE_DECL,
    NODE_DEC,

    NODE_EXPR,
    NODE_ARGS,

    NODE_CONST,
    NODE_ID
};

enum OperatorType {
    OP_NONE,
    OP_EQ,  	// ==
    OP_NE, 	    // !=
    OP_GE,	    // >=
    OP_LE,	    // <=
    OP_ASSIGN,	// =
    OP_GT,		// >
    OP_LT,		// <
    OP_ADD,		// +
    OP_SUB,		// -
    OP_UNM,	// - (一元运算符)
    OP_COMPL,   // ~
    OP_MUL,		// *
    OP_DIV,		// /
    OP_MOD,		// %
    OP_NOT,		// !
    OP_AND, 	// &&
    OP_OR,		// ||
    OP_BAND,
    OP_BOR,
    OP_XOR
};

enum StmtType {
    STMT_NONE,
    STMT_EXPR,
    STMT_RETURN,
    STMT_IF,
    STMT_IF_ELSE,
    STMT_WHILE,
    STMT_BREAK,
    STMT_CONTINUE,
};

enum ExprType {
    EXPR_NONE,
    EXPR_UNARY,
    EXPR_BINARY,
    EXPR_FUNCALL,
    EXPR_ID,
    EXPR_CONST
};

enum ValueType {
    NOTYPE,
    VALUE_INT,
    VALUE_VOID,
    COMPOSE_FUNCTION
};

class Type {
public:
    bool constvar;
    ValueType type;
    Type(ValueType valueType);
    // 将b复制到自己中
    void copy(Type *a);

public:
    int pointLevel = 0;

    unsigned short paramNum; // for function
    Type *paramType[MAX_PARAM];
    Type *retType;
    void addParam(Type *t);
    void addRet(Type *t);

    int getSize();

public:
    const char* getTypeInfo();
    const char* getTypeInfo(ValueType type);
};

// 设置几个常量Type，可以节省空间开销
static Type *TYPE_INT = new Type(VALUE_INT);
static Type *TYPE_VOID = new Type(VALUE_VOID);
static Type *TYPE_NONE = new Type(NOTYPE);

struct TreeNode {
public:
    int lineno;

    // -------------- 语法树构造 ----------------

    TreeNode *child = nullptr;
    TreeNode *sibling = nullptr;

    NodeType nodeType;
    OperatorType optype;// 运算符类型
    StmtType stype;		// 表达式类型
    ExprType etype;
    Type *type;			// 变量、类型、表达式结点，有类型。
    int int_val;
    string var_name;
    string var_scope;	// 变量作用域标识符

    TreeNode(int lineno, NodeType type);
    TreeNode(TreeNode *node);	// 仅用于叶节点拷贝，函数不复制子节点，也不复制子节点指针
    void addChild(TreeNode *);
    void addSibling(TreeNode *);
    int getChildNum();
    int getVal();

    int nodeID;
    void genNodeId();

    // -------------- 输出语法树 ----------------

    void printAST();
    void printNodeInfo();
    void printChildrenId();
    void printSpecialInfo();
    void printConstVal();

    // -------------- 类型检查 ----------------

    void typeCheck();
    void findReturn(vector<TreeNode *> &retList);

    // ------------- asm 代码生成 -------------

    int node_seq = 0;
    int temp_var_seq = 0;

    void gen_var_decl();
    void gen_header();
    void gen_operate();

    void genCode();

public:
    static const char* nodeType2String(NodeType type);
    static const char* opType2String(OperatorType type);
    static const char* sType2String(StmtType type);
    static const char* eType2String(ExprType type);
};

extern bool typeError;

#endif