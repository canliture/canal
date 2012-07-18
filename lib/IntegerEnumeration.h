#ifndef LIBCANAL_INTEGER_ENUMERATION_H
#define LIBCANAL_INTEGER_ENUMERATION_H

#include "Value.h"
#include <llvm/ADT/APInt.h>
#include <set>

namespace Canal {
namespace Integer {

struct APIntComp
{
    bool operator()(const llvm::APInt &a, const llvm::APInt &b) const
    {
        return a.ult(b);
    }
};

typedef std::set<llvm::APInt, APIntComp> APIntSet;

class Enumeration : public Value, public AccuracyValue
{
public:
    APIntSet mValues;
    bool mTop;
    unsigned mNumBits;

public:
    // Initializes to the lowest value.
    Enumeration(unsigned numBits);
    // Initializes to the given value.
    Enumeration(const llvm::APInt &number);

    // Lowest signed number represented by this abstract domain.
    // @param result
    //   Filled by the minimum value if it is known.  Otherwise, the
    //   value is undefined.
    // @return
    //   True if the result is known and the parameter was set to
    //   correct value.
    bool signedMin(llvm::APInt &result) const;

    // Highest signed number represented by this abstract domain.
    // @param result
    //   Filled by the maximum value if it is known.  Otherwise, the
    //   value is undefined.
    // @return
    //   True if the result is known and the parameter was set to
    //   correct value.
    bool signedMax(llvm::APInt &result) const;

    // Lowest unsigned number represented by this abstract domain.
    // @param result
    //   Filled by the minimum value if it is known.  Otherwise, the
    //   value is undefined.
    // @return
    //   True if the result is known and the parameter was set to
    //   correct value.
    bool unsignedMin(llvm::APInt &result) const;

    // Highest unsigned number represented by this abstract domain.
    // @param result
    //   Filled by the maximum value if it is known.  Otherwise, the
    //   value is undefined.
    // @return
    //   True if the result is known and the parameter was set to
    //   correct value.
    bool unsignedMax(llvm::APInt &result) const;

public: // Implementation of Value.
    // Implementation of Value::clone().
    // Covariant return type.
    virtual Enumeration *clone() const;
    // Implementation of Value::operator==().
    virtual bool operator==(const Value& value) const;
    // Implementation of Value::merge().
    virtual void merge(const Value &value);
    // Implementation of Value::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Value::toString().
    virtual std::string toString(const State *state) const;

    // Implementation of Value::add().
    virtual void add(const Value &a, const Value &b);
    // Implementation of Value::sub().
    virtual void sub(const Value &a, const Value &b);
    // Implementation of Value::mul().
    virtual void mul(const Value &a, const Value &b);
    // Implementation of Value::udiv().
    virtual void udiv(const Value &a, const Value &b);
    // Implementation of Value::sdiv().
    virtual void sdiv(const Value &a, const Value &b);
    // Implementation of Value::urem().
    virtual void urem(const Value &a, const Value &b);
    // Implementation of Value::srem().
    virtual void srem(const Value &a, const Value &b);
    // Implementation of Value::shl().
    virtual void shl(const Value &a, const Value &b);
    // Implementation of Value::lshr().
    virtual void lshr(const Value &a, const Value &b);
    // Implementation of Value::ashr().
    virtual void ashr(const Value &a, const Value &b);
    // Implementation of Value::and_().
    virtual void and_(const Value &a, const Value &b);
    // Implementation of Value::or_().
    virtual void or_(const Value &a, const Value &b);
    // Implementation of Value::xor_().
    virtual void xor_(const Value &a, const Value &b);

public: // Implementation of AccuracyValue.
    // Implementation of AccuracyValue::accuracy().
    virtual float accuracy() const;
    // Implementation of AccuracyValue::isBottom().
    virtual bool isBottom() const;
    // Implementation of AccuracyValue::setBottom().
    virtual void setBottom();
    // Implementation of AccuracyValue::isTop().
    virtual bool isTop() const;
    // Implementation of AccuracyValue::setTop().
    virtual void setTop();

protected:
    typedef llvm::APInt(llvm::APInt::*APIntOperation)(const llvm::APInt&) const;
    typedef llvm::APInt(llvm::APInt::*APIntOperationWithOverflow)(const llvm::APInt&, bool&) const;

    void applyOperation(const Value &a,
                        const Value &b,
                        APIntOperation operation1,
                        APIntOperationWithOverflow operation2);
};

} // namespace Integer
} // namespace Canal

#endif // LIBCANAL_INTEGER_ENUMERATION_H
