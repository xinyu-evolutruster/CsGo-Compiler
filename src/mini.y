%{
#include "ppMacro.h"
#include "ast.h"
#include "CodeGenerator.h"
#include "ObjGenerator.h"
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>

using namespace std;

void yyerror(const char *s) { 
    std::printf("Error: %s\n", s);
    std::exit(1); 
}

int yylex();
Program* root = nullptr;

%}

%union {
    int iVal;
    float fVal;
    char cVal;
    std::string* sVal;
    Program *program;
    Identifier *identifier;
    SysType *systype;
    Integer *integer;
    Float *nfloat;
    Char *nchar;
    String *nstring;
    UnderScore *underscore;
    Void *nvoid;
    Decl *decl;
    VarDecl *vardecl;
    FuncDecl *funcdecl;
    TypeSpec *typespec;
    Param *param;
    Params *params;
    ComStmt *comstmt;
    ExprStmt *exprstmt;
    FuncStmt *funcstmt;
    SelectStmt *selectstmt;
    IterStmt *iterstmt;
    RetStmt *retstmt;
    Statement *statement;
    SimpleExpr *simpleexpr;
    LogicExpr *logicexpr;
    LocalDecls *localdecls;
    Stmts *stmts;
    AddiExpr *addiexpr;
    RelOp *relop;
    AddOp *addop;
    MulOp *mulop;
    LogOp *logop;
    Term *term;
    Factor *factor;
    Call *call;
    Variable *variable;
    Args *args;

    DeclList *decllist;
    LocalList *locallist;
    ParamList *paramlist;
    StmtList *stmtlist;
    ExprList *exprlist;
    VarList *varlist;
    ArgList *arglist;
}

%token  LP RP LB RB LCP RCP
        DOT COMMA COLON UNDERSCORE
        MUL DIV PLUS MINUS MOD
        GE GT LE LT EQUAL UNEQUAL
        ASSIGN
        SEMI
        AND OR NOT
        IF ELSE WHILE FOR
        FUNC RETURN

%token<iVal> INTEGER
%token<sVal> IDENTIFIER SYS_TYPE VOID STRING
%token<fVal> FLOAT
%token<cVal> CHAR

%type<program>                          program
%type<decllist>                         decl_list
%type<decl>                             decl
%type<vardecl>                          var_decl
%type<funcdecl>                         fun_decl
%type<typespec>                         type_specifier
%type<params>                           params
%type<param>                            param
%type<comstmt>                          compound_stmt
%type<paramlist>                        param_list
%type<locallist>                        local_decls
%type<stmtlist>                         stmt_list
%type<statement>                        stmt 
%type<exprstmt>                         expr_stmt
%type<funcstmt>                         function_stmt
%type<selectstmt>                       selection_stmt
%type<iterstmt>                         iteration_stmt
%type<retstmt>                          return_stmt
%type<exprstmt>                         expr 
%type<varlist>                          var_list
%type<logicexpr>                        logic_expr
%type<simpleexpr>                       simple_expr
%type<variable>                         var
%type<addiexpr>                         additive_expr
%type<term>                             term
%type<factor>                           factor
%type<args>                             args
%type<arglist>                          arg_list
%type<exprlist>                         expr_list
%type<relop>                            relop
%type<addop>                            addop
%type<mulop>                            mulop
%type<logop>                            logop
%type<call>                             call

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%start program
%%

program: decl_list                      { 
                                            $$ = new Program($1);
                                            root = $$;  
                                        }
                                        ;
decl_list:
    decl_list decl                      {
                                            $$ = $1;
                                            $$->push_back($2);
                                        }
    | decl                              {   $$ = new DeclList();
                                            $$->push_back($1);
                                        }
                                        ;
decl: 
    var_decl                            {   $$ = $1;
                                        }
    | fun_decl                          {   $$ = $1;
                                        }
                                        ;
var_decl:
    type_specifier IDENTIFIER SEMI      {   $$ = new VarDecl($1, new Identifier($2));
                                        }
    | type_specifier IDENTIFIER LB INTEGER RB SEMI {
                                            $$ = new VarDecl($1, new Identifier($2), new Integer($4), true);
                                        }
                                        ;
type_specifier:
    SYS_TYPE                            {   $$ = new TypeSpec($1);
                                        }
                                        ;
fun_decl:
    FUNC IDENTIFIER LP params RP LP params RP compound_stmt     
                                        {   $$ = new FuncDecl(new Identifier($2), $4, $7, $9);
                                        }
                                        ;
params:
    param_list                          {   $$ = new Params($1);
                                        }
    | VOID                              {   $$ = new Params();
                                        }
                                        ;
param_list:
    param_list COMMA param              {   $$ = $1;
                                            $$->push_back($3);
                                        }
    | param                             {   $$ = new ParamList();
                                            $$->push_back($1);
                                        }
                                        ;
param:
    IDENTIFIER type_specifier           {   $$ = new Param(new Identifier($1), $2);
                                        }
    | IDENTIFIER LB RB type_specifier   {   $$ = new Param(new Identifier($1), $4, true);
                                        }
                                        ;
compound_stmt:
    LCP local_decls stmt_list RCP       {   $$ = new ComStmt(new LocalDecls($2), new Stmts($3));
                                        }
                                        ;
local_decls:
    local_decls var_decl                {   $$ = $1;
                                            $1->push_back($2);
                                        }
    | /* empty */                       {   $$ = new LocalList(); }
                                        ;
stmt_list:
    stmt_list stmt                      {   $$ = $1;
                                            $1->push_back($2);
                                        }
    | /* empty */                       {   $$ = new StmtList();
                                        }
                                        ;
stmt:
    expr_stmt                           {   $$ = $1;
                                        }
    | compound_stmt                     {   $$ = $1;
                                        }
    | selection_stmt                    {   $$ = $1;
                                        }
    | iteration_stmt                    {   $$ = $1;
                                        }
    | return_stmt                       {   $$ = $1;
                                        }
    | function_stmt                     {   $$ = $1;
                                        }
                                        ;
expr_stmt:
    expr SEMI                           {   $$ = $1;
                                        }
    | SEMI                              {   $$ = new ExprStmt();
                                        }
                                        ;
selection_stmt:
    IF LP logic_expr RP stmt  %prec LOWER_THAN_ELSE { 
                                            $$ = new SelectStmt($3, $5);
                                        }
    | IF LP logic_expr RP stmt ELSE stmt      { 
                                            $$ = new SelectStmt($3, $5, $7, true);
                                        }
                                        ;
iteration_stmt:
    WHILE LP logic_expr RP stmt         {   $$ = new IterStmt($3, $5);
                                        }
                                        ;
return_stmt:
    RETURN SEMI                         {   $$ = new RetStmt();
                                        }
    | RETURN expr_list SEMI             {   $$ = new RetStmt($2);
                                        }
                                        ;
function_stmt:
    call SEMI                           {   $$ = new FuncStmt($1);
                                        }
                                        ;
expr:
    var_list ASSIGN expr_list           {   $$ = new ExprStmt(new Vars($1), new Exprs($3), false);
                                        }
                                        ;
expr_list:
    expr_list COMMA simple_expr         {   $$ = $1;
                                            $$->push_back($3);
                                        }
    | simple_expr                       {   $$ = new ExprList();
                                            $$->push_back($1);
                                        }
                                        ;
var_list:
    var_list COMMA var                  {   $$ = $1;
                                            $$->push_back($3);
                                        }
    | var                               {   $$ = new VarList();
                                            $$->push_back($1);
                                        }
    | UNDERSCORE                        {   $$ = new VarList();
                                            $$->push_back(new UnderScore());
                                        }
                                        ;
var:
    IDENTIFIER                          {   $$ = new Variable(new Identifier($1));
                                        }
    | IDENTIFIER LB simple_expr RB      {   $$ = new Variable(new Identifier($1), $3, true);
                                        }
    | IDENTIFIER LB RB                  {   $$ = new Variable(new Identifier($1), nullptr, true);
                                        }
                                        ;

logic_expr:
    logic_expr logop simple_expr        {   $$ = new LogicExpr($3, $2, $1, true);

                                        }
    | simple_expr                       {   $$ = new LogicExpr($1);

                                        }
    ;

logop:
    AND                                 {   $$ = new LogOp(LOG_AND);

                                        }
    ;

simple_expr:
    additive_expr relop additive_expr   {   $$ = new SimpleExpr($1, $2, $3, true);
                                        }
    | additive_expr                     {   $$ = new SimpleExpr($1);
                                        }
                                        ;
relop:
    LE                                  {   $$ = new RelOp(REL_LE);
                                        }
    | LT                                {   $$ = new RelOp(REL_LT);
                                        }
    | GT                                {   $$ = new RelOp(REL_GT);
                                        }
    | GE                                {   $$ = new RelOp(REL_GE);
                                        }
    | EQUAL                             {   $$ = new RelOp(REL_EQ);
                                        }
    | UNEQUAL                           {   $$ = new RelOp(REL_UNE);
                                        }
                                        ;

additive_expr:
    additive_expr addop term            {   $$ = new AddiExpr($3, $2, $1, true);
                                        }
    | term                              {   $$ = new AddiExpr($1);
                                        }
                                        ;
addop:
    PLUS                                {   $$ = new AddOp();
                                        }
    | MINUS                             {   $$ = new AddOp(false);
                                        }
                                        ;
term:
    term mulop factor                   {   $$ = new Term($3, $2, $1, true);
                                        }
    | factor                            {   $$ = new Term($1);
                                        }
                                        ;
mulop:
    MUL                                 {   $$ = new MulOp(MT_MUL);
                                        }
    | DIV                               {   $$ = new MulOp(MT_DIV);
                                        }
    | MOD                               {   $$ = new MulOp(MT_MOD);
                                        }
                                        ;
factor:
    LP simple_expr RP                   {   $$ = new Factor($2);
                                        }
    | var                               {   $$ = new Factor($1);
                                        }
    | call                              {   $$ = new Factor($1);
                                        }
    | FLOAT                             {   $$ = new Factor(new Float($1));
                                        }
    | INTEGER                           {   $$ = new Factor(new Integer($1));
                                        }
    | MINUS FLOAT                       {   $$ = new Factor(new Float(-$2));
                                        }
    | MINUS INTEGER                     {   $$ = new Factor(new Integer(-$2));
                                        }
    | CHAR                              {   $$ = new Factor(new Char($1));
                                        }
    | STRING                            {   $$ = new Factor(new String($1));
                                        }
                                        ;
call:
    IDENTIFIER LP args RP               {   $$ = new Call(new Identifier($1), $3);
                                        }
                                        ;
args:
    arg_list                            {   $$ = new Args($1);
                                        }
    | /* empty */                       {   $$ = new Args(); }
                                        ;
arg_list:
    arg_list COMMA simple_expr          {   $$ = $1;
                                            $$->push_back($3);
                                        }
    | simple_expr                       {   $$ = new ArgList();
                                            $$->push_back($1);
                                        }
%%

int main(int argc, char** argv) {
    extern int yyparse(void);
	extern FILE* yyin;
    if (argc == 1) {
        printf("Please provide the file's name\n");
    } 
    else if (argc == 2) {
        if ((yyin = fopen(preProcess(std::string(argv[1])).c_str(), "r")) == NULL){
	        printf("Can't open file %s\n", argv[1]);
	        return 1;
	    }
        if (yyparse()) {
            fprintf(stderr, "Error\n");
            exit(1);
        }
    } 
    else {
        printf("Wrong parameters\n");
    }

    if(root != nullptr){
        std::ofstream os("ast.json");
        os << root->Visualize() << std::endl;
    }

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    std::string filename = std::string(argv[1]);
    filename = filename.substr(0, filename.length()-3);
    std::string ll_filename = filename+".ll";
    std::string o_filename = filename+".o";
    CodeGenerator generator;
    generator.generateCode(*root, ll_filename);
    ObjGen(generator, o_filename);

    return 0;
}
