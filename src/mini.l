%{
#include <stdio.h>
#include <string>
#include <cstring>
#include <sstream>
#include "ast.h"
#include "parser.hpp"

extern "C" int yywrap() {return 1;}
int clean_string () {
    char* mystr;
    mystr = strdup(yytext+1) ; // copy yytext and remove leading "
    if (!mystr) return 1;
    mystr[yyleng-2] = '\0';    // remove trailing "
    for (int i = 0, j = 0; i <= strlen(mystr); i++, j++) { // "<=" and not "<" to get /0, i : mystr indice and j : yytext indice
        if (mystr[i] == '\\') {
            i++;
            if (mystr[i] == 'n')        yytext[j]='\n';
            else if (mystr[i] == 'r')   yytext[j]='\r';
            else if (mystr[i] == 't')   yytext[j]='\t';
            else if (mystr[i] == '0')   yytext[j]='\0';
            else yytext[j] = mystr[i];
        }
        else yytext[j] = mystr[i];
    }
    yyleng=strlen(yytext);
    free(mystr);
    return 0 ;
}

int clean_char () {
    char* mystr;
    mystr = strdup(yytext) ; // copy yytext
    if (!mystr) return 1;
    for (int i = 0, j = 0; i <= strlen(mystr); i++, j++) { // "<=" and not "<" to get /0, i : mystr indice and j : yytext indice
        if (mystr[i] == '\\') {
            i++;
            if (mystr[i] == 'n')        yytext[j]='\n';
            else if (mystr[i] == 'r')   yytext[j]='\r';
            else if (mystr[i] == 't')   yytext[j]='\t';
            else if (mystr[i] == '0')   i--;
            else yytext[j] = mystr[i];
        }
        else yytext[j] = mystr[i];
    }
    yyleng=3;
    free(mystr);
    return 0 ;
}

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
"void"                              {
                                        yylval.sVal = new std::string(yytext, yyleng);
                                        return VOID; 
                                    }
"int"|"float"|"char"|"string"       {
                                        yylval.sVal = new std::string(yytext, yyleng);
                                        return SYS_TYPE;
                                    }
[a-zA-Z_][a-zA-Z0-9_]*              { 
                                        yylval.sVal = new std::string(yytext, yyleng);
                                        return IDENTIFIER; 
                                    } 
([0-9]+\.[0-9]*)([eE][-+]?[0-9]+)?  { 
                                        float ftmp;
                                        sscanf(yytext, "%f", &ftmp);
                                        yylval.fVal = ftmp;
                                        return FLOAT; 
                                    }
(0|[1-9][0-9]*)                     {
                                        int itmp;
                                        sscanf(yytext, "%d", &itmp);
                                        yylval.iVal = itmp; 
                                        return INTEGER; 
                                    }
[\']([^\n\t]|[\\]n|[\\]t|[\\]0|[\\]r)[\'] {
                                        char ctmp;
                                        if (yyleng == 4) {
                                            clean_char();
                                        }
                                        if (strcmp(yytext, "\'\\0\'") == 0) {
                                            ctmp = '\0';
                                        }
                                        else {
                                            sscanf(yytext, "\'%c\'", &ctmp);
                                        }
                                        yylval.cVal = ctmp;
                                        return CHAR;
                                    }
\"(\\.|[^"])*\"                     {
                                        clean_string();
                                        yylval.sVal = new std::string(yytext, yyleng);
                                        return STRING;
                                    }

%%      
