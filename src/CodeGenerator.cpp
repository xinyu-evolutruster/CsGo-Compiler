#include "CodeGenerator.h"

// Generate IR code
void CodeGenerator::generateCode(Program & root, std::string& filename){
    // Create main function and entry block
    FunctionType* mainFuncType = FunctionType::get(Type::getVoidTy(this->llvmContext), false);
    Function* mainFunc = Function::Create(mainFuncType, llvm::GlobalValue::ExternalLinkage, "main");
    BasicBlock* block = BasicBlock::Create(this->llvmContext, "entry");

    // Create system function
    createPrintf();
    createScanf();

    // Generate IR code from the root
    pushBlock(block);
    root.Generate(*this);
    popBlock();

    // Output IR code to a certain file
    std::error_code EC;
    llvm::raw_fd_ostream dest(filename.c_str(), EC, llvm::sys::fs::OF_None);

    if(EC) {
        llvm::errs() << "Could not open file: " << EC.message();
        return;
    }
    
    llvm::legacy::PassManager passManager;
    passManager.add(createPrintModulePass(dest));
    llvm::outs() << "IR code wrote to " << filename.c_str() << "\n";
    passManager.run(*(this->theModule.get()));
    return;
}

Value* NodeWithChildren::Generate(CodeGenerator &codeGen){
    return nullptr;
}

Value* LogOp::Generate(CodeGenerator &codeGen){
    return nullptr;
}

Value* RelOp::Generate(CodeGenerator &codeGen){
    return nullptr;
}

Value* AddOp::Generate(CodeGenerator &codeGen){
    return nullptr;
}

Value* MulOp::Generate(CodeGenerator &codeGen){
    return nullptr;
}

Value* Program::Generate(CodeGenerator &codeGen){
    DeclList decl = *(this->getDeclList());
    // Traverse all the declarations and generate
    for(auto& thisDecl : decl){
        thisDecl->Generate(codeGen);
    }
    return nullptr;
}

Value* SysType::Generate(CodeGenerator &codeGen){
    return nullptr;
}

Value* TypeSpec::Generate(CodeGenerator &codeGen){
    return nullptr;
}

Value* Identifier::Generate(CodeGenerator &codeGen){
    return nullptr;
}

// Generate IR code for function parameter declaration
Value* Param::Generate(CodeGenerator &codeGen){
    BuildInType inType = this->typeSpec->getType();
    Value* inst = nullptr;
    // If it's array type, create a pointer
    if(this->isArray){
        Type* type = codeGen.getLLVMPtrType(inType);
        inst = codeGen.builder.CreateAlloca(type);
        codeGen.setSymType(this->identifier->getID(), type);
    }else{
        Type* type = codeGen.getLLVMType(inType);
        inst = codeGen.builder.CreateAlloca(type);
        codeGen.setSymType(this->identifier->getID(), type);
    }
    return inst;
}

Value* Params::Generate(CodeGenerator &codeGen){
    return nullptr;
}

Value* Statement::Generate(CodeGenerator &codeGen){
    return nullptr;
}

// Generate IR code for Factor
Value* Factor::Generate(CodeGenerator &codeGen){
    switch(this->factorType){
        case FT_SIMPLEEXPR: return this->simpleExpr->Generate(codeGen);
        // If this is a pointer type, return a pointer; otherwise load and return the value 
        case FT_VAR: if(this->variable->isPointer()) return this->variable->Generate(codeGen);
                     else return codeGen.builder.CreateLoad(this->variable->Generate(codeGen));
        case FT_CALL: return this->call->Generate(codeGen);
        case FT_FLOAT: return this->nfloat->Generate(codeGen);
        case FT_INTEGER: return this->integer->Generate(codeGen);
        case FT_CHAR: return this->nchar->Generate(codeGen);
        case FT_STRING: return this->nstring->Generate(codeGen);
    }
    //todo : error
}

// Generate IR code for Term
Value* Term::Generate(CodeGenerator &codeGen){
    // Generate IR code for factors
    Value* right = this->factor->Generate(codeGen);
    // If there is only a factor, just return the value
    if(this->mulOp==nullptr) return right;
    Value* left = this->term->Generate(codeGen);
    
    //todo : type check
    bool isFloat = left->getType()->isDoubleTy() || right->getType()->isDoubleTy();
    if(!left || !right){
        return nullptr;
    }
    // Create IR instructions corresponding to operation type
    switch(this->mulOp->getOpType()){
        case MT_MUL: return isFloat ? codeGen.builder.CreateFMul(left, right, "multmpf") : codeGen.builder.CreateMul(left, right, "multmpi");
        case MT_DIV: return isFloat ? codeGen.builder.CreateFDiv(left, right, "divmpf") : codeGen.builder.CreateSDiv(left, right, "divmpi");
        case MT_MOD: return nullptr; //todo: mod
    }
    //todo:error
}

// Create IR code for function call
Value* Call::Generate(CodeGenerator &codeGen){
    std::string name = this->identifier->getID();
    Function* function;
    bool isScanf;
    // If it's system call, use system function
    // Otherwise use user-defined function
    // And label whether this is a scanf call
    if(name=="printf"){
        function = codeGen.printf;
        isScanf = false;
    }else if(name=="scanf"){
        function = codeGen.scanf;
        isScanf = true;
    }else{
        function = codeGen.theModule->getFunction(this->identifier->getID());
        isScanf = false;
    }
    //todo:error no such function
    
    std::vector<Value*> argsv;
    //todo:error args number check
    if(this->args->getArgList()!=nullptr){
        // Get argument list and traverse
        ArgList argsn = *(this->args->getArgList());
        for(auto &thisArg : argsn){
            Value* value;
            bool isString;
            Variable* var = thisArg->getVariable(isString);
            // If this is scanf function and not the first string, We need to transfer pointer types
            // Otherwise we need to transfer the value
            if(isScanf && !isString){
                //todo:error
                value = var->Generate(codeGen);
            }else{ 
                value = thisArg->Generate(codeGen);
            }
            argsv.push_back(value);
            // If any argument codeGen fail, return null
            if( !argsv.back() ){
                return nullptr;
            }
        }
    }
    // Create IR instruction for function call
    Value *res = codeGen.builder.CreateCall(function, argsv);
    return res;
}

// Create IR code for variable, always return the pointer
Value* Variable::Generate(CodeGenerator &codeGen){
    // If it's array type
    if(this->isArray){
        // Get the pointer
        std::vector<Value*> indices;
        indices.push_back(llvm::ConstantInt::get(codeGen.getLLVMType(CG_INTEGER), 0, false));
        Value* index;
        // If there is no index(eg: a[]), just get the first pointer
        // Otherwise index is need
        if(!this->isPointer()){
            index = this->simpleExpr->Generate(codeGen);
            indices.push_back(index);
        }else{
            indices.push_back(llvm::ConstantInt::get(codeGen.getLLVMType(CG_INTEGER), 0, false));
        }
        // Get first address and type(array or pointer)
        auto inst = codeGen.getSymValue(this->identifier->getID());
        auto type = codeGen.getSymType(this->identifier->getID());
        // If array
        if(type->isArrayTy()){
            // Get pointer with index
            auto ptr = codeGen.builder.CreateInBoundsGEP(inst, indices);
            return ptr;
        }else{
            // If there is no index, just return the first pointer
            if(this->isPointer()) return inst;
            // Otherwise we need to get the pointer from the first pointer and offset
            // Get elementtype to calculate offset
            auto elementType = inst->getType()->getPointerElementType();
            int length;
            BuildInType ptrtype;
            // Get size
            if(elementType->isIntegerTy(32)){
                length = 4;
                ptrtype = CG_INTEGER;
            }else if(elementType->isIntegerTy(8)){
                length = 1;
                ptrtype = CG_CHAR;
            }else if(elementType->isDoubleTy()){
                length = 8;
                ptrtype = CG_FLOAT;
            }else if(elementType->isPointerTy()){
                length = 8;
                ptrtype = CG_STRING;
            }
            // Traverse pointer to int, and add the offset, traverse int to pointer
            Value* begin = codeGen.builder.CreatePtrToInt(inst, codeGen.getLLVMType(CG_LONG));
            Value* offset = codeGen.builder.CreateZExt(index, codeGen.getLLVMType(CG_LONG));
            Value* realoffset = codeGen.builder.CreateMul(offset, codeGen.builder.getInt64(length));
            Value* add = codeGen.builder.CreateAdd(begin, realoffset);
            auto ptr = codeGen.builder.CreateIntToPtr(add, codeGen.getLLVMPtrType(ptrtype));
            return ptr;
        }
    }else{
        // If not pointer type, just get pointer from symbol table
        Value* value = codeGen.getSymValue(this->identifier->getID());
        return value;
    }
}

Value* UnderScore::Generate(CodeGenerator &codeGen){
    return nullptr;
}

Value* LogicExpr::Generate(CodeGenerator &codeGen){
    Value* right = this->simpleExpr->Generate(codeGen);
    if(this->logOp==nullptr) return right;
    Value* left = this->logicExpr->Generate(codeGen);
    
    //todo : type check
    bool isFloat = left->getType()->isDoubleTy() || right->getType()->isDoubleTy();
    if(!left || !right){
        return nullptr;
    }
    if(!this->logOp->getOpType()){
        return codeGen.builder.CreateAnd(left, right, "andtmp");
    }
}

Value* AddiExpr::Generate(CodeGenerator &codeGen){
    Value* right = this->term->Generate(codeGen);
    if(this->addOp==nullptr) return right;
    Value* left = this->addiExpr->Generate(codeGen);
    
    //todo : type check
    bool isFloat = left->getType()->isDoubleTy() || right->getType()->isDoubleTy();
    if(!left || !right){
        return nullptr;
    }
    // Pointer type + int(eg: a[] + 1), need return a new pointer
    if(left->getType()->isPointerTy())
    {
        auto elementType = left->getType()->getPointerElementType();
        int length;
        BuildInType ptrtype;
        if(elementType->isIntegerTy(32)){
            length = 4;
            ptrtype = CG_INTEGER;
        }else if(elementType->isIntegerTy(8)){
            length = 1;
            ptrtype = CG_CHAR;
        }else if(elementType->isDoubleTy()){
            length = 8;
            ptrtype = CG_FLOAT;
        }else if(elementType->isPointerTy()){
            length = 8;
            ptrtype = CG_STRING;
        }
        Value* begin = codeGen.builder.CreatePtrToInt(left, codeGen.getLLVMType(CG_LONG));
        Value* offset = codeGen.builder.CreateZExt(right, codeGen.getLLVMType(CG_LONG));
        Value* realoffset = codeGen.builder.CreateMul(offset, codeGen.builder.getInt64(length));
        Value* add;
        if(this->addOp->getOpType()) {
            add = codeGen.builder.CreateAdd(begin, realoffset);
        }else{
            add = codeGen.builder.CreateSub(begin, realoffset);
        }
        return codeGen.builder.CreateIntToPtr(add, codeGen.getLLVMPtrType(ptrtype));
    }
    // Create IR instructions and get value
    if(this->addOp->getOpType()){
        return isFloat ? codeGen.builder.CreateFAdd(left, right, "addtmpf") : codeGen.builder.CreateAdd(left, right, "addtmpi");
    }else{
        return isFloat ? codeGen.builder.CreateFSub(left, right, "subtmpf") : codeGen.builder.CreateSub(left, right, "subtmpi");
    }
    //todo: error
}

Value* SimpleExpr::Generate(CodeGenerator &codeGen){
    Value* left = this->addiExpr1->Generate(codeGen);
    if(this->relOp==nullptr) return left;
    Value* right = this->addiExpr2->Generate(codeGen);
    //todo : type check
    bool isFloat = left->getType()->isDoubleTy() || right->getType()->isDoubleTy();
    if(!left || !right){
        return nullptr;
    }
    switch(this->relOp->getOpType()){
        case REL_LE : return codeGen.builder.CreateICmpSLE(left, right, "tmpSLE");
        case REL_LT : return codeGen.builder.CreateICmpSLT(left, right, "tmpSLT");
        case REL_GE : return codeGen.builder.CreateICmpSGE(left, right, "tmpSGE");
        case REL_GT : return codeGen.builder.CreateICmpSGT(left, right, "tmpSGT");
        case REL_EQ : return codeGen.builder.CreateICmpEQ(left, right, "tmpEQ");
        case REL_UNE : return codeGen.builder.CreateICmpNE(left, right, "tmpNE");
    }
    //todo: error
}

Value* Decl::Generate(CodeGenerator &codeGen){
    return nullptr;
}

Value* LocalDecls::Generate(CodeGenerator &codeGen){
    // Traverse all declarations and generate IR code
    for(auto &thisVar: *(this->localList)){
        thisVar->Generate(codeGen);
    }
    return nullptr;
}

Value* Stmts::Generate(CodeGenerator &codeGen){
    // Traverse all statements and generate IR code
    for(auto &thisStmt : *(this->stmtList)){
        thisStmt->Generate(codeGen);
    }
    return nullptr;
}

Value* ComStmt::Generate(CodeGenerator &codeGen){
    // Generate localdecl and statements
    this->localDecls->Generate(codeGen);
    this->stmts->Generate(codeGen);
    return nullptr;
}

Value* Vars::Generate(CodeGenerator &codeGen){
    return nullptr;
}

Value* Exprs::Generate(CodeGenerator &codeGen){
    return nullptr;
}

// Generate assign statements
Value* ExprStmt::Generate(CodeGenerator &codeGen){
    // Get var list and expr list
    VarList var = *(this->vars->getVarlist());
    ExprList expr = *(this->exprs->getExprlist());
    //todo: error(different terms)
    std::vector<Value*> tmp;
    // Traverse
    for(auto &thisExpr : expr){
        auto right = thisExpr->Generate(codeGen);
        std::string funcname = thisExpr->getCallName();
        if(funcname!="" && funcname != "printf" && funcname != "scanf"){
            // Get number of struct elements and traverse
            int num = codeGen.getReturnType(funcname)->getStructNumElements();
            for(int i=0;i<num;i++){
                // Extract value and assign to a variable
                Value* structValue = codeGen.builder.CreateExtractValue(right, (uint64_t)i);
                tmp.push_back(structValue);
            }
        }else{
            // Assign directly
            tmp.push_back(right);
        }
    }
    auto thisValue = tmp.begin();
    for(auto &thisVar : var){
        std::string name = thisVar->getID();
        if(name=="") {
            thisValue++;
            continue;
        }
        codeGen.builder.CreateStore(*thisValue, thisVar->Generate(codeGen));
        thisValue++;
    }
    return nullptr;
}

Value* FuncStmt::Generate(CodeGenerator &codeGen){
    this->call->Generate(codeGen);
    return nullptr;
}

// Generate IR code for if statement
Value* SelectStmt::Generate(CodeGenerator &codeGen){
    // Get condition value
    Value* condValue = this->logicExpr->Generate(codeGen);
    if( !condValue )
        return nullptr;
    // Get function where if statement is in
    Function* theFunction = codeGen.builder.GetInsertBlock()->getParent();

    // Create 3 bodies: if, else, and end(after if or else it will jump tp end)
    BasicBlock *ifbody = BasicBlock::Create(codeGen.llvmContext, "ifbody", theFunction);
    BasicBlock *elsebody = BasicBlock::Create(codeGen.llvmContext, "elsebody");
    BasicBlock *end = BasicBlock::Create(codeGen.llvmContext, "end");

    // whether there is else
    if( this->isIfelse ){
        codeGen.builder.CreateCondBr(condValue, ifbody, elsebody);
    } else{
        codeGen.builder.CreateCondBr(condValue, ifbody, end);
    }

    codeGen.builder.SetInsertPoint(ifbody);

    // Generate IR code for if body
    codeGen.pushBlock(ifbody);
    this->ifStmt->Generate(codeGen);
    codeGen.popBlock();

    // If there is return in if body, it will not jump to end block 
    ifbody = codeGen.builder.GetInsertBlock();
    if( ifbody->getTerminator() == nullptr ){   
        // Create jump to end block 
        codeGen.builder.CreateBr(end);
    }

    // If there has a else, create else block
    if( this->isIfelse ){
        theFunction->getBasicBlockList().push_back(elsebody);    
        codeGen.builder.SetInsertPoint(elsebody);            

        // Generate IR code for else body
        codeGen.pushBlock(elsebody);
        this->elStmt->Generate(codeGen);
        codeGen.popBlock();

        // If there is return in else body, it will not jump to end block
        elsebody = codeGen.builder.GetInsertBlock();
        if(elsebody->getTerminator() == nullptr){
            // Create jump to end block
            codeGen.builder.CreateBr(end);
        }
    }

    // Create end block
    theFunction->getBasicBlockList().push_back(end);        
    codeGen.builder.SetInsertPoint(end);        

    return nullptr;
}

// Generate IR code for while statement
Value* IterStmt::Generate(CodeGenerator &codeGen){
    // Get the function where while statement is in
    Function* theFunction = codeGen.builder.GetInsertBlock()->getParent();

    // Create blocks for cond value, iter body and end
    BasicBlock *cond = BasicBlock::Create(codeGen.llvmContext, "cond", theFunction);
    BasicBlock *body = BasicBlock::Create(codeGen.llvmContext, "body");
    BasicBlock *end = BasicBlock::Create(codeGen.llvmContext, "end");

    // Former code jump to cond
    codeGen.builder.CreateBr(cond);
    codeGen.builder.SetInsertPoint(cond);
    
    // Generate cond value
    Value* condValue = this->logicExpr->Generate(codeGen);
    if( !condValue )
        return nullptr;

    // Create cond jump
    codeGen.builder.CreateCondBr(condValue, body, end);

    theFunction->getBasicBlockList().push_back(body);
    codeGen.builder.SetInsertPoint(body);

    // Create IR code for iter body
    codeGen.pushBlock(body);
    this->statement->Generate(codeGen);
    codeGen.popBlock();

    // If there is return in iter body, it will not jump to end
    body = codeGen.builder.GetInsertBlock();
    if(body->getTerminator() == nullptr){
        codeGen.builder.CreateBr(cond);
    }

    // Create end
    theFunction->getBasicBlockList().push_back(end);
    codeGen.builder.SetInsertPoint(end);

    return nullptr;
}

// Generate IR code for return statement
Value* RetStmt::Generate(CodeGenerator &codeGen){
    // Get current function and returntype
    Function* function = codeGen.builder.GetInsertBlock()->getParent();
    Type* retType = function->getReturnType();
    // If it's void, just create a void return instruction
    if(retType->isVoidTy()) return codeGen.builder.CreateRetVoid();
    // Create space for return value
    ExprList expr = *(this->exprs->getExprlist());
    auto inst = codeGen.builder.CreateAlloca(retType);
    // If it's struct
    if(retType->isStructTy()){
        // Get all member types
        for(auto thisExpr=expr.begin(); thisExpr!=expr.end(); thisExpr++){
            Value* thisValue = (*thisExpr)->Generate(codeGen);
            std::vector<Value*> indices;
            indices.push_back(llvm::ConstantInt::get(codeGen.getLLVMType(CG_INTEGER), 0, false));
            indices.push_back(llvm::ConstantInt::get(codeGen.getLLVMType(CG_INTEGER), (uint64_t)distance(expr.begin(),thisExpr), false));
            // Get the pointer and store the value
            auto ptr = codeGen.builder.CreateInBoundsGEP(inst, indices);
            codeGen.builder.CreateStore(thisValue, ptr);
        }    
    }else{
        // Just store the value, for main, scanf, printf
        codeGen.builder.CreateStore(expr.at(0)->Generate(codeGen), inst);
    }
    // Load the value and create a return instruction
    Value* returnValue = codeGen.builder.CreateLoad(inst);
    codeGen.builder.CreateRet(returnValue);
    return returnValue;
}

Value* Args::Generate(CodeGenerator &codeGen){
    return nullptr;
}

Value* Void::Generate(CodeGenerator &codeGen){
    return nullptr;
}

// Generate int type
Value* Integer::Generate(CodeGenerator &codeGen){
    return codeGen.builder.getInt32(this->value);
}

// Generate float type, we use double here
Value* Float::Generate(CodeGenerator &codeGen){
    return llvm::ConstantFP::get(codeGen.builder.getDoubleTy(), this->value);
}

// Generate string type, we use global string here
// This is different from string variable, variable needs to be a pointer
// String in the code can be store in global data area
Value* String::Generate(CodeGenerator &codeGen){
    return codeGen.builder.CreateGlobalStringPtr(this->value);
}

// Generate char type, int8
Value* Char::Generate(CodeGenerator &codeGen){
    return codeGen.builder.getInt8(this->value);
}

// Generate IR code for variable declaration
Value* VarDecl::Generate(CodeGenerator &codeGen){
    // Get type
    BuildInType inType = this->typeSpec->getType();
    Type* type = codeGen.getLLVMType(inType);
    Value* inst = nullptr;
    // If it's array, create arraytype
    if(this->isArray){
        llvm::ArrayType* arrayType = llvm::ArrayType::get(type, (uint64_t)(this->integer->getValue()));
        inst = codeGen.builder.CreateAlloca(arrayType);
        codeGen.setSymType(this->identifier->getID(), arrayType);
    }else{
        inst = codeGen.builder.CreateAlloca(type);
        codeGen.setSymType(this->identifier->getID(), type);
    }
    codeGen.setSymValue(this->identifier->getID(), inst);
    return inst;
}

// Create IR code for function declaration
Value* FuncDecl::Generate(CodeGenerator &codeGen){
    std::vector<Type*> argType, retType;
    // Get input parameter type
    if(!this->inParams->getVoid()){
        ParamList inParamList = *(this->inParams->getParamList());
        // Arraytype must be stored as pointer
        for(auto &param: inParamList){
            if(param->getIsArray()){
                argType.push_back(codeGen.getLLVMPtrType(param->getType()));
            }else{
                argType.push_back(codeGen.getLLVMType(param->getType()));
            }
        }
    } 
    // Get return parameter type, cannot return an array
    if(!this->outParams->getVoid()){
        ParamList outParamList = *(this->outParams->getParamList());
        for(auto &param: outParamList){
            retType.push_back(codeGen.getLLVMType(param->getType()));
        }
    }

    FunctionType*  functionType;
    Function* function;
    std::string funcName = this->identifier->getID();
    Type* type;

    // If return void, return type is void
    // If function is main, do not return a struct
    // Otherwise return struct
    if(this->outParams->getVoid()){
        type = codeGen.getLLVMType(CG_VOID);    
    }else if(this->identifier->getID()=="main"){
        //todo:error
        type = retType.at(0);
    }
    else{
        // Set a struct type
        auto structType = StructType::create(codeGen.llvmContext, "retType");
        structType->setBody(retType);
        type = structType;
    }

    // Create function
    functionType = FunctionType::get(type, argType, false);
    function = Function::Create(functionType, llvm::GlobalValue::ExternalLinkage, this->identifier->getID(), codeGen.theModule.get());
    BasicBlock* basicBlock = BasicBlock::Create(codeGen.llvmContext, "entry", function, nullptr);
    
    // Add function and push block
    codeGen.builder.SetInsertPoint(basicBlock);
    codeGen.pushBlock(basicBlock);
    codeGen.addFunc(funcName, type);

    // Create stores for input parameter
    if(!this->inParams->getVoid()){
        ParamList inParamList = *(this->inParams->getParamList());
        auto thisParam = inParamList.begin();
        auto thisValue = function->args().begin();
        // Traverse input parameters
        for(;thisParam!=inParamList.end();thisParam++, thisValue++){
            thisValue->setName((*thisParam)->getName());
            Value* alloc = (*thisParam)->Generate(codeGen);
            // Store the value in local variables
            codeGen.builder.CreateStore(thisValue, alloc, false);
            // If it's pointer, just store it's value
            // Otherwise store it's pointer
            if((*thisParam)->getIsArray()){
                codeGen.setSymValue((*thisParam)->getName(), codeGen.builder.CreateLoad(alloc));
            }
            else codeGen.setSymValue((*thisParam)->getName(), alloc);
        }
    }
    // Generate IR code for function body
    this->comStmt->Generate(codeGen);
    codeGen.popBlock();
    return function;
}