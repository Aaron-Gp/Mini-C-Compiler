#include"ast.h"

Type::Type(ValueType valueType) {
    this->type = valueType;
    this->paramNum = 0;
    this->constvar = false;
    this->retType = nullptr;
}

void Type::copy(Type *a) {
    this->type = a->type;
    this->constvar = a->constvar;
    if (a->paramNum) {
        this->paramNum = a->paramNum;
        for (unsigned short i = 0;i < a->paramNum;i++) {
            this->paramType[i] = a->paramType[i];
        }
        this->retType = a->retType;
    }
}

const char *Type::getTypeInfo() {
    return getTypeInfo(this->type);
}

const char *Type::getTypeInfo(ValueType type) {
    switch (type) {
    case COMPOSE_FUNCTION:
        return "function";
    case VALUE_INT:
        return "int";
    case VALUE_VOID:
        return "void";
    case NOTYPE:
        return "no type";
    default:
        return "?";
    }
}

void Type::addParam(Type *param) {
    this->paramType[paramNum++] = param;
}

void Type::addRet(Type *t) {
    this->retType = t;
}

int Type::getSize() {
    int size = 1;
    int eleSize;
    switch (type) {
    case VALUE_INT:
        return 4;
    default:
        return 0;
    }
}

string operator + (string &content, int number) {
    return content + to_string(number);
}

string &operator += (string &content, int number) {
    return content = content + to_string(number);
}

// multimap <标识符名称， 作用域> 变量名列表
extern multimap<string, string> idNameList;
// map <<标识符名称， 作用域>, 结点指针> 变量列表
extern map<pair<string, string>, TreeNode *> idList;

// map <字符串， 标签序列号> 字符串表
map<string, int> strList;

// map <变量名， 变量相对于ebp偏移量> 局部变量表，在每次函数定义前清空
// <"a", "-12"> 表示第一个函数的栈上第一个分配的局部变量（前3个4字节为bx,cx,dx备份用，始终保留）
map<string, int> LocalVarList;
// 栈上为局部变量分配的空间总大小，在return时进行清理
int stackSize;

// 当前所处函数的声明结点指针，return使用
TreeNode *pFunction;

// label 计数
int cnt_if = 1;
int cnt_while = 1;
// label 栈
stack<int> label_stack;

TreeNode::TreeNode(int lineno, NodeType type) {
    this->lineno = lineno;
    this->nodeType = type;
}

TreeNode::TreeNode(TreeNode *node) {
    this->lineno = node->lineno;
    this->nodeType = node->nodeType;
    this->optype = node->optype;
    this->etype = node->etype;
    this->stype = node->stype;
    this->type = node->type;
    this->int_val = node->int_val;
    this->var_name = node->var_name;
    this->var_scope = node->var_scope;
}

void TreeNode::addChild(TreeNode *child) {
    if (this->child == nullptr) {
        this->child = child;
    } else {
        this->child->addSibling(child);
    }
}

void TreeNode::addSibling(TreeNode *sibling) {
    TreeNode *p = this;
    while (p->sibling != nullptr) {
        p = p->sibling;
    }
    p->sibling = sibling;
}

int TreeNode::getChildNum() {
    int num = 0;
    for (TreeNode *p = child; p != nullptr; p = p->sibling)
        num++;
    return num;
}

int TreeNode::getVal() {
    if (nodeType == NODE_CONST) {
        switch (type->type) {
        case VALUE_INT:
            return int_val;
        default:
            return 0;
        }
    } else if (child->nodeType == NODE_CONST) {
        return child->getVal();
    }
    return 0;
}

void TreeNode::genNodeId() {
    static unsigned int maxid = 0;
    this->nodeID = maxid++;
    if (this->child)
        this->child->genNodeId();
    if (this->sibling)
        this->sibling->genNodeId();
}

void TreeNode::typeCheck() {

#ifdef typeCheck_debug
    cout << "# Type check reach @" << nodeID << endl;
#endif

#ifdef typeCheck_debug
    cout << "# Type check start @" << nodeID << endl;
#endif

#ifdef typeCheck_debug
    cout << "# Type check end @" << nodeID << endl;
#endif

}

void TreeNode::findReturn(vector<TreeNode *> &retList) {
    if (nodeType == NODE_STMT && stype == STMT_RETURN)
        retList.push_back(this);
    else {
        TreeNode *p = child;
        while (p) {
            p->findReturn(retList);
            p = p->sibling;
        }
    }
}

void TreeNode::genCode() {
    TreeNode *p = child;
    switch (nodeType) {
    case NODE_PROG:
#ifdef DEBUG
        assert(p->nodeType == NODE_EXTDEFL);
#endif
        p->gen_header();
        p = p->child; // extdef
        while (p) {
            p->genCode();
            p = p->sibling; // extdef
        }
        break;
    case NODE_EXTDEF:
        p = p->sibling; // funcdec
        printf("%s:\n", p->child->var_name.c_str());
        printf("  push ebp\n  mov ebp, esp\n");
#ifdef DEBUG
        assert(p->nodeType == NODE_FUNCDEC);
#endif
        p->gen_var_decl();
#ifdef varDeclDebug
        PR("up to here, the stacksize is %d\n", stackSize);
#endif
        printf("  sub esp, %d\n", -stackSize);

        p = p->sibling; // compst
#ifdef DEBUG        
        assert(p->nodeType == NODE_COMPST);
#endif
        p->genCode();

        if (child->child->type->type == VALUE_VOID) {
            printf("  leave\n  ret\n");
        }

        printf("\n");
        break;
    case NODE_COMPST:
        p = p->child; // deflist
        while (p) // deflist stmtlist
        {
            p->genCode();
            p = p->sibling;
        }
        break;
    case NODE_DEFL:
        while (p) {
            TreeNode *dec = p->child->sibling->child;
            while (dec) {
                if (dec->optype == OP_ASSIGN) {
                    int pos = LocalVarList[dec->child->var_name];
                    TreeNode *expr = dec->child->sibling;
                    expr->genCode();
                    printf("  pop eax\n  mov DWORD PTR[ebp%+d], eax\n", pos);
                }
                dec = dec->sibling; //dec
            }
            p = p->sibling; // def
        }
        break;
    case NODE_STMTL:
        while (p) {
            if (p->stype == STMT_RETURN) {
                p->child->genCode(); // expr
                if (p->child->etype == EXPR_FUNCALL) {
                    printf("  leave\n  ret\n");
                } else {
                    printf("  pop eax\n  leave\n  ret\n");
                }
            } else if (p->stype == STMT_EXPR) {
                p->child->genCode(); // expr
            } else if (p->stype == STMT_IF) {
                p->child->genCode(); // expr cond
                int tmp = cnt_if;
                cnt_if++;
                printf("  pop eax\n  cmp eax, 0\n  je .L_if_end_%d\n", tmp);
                p->child->sibling->genCode(); // compSt
                printf(".L_if_end_%d:\n", tmp);
            } else if (p->stype == STMT_IF_ELSE) {
                p->child->genCode(); // expr cond
                int tmp = cnt_if;
                cnt_if++;
                printf("  pop eax\n  cmp eax, 0\n  je .L_if_end_%d\n", tmp);
                p->child->sibling->genCode(); // compSt1
                printf("  jmp .L_else_end_%d\n", tmp);
                printf(".L_if_end_%d:\n", tmp);
                p->child->sibling->sibling->genCode(); // compSt2
                printf(".L_else_end_%d:\n", tmp);
            } else if (p->stype == STMT_WHILE) {
                // 进入时将label压栈
                label_stack.push(cnt_while);
                cnt_while++;
                printf(".L_while_cond_%d:\n", label_stack.top());
                // gen cond
                p->child->genCode();
                printf("  pop eax\n  cmp eax, 0\n  je .L_while_end_%d\n", label_stack.top());
                // gen compst
                p->child->sibling->genCode();
                printf("  jmp .L_while_cond_%d\n", label_stack.top());
                // 退出
                printf(".L_while_end_%d:\n", label_stack.top());
                label_stack.pop();
            } else if (p->stype == STMT_CONTINUE) {
                printf("  jmp .L_while_cond_%d\n", label_stack.top());
            } else if (p->stype == STMT_BREAK) {
                printf("  jmp .L_while_end_%d\n", label_stack.top());
            }
            p = p->sibling; // stmt
        }
        break;
    case NODE_EXPR:
        if (etype == EXPR_CONST) {
            printf("  push %d\n", p->int_val);
        } else if (etype == EXPR_ID) {
            int pos = LocalVarList[p->var_name];
            printf("  mov eax, DWORD PTR[ebp%+d]\n  push eax\n", pos);
        } else if (etype == EXPR_UNARY) {
            p->genCode();
            if (p->etype == EXPR_FUNCALL) {
                printf("  push eax\n");
            }
            printf("  pop eax\n");
            this->gen_operate();
        } else if (etype == EXPR_BINARY) {
            if (optype == OP_ASSIGN) {
                int pos = LocalVarList[p->child->var_name];
                p->sibling->genCode();
                if (p->sibling->etype == EXPR_FUNCALL) {
                    printf("  mov DWORD PTR[ebp%+d], eax\n", pos);
                } else {
                    printf("  pop eax\n  mov DWORD PTR[ebp%+d], eax\n", pos);
                }
            } else {
                p->genCode();
                if (p->etype == EXPR_FUNCALL) {
                    printf("  push eax\n");
                }
                p->sibling->genCode();
                if (p->sibling->etype == EXPR_FUNCALL) {
                    printf("  push eax\n");
                }
                printf("  pop ebx\n  pop eax\n");
                this->gen_operate();
            }
        } else if (etype == EXPR_FUNCALL) {
            TreeNode *args = p->sibling;
            if (args) {
                int N = 0, n = 1;
                p = args->child;
                N = args->getChildNum();
#ifdef childNumdebug
                cout << "# ChildNum = " << N << endl;
#endif
                // 反转链表
                TreeNode **q = new TreeNode * [N];
                while (p) {
                    q[N - n++] = p;
                    p = p->sibling;
                }
                for (int i = 0; i < N; i++) {
                    q[i]->genCode();
                    if (q[i]->etype == EXPR_FUNCALL) {
                        printf("  push eax\n");
                    }
                }
                if (child->var_name == "println_int") {
                    printf("  push offset format_str\n  call printf\n  add esp, 8\n");
                } else {
                    int esp = N * 4;
                    printf("  call %s\n  add esp, %d\n", child->var_name.c_str(), esp);
                }
            } else {
                printf("  call %s\n", child->var_name.c_str());
            }
        }
        break;
    default:
        break;
    }
}

void TreeNode::gen_header() {
    printf(".intel_syntax noprefix\n");
    TreeNode *p = child;
    while (p) {
#ifdef DEBUG
        assert(p->nodeType == NODE_EXTDEF);
#endif
        printf(".global %s\n", p->child->sibling->child->var_name.c_str());
        p = p->sibling;
    }
    printf(".data\nformat_str:\n.asciz \"%%d\\n\"\n");
    printf(".extern printf\n.text\n\n");
}

void TreeNode::gen_var_decl() {
    if (nodeType == NODE_PROG) {
        return;
    } else if (nodeType == NODE_FUNCDEC) {
        // 对于函数声明语句，递归查找局部变量声明
        LocalVarList.clear();
        stackSize = -4;
        int paramSize = 8; // ebp+8
#ifdef varDeclDebug
        PR("# gen_var_decl in funcDecl init\n");
#endif
        // 遍历参数定义列表
        TreeNode *p = child; // vardec
        if (p->sibling) {
            p = p->sibling->child; // paramdec
            while (p) {
                LocalVarList[p->child->sibling->var_name] = paramSize;
                paramSize += 4;
                p = p->sibling; // paramdec
            }
        }
#ifdef varDeclDebug
        PR("# gen_var_decl in funcDecl param fin\n");
#endif
        // 遍历代码段，查找函数内声明的局部变量
        p = sibling->child; // complist
        assert(p->nodeType == NODE_COMPL);
        if (p->child) {
            p = p->child; // deflist
        }
        while (p) {
            if (p->nodeType == NODE_DEFL) {
                p->gen_var_decl();
            }
            p = p->sibling;
        }
#ifdef varDeclDebug
        PR("# gen_var_decl in funcDecl fin\n");
#endif
    } else if (nodeType == NODE_DEFL) {
        // 在局部变量定义语句块内部查找
        if (!child) return;
        TreeNode *p = child; // def
        assert(p->nodeType == NODE_DEF);
#ifdef varDeclDebug
        PR("# gen_var_decl found varDecl stmt at node %d\n", nodeID);
#endif
        while (p) {
            TreeNode *q = p->child->sibling->child; // dec
            while (q) {
                TreeNode *t = q->child; // vardec
                LocalVarList[t->var_name] = stackSize;
                stackSize -= 4;
                q = q->sibling; // dec
            }
            p = p->sibling; // def
        }
    }
}

void TreeNode::gen_operate() {
    TreeNode *p = child;
    switch (optype) {
    case OP_ADD:
        printf("  add eax, ebx\n  push eax\n");
        break;
    case OP_SUB:
        printf("  sub eax, ebx\n  push eax\n");
        break;
    case OP_MUL:
        printf("  imul eax, ebx\n  push eax\n");
        break;
    case OP_DIV:
        printf("  cdq\n  idiv ebx\n  push eax\n");
        break;
    case OP_MOD:
        printf("  cdq\n  idiv ebx\n  push edx\n");
        break;
    case OP_LT:
        printf("  cmp eax, ebx\n  setl al\n  movzx eax, al\n  push eax\n");
        break;
    case OP_LE:
        printf("  cmp eax, ebx\n  setle al\n  movzx eax, al\n  push eax\n");
        break;
    case OP_GT:
        printf("  cmp eax, ebx\n  setg al\n  movzx eax, al\n  push eax\n");
        break;
    case OP_GE:
        printf("  cmp eax, ebx\n  setge al\n  movzx eax, al\n  push eax\n");
        break;
    case OP_EQ:
        printf("  cmp eax, ebx\n  sete al\n  movzx eax, al\n  push eax\n");
        break;
    case OP_NE:
        printf("  cmp eax, ebx\n  setne al\n  movzx eax, al\n  push eax\n");
        break;
    case OP_BAND:
        printf("  and eax, ebx\n  push eax\n");
        break;
    case OP_BOR:
        printf("  or eax, ebx\n  push eax\n");
        break;
    case OP_XOR:
        printf("  xor eax, ebx\n  push eax\n");
        break;
    case OP_AND:
        printf("  cmp eax, 0\n  setne al\n  movzx eax, al\n  push eax\n");
        printf("  cmp ebx, 0\n  setne al\n  movzx eax, al\n  push eax\n");
        printf("  pop ebx\n  pop eax\n  and eax, ebx\n  push eax\n");
        break;
    case OP_OR:
        printf("  cmp eax, 0\n  setne al\n  movzx eax, al\n  push eax\n");
        printf("  cmp ebx, 0\n  setne al\n  movzx eax, al\n  push eax\n");
        printf("  pop ebx\n  pop eax\n  or eax, ebx\n  push eax\n");
        break;
    case OP_COMPL:
        printf("  not eax\n  push eax\n");
        break;
    case OP_NOT:
        printf("  cmp eax, 0\n  sete al\n  movzx eax, al\n  push eax\n");
        break;
    case OP_UNM:
        printf("  neg eax\n  push eax\n");
        break;
    default:
        break;
    }
}

string PRPREFIX = "";

void TreeNode::printAST() {
    printNodeInfo();
    printChildrenId();
    PR("\n");
    TreeNode *p = this->child;
    PRPREFIX += "  ";
    while (p != nullptr) {
        p->printAST();
        p = p->sibling;
    }
    PRPREFIX = PRPREFIX.substr(0, PRPREFIX.length() - 2);
}

void TreeNode::printNodeInfo() {
    PR(PRPREFIX.c_str());
    PR("# @%d  ", this->nodeID);
    PR("line %d  ", lineno);
    PR(nodeType2String(this->nodeType));
    this->printSpecialInfo();
}

void TreeNode::printChildrenId() {
    PR(",  child:");
    TreeNode *p = this->child;
    if (p == nullptr)
        PR(" -");
    while (p) {
        PR(" @%d", p->nodeID);
        p = p->sibling;
    }
}

void TreeNode::printSpecialInfo() {
    switch (this->nodeType) {
    case NODE_CONST:
        PR(", ");
        this->printConstVal();
        break;
    case NODE_VARDEC:
        PR(YLW ",  name: ");
        PR("%s,  scope: %s" NC, var_name.c_str(), var_scope.c_str());
        break;
    case NODE_ID:
        PR(YLW ",  name: %s" NC, var_name.c_str());
        break;
    case NODE_EXPR:
        PR(", %s", eType2String(this->etype));
        if (this->etype == EXPR_BINARY || this->etype == EXPR_UNARY)
            PR(", %s", opType2String(this->optype));
        break;
    case NODE_STMT:
        PR(", %s", sType2String(this->stype));
        break;
    case NODE_TYPE:
        PR(", %s", this->type->getTypeInfo());
        break;
    case NODE_DEC:
        PR(", %s", opType2String(this->optype));
        break;
    default:
        break;
    }
}

const char *TreeNode::nodeType2String(NodeType type) {
    switch (type) {
    case NODE_ARGS:
        return GRN "<args>"  NC;
    case NODE_COMPL:
        return GRN "<compl>" NC;
    case NODE_COMPST:
        return GRN "<compst>" NC;
    case NODE_DEC:
        return GRN "<dec>" NC;
    case NODE_DECL:
        return GRN "<decl>" NC;
    case NODE_DEF:
        return GRN "<def>" NC;
    case NODE_DEFL:
        return GRN "<defl>" NC;
    case NODE_EXTDEF:
        return GRN "<extdef>" NC;
    case NODE_EXTDEFL:
        return GRN "<extdefl>" NC;
    case NODE_FUNCDEC:
        return GRN "<funcdec>" NC;
    case NODE_PARAMDEC:
        return GRN "<paramdec>" NC;
    case NODE_SPEC:
        return GRN "<spec>" NC;
    case NODE_STMTL:
        return GRN "<stmtl>" NC;
    case NODE_CONST:
        return GRN "<const>" NC;
    case NODE_VARDEC:
        return GRN "<vardec>" NC;
    case NODE_EXPR:
        return GRN "<expr>" NC;
    case NODE_TYPE:
        return GRN "<type>" NC;
    case NODE_STMT:
        return GRN "<stmt>" NC;
    case NODE_PROG:
        return GRN "<prog>" NC;
    case NODE_VARLIST:
        return GRN "<varlist>" NC;
    case NODE_ID:
        return GRN "<id>" NC;
    default:
        return GRN "<?>" NC;
    }
}

const char *TreeNode::sType2String(StmtType type) {
    switch (type) {
    case STMT_EXPR:
        return BLU "expr" NC;
    case STMT_RETURN:
        return BLU "return" NC;
    case STMT_IF:
        return BLU "if" NC;
    case STMT_IF_ELSE:
        return BLU "if-else" NC;
    case STMT_WHILE:
        return BLU "while" NC;
    case STMT_BREAK:
        return BLU "break" NC;
    case STMT_CONTINUE:
        return BLU "continue" NC;
    default:
        return BLU "(sType)?" NC;
    }
}

const char *TreeNode::opType2String(OperatorType type) {
    switch (type) {
    case OP_EQ:
        return CYN "equal" NC;
    case OP_NE:
        return CYN "not equal" NC;
    case OP_GE:
        return CYN "greater equal" NC;
    case OP_LE:
        return CYN "less equal" NC;
    case OP_ASSIGN:
        return CYN "assign" NC;
    case OP_GT:
        return CYN "greater" NC;
    case OP_LT:
        return CYN "less" NC;
    case OP_ADD:
        return CYN "add" NC;
    case OP_SUB:
        return CYN "sub" NC;
    case OP_UNM:
        return CYN "unary minus" NC;
    case OP_COMPL:
        return CYN "complement" NC;
    case OP_MUL:
        return CYN "multiply" NC;
    case OP_DIV:
        return CYN "divide" NC;
    case OP_MOD:
        return CYN "modulo" NC;
    case OP_NOT:
        return CYN "not" NC;
    case OP_AND:
        return CYN "and" NC;
    case OP_OR:
        return CYN "or" NC;
    case OP_BAND:
        return CYN "band" NC;
    case OP_BOR:
        return CYN "bor" NC;
    case OP_XOR:
        return CYN "xor" NC;
    default:
        return CYN "(opType)?" NC;
    }
}

const char *TreeNode::eType2String(ExprType type) {
    switch (type) {
    case EXPR_UNARY:
        return MGT "unary" NC;
    case EXPR_BINARY:
        return MGT "binary" NC;
    case EXPR_FUNCALL:
        return MGT "funcall" NC;
    case EXPR_ID:
        return MGT "id" NC;
    case EXPR_CONST:
        return MGT "const" NC;
    default:
        return MGT "(eType)?" NC;
    }
}

void TreeNode::printConstVal() {
    if (this->nodeType == NODE_CONST) {
        PR("%s:", this->type->getTypeInfo());
        switch (this->type->type) {
        case VALUE_INT:
            PR(RED "%d" NC, int_val);
            break;
        default:
            PR(RED "-" NC);
            break;
        }
    }
}

