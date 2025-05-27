#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

// --- Function Table ---
#define MAX_FUNCS 100
typedef struct {
    char *name;
    ASTNode *def;
} FuncEntry;

FuncEntry func_table[MAX_FUNCS];
int func_count = 0;

void register_func(ASTNode *node) {
    if (node->type == NODE_FUNCDEF) {
        func_table[func_count].name = strdup(node->data.funcdef.name);
        func_table[func_count].def = node;
        func_count++;
    }
}

ASTNode *find_func(const char *name) {
    for (int i = 0; i < func_count; i++) {
        if (strcmp(func_table[i].name, name) == 0)
            return func_table[i].def;
    }
    return NULL;
}

// --- Symbol Table ---
typedef struct {
    char *name;
    int value;
    int is_global;
} VarEntry;

VarEntry symbol_table[100];
int symbol_count = 0;

void push_var(const char *name, int value, int is_global) {
    symbol_table[symbol_count].name = strdup(name);
    symbol_table[symbol_count].value = value;
    symbol_table[symbol_count].is_global = is_global;
    symbol_count++;
}

void pop_vars(int n) {
    for (int i = 0; i < n; i++) {
        free(symbol_table[symbol_count - 1].name);
        symbol_count--;
    }
}

int get_var(const char *name) {
    for (int i = symbol_count - 1; i >= 0; i--) {
        if (strcmp(symbol_table[i].name, name) == 0)
            return symbol_table[i].value;
    }
    printf("Undefined variable: %s\n", name);
    exit(1);
}

static int in_function = 0;

void set_var(const char *name, int value) {
    for (int i = symbol_count - 1; i >= 0; i--) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            symbol_table[i].value = value;
            return;
        }
    }
    push_var(name, value, in_function == 0 ? 1 : 0);
}

void print_symbol_table(void) {
    printf("\n--- Symbol Table ---\n");
    for (int i = 0; i < symbol_count; i++) {
        int is_latest = 1;
        for (int j = i + 1; j < symbol_count; j++) {
            if (strcmp(symbol_table[i].name, symbol_table[j].name) == 0) {
                is_latest = 0;
                break;
            }
        }
        if (is_latest && symbol_table[i].is_global) {
            printf("%s = %d\n", symbol_table[i].name, symbol_table[i].value);
        }
    }
}

// --- AST Node Constructors ---
ASTNode *new_num(int val) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_NUM;
    node->data.num_val = val;
    return node;
}

ASTNode *new_id(char *name) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_ID;
    node->data.id_name = strdup(name);
    return node;
}

ASTNode *new_binop(const char *op, ASTNode *left, ASTNode *right) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_BINOP;
    strncpy(node->data.binop.op, op, sizeof(node->data.binop.op) - 1);
    node->data.binop.op[sizeof(node->data.binop.op) - 1] = '\0';
    node->data.binop.left = left;
    node->data.binop.right = right;
    return node;
}

ASTNode *new_assign(char *id, ASTNode *expr) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_ASSIGN;
    node->data.assign.id = strdup(id);
    node->data.assign.expr = expr;
    return node;
}

ASTNode *new_return(ASTNode *expr) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_RETURN;
    node->data.ret.expr = expr;
    return node;
}

ASTNode *new_if(ASTNode *cond, ASTNode *thenb, ASTNode *elseb) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_IF;
    node->data.if_stmt.cond = cond;
    node->data.if_stmt.then_branch = thenb;
    node->data.if_stmt.else_branch = elseb;
    return node;
}

ASTNode *new_while(ASTNode *cond, ASTNode *body) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_WHILE;
    node->data.while_stmt.cond = cond;
    node->data.while_stmt.body = body;
    return node;
}

ASTNode *new_for(ASTNode *init, ASTNode *cond, ASTNode *inc, ASTNode *body) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_FOR;
    node->data.for_stmt.init = init;
    node->data.for_stmt.cond = cond;
    node->data.for_stmt.inc = inc;
    node->data.for_stmt.body = body;
    return node;
}

ASTNode *new_block(ASTNode **stmts, int count) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_BLOCK;
    node->data.block.statements = stmts;
    node->data.block.count = count;
    return node;
}

ASTNode *new_print(char *id) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_PRINT;
    node->data.print_stmt.id = strdup(id);
    return node;
}

ASTNode *new_funcdef(char *name, char **params, int param_count, ASTNode *body) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_FUNCDEF;
    node->data.funcdef.name = strdup(name);
    node->data.funcdef.params = params;
    node->data.funcdef.param_count = param_count;
    node->data.funcdef.body = body;
    return node;
}

ASTNode *new_funccall(char *name, ASTNode **args, int arg_count) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_FUNCCALL;
    node->data.funccall.name = strdup(name);
    node->data.funccall.args = args;
    node->data.funccall.arg_count = arg_count;
    return node;
}

ASTNode *new_break(void) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_BREAK;
    return node;
}

// --- Print AST as vertical tree ---
void print_ast_tree_vertical(ASTNode *node, int depth, int is_last, int *draw_vertical) {
    if (!node) return;
    for (int i = 0; i < depth; i++) {
        if (i == depth - 1) {
            printf("%s", is_last ? "└── " : "├── ");
        } else {
            printf("%s", draw_vertical[i] ? "│   " : "    ");
        }
    }
    switch (node->type) {
        case NODE_NUM:
            printf("%d\n", node->data.num_val);
            break;
        case NODE_ID:
            printf("%s\n", node->data.id_name);
            break;
        case NODE_BINOP: {
            printf("%s\n", node->data.binop.op);
            int child_draw[64];
            memcpy(child_draw, draw_vertical, sizeof(int) * depth);
            child_draw[depth] = 1;
            print_ast_tree_vertical(node->data.binop.left, depth + 1, 0, child_draw);
            child_draw[depth] = 0;
            print_ast_tree_vertical(node->data.binop.right, depth + 1, 1, child_draw);
            break;
        }
        case NODE_ASSIGN: {
            printf("=\n");
            int child_draw[64];
            memcpy(child_draw, draw_vertical, sizeof(int) * depth);
            child_draw[depth] = 1;
            print_ast_tree_vertical(new_id(node->data.assign.id), depth + 1, 0, child_draw);
            child_draw[depth] = 0;
            print_ast_tree_vertical(node->data.assign.expr, depth + 1, 1, child_draw);
            break;
        }
        case NODE_RETURN:
            printf("return\n");
            print_ast_tree_vertical(node->data.ret.expr, depth + 1, 1, draw_vertical);
            break;
        case NODE_IF: {
            printf("if\n");
            int child_draw[64];
            memcpy(child_draw, draw_vertical, sizeof(int) * depth);
            child_draw[depth] = 1;
            print_ast_tree_vertical(node->data.if_stmt.cond, depth + 1, 0, child_draw);
            child_draw[depth] = node->data.if_stmt.else_branch ? 1 : 0;
            print_ast_tree_vertical(node->data.if_stmt.then_branch, depth + 1, node->data.if_stmt.else_branch ? 0 : 1, child_draw);
            if (node->data.if_stmt.else_branch) {
                child_draw[depth] = 0;
                print_ast_tree_vertical(node->data.if_stmt.else_branch, depth + 1, 1, child_draw);
            }
            break;
        }
        case NODE_WHILE: {
            printf("while\n");
            int child_draw[64];
            memcpy(child_draw, draw_vertical, sizeof(int) * depth);
            child_draw[depth] = 1;
            print_ast_tree_vertical(node->data.while_stmt.cond, depth + 1, 0, child_draw);
            child_draw[depth] = 0;
            print_ast_tree_vertical(node->data.while_stmt.body, depth + 1, 1, child_draw);
            break;
        }
        case NODE_FOR: {
            printf("for\n");
            int child_draw[64];
            memcpy(child_draw, draw_vertical, sizeof(int) * depth);
            child_draw[depth] = 1;
            print_ast_tree_vertical(node->data.for_stmt.init, depth + 1, 0, child_draw);
            print_ast_tree_vertical(node->data.for_stmt.cond, depth + 1, 0, child_draw);
            print_ast_tree_vertical(node->data.for_stmt.inc, depth + 1, 0, child_draw);
            child_draw[depth] = 0;
            print_ast_tree_vertical(node->data.for_stmt.body, depth + 1, 1, child_draw);
            break;
        }
        case NODE_BLOCK: {
            printf("block\n");
            int n = node->data.block.count;
            for (int i = 0; i < n; i++) {
                int child_draw[64];
                memcpy(child_draw, draw_vertical, sizeof(int) * depth);
                child_draw[depth] = (i != n - 1);
                print_ast_tree_vertical(node->data.block.statements[i], depth + 1, i == n - 1, child_draw);
            }
            break;
        }
        case NODE_PRINT:
            printf("print\n");
            print_ast_tree_vertical(new_id(node->data.print_stmt.id), depth + 1, 1, draw_vertical);
            break;
        case NODE_FUNCDEF:
            printf("func %s\n", node->data.funcdef.name);
            break;
        case NODE_FUNCCALL: {
            printf("call %s\n", node->data.funccall.name);
            int n = node->data.funccall.arg_count;
            for (int i = 0; i < n; i++) {
                int child_draw[64];
                memcpy(child_draw, draw_vertical, sizeof(int) * depth);
                child_draw[depth] = (i != n - 1);
                print_ast_tree_vertical(node->data.funccall.args[i], depth + 1, i == n - 1, child_draw);
            }
            break;
        }
        case NODE_BREAK:
            printf("break\n");
            break;
    }
}

void print_ast(ASTNode *node, int indent) {
    int draw_vertical[64] = {0};
    print_ast_tree_vertical(node, 0, 1, draw_vertical);
}

// --- Evaluation ---
int eval_expr(ASTNode *node) {
    switch (node->type) {
        case NODE_NUM: return node->data.num_val;
        case NODE_ID: return get_var(node->data.id_name);
        case NODE_BINOP: {
            int l = eval_expr(node->data.binop.left);
            int r = eval_expr(node->data.binop.right);
            if (strcmp(node->data.binop.op, "+") == 0) return l + r;
            if (strcmp(node->data.binop.op, "-") == 0) return l - r;
            if (strcmp(node->data.binop.op, "*") == 0) return l * r;
            if (strcmp(node->data.binop.op, "/") == 0) return l / r;
            if (strcmp(node->data.binop.op, "==") == 0) return l == r;
            if (strcmp(node->data.binop.op, "!=") == 0) return l != r;
            if (strcmp(node->data.binop.op, "<") == 0) return l < r;
            if (strcmp(node->data.binop.op, "<=") == 0) return l <= r;
            if (strcmp(node->data.binop.op, ">") == 0) return l > r;
            if (strcmp(node->data.binop.op, ">=") == 0) return l >= r;
        }
        default: printf("Unsupported expr\n"); exit(1);
    }
}

// --- Interpretation ---
static int break_encountered = 0;

void interpret(ASTNode *node) {
    if (!node) return;
    switch (node->type) {
        case NODE_ASSIGN:
            set_var(node->data.assign.id, eval_expr(node->data.assign.expr));
            break;
        case NODE_PRINT:
            printf("%d\n", get_var(node->data.print_stmt.id));
            break;
        case NODE_BLOCK:
            for (int i = 0; i < node->data.block.count; i++) {
                interpret(node->data.block.statements[i]);
                if (break_encountered) break;
            }
            break;
        case NODE_WHILE:
            while (eval_expr(node->data.while_stmt.cond)) {
                break_encountered = 0;
                interpret(node->data.while_stmt.body);
                if (break_encountered) {
                    break_encountered = 0;
                    break;
                }
            }
            break;
        case NODE_FOR:
            interpret(node->data.for_stmt.init);
            while (eval_expr(node->data.for_stmt.cond)) {
                break_encountered = 0;
                interpret(node->data.for_stmt.body);
                if (break_encountered) {
                    break_encountered = 0;
                    break;
                }
                interpret(node->data.for_stmt.inc);
            }
            break;
        case NODE_IF:
            if (eval_expr(node->data.if_stmt.cond)) {
                interpret(node->data.if_stmt.then_branch);
            } else if (node->data.if_stmt.else_branch) {
                interpret(node->data.if_stmt.else_branch);
            }
            break;
        case NODE_FUNCDEF:
            register_func(node);
            break;
        case NODE_FUNCCALL: {
            ASTNode *func = find_func(node->data.funccall.name);
            if (!func) {
                printf("Undefined function: %s\n", node->data.funccall.name);
                exit(1);
            }
            int arg_values[func->data.funcdef.param_count];
            for (int i = 0; i < func->data.funcdef.param_count; i++) {
                arg_values[i] = eval_expr(node->data.funccall.args[i]);
            }
            in_function++;
            for (int i = 0; i < func->data.funcdef.param_count; i++) {
                push_var(func->data.funcdef.params[i], arg_values[i], 0);
            }
            interpret(func->data.funcdef.body);
            pop_vars(func->data.funcdef.param_count);
            in_function--;
            break;
        }
        case NODE_BREAK:
            break_encountered = 1;
            break;
        default:
            break;
    }
}

// --- AST Optimisation ---
ASTNode* fold_constants(ASTNode *node) {
    if (!node) return NULL;
    switch (node->type) {
        case NODE_BINOP: {
            node->data.binop.left = fold_constants(node->data.binop.left);
            node->data.binop.right = fold_constants(node->data.binop.right);
            ASTNode *l = node->data.binop.left;
            ASTNode *r = node->data.binop.right;
            if (l && r && l->type == NODE_NUM && r->type == NODE_NUM) {
                int result = 0;
                if (strcmp(node->data.binop.op, "+") == 0) result = l->data.num_val + r->data.num_val;
                else if (strcmp(node->data.binop.op, "-") == 0) result = l->data.num_val - r->data.num_val;
                else if (strcmp(node->data.binop.op, "*") == 0) result = l->data.num_val * r->data.num_val;
                else if (strcmp(node->data.binop.op, "/") == 0 && r->data.num_val != 0) result = l->data.num_val / r->data.num_val;
                else return node;
                free_ast(l);
                free_ast(r);
                ASTNode *num = new_num(result);
                free(node);
                return num;
            }
            return node;
        }
        case NODE_ASSIGN:
            node->data.assign.expr = fold_constants(node->data.assign.expr);
            return node;
        case NODE_RETURN:
            node->data.ret.expr = fold_constants(node->data.ret.expr);
            return node;
        case NODE_IF:
            node->data.if_stmt.cond = fold_constants(node->data.if_stmt.cond);
            node->data.if_stmt.then_branch = fold_constants(node->data.if_stmt.then_branch);
            if (node->data.if_stmt.else_branch)
                node->data.if_stmt.else_branch = fold_constants(node->data.if_stmt.else_branch);
            return node;
        case NODE_WHILE:
            node->data.while_stmt.cond = fold_constants(node->data.while_stmt.cond);
            node->data.while_stmt.body = fold_constants(node->data.while_stmt.body);
            return node;
        case NODE_FOR:
            node->data.for_stmt.init = fold_constants(node->data.for_stmt.init);
            node->data.for_stmt.cond = fold_constants(node->data.for_stmt.cond);
            node->data.for_stmt.inc = fold_constants(node->data.for_stmt.inc);
            node->data.for_stmt.body = fold_constants(node->data.for_stmt.body);
            return node;
        case NODE_BLOCK:
            for (int i = 0; i < node->data.block.count; i++)
                node->data.block.statements[i] = fold_constants(node->data.block.statements[i]);
            return node;
        case NODE_FUNCDEF:
            node->data.funcdef.body = fold_constants(node->data.funcdef.body);
            return node;
        case NODE_FUNCCALL: {
            for (int i = 0; i < node->data.funccall.arg_count; i++)
                node->data.funccall.args[i] = fold_constants(node->data.funccall.args[i]);
            return node;
        }
        default:
            return node;
    }
}

#include <stdbool.h>
#include <stdint.h>

#define MAX_VARS 256
static char *used_vars[MAX_VARS];
static int used_var_count = 0;

static void mark_var_used(const char *name) {
    for (int i = 0; i < used_var_count; i++)
        if (strcmp(used_vars[i], name) == 0) return;
    if (used_var_count < MAX_VARS)
        used_vars[used_var_count++] = (char*)name;
}

static void collect_used_vars(ASTNode *node) {
    if (!node) return;
    switch (node->type) {
        case NODE_ID:
            mark_var_used(node->data.id_name);
            break;
        case NODE_BINOP:
            collect_used_vars(node->data.binop.left);
            collect_used_vars(node->data.binop.right);
            break;
        case NODE_ASSIGN:
            collect_used_vars(node->data.assign.expr);
            break;
        case NODE_RETURN:
            collect_used_vars(node->data.ret.expr);
            break;
        case NODE_IF:
            collect_used_vars(node->data.if_stmt.cond);
            collect_used_vars(node->data.if_stmt.then_branch);
            if (node->data.if_stmt.else_branch)
                collect_used_vars(node->data.if_stmt.else_branch);
            break;
        case NODE_WHILE:
            collect_used_vars(node->data.while_stmt.cond);
            collect_used_vars(node->data.while_stmt.body);
            break;
        case NODE_FOR:
            collect_used_vars(node->data.for_stmt.init);
            collect_used_vars(node->data.for_stmt.cond);
            collect_used_vars(node->data.for_stmt.inc);
            collect_used_vars(node->data.for_stmt.body);
            break;
        case NODE_BLOCK:
            for (int i = 0; i < node->data.block.count; i++)
                collect_used_vars(node->data.block.statements[i]);
            break;
        case NODE_PRINT:
            mark_var_used(node->data.print_stmt.id);
            break;
        case NODE_FUNCCALL:
            for (int i = 0; i < node->data.funccall.arg_count; i++)
                collect_used_vars(node->data.funccall.args[i]);
            break;
        case NODE_FUNCDEF:
            collect_used_vars(node->data.funcdef.body);
            break;
        default:
            break;
    }
}

static bool is_var_used(const char *name) {
    for (int i = 0; i < used_var_count; i++)
        if (strcmp(used_vars[i], name) == 0) return true;
    return false;
}

ASTNode* eliminate_dead_assignments(ASTNode *node) {
    if (!node) return NULL;
    switch (node->type) {
        case NODE_BLOCK: {
            int n = node->data.block.count;
            ASTNode **stmts = node->data.block.statements;
            int new_count = 0;
            for (int i = 0; i < n; i++) {
                stmts[i] = eliminate_dead_assignments(stmts[i]);
                if (stmts[i] && stmts[i]->type == NODE_ASSIGN) {
                    if (!is_var_used(stmts[i]->data.assign.id)) {
                        free_ast(stmts[i]);
                        stmts[i] = NULL;
                        continue;
                    }
                }
                if (stmts[i]) stmts[new_count++] = stmts[i];
            }
            node->data.block.count = new_count;
            return node;
        }
        case NODE_ASSIGN:
            node->data.assign.expr = eliminate_dead_assignments(node->data.assign.expr);
            return node;
        case NODE_BINOP:
            node->data.binop.left = eliminate_dead_assignments(node->data.binop.left);
            node->data.binop.right = eliminate_dead_assignments(node->data.binop.right);
            return node;
        case NODE_IF:
            node->data.if_stmt.cond = eliminate_dead_assignments(node->data.if_stmt.cond);
            node->data.if_stmt.then_branch = eliminate_dead_assignments(node->data.if_stmt.then_branch);
            if (node->data.if_stmt.else_branch)
                node->data.if_stmt.else_branch = eliminate_dead_assignments(node->data.if_stmt.else_branch);
            return node;
        case NODE_WHILE:
            node->data.while_stmt.cond = eliminate_dead_assignments(node->data.while_stmt.cond);
            node->data.while_stmt.body = eliminate_dead_assignments(node->data.while_stmt.body);
            return node;
        case NODE_FOR:
            node->data.for_stmt.init = eliminate_dead_assignments(node->data.for_stmt.init);
            node->data.for_stmt.cond = eliminate_dead_assignments(node->data.for_stmt.cond);
            node->data.for_stmt.inc = eliminate_dead_assignments(node->data.for_stmt.inc);
            node->data.for_stmt.body = eliminate_dead_assignments(node->data.for_stmt.body);
            return node;
        case NODE_FUNCDEF:
            node->data.funcdef.body = eliminate_dead_assignments(node->data.funcdef.body);
            return node;
        default:
            return node;
    }
}

void optimise_ast(ASTNode *root) {
    fold_constants(root);
    used_var_count = 0;
    collect_used_vars(root);
    eliminate_dead_assignments(root);
}

// --- Intermediate Code Generation ---
int temp_counter = 0;
char* new_temp() {
    char *buf = malloc(16);
    sprintf(buf, "t%d", temp_counter++);
    return buf;
}

char* gen_expr_code(ASTNode *node) {
    if (!node) return NULL;
    char *res, *l, *r;
    switch (node->type) {
        case NODE_NUM: {
            res = new_temp();
            printf("%s = %d\n", res, node->data.num_val);
            return res;
        }
        case NODE_ID: {
            return strdup(node->data.id_name);
        }
        case NODE_BINOP: {
            l = gen_expr_code(node->data.binop.left);
            r = gen_expr_code(node->data.binop.right);
            res = new_temp();
            printf("%s = %s %s %s\n", res, l, node->data.binop.op, r);
            free(l); free(r);
            return res;
        }
        default:
            return NULL;
    }
}

void generate_intermediate_code(ASTNode *node) {
    if (!node) return;
    switch (node->type) {
        case NODE_ASSIGN: {
            char *rhs = gen_expr_code(node->data.assign.expr);
            printf("%s = %s\n", node->data.assign.id, rhs);
            free(rhs);
            break;
        }
        case NODE_PRINT: {
            printf("print %s\n", node->data.print_stmt.id);
            break;
        }
        case NODE_BLOCK:
            for (int i = 0; i < node->data.block.count; i++)
                generate_intermediate_code(node->data.block.statements[i]);
            break;
        case NODE_WHILE: {
            static int label_counter = 0;
            int start = label_counter++;
            int end = label_counter++;
            printf("L%d:\n", start);
            char *cond = gen_expr_code(node->data.while_stmt.cond);
            printf("ifnot %s goto L%d\n", cond, end);
            free(cond);
            generate_intermediate_code(node->data.while_stmt.body);
            printf("goto L%d\n", start);
            printf("L%d:\n", end);
            break;
        }
        case NODE_FOR: {
            static int label_counter = 1000;
            int start = label_counter++;
            int end = label_counter++;
            generate_intermediate_code(node->data.for_stmt.init);
            printf("L%d:\n", start);
            char *cond = gen_expr_code(node->data.for_stmt.cond);
            printf("ifnot %s goto L%d\n", cond, end);
            free(cond);
            generate_intermediate_code(node->data.for_stmt.body);
            generate_intermediate_code(node->data.for_stmt.inc);
            printf("goto L%d\n", start);
            printf("L%d:\n", end);
            break;
        }
        case NODE_IF: {
            static int label_counter = 2000;
            int else_label = label_counter++;
            int end_label = label_counter++;
            char *cond = gen_expr_code(node->data.if_stmt.cond);
            printf("ifnot %s goto L%d\n", cond, else_label);
            free(cond);
            generate_intermediate_code(node->data.if_stmt.then_branch);
            printf("goto L%d\n", end_label);
            printf("L%d:\n", else_label);
            if (node->data.if_stmt.else_branch)
                generate_intermediate_code(node->data.if_stmt.else_branch);
            printf("L%d:\n", end_label);
            break;
        }
        case NODE_FUNCDEF: {
            printf("func %s:\n", node->data.funcdef.name);
            generate_intermediate_code(node->data.funcdef.body);
            printf("endfunc %s\n", node->data.funcdef.name);
            break;
        }
        case NODE_FUNCCALL: {
            printf("call %s\n", node->data.funccall.name);
            for (int i = 0; i < node->data.funccall.arg_count; i++) {
                char *arg = gen_expr_code(node->data.funccall.args[i]);
                printf("arg %s\n", arg);
                free(arg);
            }
            printf("endcall\n");
            break;
        }
        default:
            break;
    }
}

// --- Free AST ---
void free_ast(ASTNode *node) {
    if (!node) return;
    switch (node->type) {
        case NODE_ID: free(node->data.id_name); break;
        case NODE_ASSIGN:
            free(node->data.assign.id);
            free_ast(node->data.assign.expr);
            break;
        case NODE_BINOP:
            free_ast(node->data.binop.left);
            free_ast(node->data.binop.right);
            break;
        case NODE_RETURN:
            free_ast(node->data.ret.expr);
            break;
        case NODE_IF:
            free_ast(node->data.if_stmt.cond);
            free_ast(node->data.if_stmt.then_branch);
            if (node->data.if_stmt.else_branch)
                free_ast(node->data.if_stmt.else_branch);
            break;
        case NODE_WHILE:
            free_ast(node->data.while_stmt.cond);
            free_ast(node->data.while_stmt.body);
            break;
        case NODE_FOR:
            free_ast(node->data.for_stmt.init);
            free_ast(node->data.for_stmt.cond);
            free_ast(node->data.for_stmt.inc);
            free_ast(node->data.for_stmt.body);
            break;
        case NODE_BLOCK:
            for (int i = 0; i < node->data.block.count; i++)
                free_ast(node->data.block.statements[i]);
            free(node->data.block.statements);
            break;
        case NODE_PRINT:
            free(node->data.print_stmt.id);
            break;
        case NODE_NUM:
            break;
        case NODE_FUNCDEF:
            free(node->data.funcdef.name);
            for (int i = 0; i < node->data.funcdef.param_count; i++) {
                free(node->data.funcdef.params[i]);
            }
            free(node->data.funcdef.params);
            free_ast(node->data.funcdef.body);
            break;
        case NODE_FUNCCALL:
            free(node->data.funccall.name);
            free(node->data.funccall.args);
            break;
        case NODE_BREAK:
            break;
    }
    free(node);
}
