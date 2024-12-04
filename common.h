#ifndef COMMON_H
#define COMMON_H

// #define DEBUG

#include <cstdio>
#include <cmath>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <assert.h>

#include <vector>
#include <map>
#include <stack>
#include <unordered_map>

using namespace std;
#define YYSTYPE TreeNode*

#ifdef DEBUG

#define VERBOSE
#define LOCAL
#define AST
#define PARSER_DEBUG
#ifdef PARSER_DEBUG
#define SCOPE_DEBUG
// #define ID_TOKEN_DEBUG
// #define ID_REDUCE_DEBUG
// #define DECL_DEBUG
// #define ASSIGN_DEBUG
// #define FUNCALL_DEBUG
#endif
// #define typeCheck_debug
// #define childNumdebug
#define varDeclDebug

#define PR(...) fprintf(stderr,__VA_ARGS__)
#else

#define PR(...) 

#endif //DEBUG

#define NC "\e[0m"
#define RED "\e[1;31m"
#define GRN "\e[1;32m"
#define YLW "\e[1;33m"
#define BLU "\e[1;34m"
#define MGT "\e[1;35m"
#define CYN "\e[1;36m"

#include "./ast.h"

#endif