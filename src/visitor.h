/**
 * Copyright (c) 2012, Fedor Indutny.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _SRC_VISITOR_H_
#define _SRC_VISITOR_H_

#include "utils.h"
#include "zone.h"  // ZoneList

namespace candor {
namespace internal {

#define VISITOR_MAPPING_BLOCK(V, T) \
    V(Function, T) \
    V(Call, T) \
    V(Block, T) \
    V(If, T) \
    V(While, T) \
    V(Assign, T) \
    V(Member, T) \
    V(VarArg, T) \
    V(ObjectLiteral, T) \
    V(ArrayLiteral, T) \
    V(Clone, T) \
    V(Delete, T) \
    V(Return, T) \
    V(Break, T) \
    V(Continue, T) \
    V(Typeof, T) \
    V(Sizeof, T) \
    V(Keysof, T) \
    V(UnOp, T) \
    V(BinOp, T)

#define VISITOR_MAPPING_REGULAR(V, T) \
    V(Name, T) \
    V(Value, T) \
    V(Number, T) \
    V(Property, T) \
    V(String, T) \
    V(Nil, T) \
    V(True, T) \
    V(False, T)

#define VISITOR_SUB_DECLARE(V, T) \
    template T* Visitor<T>::Visit##V(AstNode* node);

#define VISITOR_DECLARE(T) \
    template Visitor<T>::Visit(AstNode* node); \
    template void Visitor<T>::VisitChildren(AstNode* node); \
    VISITOR_MAPPING_BLOCK(VISITOR_SUB_DECLARE, T) \
    VISITOR_MAPPING_REGULAR(VISITOR_SUB_DECLARE, T)

// Forward declaration
class AstNode;
class FunctionLiteral;

// AST Visiting abstraction
template <class T>
class Visitor {
 public:
  enum Type {
    kPreorder,
    kBreadthFirst
  };

  explicit Visitor(Type type) : type_(type), queue_(NULL) {
  }
  virtual ~Visitor() {
  }

  virtual T* Visit(AstNode* node);
  virtual void VisitChildren(AstNode* node);

  virtual T* VisitFunction(AstNode* node);
  virtual T* VisitCall(AstNode* node);
  virtual T* VisitBlock(AstNode* node);
  virtual T* VisitIf(AstNode* node);
  virtual T* VisitWhile(AstNode* node);
  virtual T* VisitAssign(AstNode* node);
  virtual T* VisitMember(AstNode* node);
  virtual T* VisitName(AstNode* node);
  virtual T* VisitValue(AstNode* node);
  virtual T* VisitVarArg(AstNode* node);
  virtual T* VisitNumber(AstNode* node);
  virtual T* VisitObjectLiteral(AstNode* node);
  virtual T* VisitArrayLiteral(AstNode* node);
  virtual T* VisitNil(AstNode* node);
  virtual T* VisitClone(AstNode* node);
  virtual T* VisitDelete(AstNode* node);
  virtual T* VisitTrue(AstNode* node);
  virtual T* VisitFalse(AstNode* node);
  virtual T* VisitReturn(AstNode* node);
  virtual T* VisitBreak(AstNode* node);
  virtual T* VisitContinue(AstNode* node);
  virtual T* VisitProperty(AstNode* node);
  virtual T* VisitString(AstNode* node);

  virtual T* VisitTypeof(AstNode* node);
  virtual T* VisitSizeof(AstNode* node);
  virtual T* VisitKeysof(AstNode* node);

  virtual T* VisitUnOp(AstNode* node);
  virtual T* VisitBinOp(AstNode* node);

  inline AstNode* current_node() { return current_node_; }

 protected:
  Type type_;

  AstNode* current_node_;
  ZoneList<ZoneList<AstNode*>::Item*>* queue_;
};

class FunctionIterator : public Visitor<AstNode> {
 public:
  explicit FunctionIterator(AstNode* root);

  bool IsEnded();
  void Advance();
  FunctionLiteral* Value();

  AstNode* VisitFunction(AstNode* node);

 private:
  ZoneList<AstNode*> work_queue_;
};

}  // namespace internal
}  // namespace candor

#endif  // _SRC_VISITOR_H_
