#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"


extern int yyparse();
extern FILE *yyin;
extern ASTNode *root;
extern int yylineno;

int main(int argc, char **argv) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            perror("fopen");
            return 1;
        }
    } else {
        yyin = stdin;
    }

    yylineno = 1;
    if (yyparse() == 0) {
        if (!root) {
            fprintf(stderr, "Error: AST root is NULL after parsing.\n");
            if (yyin && yyin != stdin) fclose(yyin);
            return 2;
        }
        printf("--- Abstract Syntax Tree (AST) ---\n");
        print_ast(root, 0);

        // Optimise and print only optimized intermediate code
        optimise_ast(root);
        printf("\n--- Intermediate Code ---\n");
        generate_intermediate_code(root);

        printf("\nOutput\n");
        interpret(root);

        print_symbol_table();

        free_ast(root);
    } else {
        fprintf(stderr, "Parsing failed.\n");
    }

    if (yyin && yyin != stdin) {
        fclose(yyin);
    }
    return 0;
}