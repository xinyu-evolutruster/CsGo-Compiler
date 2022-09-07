#include "ast.h"

std::string Node::Format()
{
    return "{ \"name\" : \"" + this->name + "\" }";
}
template <typename base>
std::string Node::Format(base *child)
{
    return "{ \"name\" : \"" + this->name + "\", \"children\" : [ " + child->Visualize() + " ] }";
}
template <typename base>
std::string Node::Format(std::vector<base *> *children)
{
    std::string subtree = "{ \"name\" : \"" + this->name + "\", \"children\" : [ ";
    for (auto child : *children)
    {
        subtree += child->Visualize() + ", ";
    }
    if (children->size() != 0)
        subtree[subtree.size() - 2] = ' ';
    return subtree + "] }";
}
void Node::setName(std::string newName)
{
    this->name = newName;
}

NodeWithChildren::NodeWithChildren(std::string name) : Node(name)
{
    this->children = new NodeList();
}
std::string NodeWithChildren::Visualize()
{
    return Format<Node>(this->children);
}

LogOp::LogOp(LogType logtype) : Node("LogOp"), logType(logtype)
{
    switch (logtype)
    {
    case LOG_AND:
        this->setName("AND");
        break;
    default:
        this->setName("OR");
        break;
    }
}

RelOp::RelOp(RelType reltype) : Node("RelOp"), relType(reltype)
{
    switch (reltype)
    {
    case REL_LE:
        this->setName("LE");
        break;
    case REL_LT:
        this->setName("LT");
        break;
    case REL_GE:
        this->setName("GE");
        break;
    case REL_GT:
        this->setName("GT");
        break;
    case REL_EQ:
        this->setName("EQUAL");
        break;
    case REL_UNE:
        this->setName("UNEQUAL");
        break;
    default:
        break;
    }
}

AddOp::AddOp(bool isplus) : Node("AddOp"), isPuls(isplus)
{
    if (this->isPuls)
        this->setName("Plus");
    else
        this->setName("Minus");
}

MulOp::MulOp(MulType multype) : Node("MulOp"), mulType(multype)
{
    switch (multype)
    {
    case MT_MUL:
        this->setName("Mul");
        break;
    case MT_DIV:
        this->setName("Div");
        break;
    case MT_MOD:
        this->setName("Mod");
        break;
    }
}

SysType::SysType(std::string datatype) : Node(datatype)
{
    if (datatype == "int")
        dataType = CG_INTEGER;
    else if (datatype == "float")
        dataType = CG_FLOAT;
    else if (datatype == "char")
        dataType = CG_CHAR;
    else if (datatype == "string")
        dataType = CG_STRING;
    else
        dataType = CG_VOID;
}

TypeSpec::TypeSpec(std::string *childtype) : Node("TypeSpec")
{
    childType = new SysType(*childtype);
}
std::string TypeSpec::Visualize()
{
    return Format<SysType>(this->childType);
}

std::string Program::Visualize()
{
    return Format<Decl>(this->declList);
}

Param::Param(Identifier *iden, TypeSpec *typespec, bool isarray) : NodeWithChildren("Param"), identifier(iden), typeSpec(typespec), isArray(isarray)
{
    this->children->push_back(dynamic_cast<Node *>(identifier));
    this->children->push_back(dynamic_cast<Node *>(typespec));
}

std::string Params::Visualize()
{
    if (this->isVoid)
        return Format();
    else
        return Format<Param>(this->paramList);
}

std::string LocalDecls::Visualize()
{
    return Format<VarDecl>(this->localList);
}

std::string Stmts::Visualize()
{
    return Format<Statement>(this->stmtList);
}

std::string Vars::Visualize()
{
    return Format<Variable>(this->varList);
}

std::string Exprs::Visualize()
{
    return Format<SimpleExpr>(this->exprList);
}

std::string ExprStmt::Visualize()
{
    if (isNull)
    {
        setName("empty");
        return Format();
    }
    else
        return Format<Node>(this->children);
}

std::string RetStmt::Visualize()
{
    switch (this->retType)
    {
    case RET_VOID:
        return Format<Void>(this->nvoid);
    default:
        return Format<Exprs>(this->exprs);
    }
}

std::string FuncStmt::Visualize()
{
    return Format<Call>(this->call);
}

std::string Factor::Visualize()
{
    switch (this->factorType)
    {
    case FT_SIMPLEEXPR:
        return Format<SimpleExpr>(this->simpleExpr);
    case FT_VAR:
        return Format<Variable>(this->variable);
    case FT_CALL:
        return Format<Call>(this->call);
    case FT_FLOAT:
        return Format<Float>(this->nfloat);
    case FT_INTEGER:
        return Format<Integer>(this->integer);
    case FT_CHAR:
        return Format<Char>(this->nchar);
    case FT_STRING:
        return Format<String>(this->nstring);
    }
}

std::string Args::Visualize()
{
    if (this->argList != nullptr)
        return Format<SimpleExpr>(this->argList);
    else
        return Format();
}

Term::Term(Factor *factor, MulOp *mulop, Term *term, bool isrec) : NodeWithChildren("Term"), factor(factor), mulOp(mulop), term(term), isRec(isrec)
{
    this->children->push_back(dynamic_cast<Node *>(this->factor));
    if (this->isRec)
    {
        this->children->push_back(dynamic_cast<Node *>(this->mulOp));
        this->children->push_back(dynamic_cast<Node *>(this->term));
    }
}

Call::Call(Identifier *iden, Args *args) : NodeWithChildren("Call"), identifier(iden), args(args)
{
    this->children->push_back(dynamic_cast<Node *>(this->identifier));
    this->children->push_back(dynamic_cast<Node *>(this->args));
}

Variable::Variable(Identifier *iden, SimpleExpr *simpleexpr, bool isarray) : NodeWithChildren("Variable"), identifier(iden), simpleExpr(simpleexpr), isArray(isarray)
{
    this->children->push_back(dynamic_cast<Node *>(identifier));
    if (this->isArray && this->simpleExpr != nullptr)
        this->children->push_back(dynamic_cast<Node *>(simpleExpr));
}

LogicExpr::LogicExpr(SimpleExpr *simpleexpr, LogOp *logop, LogicExpr *logicexpr, bool islogicexpr) : NodeWithChildren("LogicExpr"), simpleExpr(simpleexpr), logOp(logop), logicExpr(logicexpr), isLogicExpr(islogicexpr)
{
    this->children->push_back(dynamic_cast<Node *>(this->simpleExpr));
    if (this->isLogicExpr)
    {
        this->children->push_back(dynamic_cast<Node *>(this->logOp));
        this->children->push_back(dynamic_cast<Node *>(this->logicExpr));
    }
}

AddiExpr::AddiExpr(Term *term, AddOp *addop, AddiExpr *addiexpr, bool isrec) : NodeWithChildren("AddiExpr"), term(term), addOp(addop), addiExpr(addiexpr), isRec(isrec)
{
    this->children->push_back(dynamic_cast<Node *>(this->term));
    if (this->isRec)
    {
        this->children->push_back(dynamic_cast<Node *>(this->addOp));
        this->children->push_back(dynamic_cast<Node *>(this->addiExpr));
    }
}

SimpleExpr::SimpleExpr(AddiExpr *addiexpr1, RelOp *relop, AddiExpr *addiexpr2, bool iscompareexpr) : NodeWithChildren("SimpleExpr"), addiExpr1(addiexpr1), relOp(relop), addiExpr2(addiexpr2), isCompareExpr(iscompareexpr)
{
    this->children->push_back(dynamic_cast<Node *>(addiExpr1));
    if (this->isCompareExpr)
    {
        this->children->push_back(dynamic_cast<Node *>(relOp));
        this->children->push_back(dynamic_cast<Node *>(addiExpr2));
    }
}

ComStmt::ComStmt(LocalDecls *localdecls, Stmts *stmts) : Statement("ComStmt"), localDecls(localdecls), stmts(stmts)
{
    this->children->push_back(dynamic_cast<Node *>(this->localDecls));
    this->children->push_back(dynamic_cast<Node *>(this->stmts));
}

ExprStmt::ExprStmt(Vars *vars, Exprs *exprs, bool isnull) : Statement("ExprStmt"), vars(vars), exprs(exprs), isNull(isnull)
{
    if (!isNull)
    {
        this->children->push_back(dynamic_cast<Node *>(vars));
        this->children->push_back(dynamic_cast<Node *>(exprs));
    }
}

SelectStmt::SelectStmt(LogicExpr *logicexpr, Statement *ifstmt, Statement *elstmt, bool isifelse) : Statement("SelectStmt"), logicExpr(logicexpr), ifStmt(ifstmt), elStmt(elstmt), isIfelse(isifelse)
{
    this->children->push_back(dynamic_cast<Node *>(logicExpr));
    this->children->push_back(dynamic_cast<Node *>(ifStmt));
    if (isIfelse)
        this->children->push_back(dynamic_cast<Node *>(elStmt));
}

IterStmt::IterStmt(LogicExpr *logicexpr, Statement *stmt) : Statement("IterStmt"), logicExpr(logicexpr), statement(stmt)
{
    this->children->push_back(dynamic_cast<Node *>(this->logicExpr));
    this->children->push_back(dynamic_cast<Node *>(this->statement));
}

VarDecl::VarDecl(TypeSpec *typespec, Identifier *iden, Integer *inte, bool isarray) : Decl("VarDecl"), typeSpec(typespec), identifier(iden), integer(inte), isArray(isarray)
{
    this->children->push_back(dynamic_cast<Node *>(this->typeSpec));
    this->children->push_back(dynamic_cast<Node *>(this->identifier));
    if (this->isArray)
        this->children->push_back(dynamic_cast<Node *>(this->integer));
}

FuncDecl::FuncDecl(Identifier *iden, Params *inparams, Params *outparams, ComStmt *comstmt) : Decl("FuncDecl"), identifier(iden), inParams(inparams), outParams(outparams), comStmt(comstmt)
{
    this->children->push_back(dynamic_cast<Node *>(this->identifier));
    this->children->push_back(dynamic_cast<Node *>(this->inParams));
    this->children->push_back(dynamic_cast<Node *>(this->outParams));
    this->children->push_back(dynamic_cast<Node *>(this->comStmt));
}

Decl::Decl(std::string declname) : NodeWithChildren(declname) {}

LocalDecls::LocalDecls(LocalList *locallist) : Decl("LocalDecls"), localList(locallist) {}

Stmts::Stmts(StmtList *stmtlist) : Statement("StmtList"), stmtList(stmtlist) {}