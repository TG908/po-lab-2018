#ifndef ABSTRACTDOMAIN_H_
#define ABSTRACTDOMAIN_H_

#include "llvm/IR/InstrTypes.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/APSInt.h"
#include <memory>

namespace pcpo {

using namespace llvm;
using std::shared_ptr;

const int OUTPUT_BASE = 10;
const int OUTPUT_SIGNED = false;

enum DomainType { stridedInterval, boundedSet, compositeDomain, simpleInterval };

class AbstractDomain {
public:

  ///Destructor
  virtual ~AbstractDomain() = default;

  /// Class info
  virtual DomainType getDomainType() const = 0;

  /// Lattice interface
  virtual bool operator<=(AbstractDomain &other) = 0;
  virtual shared_ptr<AbstractDomain> leastUpperBound(AbstractDomain &other) = 0;

  virtual shared_ptr<AbstractDomain> widen(AbstractDomain &other) {
    // The default implementation simply returns a new shared_ptr to this
    return shared_ptr<AbstractDomain>(this);
  }

  virtual bool requiresWideningAndNarrowing() {
    // The default is that widening is neither supported nor required
    return false;
  }

  virtual shared_ptr<AbstractDomain> intersect(AbstractDomain& other);

    /// Binary Arithmetic Operations
  virtual shared_ptr<AbstractDomain> add(unsigned numBits, AbstractDomain &other, bool nuw, bool nsw) = 0;
  virtual shared_ptr<AbstractDomain> sub(unsigned numBits, AbstractDomain &other, bool nuw, bool nsw) = 0;
  virtual shared_ptr<AbstractDomain> mul(unsigned numBits, AbstractDomain &other, bool nuw, bool nsw) = 0;
  virtual shared_ptr<AbstractDomain> udiv(unsigned numBits, AbstractDomain &other) = 0;
  virtual shared_ptr<AbstractDomain> sdiv(unsigned numBits, AbstractDomain &other) = 0;
  virtual shared_ptr<AbstractDomain> urem(unsigned numBits, AbstractDomain &other) = 0;
  virtual shared_ptr<AbstractDomain> srem(unsigned numBits, AbstractDomain &other) = 0;

  /// Binary Bitwise Operations
  virtual shared_ptr<AbstractDomain> shl(unsigned numBits, AbstractDomain &other, bool nuw, bool nsw) = 0;
  virtual shared_ptr<AbstractDomain> lshr(unsigned numBits, AbstractDomain &other) = 0;
  virtual shared_ptr<AbstractDomain> ashr(unsigned numBits, AbstractDomain &other) = 0;
  virtual shared_ptr<AbstractDomain> and_(unsigned numBits, AbstractDomain &other) = 0;
  virtual shared_ptr<AbstractDomain> or_(unsigned numBits, AbstractDomain &other) = 0;
  virtual shared_ptr<AbstractDomain> xor_(unsigned numBits, AbstractDomain &other) = 0;

  /// Other operations
  virtual std::pair<shared_ptr<AbstractDomain>, shared_ptr<AbstractDomain>>
  icmp(CmpInst::Predicate pred, unsigned numBits, AbstractDomain &other) = 0;

  /// Member functions
  virtual bool contains(APInt value) const = 0;
  virtual unsigned getBitWidth()const = 0;
  virtual bool isTop() const = 0;
  virtual bool isBottom() const = 0;
  virtual size_t size() const = 0;

  /// Member functions for API
  virtual APInt getValueAt(uint64_t i) const = 0;
  virtual APInt getUMin() const = 0;
  virtual APSInt getSMin() const = 0;
  virtual APInt getUMax() const = 0;
  virtual APSInt getSMax() const = 0;

  /// print
  virtual llvm::raw_ostream &print(llvm::raw_ostream &os) = 0;
  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os, AbstractDomain &bs) { return bs.print(os); }
  virtual void printOut() const = 0;
};
} // namespace pcpo
#endif
