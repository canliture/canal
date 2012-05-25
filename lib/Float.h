#ifndef LIBCANAL_FLOAT_H
#define LIBCANAL_FLOAT_H

#include "Value.h"
#include <llvm/ADT/APFloat.h>

namespace Canal {
namespace Float {

class Range : public Value, public AccuracyValue
{
public:
    bool mEmpty;
    bool mTop;
    llvm::APFloat mFrom;
    llvm::APFloat mTo;

public:
    Range(const llvm::fltSemantics &semantics);

public: // Implementation of Value.
    // Implementation of Value::clone().
    // Covariant return type.
    virtual Range *clone() const;
    // Implementation of Value::operator==().
    virtual bool operator==(const Value& value) const;
    // Implementation of Value::merge().
    virtual void merge(const Value &value);
    // Implementation of Value::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Value::toString().
    virtual std::string toString(const State *state) const;

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
};

} // namespace Float
} // namespace Canal

#endif // LIBCANAL_FLOAT_H
