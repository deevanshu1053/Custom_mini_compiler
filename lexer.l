%{
#include "parser.tab.h"
#include <string.h>
%}

digit   [0-9]
id      [a-zA-Z_][a-zA-Z0-9_]*
space      [ \t\r\n]+

%%

"if"        { return IF; }
"else"      { return ELSE; }
"return"    { return RETURN; }
"while"     { return WHILE; }
"for"       { return FOR; }
"print"     { return PRINT; }
"=="        { return EQ; }
"!="        { return NE; }
"<="        { return LE; }
">="        { return GE; }
"<"         { return LT; }
">"         { return GT; }
"{"         { return '{'; }
"}"         { return '}'; }
"("         { return '('; }
")"         { return ')'; }
";"         { return ';'; }
"="         { return '='; }
"+"         { return '+'; }
"-"         { return '-'; }
"*"         { return '*'; }
"/"         { return '/'; }

{digit}+    { yylval.num = atoi(yytext); return NUMBER; }
{id}        { yylval.id = strdup(yytext); return ID; }

{space}        {}

.           { return yytext[0]; }

%%
