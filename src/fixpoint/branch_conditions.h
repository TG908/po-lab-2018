#ifndef PROJECT_BRANCH_CONDITIONS_H
#define PROJECT_BRANCH_CONDITIONS_H

#include "../abstract_domain/AbstractDomain.h"
#include "state.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"
#include <map>
#include <memory>
#include <utility>

using namespace llvm;
namespace pcpo {

class BranchConditions {

public:
  BranchConditions(std::map<BasicBlock *, State> &programPoints);

  bool isBasicBlockReachable(BasicBlock *pred, BasicBlock *bb);

  // apply conditions and return true if non of the variables are bottom
  bool applyCondition(BasicBlock *pred, BasicBlock *bb);

  void unApplyCondition(BasicBlock *pred);

  void putBranchConditions(BasicBlock *pred, BasicBlock *bb, Value *val,
                           std::shared_ptr<AbstractDomain> ad);

private:
  std::map<BasicBlock *, State> &programPoints;

  std::map<BasicBlock *,
           std::map<BasicBlock *,
  std::map<BasicBlock * , // Source block
           std::map<BasicBlock *, // Target block
                    std::map<Value *, std::shared_ptr<AbstractDomain>>>>
      branchConditions;

  std::map<Value *, std::shared_ptr<AbstractDomain>> conditionCache;
  bool conditionCacheUsed;
};

} /// namespace

#endif