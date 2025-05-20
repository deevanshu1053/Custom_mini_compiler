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
}

%token IF ELSE RETURN FOR WHILE
%token PRINT
%token <num> NUMBER
%token <id> ID
%token EQ NE LT GT LE GE

%type <node> program statement expr block for_init for_inc
%type <stmt_list_struct> stmt_list

%left '+' '-'
%left '*' '/'
%left EQ NE LT GT LE GE
%right '='

%%

program:
    stmt_list { root = new_block($1.stmts, $1.count); }
    ;

stmt_list:
    statement {
        ASTNode **stmts = malloc(sizeof(ASTNode*));
        stmts[0] = $1;
        $$.stmts = stmts;
        $$.count = 1;
    }
    | stmt_list statement {
        ASTNode **stmts = realloc($1.stmts, sizeof(ASTNode*) * ($1.count + 1));
        stmts[$1.count] = $2;
        $$.stmts = stmts;
        $$.count = $1.count + 1;
    }
    ;

block:
    '{' stmt_list '}' { $$ = new_block($2.stmts, $2.count); }
    | /* empty */ { $$ = new_block(NULL, 0); }
    ;

statement:
    IF '(' expr ')' statement {
        $$ = new_if($3, $5, NULL);
    }
    | IF '(' expr ')' statement ELSE statement {
        $$ = new_if($3, $5, $7);
    }
    | FOR '(' for_init ';' expr ';' for_inc ')' statement {
        $$ = new_for($3, $5, $7, $9);
    }
    | WHILE '(' expr ')' statement {
        $$ = new_while($3, $5);
    }
    | ID '=' expr ';' {
        $$ = new_assign($1, $3);
        free($1);
    }
    | RETURN expr ';' {
        $$ = new_return($2);
    }
    | PRINT ID ';' {
        $$ = new_print($2);
        free($2);
    }
    | block
    ;

for_init:
    ID '=' expr { $$ = new_assign($1, $3); free($1); }
    | /* empty */ { $$ = NULL; }
    ;

for_inc:
    ID '=' expr { $$ = new_assign($1, $3); free($1); }
    | /* empty */ { $$ = NULL; }
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
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Parse error: %s\n", s);
}

