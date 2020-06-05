#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DerivedTypes.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <regex>

using namespace llvm;

namespace {

SmallVector<CallInst *, 50> getMatchingCalls(Function &F,  
                                            SmallVector<std::string, 10> fnNames, bool instrAll){
  SmallVector<CallInst *, 50> outvec;
  for (Function::iterator bb = F.begin(), e=F.end(); bb!=e; ++bb){
    for(BasicBlock::iterator instr = bb->begin(), ie = bb->end(); instr != ie; ++instr){
      if (CallInst *call = dyn_cast<CallInst>(instr)){
          if(instrAll){
              outvec.push_back(call);
          }
          else{
            Function *fn = call->getCalledFunction();
            for(std::string *cstr = fnNames.begin(), *e=fnNames.end(); cstr!=e; ++cstr){
                std::regex re(*cstr);
                if(fn && std::regex_match((std::string)fn->getName(), re)){
                    // std::cerr << *cstr << "\n";
                    outvec.push_back(call);
                    break;
                }
            }
        }
      }
    }
  }
  return outvec;
}

struct Instr : public FunctionPass {
  static char ID;
  SmallVector<std::string, 10> fnNames;
  bool list = false;
  Instr() : FunctionPass(ID) {
      std::ifstream InstrFile("instrfile.txt");
      if(InstrFile.good()){
          list = true;
          std::string str;
          std::cerr << "Instrumentation file found -- instr only these files\n";
          while(!InstrFile.eof()){
              std::getline(InstrFile, str);
              if(!str.empty()){
                std::cerr << str << "\n";
                fnNames.push_back(str);
              }
          }
        //   for(std::string *cstr = fnNames.begin(), *e=fnNames.end(); cstr!=e; ++cstr){
        //       std::cerr << *cstr << "\n";
        //       StringRef testStr = (StringRef)(*cstr);
        //       std::string tStr = (std::string)testStr;
        //       std::cerr << tStr << "\n";
        //   }
      }
      else{
          std::cerr << "Instumentation file not found - - instrumenting all\n";
      }
  }

  bool runOnFunction(Function &F) override {
    LLVMContext &ctx = F.getContext();
    StringRef name = F.getName();

    IRBuilder<> builder(ctx);
    // IRBuilder<> cnstBuilder(ctx);
    builder.SetInsertPoint(dyn_cast<Instruction>(F.begin()->begin()));
    Constant *fn_name = builder.CreateGlobalStringPtr(F.getName());
    // FunctionType *logGenFuncType = FunctionType::get(Type::getVoidTy(ctx), {Type::getInt8PtrTy(ctx), Type::getInt64Ty(ctx)}, false);
    // FunctionCallee logGenFunc = F.getParent()->getOrInsertFunction("loggen", logGenFuncType);
    // builder.CreateCall(logGenFunc, {fn_name, builder.getInt64(0)});
    // // Set log instructions at terminating instructions
    // for(Function::iterator bb = F.begin(), e=F.end(); bb!=e; ++bb){
    //   for(BasicBlock::iterator i = bb->begin(), ie=bb->end(); i!=ie; ++i){
    //     if(ReturnInst *ri = dyn_cast<ReturnInst>(i)){
    //       builder.SetInsertPoint(ri);
    //       builder.CreateCall(logGenFunc, {fn_name, builder.getInt64(1)});
    //     }
    //   }
    // }
    SmallVector<CallInst *, 50>  calls = getMatchingCalls(F, fnNames, !list);
    // SmallVector<CallInst *, 5> posts = getCallsWithName(F, "sem_post");
    for(CallInst **i= calls.begin(), **e=calls.end(); i!=e; ++i){
      builder.SetInsertPoint(*i);
      std::vector<Type*> paramTypes = {Type::getInt8PtrTy(ctx), Type::getInt8PtrTy(ctx)};
      FunctionType *logFuncType = FunctionType::get(Type::getVoidTy(ctx), paramTypes, false); //paramtypes
      FunctionCallee logFunc = F.getParent()->getOrInsertFunction("genlog", logFuncType);
      StringRef callName = (*i)->getCalledFunction()->getName();
      builder.CreateCall(logFunc, {fn_name, builder.CreateGlobalStringPtr(callName)});
    }
    // for(CallInst **i= posts.begin(), **e=posts.end(); i!=e; ++i){
    //   builder.SetInsertPoint((*i)->getNextNode());
    //   std::vector<Type*> paramTypes = {Type::getInt8PtrTy(ctx), (*i)->getArgOperand(0)->getType(), Type::getInt64Ty(ctx)};
    //   FunctionType *logFuncType = FunctionType::get(Type::getVoidTy(ctx), paramTypes, false);
    //   FunctionCallee logFunc = F.getParent()->getOrInsertFunction("logsem", logFuncType);
    //   builder.CreateCall(logFunc,{fn_name, (*i)->getArgOperand(0), builder.getInt64(0)});
    // }
    return true;
  }
}; // end of struct test
}  // end of anonymous namespace

char Instr::ID = 0;
static RegisterPass<Instr> X("Instr", "pass to check semaphore semantics for a bounded buffer problem",
                             false /* Only looks at CFG */,
                             true /* Analysis Pass */);

static RegisterStandardPasses Y(
    PassManagerBuilder::EP_EarlyAsPossible,
    [](const PassManagerBuilder &Builder,
       legacy::PassManagerBase &PM) { PM.add(new Instr()); });
