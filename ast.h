#ifndef AST_H
#define AST_H

typedef enum { 
    NODE_NUM,
    NODE_ID,
    NODE_BINOP,
    NODE_ASSIGN,
    NODE_RETURN,
    NODE_IF,
    NODE_WHILE,
    NODE_FOR,
    NODE_BLOCK,
    NODE_PRINT,
    NODE_FUNCDEF,
    NODE_FUNCCALL,
    NODE_BREAK
} NodeType;

typedef struct ASTNode ASTNode;

struct ASTNode {
    NodeType type;
    union {
        int num_val;
        char *id_name;
        struct {
            char op[3];
            ASTNode *left;
            ASTNode *right;
        } binop;
        struct {
            char *id;
            ASTNode *expr;
        } assign;
        struct {
            ASTNode *expr;
        } ret;
        struct {
            ASTNode *cond;
            ASTNode *then_branch;
            ASTNode *else_branch;
        } if_stmt;
        struct {
            ASTNode *cond;
            ASTNode *body;
        } while_stmt;
        struct {
            ASTNode *init;
            ASTNode *cond;
            ASTNode *inc;
            ASTNode *body;
        } for_stmt;
        struct {
            ASTNode **statements;
            int count;
        } block;
        struct {           
            char *id;
        } print_stmt;
        struct { // Function definition
            char *name;
            char **params;
            int param_count;
            ASTNode *body;
        } funcdef;
        struct { // Function call
            char *name;
            ASTNode **args;
            int arg_count;
        } funccall;
    } data;
};

ASTNode *new_num(int val);
ASTNode *new_id(char *name);
ASTNode *new_binop(const char *op, ASTNode *left, ASTNode *right);
ASTNode *new_assign(char *id, ASTNode *expr);
ASTNode *new_funcdef(char *name, char **params, int param_count, ASTNode *body);
ASTNode *new_funccall(char *name, ASTNode **args, int arg_count);
ASTNode *new_block(ASTNode **stmts, int count);
ASTNode *new_print(char *id);
ASTNode *new_return(ASTNode *expr);
ASTNode *new_if(ASTNode *cond, ASTNode *thenb, ASTNode *elseb);
ASTNode *new_while(ASTNode *cond, ASTNode *body);
ASTNode *new_for(ASTNode *init, ASTNode *cond, ASTNode *inc, ASTNode *body);
ASTNode *new_break(void);

void print_ast(ASTNode *node, int indent);
void interpret(ASTNode *node);
void free_ast(ASTNode *node);
void optimise_ast(ASTNode *root);
void generate_intermediate_code(ASTNode *root);
void print_symbol_table(void);

#endif
