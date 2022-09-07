#ifndef __CODEGENERATOR_H__
#define __CODEGENERATOR_H__

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/ValueSymbolTable.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Target/TargetMachine.h>

#include <stack>
#include <vector>
#include <memory>
#include <string>
#include <map>

#include "ast.h"

using llvm::BasicBlock;
using llvm::Value;
using llvm::LLVMContext;
using llvm::Module;
using llvm::StructType;
using llvm::Type;
using llvm::Function;
using llvm::PointerType;
using llvm::FunctionType;

/*
    The class for basicblock, created when a new block is created(a pair of brace : {})
    Stores an llvm::BasicBlock object, local variables and their llvm types
    localVar stores the pointers of variables
*/
class Block{
public:
    BasicBlock * block;
    std::map<std::string, Value*> localVar;
    std::map<std::string, Type*> types;

    Block(BasicBlock* _block): block(_block){}
};

/*
    The class for generate llvm ir for an gc code
    Includes a stack for basicblocks and user-defined functions, tools for generating ir code, system function(printf, scanf)
*/
class CodeGenerator{
private:
    std::vector<Block*> blockStack;

public:
    LLVMContext llvmContext;
    llvm::IRBuilder<> builder;
    std::unique_ptr<Module> theModule;
    Function *printf, *scanf;
    std::map<std::string, Type*> funcStack; 

    CodeGenerator(): builder(llvmContext){
        theModule = std::unique_ptr<Module>(new Module("main", this->llvmContext));
    }

    // Transform from CsGo types to llvm types
    Type* getLLVMType(BuildInType type){
        switch(type){
            case CG_INTEGER : return Type::getInt32Ty(llvmContext);
            case CG_FLOAT : return Type::getDoubleTy(llvmContext);
            case CG_CHAR : return Type::getInt8Ty(llvmContext);
            case CG_STRING : return Type::getInt8PtrTy(llvmContext);
            case CG_VOID : return Type::getVoidTy(llvmContext);
            case CG_LONG : return Type::getInt64Ty(llvmContext);
        }
        //todo: error, no such type
    }

    // Get llvm pointer type of CsGo type
    Type* getLLVMPtrType(BuildInType type){
        switch(type){
            case CG_INTEGER : return Type::getInt32PtrTy(llvmContext);
            case CG_FLOAT : return Type::getDoublePtrTy(llvmContext);
            case CG_CHAR : return Type::getInt8PtrTy(llvmContext);
            case CG_STRING : return PointerType::getUnqual(Type::getInt8PtrTy(llvmContext));
        }
        //todo: error, no such type
    }

    // Get the pointer of a variable
    Value* getSymValue(std::string name){
        // Search from the top of the stack
        // So if there are variables of the same name, we will get the closest one 
        for(auto current=blockStack.rbegin(); current!=blockStack.rend(); current++){
            if((*current)->localVar.find(name) != (*current)->localVar.end()){
                return (*current)->localVar[name];
            }
        }
        return nullptr;
    }

    // Store the pointer of a variable in the current block
    void setSymValue(std::string name, Value* value){
        blockStack.back()->localVar[name] = value;
    }

    // Get the llvm type of a variable
    Type* getSymType(std::string name){
        // Search from the top of the stack
        // So if there are variables of the same name, we will get the closest one 
        for(auto current=blockStack.rbegin(); current!=blockStack.rend(); current++){
            if((*current)->types.find(name) != (*current)->types.end()){
                return (*current)->types[name];
            }
        }
        return getLLVMType(CG_VOID);
    }

    // Set variable types in the current block
    void setSymType(std::string name, Type* type){
        blockStack.back()->types[name] = type;
    }

    // Add return types of a user defined function(which is always llvm structtype)
    void addFunc(std::string name, Type* type){
        funcStack[name] = type;
    }

    // Get return types of a user defined function
    Type* getReturnType(std::string name){
        return funcStack[name];
    }

    // Push a block to the stack
    void pushBlock(BasicBlock* block){
        Block* newBlock = new Block(block);
        blockStack.push_back(newBlock);
    }

    // Pop a block from the stack
    void popBlock(){
        Block* endBlock = blockStack.back();
        blockStack.pop_back();
        // Release space 
        delete endBlock;
    }

    // Create printf
    void createPrintf()
    {
        // Input type, the first argument must be a string
        std::vector<Type*> arg_types;
        arg_types.push_back(builder.getInt8PtrTy());
        // Return type is int32, "true" means alterable number of arguments
        auto printf_type = FunctionType::get(builder.getInt32Ty(), llvm::makeArrayRef(arg_types), true);
        auto func = Function::Create(printf_type, Function::ExternalLinkage, llvm::Twine("printf"), theModule.get());
        func->setCallingConv(llvm::CallingConv::C);
        printf = func;
    }
    
    // Create scanf
    void createScanf()
    {
        // Input type, the first argument must be a string
        std::vector<Type*> arg_types;
        arg_types.push_back(builder.getInt8PtrTy());
        // Return type is int32, "true" means alterable number of arguments
        auto scanf_type = FunctionType::get(builder.getInt32Ty(), llvm::makeArrayRef(arg_types), true);
        auto func = Function::Create(scanf_type, Function::ExternalLinkage, llvm::Twine("scanf"), theModule.get());
        func->setCallingConv(llvm::CallingConv::C);
        scanf = func;
    }

    // Generate IR code
    void generateCode(Program &, std::string&);
};

#endif
