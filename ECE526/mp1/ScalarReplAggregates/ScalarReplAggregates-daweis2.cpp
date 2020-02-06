//===- ScalarReplAggregates.cpp - Scalar Replacement of Aggregates --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This transformation implements the well known scalar replacement of
// aggregates transformation.  This xform breaks up alloca instructions of
// structure type into individual alloca instructions for
// each member (if possible).  Then, if possible, it transforms the individual
// alloca instructions into nice clean scalar SSA form.
//
// This combines an SRoA algorithm with Mem2Reg because they
// often interact, especially for C++ programs.  As such, this code
// iterates between SRoA and Mem2Reg until we run out of things to promote.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "scalarrepl"
#include "llvm/Transforms/Scalar.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <set>
#include <vector>

using namespace llvm;

STATISTIC(NumReplaced,  "Number of aggregate allocas broken up");
STATISTIC(NumPromoted,  "Number of scalar allocas promoted to register");

namespace {
  struct SROA : public FunctionPass {
    static char ID; // Pass identification
    SROA() : FunctionPass(ID) { }

    // Entry point for the overall scalar-replacement pass
    bool runOnFunction(Function &F);

    // getAnalysisUsage - List passes required by this pass.  We also know it
    // will not alter the CFG, so say so.
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<DominatorTreeWrapperPass>();
      AU.setPreservesCFG();
    }

  private:
    // Some helper functions
    // FIXME: ref or pointer? turn some to const finally.
    bool isZero(const Value *V);
    bool checkU1_2U(const User *U, const Value *ptr);
    bool checkU1(const User *U, const Value *ptr);
    bool checkU2(const User *U);
    bool expandAggreateAlloca(AllocaInst *AI, std::set<AllocaInst *> *worklist);
    bool isAllocaPromotable(const AllocaInst *AI);
    bool isExpandable(const AllocaInst *AI);
    // FIXME: maybe a merging here
  };
}

char SROA::ID = 0;
static RegisterPass<SROA> X("scalarrepl-daweis2",
			    "Scalar Replacement of Aggregates (by <daweis2>)",
			    false /* does not modify the CFG */,
			    false /* transformation, not just analysis */);


// Public interface to create the ScalarReplAggregates pass.
// This function is provided to you.
FunctionPass *createMyScalarReplAggregatesPass() { return new SROA(); }


//===----------------------------------------------------------------------===//
//                      SKELETON FUNCTION TO BE IMPLEMENTED
//===----------------------------------------------------------------------===//
//
// Function runOnFunction:
// Entry point for the overall ScalarReplAggregates function pass.
// This function is provided to you.
bool SROA::runOnFunction(Function &F) {

  bool Changed = false;
  bool finished = false;
  while (!finished) {
    finished = true;
    // step 1: promote mem to reg
    std::set<AllocaInst*> worklist_step1;
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
      if (AllocaInst *AI = dyn_cast<AllocaInst>(&*I)) {
        if (isAllocaPromotable(AI)) {
          worklist_step1.insert(AI);
        }
      }
    }
    while (!worklist_step1.empty()){
      AllocaInst *AI = *worklist_step1.begin();
      worklist_step1.erase(worklist_step1.begin());
      errs() << "Promotable AI: ";
      AI->print(errs());
      errs() << "\n";
      DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
      errs() << "got DT\n";
      PromoteMemToReg(AI, *DT);
      ++NumPromoted;
      errs() << "promoted\n";
      finished = false;
      Changed = true;
     }

    // step 2: expand allocas
    std::set<AllocaInst*> worklist;
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
      if (AllocaInst *AI = dyn_cast<AllocaInst>(&*I)) {
        // TODO: aggregate type
        if (dyn_cast<StructType>(AI->getType()->getElementType()))
          worklist.insert(AI);
      }
    }
    errs() << "worklist.size(): " << worklist.size() << "\n";

    while (!worklist.empty()){
      errs() << "while";

      AllocaInst *AI = *worklist.begin();
      worklist.erase(worklist.begin());
      if (isExpandable(AI)) {
        expandAggreateAlloca(AI, &worklist);
        ++NumReplaced;
        finished = false;
        Changed = true;
      }
    }
  }
  return Changed;
}

bool SROA::isZero(const Value *V) {
  if (const ConstantInt *CI = dyn_cast<ConstantInt>(V)) {
    if (CI->isZero())
      return true;
  }
  return false;
}

// TODO: Instruction or User?
bool SROA::checkU1_2U(const User *U, const Value *ptr) {
  errs() << "\n CheckU1_2U, User: ";
  U->print(errs());
  errs() << "\n";

  if (const LoadInst *LI = dyn_cast<LoadInst>(U)) {
    // TODO: any check here?
    errs() << "cp1 ";
    return true;
  } else if (const StoreInst *SI = dyn_cast<StoreInst>(U)) {
    // TODO: any check else here? Volatile?
    errs() << "cp2 ";
    if (SI->getOperand(1) == ptr)
      return true; // Don't allow a store OF the AI, only INTO the AI.
  } else if (checkU1(U, ptr)) {
    errs() << "cp3 ";
    return true;
  } else if (checkU2(U)) {
    errs() << "cp4 ";
    return true;
  }
  return false;
}

bool SROA::checkU1(const User *U, const Value *ptr) {
  errs() << "\n CheckU1, User: ";
  U->print(errs());
  errs() << "\n";

  // U1: 1. is a GEPI; 2. get a const-indexed element of the current struct;
  // 3. result of this GEPI is used only in (U1 or U2 or load or store) (U1_2U).
  if (const GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(U)) {
    errs() << "GEPI ";

    // U1_1
    // TODO: check type
    if (ptr != GEPI->getOperand(0))
      return false;
    errs() << "cp1 ";

    if (!isZero(GEPI->getOperand(1)))
      return false;
    errs() << "cp2 ";

    if (GEPI->getNumOperands() < 3)
      return false;

    errs() << "cp3 ";

    for (unsigned I = 2, IE = GEPI->getNumOperands(); I != IE; ++I) {
      if (!isa<Constant>(GEPI->getOperand(I)))
        return false;
      errs() << "cpfor"<<I<<" ";
    }
    // U1_2
    for (const User *U : GEPI->users())
      if (!checkU1_2U(U, GEPI))
        return false;

  } else {
    return false;
  }
  return true;
}

bool SROA::checkU2(const User *U) {
  errs() << "\n CheckU2, User: ";
  U->print(errs());
  errs() << "\n";

  if (const CmpInst *CI = dyn_cast<CmpInst>(U)) {
    auto predicateName = CmpInst::getPredicateName(CI->getPredicate());
    if ((predicateName == "eq" || predicateName == "ne") &&
      (isZero(CI->getOperand(0)) || isZero(CI->getOperand(1))))
      return true;
  }
  return false;
}

bool SROA::expandAggreateAlloca(AllocaInst *AI, std::set<AllocaInst *> *worklist) {
  errs() << "expandAggreateAlloca\n";

  auto type = AI->getType()->getElementType();
  if (const StructType *STy = dyn_cast<StructType>(type)) {
    std::vector<AllocaInst *> newAllocas;
    for (int i = 0; i < STy->getNumElements(); i++) {
        errs() << "for i="<<i<<"/"<<STy->getNumElements();

           // TODO: array in struct. What is the type.
           AllocaInst *newAlloca = new AllocaInst(STy->getElementType(i),
             AI->getType()->getAddressSpace());
           newAlloca->insertAfter(AI);
           newAllocas.push_back(newAlloca);
           // TODO: aggregate type
           // if (dyn_cast<StructType>(newAlloca->getType()))
           if (dyn_cast<StructType>(STy->getElementType(i)))
             worklist->insert(newAlloca);
    }
    errs()<<"cp1 \n";
    std::set<User *> userList;
    for (auto U: AI->users()) {
      userList.insert(U);
    }

    for (auto I=userList.begin(), E=userList.end(); I!=E; I++) {
      User *U = *I;
      errs() << "User: ";
      U->print(errs());
      errs()<<"\n";
      if (GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(U)) {
        if (GEPI->getNumOperands() == 3) {
          errs()<<"GEPI->getNumOperands() == 3\n";
          // point to an element. Use the AllocaInst directly.
          GEPI->replaceAllUsesWith(
            newAllocas[cast<ConstantInt>(GEPI->getOperand(2))->getSExtValue()]);
          GEPI->eraseFromParent();
          errs()<<"cp3\n";
        } else {
          // point to an element of an element or even more complicated.
          // auto STy = cast<StructType>(GEPI->getType());
          errs() << "GEPI->getType()->print(): ";
          GEPI->getType()->print(errs());
          errs() << "\n";
          errs() << "dyn_cast<PointerType>(GEPI->getPointerOperandType())->getElementType()->print(): ";
          dyn_cast<PointerType>(GEPI->getPointerOperandType())->getElementType()->print(errs());
          errs() << "\n";
          auto STy = cast<StructType>(dyn_cast<PointerType>(GEPI->getPointerOperandType())->getElementType());
          int eId = cast<ConstantInt>(GEPI->getOperand(2))->getSExtValue();
          Type *eTy = STy->getElementType(eId);
          Value *ePtr = newAllocas[eId];
          std::vector<Value *> idxList;
          idxList.push_back(ConstantInt::get(GEPI->idx_begin()->get()->getType(), 0));
          for (auto ptr = GEPI->idx_begin()+2; ptr < GEPI->idx_end(); ptr++) {
            idxList.push_back(ptr->get());
          }
          auto newGEPI = GetElementPtrInst::Create(eTy, ePtr, idxList);
          BasicBlock::iterator ii(GEPI);
          ReplaceInstWithInst(GEPI->getParent()->getInstList(), ii, newGEPI);
       }
     } else if (CmpInst *CI = dyn_cast<CmpInst>(U)) {
        BasicBlock::iterator ii(CI);
        ReplaceInstWithValue(AI->getParent()->getInstList(), ii,
        ConstantInt::getTrue(AI->getParent()->getParent()->getContext()));
     } else {
       // TODO: raise an exception here. It seems checkU1/2 failed.
     }
   }
   AI->eraseFromParent();
 } else {
   // TODO: array
 }
}

bool SROA::isAllocaPromotable(const AllocaInst *AI) {
  errs() << "isAllocaPromotable, AI: ";
  AI->print(errs());
  errs() << "\n";
  auto type = AI->getType()->getElementType();
  if (!(type->isFPOrFPVectorTy() || type->isIntOrIntVectorTy()\
   || type->isPtrOrPtrVectorTy()))
   return false;
  for (auto U: AI->users()) {
    if (const LoadInst *LI = dyn_cast<LoadInst>(U)) {
      if (LI->isVolatile())
        return false;
    } else if (const StoreInst *SI = dyn_cast<StoreInst>(U)) {
      if (SI->isVolatile())
        return false;
      if (SI->getOperand(0) == AI)
        return false; // Don't allow a store OF the AI, only INTO the AI.
    }
  }
  return true;
}

bool SROA::isExpandable(const AllocaInst *AI) {

  for (auto U: AI->users())
    if (!(checkU1(U, AI) || checkU2(U)))
      return false;
  return true;
}
