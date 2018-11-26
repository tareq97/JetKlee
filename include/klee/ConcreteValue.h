#ifndef KLEE_CONCRETE_VALUE_
#define KLEE_CONCRETE_VALUE_

#include <llvm/ADT/APInt.h>
#include <experimental/optional>

namespace klee {
// wrapper around APInt that remembers the signdness
struct ConcreteValue {
    llvm::APInt value;
    bool issigned{false};

    ConcreteValue(unsigned numBits, uint64_t val, bool isSigned)
    : value(numBits, val, isSigned), issigned(isSigned) {}

    ConcreteValue(const llvm::APInt& val, bool isSigned)
    : value(val), issigned(isSigned) {}

    ConcreteValue(llvm::APInt&& val, bool isSigned)
    : value(std::move(val)), issigned(isSigned) {}

    bool isSigned() const { return issigned; }
    uint64_t getZExtValue() const { return value.getZExtValue(); }
    // makes sense also for unsigned
    uint64_t getSExtValue() const { return value.getSExtValue(); }

    unsigned getBitWidth() const { return value.getBitWidth(); }
    // WARNING: not efficient
    std::string toString() const { return value.toString(10, issigned); }

    llvm::APInt& getValue() { return value; }
    const llvm::APInt& getValue() const { return value; }
};

struct MaybeConcreteValue {
    std::experimental::optional<ConcreteValue> value;

    bool hasValue() const { return static_cast<bool>(value); }
};

} // namespace klee

#endif
