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
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"

#include <set>
#include <vector>

using namespace llvm;

STATISTIC(NumReplaced, "Number of aggregate allocas broken up");
STATISTIC(NumPromoted, "Number of scalar allocas promoted to register");

namespace {
struct SROA : public FunctionPass {
  static char ID; // Pass identification
  SROA() : FunctionPass(ID) {}

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
  bool isNullValue(const Value *V);
  bool checkU1_2U(const User *U, const Value *ptr);
  bool checkU1(const User *U, const Value *ptr);
  bool checkU2(const User *U);
  bool isAllocaPromotable(const AllocaInst *AI);
  bool isExpandable(const AllocaInst *AI);
  bool expandAggreateAlloca(AllocaInst *AI, std::set<AllocaInst *> *worklist);
  bool ProcessAllUseofAlloca(AllocaInst *AI,
                             const std::vector<AllocaInst *> &newAllocas,
                             std::vector<Type *> &elementTypes);
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
  int num_outer_iterations = 0;
  while (!finished) {
    num_outer_iterations++;
    finished = true;
    // step 1: promote mem to reg
    std::vector<AllocaInst *> allocasToPromote;
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
      if (AllocaInst *AI = dyn_cast<AllocaInst>(&*I)) {
        if (isAllocaPromotable(AI)) {
          allocasToPromote.push_back(AI);
        }
      }
    }
    if (allocasToPromote.size() > 0) {
      NumPromoted += allocasToPromote.size();
      DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
      PromoteMemToReg(allocasToPromote, *DT);
      finished = false;
      Changed = true;
    }
    // end of step 1

    // step 2: expand allocas
    std::set<AllocaInst *> worklist;
    for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
      if (AllocaInst *AI = dyn_cast<AllocaInst>(&*I)) {
        if (dyn_cast<ArrayType>(AI->getType()->getElementType()) ||
            dyn_cast<StructType>(AI->getType()->getElementType()))
          worklist.insert(AI);
      }
    }
    // errs() << "worklist.size(): " << worklist.size() << "\n";

    while (!worklist.empty()) {
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
  errs() << "TEST NUM_OUTER_ITERATIONS " << num_outer_iterations << "\n";
  return Changed;
}

// check if a Value is a constant int 0
bool SROA::isNullValue(const Value *V) {
  if (const Constant *C = dyn_cast<Constant>(V)) {
    return C->isNullValue();
  }
  return false;
}

void CHECKPOINT(int number) {
  // errs() << "CHECKPOINT " << number << "\n";
}

// second part of U1 condition. check user if of type U1/U2 or load/store
bool SROA::checkU1_2U(const User *U, const Value *ptr) {
  if (const LoadInst *LI = dyn_cast<LoadInst>(U)) {
    if (!LI->isVolatile()) {
      CHECKPOINT(1);
      return true;
    }
  } else if (const StoreInst *SI = dyn_cast<StoreInst>(U)) {
    if (SI->getOperand(1) == ptr && !SI->isVolatile()) {
      CHECKPOINT(2);
      return true; // Don't allow a store OF the AI, only INTO the AI.
    }
    CHECKPOINT(3);
  } else if (checkU1(U, ptr)) {
    CHECKPOINT(4);
    return true;
  } else if (checkU2(U)) {
    CHECKPOINT(5);
    return true;
  }
  CHECKPOINT(6);
  return false;
}

// U1
bool SROA::checkU1(const User *U, const Value *ptr) {

  // U1: 1. is a GEPI; 2. get a *const-indexed element* of the *current* struct;
  // 3. result of this GEPI is used only in (U1 or U2 or load or store)
  // (checkU1_2U).
  if (const GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(U)) {
    // U1_1
    // TODO: check type
    if (ptr != GEPI->getOperand(0)) {
      CHECKPOINT(7);
      return false;
    }

    if (!isNullValue(GEPI->getOperand(1))) {
      CHECKPOINT(8);
      return false;
    }

    if (GEPI->getNumOperands() < 3) {
      CHECKPOINT(9);
      return false;
    }

    for (unsigned I = 2, IE = GEPI->getNumOperands(); I != IE; ++I) {
      if (!isa<ConstantInt>(GEPI->getOperand(I))) {
        CHECKPOINT(10);
        return false;
      }
    }
    // U1_2
    for (const User *U : GEPI->users()) {
      if (!checkU1_2U(U, GEPI)) {
        CHECKPOINT(12);
        return false;
      }
    }
  } else {
    CHECKPOINT(13);
    return false;
  }
  CHECKPOINT(14);
  return true;
}

// U2
bool SROA::checkU2(const User *U) {
  // in SSA form, this check is meaning less
  // ptr is always valid
  if (const CmpInst *CI = dyn_cast<CmpInst>(U)) {
    auto predicateName = CmpInst::getPredicateName(CI->getPredicate());
    // errs() << predicateName << ": " << (predicateName == "ne") << "; "
           // << isNullValue(CI->getOperand(1));

    if ((predicateName == "eq" || predicateName == "ne") &&
        (isNullValue(CI->getOperand(0)) || isNullValue(CI->getOperand(1)))) {
      CHECKPOINT(15);
      return true;
    }
    CHECKPOINT(16);
  }
  CHECKPOINT(17);
  return false;
}

// expand an alloca of aggreate
bool SROA::expandAggreateAlloca(AllocaInst *AI,
                                std::set<AllocaInst *> *worklist) {

  auto type = AI->getType()->getElementType();
  if (!dyn_cast<StructType>(type) &&
      !dyn_cast<ArrayType>(type)) // this check is useless
    return false;

  unsigned int numElements = 0;
  std::vector<Type *> elementTypes;
  if (const ArrayType *ATy = dyn_cast<ArrayType>(type)) {
    numElements = ATy->getNumElements();
    if (numElements > 5)
      return false;

    for (unsigned i = 0; i < numElements; i++) {
      elementTypes.push_back(ATy->getElementType());
    }
  }

  if (const StructType *STy = dyn_cast<StructType>(type)) {
    numElements = STy->getNumElements();
    for (unsigned i = 0; i < numElements; i++) {
      elementTypes.push_back(STy->getElementType(i));
    }
  }

  std::vector<AllocaInst *> newAllocas;
  for (unsigned i = 0; i < numElements; i++) {
    Type *elementType;
    elementType = elementTypes[i];
    AllocaInst *newAlloca =
        new AllocaInst(elementType, AI->getType()->getAddressSpace());
    newAlloca->insertAfter(AI);
    newAllocas.push_back(newAlloca);
    if (dyn_cast<ArrayType>(elementType) || dyn_cast<StructType>(elementType)) {
      CHECKPOINT(18);
      worklist->insert(newAlloca);
    }
  }

  ProcessAllUseofAlloca(AI, newAllocas, elementTypes);

  AI->eraseFromParent(); // minimal changes
  return true;
}

bool SROA::ProcessAllUseofAlloca(AllocaInst *AI,
                                 const std::vector<AllocaInst *> &newAllocas,
                                 std::vector<Type *> &elementTypes) {
  std::set<User *> userList;
  for (auto U : AI->users()) {
    userList.insert(U);
  }

  for (auto I = userList.begin(), E = userList.end(); I != E; I++) {
    User *U = *I;
    if (GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(U)) {
      if (GEPI->getNumOperands() == 3) {
        CHECKPOINT(19);
        // point to an element. Use the AllocaInst directly.
        GEPI->replaceAllUsesWith(
            newAllocas[cast<ConstantInt>(GEPI->getOperand(2))->getSExtValue()]);
        GEPI->eraseFromParent(); // minimal changes
      } else {
        CHECKPOINT(20);
        int elementId = cast<ConstantInt>(GEPI->getOperand(2))->getSExtValue();
        Type *elementType = elementTypes[elementId];
        Value *elementPtr = newAllocas[elementId];
        std::vector<Value *> idxList;
        idxList.push_back(
            ConstantInt::get(GEPI->idx_begin()->get()->getType(), 0));
        for (auto idx = GEPI->idx_begin() + 2; idx < GEPI->idx_end(); idx++) {
          idxList.push_back(idx->get());
        }
        auto newGEPI =
            GetElementPtrInst::Create(elementType, elementPtr, idxList);
        BasicBlock::iterator ii(GEPI);
        ReplaceInstWithInst(GEPI->getParent()->getInstList(), ii, newGEPI);
      }
    } else if (CmpInst *CI = dyn_cast<CmpInst>(U)) {
      CHECKPOINT(21);
      auto predicateName = CmpInst::getPredicateName(CI->getPredicate());
      BasicBlock::iterator ii(CI);
      if (predicateName == "eq") {
        ReplaceInstWithValue(
            AI->getParent()->getInstList(), ii,
            ConstantInt::getFalse(AI->getParent()->getParent()->getContext()));
      } else if (predicateName == "ne") {
        ReplaceInstWithValue(
            AI->getParent()->getInstList(), ii,
            ConstantInt::getTrue(AI->getParent()->getParent()->getContext()));
      } else {
        // TODO: raise an exception here. It seems checkU2 failed.
        errs() << "unexpected predicate name: " << predicateName << "\n";
        return false;
      }
    } else {
      // TODO: raise an exception here. It seems checkU1/2 failed.
      errs() << "unexpected Instruction type: ";
      U->print(errs());
      errs() << "\n";
      return false;
    }
  }
  return true;
}

// check if an AllocaInst is Promotable
bool SROA::isAllocaPromotable(const AllocaInst *AI) {
  auto type = AI->getType()->getElementType();
  if (!(type->isFPOrFPVectorTy() || type->isIntOrIntVectorTy() ||
        type->isPtrOrPtrVectorTy())) {
    CHECKPOINT(22);
    return false;
  }
  for (auto U : AI->users()) {
    if (const LoadInst *LI = dyn_cast<LoadInst>(U)) {
      if (LI->isVolatile()) {
        CHECKPOINT(23);
        return false;
      }
      CHECKPOINT(24);
    } else if (const StoreInst *SI = dyn_cast<StoreInst>(U)) {
      if (SI->isVolatile()) {
        CHECKPOINT(25);
        return false;
      }
      if (SI->getOperand(0) == AI) {
        CHECKPOINT(26);
        return false; // Don't allow a store OF the AI, only INTO the AI.
      }
      CHECKPOINT(27);
    } else {
      return false;
    }
  }
  return true;
}

bool SROA::isExpandable(const AllocaInst *AI) {
  for (auto U : AI->users())
    if (!(checkU1(U, AI) || checkU2(U)))
      return false;
  return true;
}
