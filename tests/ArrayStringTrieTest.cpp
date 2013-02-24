#include "lib/ArrayStringTrie.h"
#include "lib/Environment.h"
#include "lib/Utils.h"
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/ManagedStatic.h>
#include <iostream>

using namespace Canal;

static Environment *gEnvironment;

static llvm::ArrayType *getTestType()
{
    return llvm::ArrayType::get(llvm::Type::getInt8Ty(
        gEnvironment->getContext()), 10);
}

static void testTrieEqualityOperator()
{
    Array::TrieNode trie1 = Array::TrieNode("");
    CANAL_ASSERT(trie1 == trie1);

    Array::TrieNode trie2 = Array::TrieNode("abc");
    Array::TrieNode trie3 = Array::TrieNode("defgh");
    Array::TrieNode trie4 = Array::TrieNode("abc");
    CANAL_ASSERT((trie2 == trie3) == false);
    CANAL_ASSERT(trie2 == trie4);

    Array::TrieNode trie5 = Array::TrieNode("asdf");
    Array::TrieNode *node1 = new Array::TrieNode("qwe");
    trie5.mChildren.insert(node1);
    CANAL_ASSERT(trie5.mChildren.size() == 1);
    Array::TrieNode trie6 = Array::TrieNode("asdf");
    CANAL_ASSERT(trie6.mChildren.empty());
    CANAL_ASSERT((trie5 == trie6) == false);

    Array::TrieNode trie7 = Array::TrieNode("asdf");
    Array::TrieNode *node2 = new Array::TrieNode("poi");
    trie7.mChildren.insert(node2);
    CANAL_ASSERT((trie5 == trie7) == false);

    Array::TrieNode trie8 = Array::TrieNode("asdf");
    Array::TrieNode *node3 = new Array::TrieNode("qwe");
    trie8.mChildren.insert(node3);
    CANAL_ASSERT(trie5 == trie8);
}

static void testTrieInequalityOperator()
{
    Array::TrieNode trie1 = Array::TrieNode("");
    CANAL_ASSERT((trie1 != trie1) == false);

    Array::TrieNode trie2 = Array::TrieNode("xyz");
    Array::TrieNode trie3 = Array::TrieNode("ijklm");
    Array::TrieNode trie4 = Array::TrieNode("xyz");
    CANAL_ASSERT(trie2 != trie3);
    CANAL_ASSERT((trie2 != trie4) == false);

    Array::TrieNode trie5 = Array::TrieNode("qwer");
    Array::TrieNode *node1 = new Array::TrieNode("zxc");
    trie5.mChildren.insert(node1);
    CANAL_ASSERT(trie5.mChildren.size() == 1);
    Array::TrieNode trie6 = Array::TrieNode("qwer");
    CANAL_ASSERT(trie6.mChildren.empty());
    CANAL_ASSERT(trie5 != trie6);

    Array::TrieNode trie7 = Array::TrieNode("qwer");
    Array::TrieNode *node2 = new Array::TrieNode("abc");
    trie7.mChildren.insert(node2);
    CANAL_ASSERT(trie5 != trie7);

    Array::TrieNode trie8 = Array::TrieNode("qwer");
    Array::TrieNode *node3 = new Array::TrieNode("zxc");
    trie8.mChildren.insert(node3);
    CANAL_ASSERT((trie5 != trie8) == false);
}

static void testTrieSize()
{
    Array::TrieNode trie1 = Array::TrieNode("");
    CANAL_ASSERT(trie1.size() == 0);

    Array::TrieNode trie2 = Array::TrieNode("test");
    CANAL_ASSERT(trie2.size() == 4);

    Array::TrieNode trie3 = Array::TrieNode("");
    Array::TrieNode *node1 = new Array::TrieNode("abc");
    Array::TrieNode *node2 = new Array::TrieNode("defgh");
    trie3.mChildren.insert(node1);
    trie3.mChildren.insert(node2);
    Array::TrieNode *node3 = new Array::TrieNode("ij");
    Array::TrieNode *first = *trie3.mChildren.begin();
    CANAL_ASSERT(first->mValue == "abc");
    first->mChildren.insert(node3);
    CANAL_ASSERT(trie3.size() == 10);
}

static void testConstructors()
{
    const llvm::ArrayType *type = getTestType();
    Array::StringTrie stringTrie(*gEnvironment, *type);
    CANAL_ASSERT(stringTrie.isBottom());
    CANAL_ASSERT(stringTrie.mRoot->mValue == "");
    CANAL_ASSERT(stringTrie.mRoot->mChildren.empty());

    // TODO test second constructor

    Array::StringTrie stringTrie2(*gEnvironment, "test");
    CANAL_ASSERT(!stringTrie2.isBottom());
    CANAL_ASSERT(stringTrie2.mRoot->mValue == "");
    CANAL_ASSERT(stringTrie2.mRoot->mChildren.size() == 1);
    Array::TrieNode *node = *stringTrie2.mRoot->mChildren.begin();
    CANAL_ASSERT(node->mValue == "test");
}

static void testEqualityOperator()
{
    // TODO
}

static void testSetTop()
{
    const llvm::ArrayType *type = getTestType();
    Array::StringTrie trie(*gEnvironment, *type);
    trie.setTop();
    CANAL_ASSERT(trie.isTop());
}

static void testSetBottom()
{
    const llvm::ArrayType *type = getTestType();
    Array::StringTrie trie(*gEnvironment, *type);
    trie.setBottom();
    CANAL_ASSERT(trie.isBottom());
}

int
main(int argc, char **argv)
{
    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::llvm_shutdown_obj y;

    llvm::Module *module = new llvm::Module("testModule", context);
    gEnvironment = new Environment(module);

    testTrieEqualityOperator();
    testTrieInequalityOperator();
    testTrieSize();
    testConstructors();
    testEqualityOperator();
    testSetTop();
    testSetBottom();

    delete gEnvironment;
    return 0;
}

