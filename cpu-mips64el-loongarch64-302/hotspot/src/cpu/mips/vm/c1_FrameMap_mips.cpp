/*
 * Copyright (c) 1999, 2013, Oracle and/or its affiliates. All rights reserved.
 * Copyright (c) 2015, 2019, Loongson Technology. All rights reserved.
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

#include "precompiled.hpp"
#include "c1/c1_FrameMap.hpp"
#include "c1/c1_LIR.hpp"
#include "runtime/sharedRuntime.hpp"
#include "vmreg_mips.inline.hpp"

const int FrameMap::pd_c_runtime_reserved_arg_size = 0;

FloatRegister FrameMap::_fpu_regs[32];
LIR_Opr FrameMap::map_to_opr(BasicType type, VMRegPair* reg, bool) {
  LIR_Opr opr = LIR_OprFact::illegalOpr;
  VMReg r_1 = reg->first();
  VMReg r_2 = reg->second();
  if (r_1->is_stack()) {
    // Convert stack slot to an SP offset
    // The calling convention does not count the
    // SharedRuntime::out_preserve_stack_slots() value
    // so we must add it in here.
    int st_off = (r_1->reg2stack() + SharedRuntime::out_preserve_stack_slots())
      * VMRegImpl::stack_slot_size;
    opr = LIR_OprFact::address(new LIR_Address(_sp_opr, st_off, type));
  } else if (r_1->is_Register()) {
    Register reg = r_1->as_Register();
    if (r_2->is_Register() && (type == T_LONG || type == T_DOUBLE)) {
      Register reg2 = r_2->as_Register();
#ifdef _LP64
      assert(reg2 == reg, "must be same register");
#endif
      opr = as_long_opr(reg, reg2);
    } else if (type == T_OBJECT || type == T_ARRAY) {
      opr = as_oop_opr(reg);
    } else {
      opr = as_opr(reg);
    }
  } else if (r_1->is_FloatRegister()) {
    assert(type == T_DOUBLE || type == T_FLOAT, "wrong type");
    int num = r_1->as_FloatRegister()->encoding();
    if (type == T_FLOAT) {
      opr =  LIR_OprFact::single_fpu(num);
    } else {
      opr =  LIR_OprFact::double_fpu(num);
    }
  } else {
     ShouldNotReachHere();
      }
  return opr;
}

// some useful constant RInfo's:
LIR_Opr FrameMap::_zero_opr;
LIR_Opr FrameMap::_k0_opr;
LIR_Opr FrameMap::_k1_opr;
LIR_Opr FrameMap::_at_opr;
LIR_Opr FrameMap::_v0_opr;
LIR_Opr FrameMap::_v1_opr;
LIR_Opr FrameMap::_a0_opr;
LIR_Opr FrameMap::_a1_opr;
LIR_Opr FrameMap::_a2_opr;
LIR_Opr FrameMap::_a3_opr;
LIR_Opr FrameMap::_t0_opr;
LIR_Opr FrameMap::_t1_opr;
LIR_Opr FrameMap::_t2_opr;
LIR_Opr FrameMap::_t3_opr;
#ifndef _LP64
LIR_Opr FrameMap::_t4_opr;
LIR_Opr FrameMap::_t5_opr;
LIR_Opr FrameMap::_t6_opr;
LIR_Opr FrameMap::_t7_opr;
#else
LIR_Opr FrameMap::_a4_opr;
LIR_Opr FrameMap::_a5_opr;
LIR_Opr FrameMap::_a6_opr;
LIR_Opr FrameMap::_a7_opr;
#endif
LIR_Opr FrameMap::_t8_opr;
LIR_Opr FrameMap::_t9_opr;
LIR_Opr FrameMap::_s0_opr;
LIR_Opr FrameMap::_s1_opr;
LIR_Opr FrameMap::_s2_opr;
LIR_Opr FrameMap::_s3_opr;
LIR_Opr FrameMap::_s4_opr;
LIR_Opr FrameMap::_s5_opr;
LIR_Opr FrameMap::_s6_opr;
LIR_Opr FrameMap::_s7_opr;
LIR_Opr FrameMap::_gp_opr;
LIR_Opr FrameMap::_fp_opr;
LIR_Opr FrameMap::_sp_opr;
LIR_Opr FrameMap::_ra_opr;



LIR_Opr FrameMap::_a0_a1_opr;
LIR_Opr FrameMap::_a2_a3_opr;
LIR_Opr FrameMap::_v0_v1_opr;


LIR_Opr FrameMap::_f0_opr;
LIR_Opr FrameMap::_f12_opr;
LIR_Opr FrameMap::_f14_opr;
LIR_Opr FrameMap::_d0_opr;
LIR_Opr FrameMap::_d12_opr;
LIR_Opr FrameMap::_d14_opr;


LIR_Opr FrameMap::receiver_opr;

//caller saved register
LIR_Opr FrameMap::_v0_oop_opr;
LIR_Opr FrameMap::_v1_oop_opr;
LIR_Opr FrameMap::_a0_oop_opr;
LIR_Opr FrameMap::_a1_oop_opr;
LIR_Opr FrameMap::_a2_oop_opr;
LIR_Opr FrameMap::_a3_oop_opr;
LIR_Opr FrameMap::_t0_oop_opr;
LIR_Opr FrameMap::_t1_oop_opr;
LIR_Opr FrameMap::_t2_oop_opr;
LIR_Opr FrameMap::_t3_oop_opr;
#ifndef _LP64
LIR_Opr FrameMap::_t4_oop_opr;
LIR_Opr FrameMap::_t5_oop_opr;
LIR_Opr FrameMap::_t6_oop_opr;
LIR_Opr FrameMap::_t7_oop_opr;
#else
LIR_Opr FrameMap::_a4_oop_opr;
LIR_Opr FrameMap::_a5_oop_opr;
LIR_Opr FrameMap::_a6_oop_opr;
LIR_Opr FrameMap::_a7_oop_opr;
#endif
LIR_Opr FrameMap::_t8_oop_opr;
LIR_Opr FrameMap::_t9_oop_opr;
LIR_Opr FrameMap::_s0_oop_opr;
LIR_Opr FrameMap::_s1_oop_opr;
LIR_Opr FrameMap::_s2_oop_opr;
LIR_Opr FrameMap::_s3_oop_opr;
LIR_Opr FrameMap::_s4_oop_opr;
LIR_Opr FrameMap::_s5_oop_opr;
LIR_Opr FrameMap::_s6_oop_opr;
LIR_Opr FrameMap::_s7_oop_opr;

//add metadata_opr
LIR_Opr FrameMap::_v0_metadata_opr;
LIR_Opr FrameMap::_v1_metadata_opr;
LIR_Opr FrameMap::_a0_metadata_opr;
LIR_Opr FrameMap::_a1_metadata_opr;
LIR_Opr FrameMap::_a2_metadata_opr;
LIR_Opr FrameMap::_a3_metadata_opr;
LIR_Opr FrameMap::_t0_metadata_opr;
LIR_Opr FrameMap::_t1_metadata_opr;
LIR_Opr FrameMap::_t2_metadata_opr;
LIR_Opr FrameMap::_t3_metadata_opr;
LIR_Opr FrameMap::_a4_metadata_opr;
LIR_Opr FrameMap::_a5_metadata_opr;
LIR_Opr FrameMap::_a6_metadata_opr;
LIR_Opr FrameMap::_a7_metadata_opr;
LIR_Opr FrameMap::_t8_metadata_opr;
LIR_Opr FrameMap::_t9_metadata_opr;
LIR_Opr FrameMap::_s0_metadata_opr;
LIR_Opr FrameMap::_s1_metadata_opr;
LIR_Opr FrameMap::_s2_metadata_opr;
LIR_Opr FrameMap::_s3_metadata_opr;
LIR_Opr FrameMap::_s4_metadata_opr;
LIR_Opr FrameMap::_s5_metadata_opr;
LIR_Opr FrameMap::_s6_metadata_opr;
LIR_Opr FrameMap::_s7_metadata_opr;

LIR_Opr FrameMap::_a0_a1_long_opr;
LIR_Opr FrameMap::_a2_a3_long_opr;
LIR_Opr FrameMap::_v0_v1_long_opr;
LIR_Opr FrameMap::_f0_float_opr;
LIR_Opr FrameMap::_f12_float_opr;
LIR_Opr FrameMap::_f14_float_opr;
LIR_Opr FrameMap::_d0_double_opr;
LIR_Opr FrameMap::_d12_double_opr;
LIR_Opr FrameMap::_d14_double_opr;




LIR_Opr FrameMap::_caller_save_cpu_regs[] = { 0, };
LIR_Opr FrameMap::_caller_save_fpu_regs[] = { 0, };


//--------------------------------------------------------
//               FrameMap
//--------------------------------------------------------
FloatRegister FrameMap::nr2floatreg (int rnr) {
  assert(_init_done, "tables not initialized");
  debug_only(fpu_range_check(rnr);)
  return _fpu_regs[rnr];
}

// returns true if reg could be smashed by a callee.
bool FrameMap::is_caller_save_register (LIR_Opr reg) {
  if (reg->is_single_fpu() || reg->is_double_fpu()) { return true; }
  if (reg->is_double_cpu()) {
    return is_caller_save_register(reg->as_register_lo()) ||
      is_caller_save_register(reg->as_register_hi());
  }
  return is_caller_save_register(reg->as_register());
}

bool FrameMap::is_caller_save_register (Register r) {
  return true;
}

void FrameMap::initialize() {
  if (_init_done) return;

  assert(nof_cpu_regs == 32, "wrong number of CPU registers");
  int i = 0;

  map_register(0,R0);  _zero_opr=LIR_OprFact::single_cpu(0);
  map_register(1,AT);
#ifdef _LP64
  _at_opr=LIR_OprFact::double_cpu(1, 1);
#else
  _at_opr=LIR_OprFact::single_cpu(1);
#endif
  map_register(2,V0);  _v0_opr=LIR_OprFact::single_cpu(2);   _v0_oop_opr=LIR_OprFact::single_cpu_oop(2);   _v0_metadata_opr=LIR_OprFact::single_cpu_metadata(2);
  map_register(3,V1);  _v1_opr=LIR_OprFact::single_cpu(3);   _v1_oop_opr=LIR_OprFact::single_cpu_oop(3);   _v1_metadata_opr=LIR_OprFact::single_cpu_metadata(3);
  map_register(4,A0);  _a0_opr=LIR_OprFact::single_cpu(4);   _a0_oop_opr=LIR_OprFact::single_cpu_oop(4);   _a0_metadata_opr=LIR_OprFact::single_cpu_metadata(4);
  map_register(5,A1);  _a1_opr=LIR_OprFact::single_cpu(5);   _a1_oop_opr=LIR_OprFact::single_cpu_oop(5);   _a1_metadata_opr=LIR_OprFact::single_cpu_metadata(5);
  map_register(6,A2);  _a2_opr=LIR_OprFact::single_cpu(6);   _a2_oop_opr=LIR_OprFact::single_cpu_oop(6);   _a2_metadata_opr=LIR_OprFact::single_cpu_metadata(6);
  map_register(7,A3);  _a3_opr=LIR_OprFact::single_cpu(7);   _a3_oop_opr=LIR_OprFact::single_cpu_oop(7);   _a3_metadata_opr=LIR_OprFact::single_cpu_metadata(7);
#ifndef _LP64
  map_register(8,T0);  _t0_opr=LIR_OprFact::single_cpu(8);    _t0_oop_opr=LIR_OprFact::single_cpu_oop(8);
  map_register(9,T1);  _t1_opr=LIR_OprFact::single_cpu(9);    _t1_oop_opr=LIR_OprFact::single_cpu_oop(9);
  map_register(10,T2);  _t2_opr=LIR_OprFact::single_cpu(10);  _t2_oop_opr=LIR_OprFact::single_cpu_oop(10);
  map_register(11,T3);  _t3_opr=LIR_OprFact::single_cpu(11);  _t3_oop_opr=LIR_OprFact::single_cpu_oop(11);
  map_register(12,T4);  _t4_opr=LIR_OprFact::single_cpu(12);  _t4_oop_opr=LIR_OprFact::single_cpu_oop(12);
  map_register(13,T5);  _t5_opr=LIR_OprFact::single_cpu(13);  _t5_oop_opr=LIR_OprFact::single_cpu_oop(13);
  map_register(14,T6);  _t6_opr=LIR_OprFact::single_cpu(14);  _t6_oop_opr=LIR_OprFact::single_cpu_oop(14);
  map_register(15,T7);  _t7_opr=LIR_OprFact::single_cpu(15);  _t7_oop_opr=LIR_OprFact::single_cpu_oop(15);
#else
  map_register(8,A4);  _a4_opr=LIR_OprFact::single_cpu(8);    _a4_oop_opr=LIR_OprFact::single_cpu_oop(8);   _a4_metadata_opr=LIR_OprFact::single_cpu_metadata(8);
  map_register(9,A5);  _a5_opr=LIR_OprFact::single_cpu(9);    _a5_oop_opr=LIR_OprFact::single_cpu_oop(9);   _a5_metadata_opr=LIR_OprFact::single_cpu_metadata(9);
  map_register(10,A6);  _a6_opr=LIR_OprFact::single_cpu(10);  _a6_oop_opr=LIR_OprFact::single_cpu_oop(10);  _a6_metadata_opr=LIR_OprFact::single_cpu_metadata(10);
  map_register(11,A7);  _a7_opr=LIR_OprFact::single_cpu(11);  _a7_oop_opr=LIR_OprFact::single_cpu_oop(11);  _a7_metadata_opr=LIR_OprFact::single_cpu_metadata(11);
  map_register(12,T0);  _t0_opr=LIR_OprFact::single_cpu(12);  _t0_oop_opr=LIR_OprFact::single_cpu_oop(12);  _t0_metadata_opr=LIR_OprFact::single_cpu_metadata(12);
  map_register(13,T1);  _t1_opr=LIR_OprFact::single_cpu(13);  _t1_oop_opr=LIR_OprFact::single_cpu_oop(13);  _t1_metadata_opr=LIR_OprFact::single_cpu_metadata(13);
  map_register(14,T2);  _t2_opr=LIR_OprFact::single_cpu(14);  _t2_oop_opr=LIR_OprFact::single_cpu_oop(14);  _t2_metadata_opr=LIR_OprFact::single_cpu_metadata(14);
  map_register(15,T3);  _t3_opr=LIR_OprFact::single_cpu(15);  _t3_oop_opr=LIR_OprFact::single_cpu_oop(15);  _t3_metadata_opr=LIR_OprFact::single_cpu_metadata(15);
#endif
  map_register(16,S0);  _s0_opr=LIR_OprFact::single_cpu(16);  _s0_oop_opr=LIR_OprFact::single_cpu_oop(16);  _s0_metadata_opr=LIR_OprFact::single_cpu_metadata(16);
  map_register(17,S1);  _s1_opr=LIR_OprFact::single_cpu(17);  _s1_oop_opr=LIR_OprFact::single_cpu_oop(17);  _s1_metadata_opr=LIR_OprFact::single_cpu_metadata(17);
  map_register(18,S2);  _s2_opr=LIR_OprFact::single_cpu(18);  _s2_oop_opr=LIR_OprFact::single_cpu_oop(18);  _s2_metadata_opr=LIR_OprFact::single_cpu_metadata(18);
  map_register(19,S3);  _s3_opr=LIR_OprFact::single_cpu(19);  _s3_oop_opr=LIR_OprFact::single_cpu_oop(19);  _s3_metadata_opr=LIR_OprFact::single_cpu_metadata(19);
  map_register(20,S4);  _s4_opr=LIR_OprFact::single_cpu(20);  _s4_oop_opr=LIR_OprFact::single_cpu_oop(20);  _s4_metadata_opr=LIR_OprFact::single_cpu_metadata(20);
  map_register(21,S5);  _s5_opr=LIR_OprFact::single_cpu(21);  _s5_oop_opr=LIR_OprFact::single_cpu_oop(21);  _s5_metadata_opr=LIR_OprFact::single_cpu_metadata(21);
  map_register(22,S6);  _s6_opr=LIR_OprFact::single_cpu(22);  _s6_oop_opr=LIR_OprFact::single_cpu_oop(22);  _s6_metadata_opr=LIR_OprFact::single_cpu_metadata(22);
  map_register(23,S7);  _s7_opr=LIR_OprFact::single_cpu(23);  _s7_oop_opr=LIR_OprFact::single_cpu_oop(23);  _s7_metadata_opr=LIR_OprFact::single_cpu_metadata(23);
  map_register(24,T8);  _t8_opr=LIR_OprFact::single_cpu(24);
  map_register(25,T9);
#ifdef _LP64
  _t9_opr=LIR_OprFact::double_cpu(25, 25);
#else
  _t9_opr=LIR_OprFact::single_cpu(25);
#endif
  map_register(26,K0);  _k0_opr=LIR_OprFact::single_cpu(26);
  map_register(27,K1);  _k1_opr=LIR_OprFact::single_cpu(27);
  map_register(28,GP);  _gp_opr=LIR_OprFact::single_cpu(28);
  map_register(29,SP);
#ifdef _LP64
  _sp_opr=LIR_OprFact::double_cpu(29, 29);
#else
  _sp_opr=LIR_OprFact::single_cpu(29);
#endif

  map_register(30,FP);  _fp_opr=LIR_OprFact::single_cpu(30);
  map_register(31,RA);  _ra_opr=LIR_OprFact::single_cpu(31);

  _caller_save_cpu_regs[0] =  _t0_opr;
  _caller_save_cpu_regs[1] =  _t1_opr;
  _caller_save_cpu_regs[2] =  _t2_opr;
  _caller_save_cpu_regs[3] =  _t3_opr;
#ifndef _LP64
  _caller_save_cpu_regs[4] =  _t4_opr;
  _caller_save_cpu_regs[5] =  _t5_opr;
  _caller_save_cpu_regs[6] =  _t6_opr;
  _caller_save_cpu_regs[7] =  _t7_opr;
#else
  _caller_save_cpu_regs[4] =  _a4_opr;
  _caller_save_cpu_regs[5] =  _a5_opr;
  _caller_save_cpu_regs[6] =  _a6_opr;
  _caller_save_cpu_regs[7] =  _a7_opr;
#endif
  _caller_save_cpu_regs[8] =  _s0_opr;
  _caller_save_cpu_regs[9] =  _s1_opr;
  _caller_save_cpu_regs[10] =  _s2_opr;
  _caller_save_cpu_regs[11] =  _s3_opr;
  _caller_save_cpu_regs[12] =  _s4_opr;
  _caller_save_cpu_regs[13] =  _s5_opr;
  _caller_save_cpu_regs[14] =  _s6_opr;
  _caller_save_cpu_regs[15] =  _s7_opr;
  _caller_save_cpu_regs[16] =  _v0_opr;
  _caller_save_cpu_regs[17] =  _v1_opr;


  _caller_save_fpu_regs[0] = LIR_OprFact::single_fpu(0);
  _caller_save_fpu_regs[1] = LIR_OprFact::single_fpu(1);
  _caller_save_fpu_regs[2] = LIR_OprFact::single_fpu(2);
  _caller_save_fpu_regs[3] = LIR_OprFact::single_fpu(3);
  _caller_save_fpu_regs[4] = LIR_OprFact::single_fpu(4);
  _caller_save_fpu_regs[5] = LIR_OprFact::single_fpu(5);
  _caller_save_fpu_regs[6] = LIR_OprFact::single_fpu(6);
  _caller_save_fpu_regs[7] = LIR_OprFact::single_fpu(7);
  _caller_save_fpu_regs[8] = LIR_OprFact::single_fpu(8);
  _caller_save_fpu_regs[9] = LIR_OprFact::single_fpu(9);
  _caller_save_fpu_regs[10] = LIR_OprFact::single_fpu(10);
  _caller_save_fpu_regs[11] = LIR_OprFact::single_fpu(11);
  _caller_save_fpu_regs[12] = LIR_OprFact::single_fpu(12);
  _caller_save_fpu_regs[13] = LIR_OprFact::single_fpu(13);
  _caller_save_fpu_regs[14] = LIR_OprFact::single_fpu(14);
  _caller_save_fpu_regs[15] = LIR_OprFact::single_fpu(15);
#ifdef _LP64
  _caller_save_fpu_regs[16] = LIR_OprFact::single_fpu(16);
  _caller_save_fpu_regs[17] = LIR_OprFact::single_fpu(17);
  _caller_save_fpu_regs[18] = LIR_OprFact::single_fpu(18);
  _caller_save_fpu_regs[19] = LIR_OprFact::single_fpu(19);
  _caller_save_fpu_regs[20] = LIR_OprFact::single_fpu(20);
  _caller_save_fpu_regs[21] = LIR_OprFact::single_fpu(21);
  _caller_save_fpu_regs[22] = LIR_OprFact::single_fpu(22);
  _caller_save_fpu_regs[23] = LIR_OprFact::single_fpu(23);
  _caller_save_fpu_regs[24] = LIR_OprFact::single_fpu(24);
  _caller_save_fpu_regs[25] = LIR_OprFact::single_fpu(25);
  _caller_save_fpu_regs[26] = LIR_OprFact::single_fpu(26);
  _caller_save_fpu_regs[27] = LIR_OprFact::single_fpu(27);
  _caller_save_fpu_regs[28] = LIR_OprFact::single_fpu(28);
  _caller_save_fpu_regs[29] = LIR_OprFact::single_fpu(29);
  _caller_save_fpu_regs[30] = LIR_OprFact::single_fpu(30);
  _caller_save_fpu_regs[31] = LIR_OprFact::single_fpu(31);
#endif

  for (int i = 0; i < 32; i++) {
    _fpu_regs[i] = as_FloatRegister(i);
  }

#ifndef _LP64
  _a0_a1_long_opr=LIR_OprFact::double_cpu(4/*a0*/,5/*a1*/);
  _a2_a3_long_opr=LIR_OprFact::double_cpu(6/*a2*/,7/*a3*/);
  _v0_v1_long_opr=LIR_OprFact::double_cpu(2/*v0*/,3/*v1*/);
#else
  _a0_a1_long_opr=LIR_OprFact::double_cpu(4/*a0*/,4/*a0*/);
  _a2_a3_long_opr=LIR_OprFact::double_cpu(6/*a2*/,6/*a2*/);
  _v0_v1_long_opr=LIR_OprFact::double_cpu(2/*v0*/,2/*v0*/);
#endif
  _f0_float_opr  =LIR_OprFact::single_fpu(0/*f0*/);
  _f12_float_opr =LIR_OprFact::single_fpu(12/*f12*/);
  _f14_float_opr =LIR_OprFact::single_fpu(14/*f14*/);
  _d0_double_opr =LIR_OprFact::double_fpu(0/*f0*/);
  _d12_double_opr=LIR_OprFact::double_fpu(12/*f12*/);
  _d14_double_opr=LIR_OprFact::double_fpu(14/*f14*/);


  _init_done = true;

  VMRegPair regs;
  BasicType sig_bt = T_OBJECT;
  SharedRuntime::java_calling_convention(&sig_bt, &regs, 1, true);

  receiver_opr = as_oop_opr(regs.first()->as_Register());
  assert(receiver_opr == _t0_oop_opr, "rcvr ought to be t0");

}


Address FrameMap::make_new_address(ByteSize sp_offset) const {
  return Address(SP, in_bytes(sp_offset));
}


// ----------------mapping-----------------------
// all mapping is based on fp, addressing, except for simple leaf methods where we access
// the locals sp based (and no frame is built)


// Frame for simple leaf methods (quick entries)
//
//   +----------+
//   | ret addr |   <- TOS
//   +----------+
//   | args     |
//   | ......   |

// Frame for standard methods
//
//   | .........|  <- TOS
//   | locals   |
//   +----------+
//   | old fp,  |  <- FP
//   +----------+
//   | ret addr |
//   +----------+
//   |  args    |
//   | .........|


// For OopMaps, map a local variable or spill index to an VMRegImpl name.
// This is the offset from sp() in the frame of the slot for the index,
// skewed by VMRegImpl::stack0 to indicate a stack location (vs.a register.)
//
//           framesize +
//           stack0         stack0          0  <- VMReg
//             |              | <registers> |
//  ...........|..............|.............|
//      0 1 2 3 x x 4 5 6 ... |                <- local indices
//      ^           ^        sp()                 ( x x indicate link
//      |           |                               and return addr)
//  arguments   non-argument locals

VMReg FrameMap::fpu_regname (int n) {
  // Return the OptoReg name for the fpu stack slot "n"
  // A spilled fpu stack slot comprises to two single-word OptoReg's.
  return as_FloatRegister(n)->as_VMReg();
}

LIR_Opr FrameMap::stack_pointer() {
  return FrameMap::_sp_opr;
}

// JSR 292
LIR_Opr FrameMap::method_handle_invoke_SP_save_opr() {
  assert(SP == mh_SP_save, "must be same register");
  return _sp_opr;
}

bool FrameMap::validate_frame() {
  return true;
}
