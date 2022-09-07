%{
#include <stdio.h>
#include <string>
#include "parser.hpp"
extern "C" int yywrap() {return 1;}
%}

%%

[ \t\n]                             { ; }
"("                                 {return LP; }
")"                                 {return RP; }
"["                                 {return LB; }
"]"                                 {return RB; }
"{"                                 {return LCP; }
"}"                                 {return RCP; }
"."                                 {return DOT; }
","                                 {return COMMA; }
":"                                 {return COLON; }
"_"                                 {return UNDERSCORE; }
"*"                                 {return MUL; }
"/"                                 {return DIV; }
"+"                                 {return PLUS; }
"-"                                 {return MINUS; }
"%"                                 {return MOD; }
"!="                                {return UNEQUAL; }
"=="                                {return EQUAL; }
">="                                {return GE; }
"<="                                {return LE; }
">"                                 {return GT; }
"<"                                 {return LT; }
"="                                 {return ASSIGN; }
";"                                 {return SEMI; }
"&&"                                {return AND; }
"||"                                {return OR; }
"!"                                 {return NOT; }
"if"                                {return IF; }
"else"                              {return ELSE; }
"while"                             {return WHILE; }
"func"                              {return FUNC; }
"return"                            {return RETURN; }
"int"|"float"|"void"                {
                                        yylval.sVal = new std::string(yytext, yyleng);
                                        return SYS_TYPE;
                                    }
[a-zA-Z_][a-zA-Z0-9_]*              { 
                                        yylval.sVal = new std::string(yytext, yyleng);
                                        return IDENTIFIER; 
                                    } 
[0-9]+\.[0-9]+                      { 
                                        float ftmp;
                                        sscanf(yytext, "%f", &ftmp);
                                        yylval.fVal = ftmp;
                                        return FLOAT; 
                                    }
[0-9]+                              {
                                        int itmp;                                                           double tmp;
                                        sscanf(yytext, "%d", &itmp);
                                        yylval.iVal = itmp; 
                                        return INTEGER; 
                                    }

%%                            
