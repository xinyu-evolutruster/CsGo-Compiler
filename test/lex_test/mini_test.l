%{
#include <stdio.h>
// #include <string>
int yywrap() 
{
    return 1;
}
%}

%%

[ \t]                               { ; }
[\n]                                { printf("\n"); }
"("                                 { printf("LP "); }
")"                                 { printf("RP "); }
"["                                 { printf("LB "); }
"]"                                 { printf("RB "); }
"{"                                 { printf("LCP "); }
"}"                                 { printf("RCP "); }
"."                                 { printf("DOT "); }
","                                 { printf("COMMA "); }
":"                                 { printf("COLON "); }
"_"                                 { printf("UNDERSCORE "); }
"*"                                 { printf("MUL "); }
"/"                                 { printf("DIV "); }
"+"                                 { printf("PLUS "); }
"-"                                 { printf("MINUS "); }
"%"                                 { printf("MOD "); }
"!="                                { printf("UNEQUAL "); }
"=="                                { printf("EQUAL "); }
">="                                { printf("GE "); }
"<="                                { printf("LE "); }
">"                                 { printf("GT "); }
"<"                                 { printf("LT "); }
"="                                 { printf("ASSIGN ");}
";"                                 { printf("SEMI "); }
"&&"                                { printf("AND "); }
"||"                                { printf("OR "); }
"!"                                 { printf("NOT "); }
"if"                                { printf("IF "); }
"else"                              { printf("ELSE "); }
"while"                             { printf("WHILE "); }
"func"                              { printf("FUNC "); }
"return"                            { printf("RETURN "); }
"call"                              { printf("CALL "); }
"int"|"float"|"void"                { printf("TYPE "); }
[a-zA-Z_][a-zA-Z0-9_]*              { 
                                        printf("IDENTIFIER ");
                                    } 
[0-9]+\.[0-9]+                      { 
                                        printf("FLOAT "); 
                                    }
[0-9]+                              {
                                        printf("INTEGER ");
                                    }

%%                            

int main(int argc, char **argv) {
    if (argc == 1) {
        printf("fatal error: no input files\n");
        printf("compilation terminated");
    }
    else if (argc == 2) {
        if ((yyin = fopen(argv[1], "r")) == NULL) {
            printf("error: %s: No such file or directory\n", argv[1]);
            printf("fatal error: no input files\n");
            printf("compilation terminated");
        }
        yylex();
    }
    else {
        printf("wrong parameters\n");
    }
}
