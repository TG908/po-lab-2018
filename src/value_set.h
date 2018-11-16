#pragma once

#include <unordered_map>
#include <vector>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"

namespace pcpo {

class AbstractDomainDummy {
public:
    // This has to initialise to either top or bottom, depending on the flagxs
    AbstractDomainDummy(bool isTop = false)
        { assert(false); };

    // Copy constructor. We need to be able to copy this type. (To be precise, we also need copy
    // assignment.) Probably you can just ignore this and leave the default, compiler-generated copy
    // constructor.
    AbstractDomainDummy(AbstractDomainDummy const&) = default;
    
    // Initialise from a constant
    AbstractDomainDummy(llvm::Constant& constant): AbstractDomainDummy(true) {}

    // @Cleanup Documentation
    static AbstractDomainDummy interpret (
        llvm::Instruction& inst, std::vector<AbstractDomainDummy> const& operands
    ) { return AbstractDomainDummy(true); }

    // @Cleanup Documentation
    bool operator== (AbstractDomainDummy o) const {
        assert(false); return false;
    }
    
    // Refine a by using the information that the value has to fulfill the predicate w.r.t. b. For
    // example, if the domain is an interval domain:
    //     refine_branch( ULE, [5, 10], [7, 8] ) => [5, 8]
    // or
    //     refine_branch( ULE, [5, 10], [7, 8] ) => [5, 10]
    // would be valid implementations, though the former is more precise. In this case the only
    // relevant information is that the number has to be less than or equal to 8.
    //  The following properties have to be fulfilled, if c = refine_branch(~, a, b):
    //     1. c <= a
    //     2. For all x in a, y in b with x~y we have x in c.
    static AbstractDomainDummy refine_branch (
        llvm::CmpInst::Predicate pred, llvm::Value& lhs, llvm::Value& rhs,
        AbstractDomainDummy a, AbstractDomainDummy b
    ) { return a; }

    // @Cleanup Documentation
    static AbstractDomainDummy upperBound(AbstractDomainDummy a, AbstractDomainDummy b)
        { return AbstractDomainDummy(true); }
};

template <typename AbstractDomain>
class AbstractStateValueSet {
public:
    std::unordered_map<llvm::Value*, AbstractDomain> values;

public:
    AbstractStateValueSet() = default;
    AbstractStateValueSet(AbstractStateValueSet const& state) = default;
    
    void apply(llvm::BasicBlock& bb) {
        std::vector<AbstractDomain> operands;

        // Go through each instruction of the basic block and apply it to the state
        for (llvm::Instruction& inst: bb) {
            for (llvm::Value* value: inst.operand_values()) {
                operands.push_back(getAbstractValue(*value));
            }

            assert(values.count(&inst) == 0 /* SSA should ensure that values are not overwritten */ );

            // There is no need to merge here. In fact, the value does not even
            // exist beforehand.
            values[&inst] = AbstractDomain::interpret(inst, operands);
        }
    }
    
    bool merge(AbstractStateValueSet const& other) {
        bool changed = false;
        for (std::pair<llvm::Value*, AbstractDomain> i: other.values) {
            // If our value did not exist before, it will be implicitely treated as bottom, which
            // works just fine.
            AbstractDomain old = values[i.first];
            values[i.first] = AbstractDomain::upperBound(old, i.second);
            changed = changed or old != values[i.first];
        }
        return changed;
    }
    
    void branch(llvm::BasicBlock& from, llvm::BasicBlock& towards) {
        llvm::Instruction* terminator = from.getTerminator();
        assert(terminator /* from is not a well-formed basic block! */);
        assert(terminator->isTerminator());

        llvm::BranchInst* branch = llvm::dyn_cast<llvm::BranchInst>(terminator);

        // If the terminator is not a simple branch, we are not interested
        if (not branch) return;

        // In the case of an unconditional branch, there is nothing to do
        if (branch->isUnconditional()) return;

        llvm::Value& condition = *branch->getCondition();

        llvm::ICmpInst* cmp = llvm::dyn_cast<llvm::ICmpInst>(&condition);

        // We only deal with integer compares here. If you want to do floating points operations as
        // well, you need to adjust the following lines of code a bit.
        if (not cmp) return;

        // We need to find out whether the towards block is on the true or the false branch
        llvm::CmpInst::Predicate pred;
        if (branch->getOperand(2) == &towards) {
            pred = cmp->getPredicate();
        } else if (branch->getOperand(1) == &towards) {
            // We are the false block, so do the negation of the predicate
            pred = cmp->getInversePredicate();
        } else {
            assert(false /* we were not passed the right 'from' block? */);
        }
        
        llvm::Value& lhs = *cmp->getOperand(0);
        llvm::Value& rhs = *cmp->getOperand(1);

        if (values.count(&lhs)) {
            values[&lhs] = AbstractDomain::refine_branch(pred, lhs, rhs, values[&lhs], getAbstractValue(rhs));
        }
        if (values.count(&rhs)) {
            llvm::CmpInst::Predicate pred_s = llvm::CmpInst::getSwappedPredicate(pred);
            values[&rhs] = AbstractDomain::refine_branch(pred_s, rhs, lhs, values[&rhs], getAbstractValue(lhs));
        }
    }

    void print(llvm::BasicBlock& bb, llvm::raw_ostream& out) const {
        for (llvm::Instruction& inst: bb) {
            out << inst;
            if (values.count(&inst)) {
                out << " // " << values.at(&inst);
            }
            out << '\n';
        }
    };
    
public:
    AbstractDomain getAbstractValue(llvm::Value& value) const {
        if (llvm::Constant* c = llvm::dyn_cast<llvm::Constant>(&value)) {
            return AbstractDomain {*c};
        } else if (values.count(&value)) {
            return values.at(&value);
        } else {
            return AbstractDomain {true};
        }
    }

};

} /* end of namespace pcpo */