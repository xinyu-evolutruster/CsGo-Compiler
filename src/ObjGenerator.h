#ifndef __OBJGENERATOR_H__
#define __OBJGENERATOR_H__

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/ADT/Optional.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/LegacyPassManager.h>

#include "CodeGenerator.h"

void ObjGen(CodeGenerator &codeGen, const std::string &filename = "output.o")
{
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    auto targetTriple = llvm::sys::getDefaultTargetTriple();
    codeGen.theModule->setTargetTriple(targetTriple);

    std::string error;
    auto Target = llvm::TargetRegistry::lookupTarget(targetTriple, error);

    if (!Target)
    {
        llvm::errs() << error;
        return;
    }

    auto CPU = "generic";
    auto features = "";

    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    auto theTargetMachine = Target->createTargetMachine(targetTriple, CPU, features, opt, RM);

    codeGen.theModule->setDataLayout(theTargetMachine->createDataLayout());

    std::error_code EC;
    llvm::raw_fd_ostream dest(filename.c_str(), EC, llvm::sys::fs::OF_None);

    if (EC)
    {
        llvm::errs() << "Could not open file: " << EC.message();
        return;
    }

    llvm::legacy::PassManager pass;
    auto fileType = llvm::CGFT_ObjectFile;

    if (theTargetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType))
    {
        llvm::errs() << "theTargetMachine can't emit a file of this type";
        return;
    }

    pass.run(*codeGen.theModule.get());
    dest.flush();

    llvm::outs() << "Object code wrote to " << filename.c_str() << "\n";

    return;
}

#endif