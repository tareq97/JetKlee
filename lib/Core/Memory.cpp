//===-- Memory.cpp --------------------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Memory.h"

#include "Context.h"
#include "MemoryManager.h"
#include "ObjectHolder.h"

#include "klee/Expr/ArrayCache.h"
#include "klee/Expr/Expr.h"
#include "klee/Internal/Support/ErrorHandling.h"
#include "klee/OptionCategories.h"
#include "klee/Solver/Solver.h"
#include "klee/util/BitArray.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/CallSite.h"



#include <cassert>
#include <sstream>

using namespace llvm;
using namespace klee;

namespace {
  cl::opt<bool>
  UseConstantArrays("use-constant-arrays",
                    cl::desc("Use constant arrays instead of updates when possible (default=true)\n"),
                    cl::init(true),
                    cl::cat(SolvingCat));
}

/***/

ObjectHolder::ObjectHolder(const ObjectHolder &b) : os(b.os) { 
  if (os) ++os->refCount; 
}

ObjectHolder::ObjectHolder(ObjectState *_os) : os(_os) { 
  if (os) ++os->refCount; 
}

ObjectHolder::~ObjectHolder() { 
  if (os && --os->refCount==0) delete os; 
}
  
ObjectHolder &ObjectHolder::operator=(const ObjectHolder &b) {
  if (b.os) ++b.os->refCount;
  if (os && --os->refCount==0) delete os;
  os = b.os;
  return *this;
}

/***/

int MemoryObject::counter = 0;

MemoryObject::~MemoryObject() {
  if (parent)
    parent->markFreed(this);
}

void MemoryObject::getAllocInfo(std::string &result) const {
  llvm::raw_string_ostream info(result);

  info << "MO" << id << "[" << getSizeString() << "]";

  if (allocSite) {
    info << " allocated at ";
    if (const Instruction *i = dyn_cast<Instruction>(allocSite)) {
      info << i->getParent()->getParent()->getName() << "():";
    } else if (const GlobalValue *gv = dyn_cast<GlobalValue>(allocSite)) {
      info << "global:" << gv->getName();
    } else {
      info << "value:" << *allocSite;
    }
  } else {
    info << " (no allocation info)";
  }
  
  info.flush();
}

void MemoryObject::getAllocCrashInfo(std::string &result) const {
  llvm::raw_string_ostream info(result);

  //info << "MO" << id << "[" << getSizeString() << "]";
  
  if (allocSite) {
    info << " allocated at ";
    if (const Instruction *i = dyn_cast<Instruction>(allocSite)) {
      info << i->getParent()->getParent()->getName() << "():";
    } else if (const GlobalValue *gv = dyn_cast<GlobalValue>(allocSite)) {
      info << "global:" << gv->getName();
    } else {
      info << "value:" << *allocSite;
    }
  } else {
    info << " (no allocation info)";
  }
  
  info.flush();
}

void MemoryObject::initializeSymbolicArray(klee::ArrayCache &array) {
  if (!symbolicAddress) {
    auto tmparray = array.CreateArray(std::string("mo_addr_for_seg:") +
                                        std::to_string(segment),
                                      Context::get().getPointerWidth());
    symbolicAddress = Expr::createTempRead(tmparray,
                                           Context::get().getPointerWidth());
  }
}

ref<Expr> MemoryObject::getSymbolicAddress(klee::ArrayCache &array) {
  if (!symbolicAddress) {
    initializeSymbolicArray(array);
  }
  return symbolicAddress.getValue();
}

/***/

ObjectStatePlane::ObjectStatePlane(const ObjectState *parent)
  : parent(parent),
    updates(0, 0),
    sizeBound(0),
    initialized(true),
    symbolic(false),
    initialValue(0) {
  if (!UseConstantArrays) {
    static unsigned id = 0;
    const Array *array =
        parent->getArrayCache()->CreateArray("tmp_arr" + llvm::utostr(++id), sizeBound);
    updates = UpdateList(array, 0);
  }
  if (ConstantExpr *CE = dyn_cast<ConstantExpr>(parent->getObject()->size)) {
    sizeBound = CE->getZExtValue();
  }
}


ObjectStatePlane::ObjectStatePlane(const ObjectState *parent, const Array *array)
  : parent(parent),
    updates(array, 0),
    sizeBound(0),
    initialized(false),
    symbolic(true),
    initialValue(0) {
  if (ConstantExpr *CE = dyn_cast<ConstantExpr>(parent->getObject()->size)) {
    sizeBound = CE->getZExtValue();
  }
}

ObjectStatePlane::ObjectStatePlane(const ObjectState *parent, const ObjectStatePlane &os)
  : parent(parent),
    concreteStore(os.concreteStore),
    concreteMask(os.concreteMask),
    flushMask(os.flushMask),
    knownSymbolics(os.knownSymbolics),
    updates(os.updates),
    sizeBound(os.sizeBound),
    initialized(os.initialized),
    symbolic(os.symbolic),
    initialValue(os.initialValue) {
  assert(!os.parent->readOnly && "no need to copy read only object?");
}

ObjectStatePlane::~ObjectStatePlane() {
}

/***/

const UpdateList &ObjectStatePlane::getUpdates() const {
  // Constant arrays are created lazily.
  if (!updates.root) {
    // Collect the list of writes, with the oldest writes first.
    
    // FIXME: We should be able to do this more efficiently, we just need to be
    // careful to get the interaction with the cache right. In particular we
    // should avoid creating UpdateNode instances we never use.
    unsigned NumWrites = updates.head ? updates.head->getSize() : 0;
    std::vector< std::pair< ref<Expr>, ref<Expr> > > Writes(NumWrites);
    const UpdateNode *un = updates.head;
    for (unsigned i = NumWrites; i != 0; un = un->next) {
      --i;
      Writes[i] = std::make_pair(un->index, un->value);
    }

    std::vector< ref<ConstantExpr> > Contents(sizeBound);

    // Initialize to zeros.
    for (unsigned i = 0, e = sizeBound; i != e; ++i)
      Contents[i] = ConstantExpr::create(0, Expr::Int8);

    // Pull off as many concrete writes as we can.
    unsigned Begin = 0, End = Writes.size();
    for (; Begin != End; ++Begin) {
      // Push concrete writes into the constant array.
      ConstantExpr *Index = dyn_cast<ConstantExpr>(Writes[Begin].first);
      if (!Index)
        break;

      ConstantExpr *Value = dyn_cast<ConstantExpr>(Writes[Begin].second);
      if (!Value)
        break;

      Contents[Index->getZExtValue()] = Value;
    }

    static unsigned id = 0;
    const Array *array = parent->getArrayCache()->CreateArray(
        "const_arr" + llvm::utostr(++id), sizeBound, &Contents[0],
        &Contents[0] + Contents.size());
    updates = UpdateList(array, 0);

    // Apply the remaining (non-constant) writes.
    for (; Begin != End; ++Begin)
      updates.extend(Writes[Begin].first, Writes[Begin].second);
  }

  return updates;
}

void ObjectStatePlane::flushToConcreteStore(TimingSolver *solver,
                                       const ExecutionState &state) {
  for (unsigned i = 0; i < concreteStore.size(); i++) {
    if (isByteKnownSymbolic(i)) {
      ref<ConstantExpr> ce;
      bool success = solver->getValue(state, read8(i), ce);
      if (!success) {
        klee_warning("Solver timed out when getting a value for external call, "
                     "segment + offset %lu+%u will have random value",
                     parent->getObject()->segment, i);
      } else {
        uint8_t value;
        ce->toMemory(&value);
        concreteStore[i] = value;
      }
    }
  }
}

void ObjectStatePlane::makeConcrete() {
  concreteMask.resize(0);
  flushMask.resize(0);
  knownSymbolics.clear();
}

void ObjectStatePlane::initializeToZero() {
  makeConcrete();
  initialValue = 0;
}

void ObjectStatePlane::initializeToRandom() {
  makeConcrete();
  // randomly selected by 256 sided die
  initialValue = 0xAB;
}

/*
Cache Invariants
--
isByteKnownSymbolic(i) => !isByteConcrete(i)
isByteConcrete(i) => !isByteKnownSymbolic(i)
!isByteFlushed(i) => (isByteConcrete(i) || isByteKnownSymbolic(i))
 */

void ObjectStatePlane::flushForRead() const {
  // TODO: iterate only over the offsets for which we have information
  // (SparseVector may be, well, sparse...)
  for (unsigned offset = 0; offset < sizeBound; offset++) {
    if (!isByteFlushed(offset)) {
      if (isByteConcrete(offset)) {
        updates.extend(ConstantExpr::create(offset, Expr::Int32),
                       ConstantExpr::create(getConcreteValue(offset), Expr::Int8));
      } else {
        assert(isByteKnownSymbolic(offset) && "invalid bit set in flushMask");
        updates.extend(ConstantExpr::create(offset, Expr::Int32),
                       knownSymbolics[offset]);
      }

      markByteFlushed(offset);
    }
  } 
}

void ObjectStatePlane::flushForWrite() {
  for (unsigned offset = 0; offset < sizeBound; offset++) {
    if (!isByteFlushed(offset)) {
      if (isByteConcrete(offset)) {
        updates.extend(ConstantExpr::create(offset, Expr::Int32),
                       ConstantExpr::create(getConcreteValue(offset), Expr::Int8));
        markByteSymbolic(offset);
      } else {
        assert(isByteKnownSymbolic(offset) && "invalid bit set in flushMask");
        updates.extend(ConstantExpr::create(offset, Expr::Int32),
                       knownSymbolics[offset]);
        setKnownSymbolic(offset, 0);
      }

      markByteFlushed(offset);
    } else {
      // flushed bytes that are written over still need to be marked out
      markByteSymbolic(offset);
      setKnownSymbolic(offset, 0);
    }
  }
  initialized = false;
}

bool ObjectStatePlane::isByteConcrete(unsigned offset) const {
  if (offset < concreteMask.size())
    return concreteMask.get(offset);
  return initialized;
}

bool ObjectStatePlane::isByteFlushed(unsigned offset) const {
  if (offset < flushMask.size())
    return !flushMask.get(offset);
  return !initialized;
}

bool ObjectStatePlane::isByteKnownSymbolic(unsigned offset) const {
  return knownSymbolics.has(offset);
}

void ObjectStatePlane::markByteConcrete(unsigned offset) {
  if (offset >= concreteMask.size()) {
    if (initialized)
      return;
    concreteMask.resize(sizeBound, initialized);
  }
  concreteMask.set(offset);
}

void ObjectStatePlane::markByteSymbolic(unsigned offset) {
  if (offset >= concreteMask.size()) {
    if (!initialized)
      return;
    concreteMask.resize(sizeBound, initialized);
  }
  concreteMask.unset(offset);
}

void ObjectStatePlane::markByteUnflushed(unsigned offset) const {
  if (offset >= flushMask.size()) {
    if (initialized)
      return;
    flushMask.resize(sizeBound, initialized);
  }
  flushMask.set(offset);
}

void ObjectStatePlane::markByteFlushed(unsigned offset) const {
  if (offset >= flushMask.size()) {
    if (!initialized)
      return;
    flushMask.resize(sizeBound, initialized);
  }
  flushMask.unset(offset);
}

void ObjectStatePlane::setKnownSymbolic(unsigned offset,
                                        Expr *value /* can be null */) {
  knownSymbolics.set(offset, value);
}

uint8_t ObjectStatePlane::getConcreteValue(unsigned offset) const {
  if (offset < concreteStore.size())
    return concreteStore[offset];
  return initialValue;
}

/***/

ref<Expr> ObjectStatePlane::read8(unsigned offset) const {
  if (isByteConcrete(offset)) {
    return ConstantExpr::create(getConcreteValue(offset), Expr::Int8);
  } else if (isByteKnownSymbolic(offset)) {
    return knownSymbolics[offset];
  } else {
    assert(isByteFlushed(offset) && "unflushed byte without cache value");
    
    return read8(ConstantExpr::create(offset, Expr::Int32));
  }    
}

ref<Expr> ObjectStatePlane::read8(ref<Expr> offset) const {
  flushForRead();

  if (sizeBound>4096) {
    std::string allocInfo;
    parent->getObject()->getAllocInfo(allocInfo);
    klee_warning_once(0, "flushing %d bytes on read, may be slow and/or crash: %s", 
                      sizeBound,
                      allocInfo.c_str());
  }

  const UpdateList &updates = getUpdates();

  if (symbolic || isa<ConstantExpr>(parent->getObject()->size)) {
    return ReadExpr::create(updates, ZExtExpr::create(offset, Expr::Int32));
  }

  ref<Expr> cond = UltExpr::create(offset,
                                   ConstantExpr::alloc(updates.root->constantValues.size(), Expr::Int32));

  for (const UpdateNode* node = updates.head; node != 0; node = node->next) {
    cond = OrExpr::create(EqExpr::create(offset, node->index), cond);
  }
  
  return SelectExpr::create(
      cond,
      ReadExpr::create(updates, ZExtExpr::create(offset, Expr::Int32)),
      ConstantExpr::alloc(initialValue, Expr::Int8));
}

void ObjectStatePlane::write8(unsigned offset, uint8_t value) {
  //assert(read_only == false && "writing to read-only object!");
  if (offset >= sizeBound)
    sizeBound = offset + 1;
  if (concreteStore.size() <= offset)
    concreteStore.resize(sizeBound, initialValue);
  concreteStore[offset] = value;
  setKnownSymbolic(offset, 0);

  markByteConcrete(offset);
  markByteUnflushed(offset);
}

void ObjectStatePlane::write8(unsigned offset, ref<Expr> value) {
  // can happen when ExtractExpr special cases
  if (ConstantExpr *CE = dyn_cast<ConstantExpr>(value)) {
    write8(offset, (uint8_t) CE->getZExtValue(8));
  } else {
    if (offset >= sizeBound)
      sizeBound = offset + 1;
    setKnownSymbolic(offset, value.get());
      
    markByteSymbolic(offset);
    markByteUnflushed(offset);
  }
}

void ObjectStatePlane::write8(ref<Expr> offset, ref<Expr> value) {
  assert(!isa<ConstantExpr>(offset) && "constant offset passed to symbolic write8");
  flushForWrite();

  if (sizeBound>4096) {
    std::string allocInfo;
    parent->getObject()->getAllocInfo(allocInfo);
    klee_warning_once(0, "flushing %d bytes on read, may be slow and/or crash: %s", 
                      sizeBound,
                      allocInfo.c_str());
  }
  
  updates.extend(ZExtExpr::create(offset, Expr::Int32), value);
}

/***/

ref<Expr> ObjectStatePlane::read(ref<Expr> offset, Expr::Width width) const {
  // Truncate offset to 32-bits.
  offset = ZExtExpr::create(offset, Expr::Int32);

  // Check for reads at constant offsets.
  if (ConstantExpr *CE = dyn_cast<ConstantExpr>(offset))
    return read(CE->getZExtValue(32), width);

  // Treat bool specially, it is the only non-byte sized write we allow.
  if (width == Expr::Bool)
    return ExtractExpr::create(read8(offset), 0, Expr::Bool);

  // Otherwise, follow the slow general case.
  unsigned NumBytes = width / 8;
  assert(width == NumBytes * 8 && "Invalid read size!");
  ref<Expr> Res(0);
  for (unsigned i = 0; i != NumBytes; ++i) {
    unsigned idx = Context::get().isLittleEndian() ? i : (NumBytes - i - 1);
    ref<Expr> Byte = read8(AddExpr::create(offset, 
                                           ConstantExpr::create(idx, 
                                                                Expr::Int32)));
    Res = i ? ConcatExpr::create(Byte, Res) : Byte;
  }

  return Res;
}

ref<Expr> ObjectStatePlane::read(unsigned offset, Expr::Width width) const {
  // Treat bool specially, it is the only non-byte sized write we allow.
  if (width == Expr::Bool)
    return ExtractExpr::create(read8(offset), 0, Expr::Bool);

  // Otherwise, follow the slow general case.
  unsigned NumBytes = width / 8;
  assert(width == NumBytes * 8 && "Invalid width for read size!");
  ref<Expr> Res(0);
  for (unsigned i = 0; i != NumBytes; ++i) {
    unsigned idx = Context::get().isLittleEndian() ? i : (NumBytes - i - 1);
    ref<Expr> Byte = read8(offset + idx);
    Res = i ? ConcatExpr::create(Byte, Res) : Byte;
  }

  return Res;
}

void ObjectStatePlane::write(ref<Expr> offset, ref<Expr> value) {
  // Truncate offset to 32-bits.
  offset = ZExtExpr::create(offset, Expr::Int32);

  // Check for writes at constant offsets.
  if (ConstantExpr *CE = dyn_cast<ConstantExpr>(offset)) {
    write(CE->getZExtValue(32), value);
    return;
  }

  // Treat bool specially, it is the only non-byte sized write we allow.
  Expr::Width w = value->getWidth();
  if (w == Expr::Bool) {
    write8(offset, ZExtExpr::create(value, Expr::Int8));
    return;
  }

  // Otherwise, follow the slow general case.
  unsigned NumBytes = w / 8;
  assert(w == NumBytes * 8 && "Invalid write size!");
  for (unsigned i = 0; i != NumBytes; ++i) {
    unsigned idx = Context::get().isLittleEndian() ? i : (NumBytes - i - 1);
    write8(AddExpr::create(offset, ConstantExpr::create(idx, Expr::Int32)),
           ExtractExpr::create(value, 8 * i, Expr::Int8));
  }
}

void ObjectStatePlane::write(unsigned offset, ref<Expr> value) {
  // Check for writes of constant values.
  if (ConstantExpr *CE = dyn_cast<ConstantExpr>(value)) {
    Expr::Width w = CE->getWidth();
    if (w <= 64 && klee::bits64::isPowerOfTwo(w)) {
      uint64_t val = CE->getZExtValue();
      switch (w) {
      default: assert(0 && "Invalid write size!");
      case  Expr::Bool:
      case  Expr::Int8:  write8(offset, val); return;
      case Expr::Int16: write16(offset, val); return;
      case Expr::Int32: write32(offset, val); return;
      case Expr::Int64: write64(offset, val); return;
      }
    }
  }

  // Treat bool specially, it is the only non-byte sized write we allow.
  Expr::Width w = value->getWidth();
  if (w == Expr::Bool) {
    write8(offset, ZExtExpr::create(value, Expr::Int8));
    return;
  }

  // Otherwise, follow the slow general case.
  unsigned NumBytes = w / 8;
  assert(w == NumBytes * 8 && "Invalid write size!");
  for (unsigned i = 0; i != NumBytes; ++i) {
    unsigned idx = Context::get().isLittleEndian() ? i : (NumBytes - i - 1);
    write8(offset + idx, ExtractExpr::create(value, 8 * i, Expr::Int8));
  }
} 

void ObjectStatePlane::write16(unsigned offset, uint16_t value) {
  unsigned NumBytes = 2;
  for (unsigned i = 0; i != NumBytes; ++i) {
    unsigned idx = Context::get().isLittleEndian() ? i : (NumBytes - i - 1);
    write8(offset + idx, (uint8_t) (value >> (8 * i)));
  }
}

void ObjectStatePlane::write32(unsigned offset, uint32_t value) {
  unsigned NumBytes = 4;
  for (unsigned i = 0; i != NumBytes; ++i) {
    unsigned idx = Context::get().isLittleEndian() ? i : (NumBytes - i - 1);
    write8(offset + idx, (uint8_t) (value >> (8 * i)));
  }
}

void ObjectStatePlane::write64(unsigned offset, uint64_t value) {
  unsigned NumBytes = 8;
  for (unsigned i = 0; i != NumBytes; ++i) {
    unsigned idx = Context::get().isLittleEndian() ? i : (NumBytes - i - 1);
    write8(offset + idx, (uint8_t) (value >> (8 * i)));
  }
}

void ObjectStatePlane::print() const {
  llvm::errs() << "-- ObjectState --\n";
  if (parent)
    llvm::errs() << "\tMemoryObject ID: " << parent->getObject()->id << "\n";
  llvm::errs() << "\tRoot Object: " << updates.root << "\n";
  llvm::errs() << "\tSize: " << sizeBound << "\n";

  llvm::errs() << "\tBytes:\n";
  for (unsigned i=0; i<sizeBound; i++) {
    llvm::errs() << "\t\t["<<i<<"]"
               << " concrete? " << isByteConcrete(i)
               << " known-sym? " << isByteKnownSymbolic(i)
               << " flushed? " << isByteFlushed(i) << " = ";
    ref<Expr> e = read8(i);
    llvm::errs() << e << "\n";
  }

  llvm::errs() << "\tUpdates:\n";
  for (const UpdateNode *un=updates.head; un; un=un->next) {
    llvm::errs() << "\t\t[" << un->index << "] = " << un->value << "\n";
  }
}

/****/

ObjectState::ObjectState(const MemoryObject *mo)
  : copyOnWriteOwner(0),
    refCount(0),
    object(mo),
    readOnly(false),
    segmentPlane(0),
    offsetPlane(new ObjectStatePlane(this)){
  mo->refCount++;
}


ObjectState::ObjectState(const MemoryObject *mo, const Array *array)
  : copyOnWriteOwner(0),
    refCount(0),
    object(mo),
    readOnly(false),
    segmentPlane(0),
    offsetPlane(new ObjectStatePlane(this, array)) {
  mo->refCount++;
}

ObjectState::ObjectState(const ObjectState &os)
  : copyOnWriteOwner(0),
    refCount(0),
    object(os.object),
    readOnly(false),
    segmentPlane(0),
    offsetPlane(new ObjectStatePlane(this, *os.offsetPlane)) {
  object->refCount++;
  if (os.segmentPlane)
    segmentPlane = new ObjectStatePlane(this, *os.segmentPlane);
}

ObjectState::ObjectState(const ObjectState &os, const MemoryObject *mo)
  : ObjectState(os) {
  object->refCount--;
  object = mo;
  object->refCount++;
}

ObjectState::~ObjectState() {
  if (segmentPlane)
    delete segmentPlane;
  delete offsetPlane;
  if (object)
  {
    assert(object->refCount > 0);
    object->refCount--;
    if (object->refCount == 0)
    {
      delete object;
    }
  }
}

KValue ObjectState::read8(unsigned offset) const {
  ref<Expr> segment;
  if (segmentPlane) {
    segment = segmentPlane->read8(offset);
  } else {
    segment = ConstantExpr::alloc(0, Expr::Int8);
  }
  ref<Expr> value = offsetPlane->read8(offset);
  return KValue(segment, value);
}

KValue ObjectState::read(unsigned offset, Expr::Width width) const {
  ref<Expr> segment;
  if (segmentPlane) {
    segment = segmentPlane->read(offset, width);
  } else {
    segment = ConstantExpr::alloc(0, width);
  }
  ref<Expr> value = offsetPlane->read(offset, width);
  return KValue(segment, value);
}

KValue ObjectState::read(ref<Expr> offset, Expr::Width width) const {
  ref<Expr> segment;
  if (segmentPlane) {
    segment = segmentPlane->read(offset, width);
  } else {
    segment = ConstantExpr::alloc(0, width);
  }
  ref<Expr> value = offsetPlane->read(offset, width);
  return KValue(segment, value);
}

bool ObjectState::prepareSegmentPlane(bool nonzero) {
  if (!segmentPlane) {
    if (nonzero) {
      segmentPlane = new ObjectStatePlane(this);
      return true;
    }
    return false;
  }
  return true;
}

bool ObjectState::prepareSegmentPlane(ref<Expr> segment) {
  if (ConstantExpr *CE = dyn_cast<ConstantExpr>(segment))
    return prepareSegmentPlane(!CE->isZero());
  return prepareSegmentPlane(true);
}

void ObjectState::write8(unsigned offset, uint8_t segment, uint8_t value) {
  if (prepareSegmentPlane(segment))
    segmentPlane->write8(offset, segment);
  offsetPlane->write8(offset, value);
}

void ObjectState::write16(unsigned offset, uint16_t segment, uint16_t value) {
  if (prepareSegmentPlane(segment))
    segmentPlane->write16(offset, segment);
  offsetPlane->write16(offset, value);
}

void ObjectState::write32(unsigned offset, uint32_t segment, uint32_t value) {
  if (prepareSegmentPlane(segment))
    segmentPlane->write32(offset, segment);
  offsetPlane->write32(offset, value);
}

void ObjectState::write64(unsigned offset, uint64_t segment, uint64_t value) {
  if (prepareSegmentPlane(segment))
    segmentPlane->write64(offset, segment);
  offsetPlane->write64(offset, value);
}

void ObjectState::write(unsigned offset, const KValue& value) {
  if (prepareSegmentPlane(value.getSegment()))
    segmentPlane->write(offset, value.getSegment());
  offsetPlane->write(offset, value.getOffset());
}

void ObjectState::write(ref<Expr> offset, const KValue& value) {
  if (prepareSegmentPlane(value.getSegment()))
    segmentPlane->write(offset, value.getSegment());
  offsetPlane->write(offset, value.getOffset());
}

void ObjectState::initializeToZero() {
  offsetPlane->initializeToZero();
}

void ObjectState::initializeToRandom() {
  offsetPlane->initializeToRandom();
}

ArrayCache* ObjectState::getArrayCache() const {
  assert(object && "object was NULL");
  return object->parent->getArrayCache();
}
