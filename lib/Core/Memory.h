//===-- Memory.h ------------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef KLEE_MEMORY_H
#define KLEE_MEMORY_H

#include "Context.h"
#include "TimingSolver.h"

#include "klee/KValue.h"
#include "klee/util/BitArray.h"

#include "llvm/ADT/Optional.h"
#include "llvm/ADT/StringExtras.h"

#include <vector>
#include <unordered_map>
#include <string>
#include <vector>

namespace llvm {
  class Value;
}

namespace klee {

class BitArray;
class MemoryManager;
class Solver;
class ArrayCache;

class MemoryObject {
  friend class STPBuilder;
  friend class ObjectState;
  friend class ExecutionState;

private:
  static int counter;
  mutable unsigned refCount;

public:
  unsigned id;
  uint64_t segment;

  /// size in bytes
  ref<Expr> size;
  // size of real virtual process memory allocated
  // for this object (this memory may be passed to external calls).
  uint64_t allocatedSize = 0;
  mutable std::string name;

  bool isLocal;
  mutable bool isGlobal;
  bool isFixed;

  bool isUserSpecified;

  MemoryManager *parent;

  /// "Location" for which this memory object was allocated. This
  /// should be either the allocating instruction or the global object
  /// it was allocated for (or whatever else makes sense).
  const llvm::Value *allocSite;
  
  /// A list of boolean expressions the user has requested be true of
  /// a counterexample. Mutable since we play a little fast and loose
  /// with allowing it to be added to during execution (although
  /// should sensibly be only at creation time).
  mutable std::vector< ref<Expr> > cexPreferences;

  /// Symbolic address for poiner comparison
  llvm::Optional<ref<Expr>> symbolicAddress;

  // DO NOT IMPLEMENT
  MemoryObject(const MemoryObject &b);
  MemoryObject &operator=(const MemoryObject &b);

public:
  // XXX this is just a temp hack, should be removed
  explicit
  MemoryObject()
    : refCount(0),
      id(counter++),
      segment(0),
      size(0),
      isFixed(true),
      parent(NULL),
      allocSite(0) {
  }

  MemoryObject(ref<Expr> _size,
               bool _isLocal, bool _isGlobal, bool _isFixed,
               const llvm::Value *_allocSite,
               MemoryManager *_parent)
    : refCount(0), 
      id(counter++),
      segment(0),
      size(ZExtExpr::create(_size, Context::get().getPointerWidth())),
      name("unnamed"),
      isLocal(_isLocal),
      isGlobal(_isGlobal),
      isFixed(_isFixed),
      isUserSpecified(false),
      parent(_parent), 
      allocSite(_allocSite) {
  }

    MemoryObject(uint64_t segment, ref<Expr> _size,
                 uint64_t _allocatedSize,
               bool _isLocal, bool _isGlobal, bool _isFixed,
               const llvm::Value *_allocSite,
               MemoryManager *_parent)
    : refCount(0),
      id(counter++),
      segment(segment),
      size(ZExtExpr::create(_size, Context::get().getPointerWidth())),
      allocatedSize(_allocatedSize),
      name("unnamed"),
      isLocal(_isLocal),
      isGlobal(_isGlobal),
      isFixed(_isFixed),
      isUserSpecified(false),
      parent(_parent),
      allocSite(_allocSite) {
  }

  ~MemoryObject();

  /// Get an identifying string for this allocation.
  void getAllocInfo(std::string &result) const;

  void getAllocCrashInfo(std::string &result) const;

  /// Initializes symbolic array
  /// \param array ArrayCache for Creatin the symbolic array
  void initializeSymbolicArray(klee::ArrayCache &array);

  /// If not initialized, creates symbolic array representing it's address
  /// and returns ref<Expr> for it
  /// \param array ArrayCache for creating the symbolic array
  /// @return symbolic array representing symbolic address of given MO
  ref<Expr> getSymbolicAddress(klee::ArrayCache &array);

  void setName(std::string name) const {
    this->name = name;
  }

  uint64_t getSegment() const {
    return segment;
  }
  ref<ConstantExpr> getSegmentExpr() const {
    return ConstantExpr::create(segment, Context::get().getPointerWidth());
  }
  ref<ConstantExpr> getBaseExpr() const { 
    return ConstantExpr::create(0, Context::get().getPointerWidth());
  }
  KValue getPointer() const {
    return KValue(getSegmentExpr(), getBaseExpr());
  }
  KValue getPointer(uint64_t offset) const {
    return KValue(getSegmentExpr(), ConstantExpr::create(offset, Context::get().getPointerWidth()));
  }
  std::string getSegmentString() const {
    return std::string("Segment: [" + std::to_string(segment) + "]");
  }
  std::string getSizeString() const {
    if (ConstantExpr *CE = dyn_cast<ConstantExpr>(size)) {
      return std::to_string(CE->getZExtValue());
    } else {
      return "symbolic";
    }
  }
  ref<Expr> getSizeExpr() const {
    return size;
  }
  ref<Expr> getOffsetExpr(ref<Expr> pointer) const {
    return SubExpr::create(pointer, getBaseExpr());
  }
  ref<Expr> getBoundsCheckPointer(KValue pointer) const {
    return AndExpr::create(
            getBoundsCheckSegment(pointer.getSegment()),
            getBoundsCheckOffset(pointer.getOffset()));
  }
  ref<Expr> getBoundsCheckPointer(KValue pointer, unsigned bytes) const {
    return AndExpr::create(
            getBoundsCheckSegment(pointer.getSegment()),
            getBoundsCheckOffset(pointer.getOffset(), bytes));
  }
  ref<Expr> getBoundsCheckOffset(ref<Expr> offset) const {
    if (isa<ConstantExpr>(size) && cast<ConstantExpr>(size)->isZero()) {
      return EqExpr::create(offset,
                            ConstantExpr::alloc(0, Context::get().getPointerWidth()));
    } else {
      return UltExpr::create(offset, getSizeExpr());
    }
  }
  ref<Expr> getBoundsCheckOffset(ref<Expr> offset, unsigned bytes) const {
    return UltExpr::create(offset,
                           SubExpr::create(size,
                                           ConstantExpr::alloc(bytes - 1,
                                                               size->getWidth())));
  }

private:
  ref<Expr> getBoundsCheckSegment(ref<Expr> segment) const {
    return EqExpr::create(getSegmentExpr(), segment);
  }
};

// up to some offset this container is implemented
// as a vector and from some offset as a map
// (leveraging the assumption that large offsets will
// be sparse). Threshold is the maximal number of elements
// in the vector.
// This class is specialized for our needs, it is not generic...
template <typename T, const size_t Threshold = (1 << 18)>
class SparseVector {
    std::vector<T> _vector;
    std::unordered_map<size_t, T> _map;

public:
    const T& operator[](size_t n) const {
        if (n < Threshold) {
            assert(n < _vector.size());
            assert(_vector[n].get() != nullptr && "Use has() before");
            return _vector[n];
        } else {
            auto it = _map.find(n);
            if (it == _map.end()) {
                assert(false && "Cannot happen, must use has() before");
                abort();
            }
            assert(it->second.get() != nullptr);
            return it->second;
        }
    }

    void set(size_t n, const T& val) {
        if (n < Threshold) {
            if (_vector.size() <= n) {
              if (val.get() == nullptr) {
                return;
              }
              // make sure the access will be valid
              _vector.resize(n + 1);
            }

            assert(_vector.size() > n);
            _vector[n] = val;
        } else {
            if (val.get() == nullptr) {
                _map.erase(n);
            } else
                _map[n] = val;
        }
    }

    void clear() {
        _vector.clear();
        _map.clear();
    }

    bool has(size_t n) const {
        return (_vector.size() > n && _vector[n].get())
               || (_map.find(n) != _map.end());
    }
};

class ObjectStatePlane {
private:
  friend class AddressSpace;

  const ObjectState *parent;

  std::vector<uint8_t> concreteStore;

  // XXX cleanup name of flushMask (its backwards or something)
  BitArray concreteMask;

  // mutable because may need flushed during read of const
  mutable BitArray flushMask;

  SparseVector<ref<Expr> > knownSymbolics;

  // mutable because we may need flush during read of const
  mutable UpdateList updates;

public:
  unsigned sizeBound;

  bool initialized;
  bool symbolic;

  uint8_t initialValue;

public:
  /// Create a new object state for the given memory object with concrete
  /// contents. The initial contents are undefined, it is the callers
  /// responsibility to initialize the object contents appropriately.
  ObjectStatePlane(const ObjectState *parent);

  /// Create a new object state for the given memory object with symbolic
  /// contents.
  ObjectStatePlane(const ObjectState *parent, const Array *array);

  ObjectStatePlane(const ObjectState *parent, const ObjectStatePlane &os);
  ~ObjectStatePlane();

  // make contents all concrete and zero
  void initializeToZero();
  // make contents all concrete and random
  void initializeToRandom();

  ref<Expr> read(ref<Expr> offset, Expr::Width width) const;
  ref<Expr> read(unsigned offset, Expr::Width width) const;
  ref<Expr> read8(unsigned offset) const;

  // return bytes written.
  void write(unsigned offset, ref<Expr> value);
  void write(ref<Expr> offset, ref<Expr> value);

  void write8(unsigned offset, uint8_t value);
  void write16(unsigned offset, uint16_t value);
  void write32(unsigned offset, uint32_t value);
  void write64(unsigned offset, uint64_t value);
  void print() const;

  /*
    Looks at all the symbolic bytes of this object, gets a value for them
    from the solver and puts them in the concreteStore.
  */
  void flushToConcreteStore(TimingSolver *solver,
                            const ExecutionState &state);

private:
  const UpdateList &getUpdates() const;

  void makeConcrete();

  ref<Expr> read8(ref<Expr> offset) const;
  void write8(unsigned offset, ref<Expr> value);
  void write8(ref<Expr> offset, ref<Expr> value);

  void flushForRead() const;
  void flushForWrite();

  bool isByteConcrete(unsigned offset) const;
  bool isByteFlushed(unsigned offset) const;
  bool isByteKnownSymbolic(unsigned offset) const;

  void markByteConcrete(unsigned offset);
  void markByteSymbolic(unsigned offset);
  void markByteFlushed(unsigned offset) const;
  void markByteUnflushed(unsigned offset) const;
  void setKnownSymbolic(unsigned offset, Expr *value);
  uint8_t getConcreteValue(unsigned offset) const;
};

class ObjectState {
private:
  friend class AddressSpace;
  unsigned copyOnWriteOwner; // exclusively for AddressSpace

  friend class ObjectHolder;
  unsigned refCount;

  const MemoryObject *object;


public:
  bool readOnly;

private:
  ObjectStatePlane *segmentPlane;
  ObjectStatePlane *offsetPlane;

public:
  /// Create a new object state for the given memory object with concrete
  /// contents. The initial contents are undefined, it is the callers
  /// responsibility to initialize the object contents appropriately.
  ObjectState(const MemoryObject *mo);

  /// Create a new object state for the given memory object with symbolic
  /// contents.
  ObjectState(const MemoryObject *mo, const Array *array);

  ObjectState(const ObjectState &os);
  // Copy for realloc
  ObjectState(const ObjectState &os, const MemoryObject *mo);
  ~ObjectState();

  const MemoryObject *getObject() const { return object; }

  void setReadOnly(bool ro) {
    readOnly = ro;
  }

  // get upper bound on the size of this object if it is known
  uint64_t getSizeBound() const {
    return offsetPlane->sizeBound;
  }

  // make contents all concrete and zero
  void initializeToZero();
  // make contents all concrete and random
  void initializeToRandom();

  void flushToConcreteStore(TimingSolver *solver,
                            const ExecutionState &state) const {
    offsetPlane->flushToConcreteStore(solver, state);
  }

  KValue read(ref<Expr> offset, Expr::Width width) const;
  KValue read(unsigned offset, Expr::Width width) const;
  KValue read8(unsigned offset) const;

  // return bytes written.
  void write(unsigned offset, const KValue &value);
  void write(ref<Expr> offset, const KValue &value);

  void write8(unsigned offset, uint8_t segment, uint8_t value);
  void write16(unsigned offset, uint16_t segment, uint16_t value);
  void write32(unsigned offset, uint32_t segment, uint32_t value);
  void write64(unsigned offset, uint64_t segment, uint64_t value);

  ArrayCache *getArrayCache() const;

private:
  bool prepareSegmentPlane(bool nonzero);
  bool prepareSegmentPlane(ref<Expr> value);
};
  
} // End klee namespace

#endif /* KLEE_MEMORY_H */
