#ifndef UTIL
#define UTIL

#include "llvm/Support/raw_ostream.h"

#include "../abstract_domain/BoundedSet.h"
#include "../abstract_domain/CompositeDomain.h"
#include "../abstract_domain/StridedInterval.h"
/// settings

/// should debug output enabled?
//#define DEBUG
#define VISUAL_DEBUG

/// type of abstract domain
//#define AD_TYPE BoundedSet
#define AD_TYPE StridedInterval
//define AD_TYPE CompositeDomain

/// how many changes do we allow before we apply widening (for ADs in which it is reuired)
#define WIDENING_AFTER 3

/// do not to touch anything beneath here (useful functions)

#ifdef DEBUG
#define DEBUG_OUTPUT(text) errs() << text << "\n"
#else
#define DEBUG_OUTPUT(text)
#endif

#define STD_OUTPUT(text) errs() << text << "\n"

#endif
