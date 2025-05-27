%code requires {
    #include "ast.h"
}

%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
extern int yylex();
void yyerror(const char *s);
ASTNode *root = NULL;
extern int yylineno; // For error reporting
%}

%union {
    int num;
    char *id;
    ASTNode *node;
    ASTNode **node_array;
    struct {
        ASTNode **stmts;
        int count;
    } stmt_list_struct;
    struct {
        char **ids;
        int count;
    } id_list_struct;
    struct {
        ASTNode **args;
        int count;
    } arg_list_struct;
}

%token IF ELSE RETURN FOR WHILE
%token PRINT
%token FUNC
%token BREAK
%token <num> NUMBER
%token <id> ID
%token EQ NE LT GT LE GE

%type <node> program statement expr block for_init for_inc funcdef
%type <stmt_list_struct> stmt_list
%type <id_list_struct> param_list
%type <arg_list_struct> arg_list
%type <stmt_list_struct> toplevel_list
%type <node> toplevel

%left '+' '-'
%left '*' '/'
%left EQ NE LT GT LE GE
%right '='

%%

program:
    toplevel_list { root = new_block($1.stmts, $1.count); }
    ;

toplevel_list:
      /* empty */ { $$.stmts = NULL; $$.count = 0; }
    | toplevel_list toplevel {
        $$.stmts = realloc($1.stmts, sizeof(ASTNode*) * ($1.count + 1));
        $$.stmts[$1.count] = $2;
        $$.count = $1.count + 1;
    }
    ;

toplevel:
      funcdef { $$ = $1; }
    | statement { $$ = $1; }
    ;

funcdef:
    FUNC ID '(' param_list ')' block {
        $$ = new_funcdef($2, $4.ids, $4.count, $6);
        free($2);
    }
    ;

param_list:
      /* empty */ { $$.ids = NULL; $$.count = 0; }
    | ID { $$.ids = malloc(sizeof(char*)); $$.ids[0] = $1; $$.count = 1; }
    | param_list ',' ID {
        $$.ids = realloc($1.ids, sizeof(char*) * ($1.count + 1));
        $$.ids[$1.count] = $3;
        $$.count = $1.count + 1;
    }
    ;

stmt_list:
      /* empty */ { $$.stmts = NULL; $$.count = 0; }
    | stmt_list statement {
        $$.stmts = realloc($1.stmts, sizeof(ASTNode*) * ($1.count + 1));
        $$.stmts[$1.count] = $2;
        $$.count = $1.count + 1;
    }
    ;

statement:
    ID '=' expr ';' { $$ = new_assign($1, $3); free($1); }
    | PRINT ID ';' { $$ = new_print($2); free($2); }
    | RETURN expr ';' { $$ = new_return($2); }
    | RETURN ';' { $$ = new_return(NULL); }
    | IF '(' expr ')' statement ELSE statement { $$ = new_if($3, $5, $7); }
    | IF '(' expr ')' statement { $$ = new_if($3, $5, NULL); }
    | WHILE '(' expr ')' statement { $$ = new_while($3, $5); }
    | FOR '(' for_init ';' expr ';' for_inc ')' statement { $$ = new_for($3, $5, $7, $9); }
    | BREAK ';' { $$ = new_break(); }
    | block
    | expr ';' { $$ = $1; } // For function calls as statements
    ;

for_init:
    ID '=' expr { $$ = new_assign($1, $3); free($1); }
    | /* empty */ { $$ = NULL; }
    ;

for_inc:
    ID '=' expr { $$ = new_assign($1, $3); free($1); }
    | /* empty */ { $$ = NULL; }
    ;

block:
    '{' stmt_list '}' { $$ = new_block($2.stmts, $2.count); }
    ;

expr:
    expr '+' expr { $$ = new_binop("+", $1, $3); }
    | expr '-' expr { $$ = new_binop("-", $1, $3); }
    | expr '*' expr { $$ = new_binop("*", $1, $3); }
    | expr '/' expr { $$ = new_binop("/", $1, $3); }
    | expr EQ expr  { $$ = new_binop("==", $1, $3); }
    | expr NE expr  { $$ = new_binop("!=", $1, $3); }
    | expr LT expr  { $$ = new_binop("<", $1, $3); }
    | expr LE expr  { $$ = new_binop("<=", $1, $3); }
    | expr GT expr  { $$ = new_binop(">", $1, $3); }
    | expr GE expr  { $$ = new_binop(">=", $1, $3); }
    | '(' expr ')'  { $$ = $2; }
    | NUMBER        { $$ = new_num($1); }
    | ID            { $$ = new_id($1); free($1); }
    | ID '(' arg_list ')' { $$ = new_funccall($1, $3.args, $3.count); free($1); }
    ;

arg_list:
      /* empty */ { $$.args = NULL; $$.count = 0; }
    | expr { $$.args = malloc(sizeof(ASTNode*)); $$.args[0] = $1; $$.count = 1; }
    | arg_list ',' expr {
        $$.args = realloc($1.args, sizeof(ASTNode*) * ($1.count + 1));
        $$.args[$1.count] = $3;
        $$.count = $1.count + 1;
    }
    ;

%%


void yyerror(const char *s) {
    fprintf(stderr, "Parse error at line %d: %s\n", yylineno, s);
}

