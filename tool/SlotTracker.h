#ifndef CANAL_SLOT_TRACKER_H
#define CANAL_SLOT_TRACKER_H

#include <map>
#include <vector>

namespace llvm {
    class Value;
    class GlobalValue;
    class Module;
    class Function;
    class MDNode;
}

/// This class provides computation of slot numbers.  Initial version
/// was taken from LLVM source code (lib/VMCore/AsmWriter.cpp).
class SlotTracker
{
public:
    /// Construct from a module.
    SlotTracker(const llvm::Module &module);

    /// Get the slot number for a value that is local to a function.
    /// Return the slot number of the specified value in it's type
    /// plane.  If something is not in the SlotTracker, return -1.
    int getLocalSlot(const llvm::Value &value);
    const llvm::Value *getLocalSlot(int num);
    /// Get the slot number of a global value.
    int getGlobalSlot(const llvm::Value &value);
    const llvm::Value *getGlobalSlot(int num);
    /// Get the slot number of a MDNode.
    int getMetadataSlot(const llvm::MDNode &node);

    /// If you'd like to deal with a function instead of just a module, use
    /// this method to get its data into the SlotTracker.
    void setActiveFunction(const llvm::Function &function);

    /// MDNode map iterators.
    typedef std::map<const llvm::MDNode*, unsigned>::iterator mdn_iterator;
    mdn_iterator mdn_begin() { return mMetadataMap.begin(); }
    mdn_iterator mdn_end() { return mMetadataMap.end(); }
    unsigned mdn_size() const { return mMetadataMap.size(); }
    bool mdn_empty() const { return mMetadataMap.empty(); }

protected:
    /// A mapping of Values to slot numbers.
    typedef std::map<const llvm::Value*, unsigned> ValueMap;
    typedef std::vector<const llvm::Value*> ValueList;

    /// This function does the actual initialization.
    void initialize();

    /// Insert the specified Value* into the slot table.
    void createFunctionSlot(const llvm::Value &value);

    /// Insert the specified GlobalValue* into the slot table.
    void createModuleSlot(const llvm::GlobalValue &value);

    /// Insert the specified MDNode* into the slot table.
    void createMetadataSlot(const llvm::MDNode &node);

    /// Add all of the module level global variables (and their
    /// initializers) and function declarations, but not the contents
    /// of those functions.
    void processModule();

    /// Add all of the functions arguments, basic blocks, and
    /// instructions.
    void processFunction();

protected:
    /// The module for which we are holding slot numbers.
    const llvm::Module &mModule;
    bool mModuleProcessed;

    /// The function for which we are holding slot numbers.
    const llvm::Function *mFunction;
    bool mFunctionProcessed;

    /// The slot map for the module level data.
    ValueMap mModuleMap;
    ValueList mModuleList;
    unsigned mModuleNext;

    /// The slot map for the function level data.
    ValueMap mFunctionMap;
    ValueList mFunctionList;
    unsigned mFunctionNext;

    /// The slot map for MDNodes.
    std::map<const llvm::MDNode*, unsigned> mMetadataMap;
    unsigned mMetadataNext;
};

#endif // CANAL_SLOT_TRACKER_H