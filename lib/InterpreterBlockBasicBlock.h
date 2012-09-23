#ifndef LIBCANAL_INTERPRETER_BLOCK_BASIC_BLOCK_H
#define LIBCANAL_INTERPRETER_BLOCK_BASIC_BLOCK_H

#include "State.h"
#include <llvm/BasicBlock.h>

namespace Canal {

class Constructors;

namespace InterpreterBlock {

class BasicBlock
{
protected:
    const llvm::BasicBlock &mBasicBlock;

    State mInputState;
    State mOutputState;

public:
    BasicBlock(const llvm::BasicBlock &basicBlock,
               const Constructors &constructors);

    const llvm::BasicBlock &getBasicBlock() const { return mBasicBlock; }

    llvm::BasicBlock::const_iterator begin() const { return mBasicBlock.begin(); }
    llvm::BasicBlock::const_iterator end() const { return mBasicBlock.end(); }

    State &getInputState() { return mInputState; }
    State &getOutputState() { return mOutputState; }
};

} // namespace InterpreterBlock
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_BLOCK_BASIC_BLOCK_H
