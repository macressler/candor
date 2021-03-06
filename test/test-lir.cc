#include "test.h"
#include <parser.h>
#include <ast.h>
#include <hir.h>
#include <hir-inl.h>
#include <lir.h>
#include <lir-inl.h>

TEST_START(lir)
#ifdef CANDOR_ARCH_x64
  // Simpliest
  LIR_TEST("return 1\n",
           "# Block 0\n"
           "0: Label\n"
           "2: Entry\n"
           "4: c10 = Literal[1]\n"
           "5: Gap[c10 => @rax:11]\n"
           "6: @rax:0 = Move rax:11\n"
           "8: Return @rax:0\n\n")

  LIR_TEST("return 1 + 2\n",
           "# Block 0\n"
           "0: Label\n"
           "2: Entry\n"
           "4: c10 = Literal[1]\n"
           "6: c11 = Literal[2]\n"
           "7: Gap[c10 => @rax:14]\n"
           "8: @rax:0(rax) = Move rax:14\n"
           "9: Gap[c11 => @rbx:16]\n"
           "10: @rbx:1 = Move rbx:16\n"
           "12: @rax:0(rax) = BinOpNumber @rax:0(rax), @rbx:1 # scratches: @rcx:12\n"
           "14: rax:13(rax) = Move @rax:0(rax)\n"
           "16: @rax:0(rax) = Move rax:13(rax)\n"
           "18: Return @rax:0(rax)\n\n")

  // Ifs
  LIR_TEST("if (true) { a = 1 } else { a = 2}\nreturn a\n",
           "# Block 0\n"
           "0: Label\n"
           "2: Entry\n"
           "4: c10 = Literal[true]\n"
           "5: Gap[c10 => @rax:14]\n"
           "6: @rax:0 = Move rax:14\n"
           "8: Branch @rax:0 (10), (18)\n"
           "\n"
           "# Block 1\n"
           "# in: , out: 12\n"
           "10: Label\n"
           "12: c11 = Literal[1]\n"
           "13: Gap[c11 => @rax:16]\n"
           "14: rax:12(rax) = Move rax:16\n"
           "16: Goto (26)\n"
           "\n"
           "# Block 2\n"
           "# in: , out: 12\n"
           "18: Label\n"
           "20: c13 = Literal[2]\n"
           "21: Gap[c13 => @rax:18]\n"
           "22: rax:12(rax) = Move rax:18\n"
           "\n"
           "# Block 3\n"
           "# in: 12, out: \n"
           "26: Label\n"
           "28: rax:12(rax) = Phi rax:12(rax)\n"
           "30: @rax:0 = Move rax:12(rax)\n"
           "32: Return @rax:0\n\n")

  LIR_TEST("pass_through = 1\n"
           "while (i < 10) { i++ }\n"
           "return i + pass_through",
           "# Block 0\n"
           "# in: 15, out: 13, 12, 10, 15\n"
           "0: Label\n"
           "2: Entry\n"
           "4: c10 = Literal[1]\n"
           "6: c11 = Nil\n"
           "8: c12 = Literal[10]\n"
           "9: Gap[c11 => @rbx:18]\n"
           "10: rcx:13(rax) = Move rbx:18\n"
           "\n"
           "# Block 1\n"
           "# in: 13, 12, 10, 15, out: 13, 12, 10, 15\n"
           "14: Label\n"
           "16: rcx:13(rax) = Phi rcx:13(rax)\n"
           "\n"
           "# Block 2\n"
           "# in: 13, 12, 10, 15, out: 13, 10, 15, 12\n"
           "20: Label\n"
           "21: Gap[rax:15(rax) => rdx:22]\n"
           "22: @rax:0(rax) = Move rcx:13(rax)\n"
           "23: Gap[c12 => @rbx:20]\n"
           "24: @rbx:1 = Move rbx:20\n"
           "25: Gap[rcx:13(rax) => [1]:24, rdx:22 => [2]:26]\n"
           "26: @rax:0(rax) = BinOp @rax:0(rax), @rbx:1\n"
           "28: rax:14(rax) = Move @rax:0(rax)\n"
           "30: @rax:0(rax) = Move rax:14(rax)\n"
           "32: Branch @rax:0(rax) (34), (52)\n"
           "\n"
           "# Block 3\n"
           "# in: 13, 10, 12, out: 15, 12, 10\n"
           "34: Label\n"
           "36: @rax:0(rax) = Move [1]:24\n"
           "37: Gap[c10 => @rbx:16]\n"
           "38: @rbx:1 = Move rbx:16\n"
           "40: @rax:0(rax) = BinOp @rax:0(rax), @rbx:1\n"
           "42: [2]:26 = Move @rax:0(rax)\n"
           "\n"
           "# Block 4\n"
           "# in: 15, 12, 10, out: 13, 12, 10, 15\n"
           "46: Label\n"
           "48: [1]:24 = Move [2]:26\n"
           "49: Gap[[1]:24 => rcx:13(rax), [2]:26 => rax:15(rax)]\n"
           "50: Goto (14)\n"
           "\n"
           "# Block 5\n"
           "# in: 15, out: 15\n"
           "52: Label\n"
           "\n"
           "# Block 6\n"
           "# in: 15, out: \n"
           "56: Label\n"
           "58: @rax:0(rax) = Move [2]:26\n"
           "60: Return @rax:0(rax)\n\n")
#endif // CANDOR_ARCH_x64
TEST_END(lir)
