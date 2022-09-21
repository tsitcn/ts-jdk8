/*
 * Copyright (c) 1997, 2010, Oracle and/or its affiliates. All rights reserved.
 * Copyright (c) 2015, 2021, Loongson Technology. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef CPU_MIPS_VM_ASSEMBLER_MIPS_INLINE_HPP
#define CPU_MIPS_VM_ASSEMBLER_MIPS_INLINE_HPP

#include "asm/assembler.inline.hpp"
#include "asm/codeBuffer.hpp"
#include "code/codeCache.hpp"
#ifndef PRODUCT
#include "compiler/disassembler.hpp"
#endif



inline void Assembler::check_delay() {
# ifdef CHECK_DELAY
  guarantee(delay_state != at_delay_slot, "must say delayed() when filling delay slot");
  delay_state = no_delay;
# endif
}

inline void Assembler::emit_long(int x) {
  check_delay();
  AbstractAssembler::emit_int32(x);
  if (PatchContinuousLoad)
    check_load_and_patch(x);
}

inline void Assembler::emit_data(int x, relocInfo::relocType rtype) {
  relocate(rtype);
  emit_long(x);
}

inline void Assembler::emit_data(int x, RelocationHolder const& rspec) {
  relocate(rspec);
  emit_long(x);
}

// Check If an Instruction Is a Load Instruction

// All load instructions includes:
// 1. FIRST OPS
//   LDL, LDR, LB, LH, LWL, LW, LBU, LHU, LWR, LWU, LD, LWC1, PREF, LDC1
// 2. SPECIAL OPS
//   MOVF, MOVT
// 3. COP0 OPS
//   MFC0, DMFC0, MFGC0, DMFGC0
// 4. COP1 OPS
//   MFC1, DMFC1, CFC1, MFHC1, MTC1, DMTC1, CTC1, MTHC1, MOVZ.FMT, MOVN.FMT
// 5. COP1X OPS
//   LWXC1, LDXC1, LUXC1, PREFX
// 6. SPECIAL2 OPs
//   CAMPV, CAMPI, RAMRI
// 7. SPECIAL3 OPS
//   LWX, LHX, LBUX, LDX, RDHWR
// 8. LWC2 OPS
//   GSLWLC1, GSLWRC1, GSLDLC1, GSLDRC1, GSLBLE, GSLBGT,
//   GSLHLE, GSLHGT, GSLWLE, GSLWGT, GSLDLE, GSLDGT,
//   LWDIR, LWPTE, LDDIR, LDPTE
//   GSLWLEC1, GSLWGTC1, GSLDLEC1, GSLDGTC1
// 9. LDC2 OPS
//   ALL LDC2 OPS(GSLBX, GSLHX, GSLWX, GSLDX, GSLWXC1, GSLDXC1)

#define SPECIAL_MOVCI_OP_MASK 0b1111110000000100000011111111111
inline bool Assembler::is_load_op(int x) {
  assert(PatchContinuousLoad, "just checking");
  int ins = x;
  int op = (ins >> 26) & 0x3f;
  switch (op) {
    //first_ops: ldl, ldr, lb, lh, lwl, lw, lbu, lhu, lwr, lwu, ld, lwc1, pref, ldc1, ldc2_ops
    case ldl_op:
    case ldr_op:
    case lb_op:
    case lh_op:
    case lwl_op:
    case lw_op:
    case lbu_op:
    case lhu_op:
    case lwr_op:
    case lwu_op:
    case ld_op:
    case lwc1_op:
    case pref_op:
    case ldc1_op:
    //ldc2_ops: gslbx,  gslhx,  gslwx,  gsldx, gslwxc1, gsldxc1
    case gs_ldc2_op:
      return true;
    //special_ops: movf, movt
    case special_op:
      if ((ins & SPECIAL_MOVCI_OP_MASK) == movci_op)
        return true;
      else
        return false;
    //cop0_ops: mfc0, dmfc0, mfgc0, dmfgc0
    case cop0_op:
      switch ((ins >> 21) & 0x1f) {
        case mfc0_op:
        case dmfc0_op:
          return true;
        case mxgc0_op:
          if ((ins >> 9 & 1) == 0)
            return true;
        default:
          return false;
      }
    //cop1_ops: mfc1, dmfc1, cfc1, mfhc1, mtc1, dmtc1, ctc1, mthc1, movz.fmt, movn.fmt
    case cop1_op:
      switch ((ins >> 21) & 0x1f) {
        case cfc1_op:
        case mfhc1_op:
        case mtc1_op:
        case dmtc1_op:
        case ctc1_op:
        case mthc1_op:
          return true;
        case single_fmt:
        case double_fmt:
        case ps_fmt:
          if ((ins & 0x3f == movz_f_op) || (ins & 0x3f == movn_f_op))
            return true;
        default:
          return false;
      }
    //cop1x_ops: lwxc1, ldxc1, luxc1, prefx
    case cop1x_op:
      switch (ins & 0x3f) {
        case lwxc1_op:
        case ldxc1_op:
        case luxc1_op:
        case prefx_op:
          return true;
        default:
          return false;
      }
    //special2_ops: campv, campi, ramri
    case special2_op:
      switch (ins & 0xff) {
        case campv_op << 6 | gscam_op:
        case campi_op << 6 | gscam_op:
        case ramri_op << 6 | gscam_op:
          return true;
        default:
          return false;
      }
    //special3_ops: lwx, lhx, lbux, ldx, rdhwr
    case special3_op:
      switch (ins & 0x3f) {
        case lxx_op:
        case rdhwr_op:
          return true;
        default:
          return false;
      }
    //lwc2_ops: gslwlc1, gslwrc1, gsldlc1,  gsldrc1,  gslble,   gslbgt,   gslhle, gslhgt, gslwle, gslwgt,
    //           gsldle,  gsldgt, gslwlec1, gslwgtc1, gsldlec1, gsldgtc1
    case gs_lwc2_op:
      if ((ins >> 5 & 1) == 0) //gslq, gslqc1 are excluded.
        return true;
      else
        return false;
    default:
      return false;
  }
  return false;
}

#define MAX_LOADS_INSTRUCTION_SEQUENCE_LEN 3
inline void Assembler::check_load_and_patch(int x) {
  int load_count = 0;
  assert(PatchContinuousLoad, "just checking");
  if (is_load_op(x)) {
    load_count = code()->get_continuous_load_instuctions_count();
    code()->set_continuous_load_instuctions_count(++load_count);
    if (load_count >= MAX_LOADS_INSTRUCTION_SEQUENCE_LEN) {
      assert(load_count == MAX_LOADS_INSTRUCTION_SEQUENCE_LEN, "load_count should not be greater than MAX_LOADS_INSTRUCTION_SEQUENCE_LEN");
#ifndef PRODUCT
#include "compiler/disassembler.hpp"
      if (code()->get_continuous_load_instuctions_count() != MAX_LOADS_INSTRUCTION_SEQUENCE_LEN)
        tty->print_cr("get_continuous_load_instuctions_count() returns %d, which should be %d", code()->get_continuous_load_instuctions_count(), MAX_LOADS_INSTRUCTION_SEQUENCE_LEN);
      if (TracePatchContinuousLoad && PatchContinuousLoad) {
        tty->print_cr("found %d consecutive loads, separated by a nop, pc: %p", load_count, pc());
        int i;
        for (i = -MAX_LOADS_INSTRUCTION_SEQUENCE_LEN; i <= 0; i++)
          tty->print_cr("loads %d(" INTPTR_FORMAT "-" INTPTR_FORMAT "): ", i, p2i(pc()+(i*4)), p2i(pc()+(i*4)+4));
        Disassembler::decode(pc()+(i*4), pc()+(i*4)+4, tty);
        tty->print_cr("    -> nop");
        Disassembler::decode((address)&x, (address)&x+4, tty);
      }
#endif
      nop(); /*do a patch here, consecutive loads separated by a nop*/
      code()->set_continuous_load_instuctions_count(0);
    }
  } else {
    code()->set_continuous_load_instuctions_count(0);
  }
#ifndef PRODUCT
  load_count = code()->get_continuous_load_instuctions_count();
  if (load_count >= MAX_LOADS_INSTRUCTION_SEQUENCE_LEN)
    tty->print_cr("wrong load_count: %d", load_count);
  assert(load_count < MAX_LOADS_INSTRUCTION_SEQUENCE_LEN, "just checking");
#endif
}

#endif // CPU_MIPS_VM_ASSEMBLER_MIPS_INLINE_HPP
