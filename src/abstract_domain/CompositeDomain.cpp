#include "CompositeDomain.h"
#include "llvm/Support/raw_os_ostream.h"
#include <functional>
#include <memory>

namespace pcpo {
using std::function;
using std::shared_ptr;

CompositeDomain::CompositeDomain(APInt value) : bitWidth{value.getBitWidth()} {
  delegate = shared_ptr<AbstractDomain>{new BoundedSet(value)};
}

// if isTop==true then we create a top delegate
// if isTop==false then we create a bottom delegate
CompositeDomain::CompositeDomain(unsigned bitWidth, bool isTop) : bitWidth{bitWidth} {
  delegate = shared_ptr<AbstractDomain>{new BoundedSet{bitWidth, isTop}};
}

CompositeDomain::CompositeDomain(const CompositeDomain &old)
    : bitWidth{bitWidth} {
  if (old.getDelegateType() == boundedSet) {
    BoundedSet *oldBs = static_cast<BoundedSet *>(old.delegate.get());
    delegate = shared_ptr<AbstractDomain>{new BoundedSet{*oldBs}};
  } else {
    StridedInterval *oldSi = static_cast<StridedInterval *>(old.delegate.get());
    delegate = shared_ptr<AbstractDomain>{new StridedInterval{*oldSi}};
  }
}

CompositeDomain::CompositeDomain(shared_ptr<AbstractDomain> del)
    : bitWidth(del->getBitWidth()), delegate{del} {}

// computeOperation expects a CompositeDomain (CD) and a binary function to be
// evaluated on these. In case both (this and the argument) CompositeDomains
// contain a BoundedSet (BS), the function is executed on these. If this results
// in a top, both BoundedSets are converted to StridedIntervals (SI) and the
// operation is executed again. In case one of the CD contains a BS and the
// other a SI, the BS is converted to a SI. In case both CD contain a SI, the
// function is executed on the SIs.
shared_ptr<AbstractDomain> CompositeDomain::computeOperation(
    AbstractDomain &other,
    function<shared_ptr<AbstractDomain>(AbstractDomain &, AbstractDomain &)>
        op) {
  CompositeDomain &otherD = *static_cast<CompositeDomain *>(&other);

  if (otherD.getDelegateType() == boundedSet) {
    if (getDelegateType() == boundedSet) {
      // both are bounded sets
      auto resultOp = op(*delegate.get(), *otherD.delegate.get());
      // If the operation results in a top, this might be due
      // to the size limitation of the bounded set.
      // Thus, we transform them into strided intervals
      if (resultOp->isTop()) {
        BoundedSet otherBs = *static_cast<BoundedSet *>(otherD.delegate.get());
        BoundedSet thisBs = *static_cast<BoundedSet *>(this->delegate.get());
        StridedInterval otherDelegate{otherBs};
        StridedInterval thisDelegate{thisBs};
        resultOp = op(thisDelegate, otherDelegate);
        return shared_ptr<AbstractDomain>{
            new CompositeDomain{resultOp}};
      } else {
        return shared_ptr<AbstractDomain>{
            new CompositeDomain{resultOp}};
      }
    } else {

      // other has a bounded set, we have a strided interval
      // change other to strided interval
      BoundedSet otherBs = *static_cast<BoundedSet *>(otherD.delegate.get());
      StridedInterval otherDelegate{otherBs};
      return shared_ptr<AbstractDomain>{new CompositeDomain{
          op(*delegate.get(), otherDelegate)}};
    }
  } else {
    // other is a strided interval
    if (getDelegateType() == boundedSet) {
      // this is a bounded set
      // change to strided interval
      BoundedSet thisBs = *static_cast<BoundedSet *>(this->delegate.get());
      StridedInterval thisDelegate{thisBs};
      return shared_ptr<AbstractDomain>{new CompositeDomain{
          op(thisDelegate, *otherD.delegate.get())}};
    } else {
      // both are strided intervals already
      return shared_ptr<AbstractDomain>{new CompositeDomain{
          op(*delegate.get(), *otherD.delegate.get())}};
    }
  }
}




// Binary Arithmetic Operations
shared_ptr<AbstractDomain> CompositeDomain::add(unsigned numBits,
                                                AbstractDomain &other, bool nuw,
                                                bool nsw) {
  auto operation = [numBits, nuw, nsw](AbstractDomain &lhs,
                                       AbstractDomain &rhs) {
    return lhs.add(numBits, rhs, nuw, nsw);
  };
  
  return computeOperation(other, operation);
}

shared_ptr<AbstractDomain> CompositeDomain::sub(unsigned numBits,
                                                AbstractDomain &other, bool nuw,
                                                bool nsw) {
  auto operation = [numBits, nuw, nsw](AbstractDomain &lhs,
                                       AbstractDomain &rhs) {
    return lhs.sub(numBits, rhs, nuw, nsw);
  };
  return computeOperation(other, operation);
}
shared_ptr<AbstractDomain> CompositeDomain::mul(unsigned numBits,
                                                AbstractDomain &other, bool nuw,
                                                bool nsw) {
  auto operation = [numBits, nuw, nsw](AbstractDomain &lhs,
                                       AbstractDomain &rhs) {
    return lhs.mul(numBits, rhs, nuw, nsw);
  };
  return computeOperation(other, operation);
}
shared_ptr<AbstractDomain> CompositeDomain::udiv(unsigned numBits,
                                                 AbstractDomain &other) {
  auto operation = [numBits](AbstractDomain &lhs,
                                       AbstractDomain &rhs) {
    return lhs.udiv(numBits, rhs);
  };
  return computeOperation(other, operation);
}
shared_ptr<AbstractDomain> CompositeDomain::sdiv(unsigned numBits,
                                                 AbstractDomain &other) {
  auto operation = [numBits](AbstractDomain &lhs,
                                       AbstractDomain &rhs) {
    return lhs.sdiv(numBits, rhs);
  };
  return computeOperation(other, operation);
}
shared_ptr<AbstractDomain> CompositeDomain::urem(unsigned numBits,
                                                 AbstractDomain &other) {
  auto operation = [numBits](AbstractDomain &lhs,
                                       AbstractDomain &rhs) {
    return lhs.urem(numBits, rhs);
  };
  return computeOperation(other, operation);
}
shared_ptr<AbstractDomain> CompositeDomain::srem(unsigned numBits,
                                                 AbstractDomain &other) {
  auto operation = [numBits](AbstractDomain &lhs,
                                       AbstractDomain &rhs) {
    return lhs.srem(numBits, rhs);
  };
  return computeOperation(other, operation);
}

// Binary Bitwise Operations
shared_ptr<AbstractDomain> CompositeDomain::shl(unsigned numBits,
                                                AbstractDomain &other, bool nuw,
                                                bool nsw) {
  auto operation = [numBits, nuw, nsw](AbstractDomain &lhs,
                                       AbstractDomain &rhs) {
    return lhs.shl(numBits, rhs, nuw, nsw);
  };
  return computeOperation(other, operation);
}
shared_ptr<AbstractDomain> CompositeDomain::lshr(unsigned numBits,
                                                 AbstractDomain &other) {
  auto operation = [numBits](AbstractDomain &lhs,
                                       AbstractDomain &rhs) {
    return lhs.lshr(numBits, rhs);
  };
  return computeOperation(other, operation);
}
shared_ptr<AbstractDomain> CompositeDomain::ashr(unsigned numBits,
                                                 AbstractDomain &other) {
  auto operation = [numBits](AbstractDomain &lhs,
                                       AbstractDomain &rhs) {
    return lhs.ashr(numBits, rhs);
  };
  return computeOperation(other, operation);
}
shared_ptr<AbstractDomain> CompositeDomain::and_(unsigned numBits,
                                                 AbstractDomain &other) {
  auto operation = [numBits](AbstractDomain &lhs,
                                       AbstractDomain &rhs) {
    return lhs.and_(numBits, rhs);
  };
  return computeOperation(other, operation);
}
shared_ptr<AbstractDomain> CompositeDomain::or_(unsigned numBits,
                                                AbstractDomain &other) {
  auto operation = [numBits](AbstractDomain &lhs,
                                       AbstractDomain &rhs) {
    return lhs.or_(numBits, rhs);
  };
  return computeOperation(other, operation);
}
shared_ptr<AbstractDomain> CompositeDomain::xor_(unsigned numBits,
                                                 AbstractDomain &other) {
  auto operation = [numBits](AbstractDomain &lhs,
                                       AbstractDomain &rhs) {
    return lhs.xor_(numBits, rhs);
  };
  return computeOperation(other, operation);
}

// Conversion Operations (TODO?)

// Other operations
std::pair<shared_ptr<AbstractDomain>, shared_ptr<AbstractDomain>>
CompositeDomain::icmp(CmpInst::Predicate pred, unsigned numBits,
                      AbstractDomain &other) {
  CompositeDomain &otherD = *static_cast<CompositeDomain *>(&other);

  if (getDelegateType() == stridedInterval && otherD.getDelegateType() == boundedSet) {
      // other has a bounded set, we have a strided interval
      // change other to strided interval
      BoundedSet otherBs = *static_cast<BoundedSet *>(otherD.delegate.get());
      StridedInterval otherDelegate(otherBs);
      auto temp = delegate->icmp(pred, numBits, otherDelegate);
      return std::pair<shared_ptr<AbstractDomain>, shared_ptr<AbstractDomain>>(
              shared_ptr<AbstractDomain>(new CompositeDomain(temp.first)),
              shared_ptr<AbstractDomain>(new CompositeDomain(temp.second))
              );
  }
  if (getDelegateType() == boundedSet && otherD.getDelegateType() == stridedInterval) {
      // this is a bounded set
      // change to strided interval
      BoundedSet thisBs = *static_cast<BoundedSet *>(this->delegate.get());
      StridedInterval thisDelegate{thisBs};
      auto temp = thisDelegate.icmp(pred, numBits, *otherD.delegate.get());
      return std::pair<shared_ptr<AbstractDomain>, shared_ptr<AbstractDomain>>(
              shared_ptr<AbstractDomain>(new CompositeDomain(temp.first)),
              shared_ptr<AbstractDomain>(new CompositeDomain(temp.second))
              );
  }
      auto temp_ = delegate->icmp(pred, numBits, *otherD.delegate.get());
      return std::pair<shared_ptr<AbstractDomain>, shared_ptr<AbstractDomain>>(
              shared_ptr<AbstractDomain>(new CompositeDomain(temp_.first)),
              shared_ptr<AbstractDomain>(new CompositeDomain(temp_.second))
              );
}

shared_ptr<AbstractDomain> CompositeDomain::widen(AbstractDomain &other) {
  return delegate->widen(other);
}

bool CompositeDomain::requiresWidening() {
  return delegate->requiresWidening();
}


llvm::raw_ostream &CompositeDomain::print(llvm::raw_ostream &os) {
  return delegate->print(os);
}

// Lattice interface
shared_ptr<AbstractDomain>
CompositeDomain::leastUpperBound(AbstractDomain &other) {
  auto operation = [](AbstractDomain &lhs, AbstractDomain &rhs) {
    return lhs.leastUpperBound(rhs);
  };
  return computeOperation(other, operation);
}

bool CompositeDomain::operator<=(AbstractDomain &other) {
  CompositeDomain &otherD = *static_cast<CompositeDomain *>(&other);
  //return delegate->lessOrEqual(*otherD.delegate.get());
  if (getDelegateType() == stridedInterval && otherD.getDelegateType() == boundedSet) {
      BoundedSet otherBs = *static_cast<BoundedSet *>(otherD.delegate.get());
      StridedInterval otherDelegate(otherBs);
      return *delegate<=(otherDelegate);
  }
  if (getDelegateType() == boundedSet && otherD.getDelegateType() == stridedInterval) {
      BoundedSet thisBs = *static_cast<BoundedSet *>(this->delegate.get());
      StridedInterval thisDelegate{thisBs};
      return thisDelegate<=(*otherD.delegate.get());
  }
  return *delegate<=(*otherD.delegate.get());
}

unsigned CompositeDomain::getBitWidth() const { return bitWidth; }

// |gamma(this)|
size_t CompositeDomain::size() const { return delegate->size(); }

bool CompositeDomain::isTop() const { return delegate->isTop(); }
bool CompositeDomain::isBottom() const { return delegate->isBottom(); }

bool CompositeDomain::contains(APInt value) const{
  return false;
}

APInt CompositeDomain::getValueAt(uint64_t i) const {
  return delegate->getValueAt(i);
}

APInt CompositeDomain::getUMin() const {
   return delegate->getUMin();
}

APSInt CompositeDomain::getSMin() const { 
  return delegate->getSMin();
}

APInt CompositeDomain::getUMax() const { 
  return delegate->getUMax();
}

APSInt CompositeDomain::getSMax() const { 
  return delegate->getSMax();
}

// Debugging methodes
void CompositeDomain::printOut() const { delegate->printOut(); }

DomainType CompositeDomain::getDelegateType() const {
  return delegate->getDomainType();
}
} // namespace pcpo
