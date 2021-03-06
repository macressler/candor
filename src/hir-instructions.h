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

#ifndef _SRC_HIR_INSTRUCTIONS_H_
#define _SRC_HIR_INSTRUCTIONS_H_

#include "ast.h"  // AstNode
#include "scope.h"  // ScopeSlot
#include "zone.h"  // Zone, ZoneList
#include "utils.h"  // PrintBuffer

namespace candor {
namespace internal {

// Forward declarations
class HIRGen;
class HIRBlock;
class HIRInstruction;
class HIRPhi;
class LInstruction;

typedef ZoneList<HIRInstruction*> HIRInstructionList;
typedef ZoneMap<NumberKey, HIRInstruction, ZoneObject> HIRInstructionMap;
typedef ZoneList<HIRPhi*> HIRPhiList;

#define HIR_INSTRUCTION_TYPES(V) \
    V(Nop) \
    V(Nil) \
    V(Entry) \
    V(Return) \
    V(Function) \
    V(LoadArg) \
    V(LoadVarArg) \
    V(StoreArg) \
    V(StoreVarArg) \
    V(AlignStack) \
    V(LoadContext) \
    V(StoreContext) \
    V(LoadProperty) \
    V(StoreProperty) \
    V(DeleteProperty) \
    V(If) \
    V(Literal) \
    V(Goto) \
    V(Not) \
    V(BinOp) \
    V(Typeof) \
    V(Sizeof) \
    V(Keysof) \
    V(Clone) \
    V(Call) \
    V(CollectGarbage) \
    V(GetStackTrace) \
    V(AllocateObject) \
    V(AllocateArray) \
    V(Phi)

#define HIR_INSTRUCTION_ENUM(I) \
    k##I,

class HIRInstruction : public ZoneObject {
 public:
  enum Type {
    HIR_INSTRUCTION_TYPES(HIR_INSTRUCTION_ENUM)
    kNone
  };

  enum Representation {
    kUnknownRepresentation    = 0x00,  // 0000000000
    kNilRepresentation        = 0x01,  // 0000000001
    kNumberRepresentation     = 0x02,  // 0000000010
    kSmiRepresentation        = 0x06,  // 0000000110
    kHeapNumberRepresentation = 0x0A,  // 0000001010
    kStringRepresentation     = 0x10,  // 0000010000
    kBooleanRepresentation    = 0x20,  // 0000100000
    kNumMapRepresentation     = 0x40,  // 0001000000
    kObjectRepresentation     = 0xC0,  // 0011000000
    kArrayRepresentation      = 0x140,  // 0101000000
    kFunctionRepresentation   = 0x200,  // 1000000000
    kAnyRepresentation        = 0x2FF,  // 1111111111

    // no value, not for real use
    kHoleRepresentation       = 0x300  // 10000000000
  };

  explicit HIRInstruction(Type type);
  HIRInstruction(Type type, ScopeSlot* slot);

  virtual void Init(HIRGen* g, HIRBlock* block);

  int id;
  int gcm_visited;
  int gvn_visited;
  int alias_visited;
  int is_live;

  virtual void ReplaceArg(HIRInstruction* o, HIRInstruction* n);
  virtual bool HasSideEffects();
  virtual bool HasGVNSideEffects();
  virtual void CalculateRepresentation();
  virtual bool Effects(HIRInstruction* instr);
  void Remove();
  void RemoveUse(HIRInstruction* i);

  // GVN hashmap routines
  static bool IsEqual(HIRInstruction* a, HIRInstruction* b);
  static uint32_t Hash(HIRInstruction* instr);

  inline HIRInstruction* AddArg(Type type);
  inline HIRInstruction* AddArg(HIRInstruction* instr);
  inline bool Is(Type type);
  inline Type type();
  inline bool IsRemoved();
  virtual void Print(PrintBuffer* p);
  inline const char* TypeToStr(Type type);

  inline Representation representation();
  inline bool IsNumber();
  inline bool IsSmi();
  inline bool IsHeapNumber();
  inline bool IsString();
  inline bool IsBoolean();

  inline bool IsPinned();
  inline HIRInstruction* Unpin();
  inline HIRInstruction* Pin();

  inline HIRBlock* block();
  inline void block(HIRBlock* block);
  inline ScopeSlot* slot();
  inline void slot(ScopeSlot* slot);
  inline AstNode* ast();
  inline void ast(AstNode* ast);
  inline HIRInstructionList* args();
  inline HIRInstructionList* uses();
  inline HIRInstructionList* effects_in();
  inline HIRInstructionList* effects_out();

  inline HIRInstruction* left();
  inline HIRInstruction* right();
  inline HIRInstruction* third();

  inline LInstruction* lir();
  inline void lir(LInstruction* lir);

 protected:
  virtual bool IsGVNEqual(HIRInstruction* to);
  bool HasSameEffects(HIRInstruction* to);

  Type type_;
  ScopeSlot* slot_;
  AstNode* ast_;
  LInstruction* lir_;
  HIRBlock* block_;

  bool hashed_;
  uint32_t hash_;

  bool removed_;
  bool pinned_;

  // Cached representation
  Representation representation_;

  HIRInstructionList args_;
  HIRInstructionList uses_;
  HIRInstructionList effects_in_;
  HIRInstructionList effects_out_;
};

#undef HIR_INSTRUCTION_ENUM

class HIRGVNMap : public ZoneMap<HIRInstruction, HIRInstruction, ZoneObject>,
                  public ZoneObject {
 public:
};

#define HIR_DEFAULT_METHODS(V) \
  static inline HIR##V* Cast(HIRInstruction* instr) { \
    assert(instr->type() == k##V); \
    return reinterpret_cast<HIR##V*>(instr); \
  }

class HIRPhi : public HIRInstruction {
 public:
  explicit HIRPhi(ScopeSlot* slot);

  void Init(HIRGen* g, HIRBlock* b);

  void ReplaceArg(HIRInstruction* o, HIRInstruction* n);
  void CalculateRepresentation();
  bool Effects(HIRInstruction* instr);

  inline void AddInput(HIRInstruction* instr);
  inline HIRInstruction* InputAt(int i);
  inline void Nilify();

  inline int input_count();
  inline void input_count(int input_count);

  HIR_DEFAULT_METHODS(Phi)

 private:
  int input_count_;
  HIRInstruction* inputs_[2];
};

class HIRLiteral : public HIRInstruction {
 public:
  HIRLiteral(AstNode::Type type, ScopeSlot* slot);

  inline ScopeSlot* root_slot();

  void CalculateRepresentation();

  HIR_DEFAULT_METHODS(Literal)

 protected:
  bool IsGVNEqual(HIRInstruction* to);

  AstNode::Type type_;
  ScopeSlot* root_slot_;
};

class HIRFunction : public HIRInstruction {
 public:
  explicit HIRFunction(AstNode* ast);

  int arg_count;

  void CalculateRepresentation();
  void Print(PrintBuffer* p);

  inline FunctionLiteral* ast();

  HIR_DEFAULT_METHODS(Function)

 protected:
  bool IsGVNEqual(HIRInstruction* to);
  FunctionLiteral* ast_;
};

class HIRNil : public HIRInstruction {
 public:
  HIRNil();

  HIR_DEFAULT_METHODS(Nil)
};

class HIREntry : public HIRInstruction {
 public:
  HIREntry(Label* label, int context_slots);

  void Print(PrintBuffer* p);
  bool HasSideEffects();

  inline Label* label();
  inline int context_slots();

  HIR_DEFAULT_METHODS(Entry)

 private:
  Label* label_;
  int context_slots_;
};

class HIRReturn : public HIRInstruction {
  public:
  HIRReturn();

  bool HasSideEffects();

  HIR_DEFAULT_METHODS(Return)

 private:
};

class HIRIf : public HIRInstruction {
  public:
  HIRIf();

  bool HasSideEffects();

  HIR_DEFAULT_METHODS(If)

 private:
};

class HIRGoto : public HIRInstruction {
  public:
  HIRGoto();

  bool HasSideEffects();

  HIR_DEFAULT_METHODS(Goto)

 private:
};

class HIRCollectGarbage : public HIRInstruction {
  public:
  HIRCollectGarbage();

  bool HasSideEffects();

  HIR_DEFAULT_METHODS(CollectGarbage)

 private:
};

class HIRGetStackTrace : public HIRInstruction {
  public:
  HIRGetStackTrace();

  bool HasSideEffects();

  HIR_DEFAULT_METHODS(GetStackTrace)

 private:
};

class HIRBinOp : public HIRInstruction {
  public:
  explicit HIRBinOp(BinOp::BinOpType type);

  void CalculateRepresentation();
  inline BinOp::BinOpType binop_type();

  HIR_DEFAULT_METHODS(BinOp)

 protected:
  bool IsGVNEqual(HIRInstruction* to);

  BinOp::BinOpType binop_type_;
};

class HIRLoadContext : public HIRInstruction {
 public:
  explicit HIRLoadContext(ScopeSlot* slot);

  inline ScopeSlot* context_slot();
  bool HasGVNSideEffects();

  HIR_DEFAULT_METHODS(LoadContext)

 protected:
  ScopeSlot* context_slot_;
};

class HIRStoreContext : public HIRInstruction {
 public:
  explicit HIRStoreContext(ScopeSlot* slot);

  void CalculateRepresentation();
  bool HasSideEffects();
  inline ScopeSlot* context_slot();

  HIR_DEFAULT_METHODS(StoreContext)

 private:
  ScopeSlot* context_slot_;
};

class HIRLoadProperty : public HIRInstruction {
 public:
  HIRLoadProperty();

  bool HasGVNSideEffects();
  HIR_DEFAULT_METHODS(LoadProperty)

 private:
  bool IsGVNEqual(HIRInstruction* to);
};

class HIRStoreProperty : public HIRInstruction {
 public:
  HIRStoreProperty();

  bool HasSideEffects();
  void CalculateRepresentation();
  bool Effects(HIRInstruction* instr);

  HIR_DEFAULT_METHODS(StoreProperty)

 private:
};

class HIRDeleteProperty : public HIRInstruction {
 public:
  HIRDeleteProperty();

  bool HasSideEffects();
  bool Effects(HIRInstruction* instr);
  HIR_DEFAULT_METHODS(DeleteProperty)

 private:
};

class HIRAllocateObject : public HIRInstruction {
 public:
  explicit HIRAllocateObject(int size);

  bool HasGVNSideEffects();
  void CalculateRepresentation();
  inline int size();

  HIR_DEFAULT_METHODS(AllocateObject)

 private:
  int size_;
};

class HIRAllocateArray : public HIRInstruction {
 public:
  explicit HIRAllocateArray(int size);

  bool HasGVNSideEffects();
  void CalculateRepresentation();
  inline int size();

  HIR_DEFAULT_METHODS(AllocateArray)

 private:
  int size_;
};

class HIRLoadArg : public HIRInstruction {
 public:
  HIRLoadArg();

  HIR_DEFAULT_METHODS(LoadArg)

 private:
};

class HIRLoadVarArg : public HIRInstruction {
 public:
  HIRLoadVarArg();

  bool HasSideEffects();
  void CalculateRepresentation();

  HIR_DEFAULT_METHODS(LoadVarArg)

 private:
};

class HIRStoreArg : public HIRInstruction {
 public:
  HIRStoreArg();

  bool HasSideEffects();
  bool Effects(HIRInstruction* instr);

  HIR_DEFAULT_METHODS(StoreArg)

 private:
};

class HIRStoreVarArg : public HIRInstruction {
 public:
  HIRStoreVarArg();

  bool HasSideEffects();

  HIR_DEFAULT_METHODS(StoreVarArg)

 private:
};

class HIRAlignStack : public HIRInstruction {
 public:
  HIRAlignStack();

  bool HasSideEffects();

  HIR_DEFAULT_METHODS(AlignStack)

 private:
};

class HIRCall : public HIRInstruction {
 public:
  HIRCall();

  bool HasSideEffects();

  HIR_DEFAULT_METHODS(Call)

 private:
};

class HIRKeysof : public HIRInstruction {
 public:
  HIRKeysof();

  void CalculateRepresentation();
  bool IsGVNEqual(HIRInstruction* to);

  HIR_DEFAULT_METHODS(Keysof)

 private:
};

class HIRSizeof : public HIRInstruction {
 public:
  HIRSizeof();

  void CalculateRepresentation();
  bool IsGVNEqual(HIRInstruction* to);

  HIR_DEFAULT_METHODS(Sizeof)

 private:
};

class HIRTypeof : public HIRInstruction {
 public:
  HIRTypeof();

  void CalculateRepresentation();

  HIR_DEFAULT_METHODS(Typeof)

 private:
};

class HIRClone : public HIRInstruction {
 public:
  HIRClone();

  void CalculateRepresentation();
  bool HasGVNSideEffects();

  HIR_DEFAULT_METHODS(Clone)

 private:
};

#undef HIR_DEFAULT_METHODS

}  // namespace internal
}  // namespace candor

#endif  // _SRC_HIR_INSTRUCTIONS_H_
