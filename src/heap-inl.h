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

#ifndef _SRC_HEAP_INL_H_
#define _SRC_HEAP_INL_H_

#include <stdint.h>  // int64_t, intptr_t

namespace candor {
namespace internal {

inline Heap::HeapTag HValue::GetTag(char* addr) {
  if (addr == HNil::New()) return Heap::kTagNil;

  if (IsUnboxed(addr)) {
    return Heap::kTagNumber;
  }
  return static_cast<Heap::HeapTag>(*reinterpret_cast<uint8_t*>(
        addr + kTagOffset));
}


inline bool HValue::IsUnboxed(char* addr) {
  return (reinterpret_cast<intptr_t>(addr) & 0x01) == 0;
}


inline bool HValue::IsGCMarked() {
  if (IsUnboxed(addr())) return false;
  return (*reinterpret_cast<uint8_t*>(addr() + kGCMarkOffset) & 0x80) != 0;
}


inline char* HValue::GetGCMark() {
  assert(IsGCMarked());
  return *reinterpret_cast<char**>(addr() + kGCForwardOffset);
}


inline void HValue::SetGCMark(char* new_addr) {
  assert(!IsGCMarked());

  *reinterpret_cast<uint8_t*>(addr() + kGCMarkOffset) |= 0x80;
  *reinterpret_cast<char**>(addr() + kGCForwardOffset) = new_addr;
}


inline bool HValue::IsSoftGCMarked() {
  if (IsUnboxed(addr())) return false;
  return (*reinterpret_cast<uint8_t*>(addr() + kGCMarkOffset) & 0x40) != 0;
}


inline void HValue::SetSoftGCMark() {
  *reinterpret_cast<uint8_t*>(addr() + kGCMarkOffset) |= 0x40;
}


inline void HValue::ResetSoftGCMark() {
  if (IsSoftGCMarked()) {
    *reinterpret_cast<uint8_t*>(addr() + kGCMarkOffset) ^= 0x40;
  }
}


inline void HValue::IncrementGeneration() {
  // tag, generation, reserved, GC mark
  if (Generation() < Heap::kMinOldSpaceGeneration) {
    uint8_t* slot = reinterpret_cast<uint8_t*>(addr() + kGenerationOffset);
    *slot = *slot + 1;
  }
}


inline uint8_t HValue::Generation() {
  return *reinterpret_cast<uint8_t*>(addr() + kGenerationOffset);
}


inline bool HContext::HasSlot(uint32_t index) {
  return *GetSlotAddress(index) != HNil::New();
}


inline HValue* HContext::GetSlot(uint32_t index) {
  return HValue::Cast(*GetSlotAddress(index));
}


inline char** HContext::GetSlotAddress(uint32_t index) {
  return reinterpret_cast<char**>(addr() + GetIndexDisp(index));
}


inline uint32_t HContext::GetIndexDisp(uint32_t index) {
  return interior_offset(3 + index);
}


inline int64_t HNumber::Untag(int64_t value) {
  return value >> 1;
}


inline int64_t HNumber::Tag(int64_t value) {
  return value << 1;
}


inline char* HNumber::ToPointer(int64_t value) {
  intptr_t oval = static_cast<intptr_t>(HNumber::Tag(value));
  return reinterpret_cast<char*>(oval);
}


inline int64_t HNumber::IntegralValue(char* addr) {
  if (IsUnboxed(addr)) {
    return Untag(reinterpret_cast<intptr_t>(addr));
  } else {
    return *reinterpret_cast<double*>(addr + kValueOffset);
  }
}


inline double HNumber::DoubleValue(char* addr) {
  if (IsUnboxed(addr)) {
    return Untag(reinterpret_cast<intptr_t>(addr));
  } else {
    return *reinterpret_cast<double*>(addr + kValueOffset);
  }
}


inline bool HNumber::IsIntegral(char* addr) {
  return IsUnboxed(addr);
}


inline void HArray::SetLength(char* obj, int64_t length) {
  *reinterpret_cast<intptr_t*>(obj + kLengthOffset) = length;
}


inline bool HArray::IsDense(char* obj) {
  int size = HValue::As<HMap>(Map(obj))->size();
  return size <= kDenseLengthMax;
}


inline bool HMap::IsEmptySlot(uint32_t index) {
  return *GetSlotAddress(index) == HNil::New();
}


inline HValue* HMap::GetSlot(uint32_t index) {
  return HValue::Cast(*GetSlotAddress(index));
}


inline char** HMap::GetSlotAddress(uint32_t index) {
  return reinterpret_cast<char**>(space() + index * kPointerSize);
}


inline char* HFunction::GetContext(char* addr) {
  HContext* hroot = HValue::As<HContext>(HFunction::Root(addr));

  char** root_slot = hroot->GetSlotAddress(Heap::kRootGlobalIndex);
  return *root_slot;
}


inline void HFunction::SetContext(char* addr, char* context) {
  HContext* hroot = HValue::As<HContext>(HFunction::Root(addr));

  char** root_slot = hroot->GetSlotAddress(Heap::kRootGlobalIndex);
  *root_slot = context;
}

}  // namespace internal
}  // namespace candor

#endif  // _SRC_HEAP_INL_H_
