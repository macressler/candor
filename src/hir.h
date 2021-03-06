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

#ifndef _SRC_HIR_H_
#define _SRC_HIR_H_

#include <stdint.h>  // int32_t

#include "hir-instructions.h"
#include "hir-instructions-inl.h"
#include "heap.h"  // Heap
#include "heap-inl.h"  // Heap
#include "root.h"  // Root
#include "ast.h"  // AstNode
#include "scope.h"  // ScopeSlot
#include "visitor.h"  // Visitor
#include "zone.h"  // ZoneObject
#include "utils.h"  // PrintBuffer

namespace candor {
namespace internal {

// Forward declaration
class HIRGen;
class HIRBlock;
class LBlock;

typedef ZoneList<HIRBlock*> HIRBlockList;

class HIREnvironment : public ZoneObject {
 public:
  explicit HIREnvironment(int stack_slots);
  void Copy(HIREnvironment* from);

  inline HIRInstruction* At(int i);
  inline HIRPhi* PhiAt(int i);
  inline void Set(int i, HIRInstruction* value);
  inline void SetPhi(int i, HIRPhi* value);

  inline HIRInstruction* At(ScopeSlot* slot);
  inline HIRPhi* PhiAt(ScopeSlot* slot);
  inline void Set(ScopeSlot* slot, HIRInstruction* value);
  inline void SetPhi(ScopeSlot* slot, HIRPhi* value);

  inline ScopeSlot* logic_slot();
  inline int stack_slots();

 protected:
  int stack_slots_;
  ScopeSlot* logic_slot_;
  HIRInstruction** instructions_;
  HIRPhi** phis_;
};

class HIRBlock : public ZoneObject {
 public:
  explicit HIRBlock(HIRGen* g);

  int id;
  int dfs_id;
  int loop_depth;

  HIRInstruction* Assign(ScopeSlot* slot, HIRInstruction* value);
  void Remove(HIRInstruction* instr);

  inline HIRBlock* root();
  inline void root(HIRBlock* root);
  inline HIRBlock* AddSuccessor(HIRBlock* b);
  inline HIRInstruction* Add(HIRInstruction::Type type);
  inline HIRInstruction* Add(HIRInstruction::Type type, ScopeSlot* slot);
  inline HIRInstruction* Add(HIRInstruction* instr);
  inline HIRInstruction* Goto(HIRBlock* target);
  inline HIRInstruction* Branch(HIRInstruction* instr,
                                HIRBlock* t,
                                HIRBlock* f);
  inline HIRInstruction* Return(HIRInstruction* instr);
  inline bool IsEnded();
  inline bool IsEmpty();
  void MarkPreLoop();
  void MarkLoop();
  inline bool IsLoop();
  inline HIRPhi* CreatePhi(ScopeSlot* slot);

  inline BitField<EmptyClass>* reachable_from();
  inline HIREnvironment* env();
  inline void env(HIREnvironment* env);
  inline HIRInstructionList* instructions();
  inline HIRPhiList* phis();
  inline HIRBlock* SuccAt(int i);
  inline HIRBlock* PredAt(int i);
  inline int pred_count();
  inline int succ_count();

  inline HIRBlock* parent();
  inline void parent(HIRBlock* parent);
  inline HIRBlock* ancestor();
  inline void ancestor(HIRBlock* ancestor);
  inline HIRBlock* label();
  inline void label(HIRBlock* label);
  inline HIRBlock* semi();
  inline void semi(HIRBlock* semi);
  inline HIRBlock* dominator();
  inline void dominator(HIRBlock* dom);
  inline int dominator_depth();
  inline HIRBlockList* dominates();

  inline LBlock* lir();
  inline void lir(LBlock* lir);

  inline void Print(PrintBuffer* p);

 protected:
  void AddPredecessor(HIRBlock* b);
  inline void Compress();
  inline HIRBlock* Evaluate();

  HIRGen* g_;
  BitField<EmptyClass> reachable_from_;

  bool loop_;
  bool ended_;

  HIREnvironment* env_;
  HIRInstructionList instructions_;
  HIRPhiList phis_;

  int pred_count_;
  int succ_count_;
  HIRBlock* root_;
  HIRBlock* pred_[2];
  HIRBlock* succ_[2];

  // Dominators algorithm augmentation
  HIRBlock* parent_;
  HIRBlock* ancestor_;
  HIRBlock* label_;
  HIRBlock* semi_;
  HIRBlock* dominator_;
  int dominator_depth_;
  HIRBlockList dominates_;

  // Allocator augmentation
  LBlock* lir_;
  int start_id_;
  int end_id_;

  // Only HIRGen should be able to call Evaluate()
  friend class HIRGen;
};

class BreakContinueInfo : public ZoneObject {
 public:
  BreakContinueInfo(HIRGen* g, HIRBlock* end);

  HIRBlock* GetContinue();
  HIRBlock* GetBreak();

  inline HIRBlockList* continue_blocks();

 private:
  HIRGen* g_;
  HIRBlockList continue_blocks_;
  HIRBlock* brk_;
};

class HIRGen : public Visitor<HIRInstruction> {
 public:
  HIRGen(Heap* heap, Root* root, const char* filename);
  ~HIRGen();

  void Build(AstNode* root);

  void PrunePhis();
  void FindReachableBlocks();
  void DeriveDominators();
  void EnumerateDFS(HIRBlock* b, HIRBlockList* blocks);
  void EliminateDeadCode();
  void EliminateDeadCode(HIRInstruction* instr);
  void FindEffects();
  void FindOutEffects(HIRInstruction* instr);
  void FindInEffects(HIRInstruction* instr);
  void GlobalValueNumbering();
  void GlobalValueNumbering(HIRInstruction* instr, HIRGVNMap* gvn);
  void GlobalCodeMotion();
  void ScheduleEarly(HIRInstruction* instr, HIRBlock* root);
  void ScheduleLate(HIRInstruction* instr);
  HIRBlock* FindLCA(HIRBlock* a, HIRBlock* b);

  void Replace(HIRInstruction* o, HIRInstruction* n);

  HIRInstruction* Visit(AstNode* stmt);
  HIRInstruction* VisitFunction(AstNode* stmt);
  HIRInstruction* VisitAssign(AstNode* stmt);
  HIRInstruction* VisitReturn(AstNode* stmt);
  HIRInstruction* VisitValue(AstNode* stmt);
  HIRInstruction* VisitIf(AstNode* stmt);
  HIRInstruction* VisitWhile(AstNode* stmt);
  HIRInstruction* VisitBreak(AstNode* stmt);
  HIRInstruction* VisitContinue(AstNode* stmt);
  HIRInstruction* VisitUnOp(AstNode* stmt);
  HIRInstruction* VisitBinOp(AstNode* stmt);
  HIRInstruction* VisitObjectLiteral(AstNode* stmt);
  HIRInstruction* VisitArrayLiteral(AstNode* stmt);
  HIRInstruction* VisitMember(AstNode* stmt);
  HIRInstruction* VisitDelete(AstNode* stmt);
  HIRInstruction* VisitCall(AstNode* stmt);
  HIRInstruction* VisitTypeof(AstNode* stmt);
  HIRInstruction* VisitSizeof(AstNode* stmt);
  HIRInstruction* VisitKeysof(AstNode* stmt);
  HIRInstruction* VisitClone(AstNode* stmt);

  // Literals

  HIRInstruction* VisitLiteral(AstNode* stmt);
  HIRInstruction* VisitNumber(AstNode* stmt);
  HIRInstruction* VisitNil(AstNode* stmt);
  HIRInstruction* VisitTrue(AstNode* stmt);
  HIRInstruction* VisitFalse(AstNode* stmt);
  HIRInstruction* VisitString(AstNode* stmt);
  HIRInstruction* VisitProperty(AstNode* stmt);

  inline void set_current_block(HIRBlock* b);
  inline void set_current_root(HIRBlock* b);
  inline HIRBlock* current_block();
  inline HIRBlock* current_root();

  inline HIRBlockList* blocks();
  inline HIRBlockList* roots();

  inline HIRInstruction* Add(HIRInstruction::Type type);
  inline HIRInstruction* Add(HIRInstruction::Type type, ScopeSlot* slot);
  inline HIRInstruction* Add(HIRInstruction* instr);
  inline HIRInstruction* Goto(HIRBlock* target);
  inline HIRInstruction* Branch(HIRInstruction* instr,
                                HIRBlock* t,
                                HIRBlock* f);
  inline HIRInstruction* Return(HIRInstruction* instr);
  inline HIRBlock* Join(HIRBlock* b1, HIRBlock* b2);
  inline HIRInstruction* Assign(ScopeSlot* slot, HIRInstruction* value);
  inline HIRInstruction* GetNumber(uint64_t i);

  inline HIRBlock* CreateBlock(int stack_slots);
  inline HIRBlock* CreateBlock();

  inline HIRInstruction* CreateInstruction(HIRInstruction::Type type);
  inline HIRPhi* CreatePhi(ScopeSlot* slot);

  static void EnableLogging();
  static void DisableLogging();

  inline void Print(PrintBuffer* p);
  inline void Print(char* out, int32_t size);

  inline Root* root();

  inline int block_id();
  inline int instr_id();
  inline int dfs_id();

  static const int kMaxOptimizableSize = 25000;

 private:
  HIRBlock* current_block_;
  HIRBlock* current_root_;
  BreakContinueInfo* break_continue_info_;

  HIRBlockList roots_;
  HIRBlockList blocks_;
  Root* root_;
  const char* filename_;
  int loop_depth_;

  int block_id_;
  int instr_id_;
  int dfs_id_;

  static bool log_;
};

}  // namespace internal
}  // namespace candor

#endif  // _SRC_HIR_H_
