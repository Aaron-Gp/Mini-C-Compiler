#include "common.h"
#include "parser.hpp"

extern void InitIOFunctionNode();
extern TreeNode *root;
extern FILE *yyin;
extern int yylineno;
extern int yyparse();
extern void init_scanner();
bool parserError = false;
bool typeError = false;

int main(int argc, char *argv[]) {
    if (argc > 1) {
        FILE *fin = fopen(argv[1], "r");
        if (fin != nullptr) {
            yyin = fin;
        } else {
            PR("failed to open file: %s\n", argv[1]);
            return 0;
        }
    } else {
        PR("no input file in argv\n");
        return 0;
    }

#ifdef LOCAL
    FILE *fp;
    fp = freopen("./test/output.s", "w", stdout);
#endif

#ifdef VERBOSE
    init_scanner();
#endif

    yylineno = 1;

    InitIOFunctionNode();
    yyparse();

    if (parserError)
        return 0;

    root->genNodeId();
#ifdef AST
    PR("# -------------------------\n");
    root->printAST();
    PR("# -------------------------\n");
#endif

    root->genCode();

    fclose(yyin);
#ifdef LOCAL
    fclose(fp);
#endif
    return 0;
}
