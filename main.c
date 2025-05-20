#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"


extern int yyparse();
extern FILE *yyin;
extern ASTNode *root;

int main(int argc, char **argv) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            perror("Error opening file");
            return EXIT_FAILURE;
        }
    } else {
        yyin = stdin;
    }

    if (yyparse() == 0) {
        printf("Parsing succeeded.\n\n");

        if (root) {
            printf("Abstract Syntax Tree:\n");
            print_ast(root, 0);

            printf("\nIntermediate Code:\n");
            generate_intermediate_code(root);

            printf("\nOutput:\n");
            interpret(root);

            free_ast(root);
        } else {
            printf("No syntax tree produced.\n");
        }
    } else {
        printf("Parsing failed.\n");
    }

    if (yyin && yyin != stdin) {
        fclose(yyin);
    }

    return EXIT_SUCCESS;
}