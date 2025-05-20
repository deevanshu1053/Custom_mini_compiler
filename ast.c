#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

#define MAX_VARS 100

typedef struct {
    char *name;
    int value;
} VarEntry;

VarEntry symbol_table[MAX_VARS];
int symbol_count = 0;

int get_var(const char *name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0)
            return symbol_table[i].value;
    }
    printf("Undefined variable: %s\n", name);
    exit(1);
}

void set_var(const char *name, int value) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            symbol_table[i].value = value;
            return;
        }
    }
    symbol_table[symbol_count].name = strdup(name);
    symbol_table[symbol_count].value = value;
    symbol_count++;
}


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


void print_indent(int indent) {
    for (int i = 0; i < indent; i++) printf("  ");
}

void print_ast(ASTNode *node, int indent) {
    if (!node) return;

    print_indent(indent);

    switch (node->type) {
        case NODE_NUM:
            printf("Num: %d\n", node->data.num_val);
            break;
        case NODE_ID:
            printf("Id: %s\n", node->data.id_name);
            break;
        case NODE_BINOP:
            printf("BinOp: %s\n", node->data.binop.op);
            print_ast(node->data.binop.left, indent + 1);
            print_ast(node->data.binop.right, indent + 1);
            break;
        case NODE_ASSIGN:
            printf("Assign: %s =\n", node->data.assign.id);
            print_ast(node->data.assign.expr, indent + 1);
            break;
        case NODE_RETURN:
            printf("Return:\n");
            print_ast(node->data.ret.expr, indent + 1);
            break;
        case NODE_IF:
            printf("If:\n");
            print_indent(indent + 1); printf("Cond:\n");
            print_ast(node->data.if_stmt.cond, indent + 2);
            print_indent(indent + 1); printf("Then:\n");
            print_ast(node->data.if_stmt.then_branch, indent + 2);
            if (node->data.if_stmt.else_branch) {
                print_indent(indent + 1); printf("Else:\n");
                print_ast(node->data.if_stmt.else_branch, indent + 2);
            }
            break;
        case NODE_WHILE:
            printf("While:\n");
            print_indent(indent + 1); printf("Cond:\n");
            print_ast(node->data.while_stmt.cond, indent + 2);
            print_indent(indent + 1); printf("Body:\n");
            print_ast(node->data.while_stmt.body, indent + 2);
            break;
        case NODE_FOR:
            printf("For:\n");
            print_indent(indent + 1); printf("Init:\n");
            print_ast(node->data.for_stmt.init, indent + 2);
            print_indent(indent + 1); printf("Cond:\n");
            print_ast(node->data.for_stmt.cond, indent + 2);
            print_indent(indent + 1); printf("Inc:\n");
            print_ast(node->data.for_stmt.inc, indent + 2);
            print_indent(indent + 1); printf("Body:\n");
            print_ast(node->data.for_stmt.body, indent + 2);
            break;
        case NODE_BLOCK:
            printf("Block:\n");
            for (int i = 0; i < node->data.block.count; i++) {
                print_ast(node->data.block.statements[i], indent + 1);
            }
            break;
        case NODE_PRINT:
            printf("Print: %s\n", node->data.print_stmt.id);
            break;
    }
}


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
            for (int i = 0; i < node->data.block.count; i++)
                interpret(node->data.block.statements[i]);
            break;
        case NODE_WHILE:
            while (eval_expr(node->data.while_stmt.cond)) {
                interpret(node->data.while_stmt.body);
            }
            break;
        case NODE_FOR:
            interpret(node->data.for_stmt.init);
            while (eval_expr(node->data.for_stmt.cond)) {
                interpret(node->data.for_stmt.body);
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
        default:

            break;
    }
}


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
        default:
            break;
    }
}


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
    }
    free(node);
}
