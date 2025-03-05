#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Support/ToolOutputFile.h>

using namespace llvm;

static LLVMContext context;

Module *makeLLVMModule() {
  Module *mod = new Module("sum.ll", context);
  mod->setDataLayout("e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128");
  mod->setTargetTriple("x86_64-pc-linux-gnu");

  // Construction the function.
  FunctionType *FuncTy;
  SmallVector<Type*, 2> FuncTyArgs;
  Function *funcSum;

  FuncTyArgs.push_back(IntegerType::get(context, 32));
  FuncTyArgs.push_back(IntegerType::get(context, 32));
  FuncTy = FunctionType::get(IntegerType::get(context, 32),
                             FuncTyArgs, false);

  funcSum = Function::Create(FuncTy,
                             GlobalValue::ExternalLinkage,
                             "sum", mod);
  funcSum->setCallingConv(CallingConv::C);

  // set function arg names
  Function::arg_iterator args = funcSum->arg_begin();
  Value *int32_a, *int32_b;
  int32_a = args++;
  int32_b = args++;
  int32_a->setName("a");
  int32_b->setName("b");

  // set function body
  BasicBlock *labelEntry = BasicBlock::Create(context,
                                              "entry",
                                              funcSum,
                                              0);
  // allocate memory on the stack
  AllocaInst *ptrA, *ptrB;
  ptrA = new AllocaInst(IntegerType::get(context, 32),
                        mod->getDataLayout().getAllocaAddrSpace(),
                        "a.addr",
                        labelEntry);
  ptrA->setAlignment(llvm::Align(4));
  ptrB = new AllocaInst(IntegerType::get(context, 32),
                        mod->getDataLayout().getAllocaAddrSpace(),
                        "b.addr",
                        labelEntry);
  ptrB->setAlignment(llvm::Align(4));

  // store args on stack memory just allocated
  StoreInst *st0 = new StoreInst(int32_a, ptrA, false,
                                 labelEntry);
  st0->setAlignment(llvm::Align(4));
  StoreInst *st1 = new StoreInst(int32_b, ptrB, false,
                                 labelEntry);
  st1->setAlignment(llvm::Align(4));

  // load values from stack
  LoadInst *ld0 = new LoadInst(ptrA, "", false, labelEntry);
  ld0->setAlignment(llvm::Align(4));
  LoadInst *ld1 = new LoadInst(ptrB, "", false, labelEntry);
  ld1->setAlignment(llvm::Align(4));

  // add function
  BinaryOperator *addRes = BinaryOperator::Create(Instruction::Add,
                                                  ld0, ld1, "add", 
                                                  labelEntry);
  ReturnInst::Create(context, addRes, labelEntry);

  return mod;
}

int main() {
  Module *mod = makeLLVMModule();
  std::error_code ErrorInfo;
  raw_fd_ostream fos("sum.bc", ErrorInfo, sys::fs::OpenFlags::OF_None);
  
  verifyModule(*mod);
  WriteBitcodeToFile(*mod, fos);

  return 0;
}
