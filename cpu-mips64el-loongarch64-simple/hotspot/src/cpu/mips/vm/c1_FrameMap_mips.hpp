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

#ifndef CPU_MIPS_VM_C1_FRAMEMAP_MIPS_HPP
#define CPU_MIPS_VM_C1_FRAMEMAP_MIPS_HPP

//  On mips the frame looks as follows:
//
//  +----------------+---------+----------------------------+----------------+-----------
//  | size_arguments | 2 words | size_locals-size_arguments | _size_monitors | spilling .
//  +----------------+---------+----------------------------+----------------+-----------
//
 private:

  //static FloatRegister  _fpu_regs [nof_fpu_regs];
  static FloatRegister  _fpu_regs [32];

  WordSize fp_offset_for_slot          (int slot) const;
  int      local_to_slot               (int local_name, bool is_two_word) const;
  // NOTE : name consist of argument, local, spill, they are not continuous
  WordSize fp_offset_for_name          (int name, bool is_two_word, bool for_hi_word) const;
  WordSize fp_offset_for_monitor_lock  (int monitor_index) const;
  WordSize fp_offset_for_monitor_object(int monitor_index) const;
  bool     location_for_fp_offset      (WordSize word_offset_from_fp,
                                        Location::Type loc_type,
                                        Location* loc) const;
  WordSize fp2sp_offset                (WordSize fp_offset) const;


 public:
  static const int pd_c_runtime_reserved_arg_size;
  enum {
    nof_reg_args       = 5,   // registers t0,a0-a3 are available for parameter passing
    first_available_sp_in_frame   = 0,
    //frame_pad_in_bytes       = 8
    frame_pad_in_bytes       = 2 * sizeof(intptr_t)
  };

  static LIR_Opr _zero_opr;
  static LIR_Opr _at_opr;
  static LIR_Opr _v0_opr;
  static LIR_Opr _v1_opr;
  static LIR_Opr _a0_opr;
  static LIR_Opr _a1_opr;
  static LIR_Opr _a2_opr;
  static LIR_Opr _a3_opr;
  static LIR_Opr _t0_opr;
  static LIR_Opr _t1_opr;
  static LIR_Opr _t2_opr;
  static LIR_Opr _t3_opr;
#ifndef _LP64
  static LIR_Opr _t4_opr;
  static LIR_Opr _t5_opr;
  static LIR_Opr _t6_opr;
  static LIR_Opr _t7_opr;
#else
  static LIR_Opr _a4_opr;
  static LIR_Opr _a5_opr;
  static LIR_Opr _a6_opr;
  static LIR_Opr _a7_opr;
#endif
  static LIR_Opr _t8_opr;
  static LIR_Opr _t9_opr;
  static LIR_Opr _s0_opr;
  static LIR_Opr _s1_opr;
  static LIR_Opr _s2_opr;
  static LIR_Opr _s3_opr;
  static LIR_Opr _s4_opr;
  static LIR_Opr _s5_opr;
  static LIR_Opr _s6_opr;
  static LIR_Opr _s7_opr;
  static LIR_Opr _gp_opr;
  static LIR_Opr _fp_opr;
  static LIR_Opr _sp_opr;
  static LIR_Opr _ra_opr;
  static LIR_Opr _k0_opr;
  static LIR_Opr _k1_opr;

  static LIR_Opr _f0_opr;
  static LIR_Opr _f12_opr;
  static LIR_Opr _f14_opr;
  static LIR_Opr _d0_opr;
  static LIR_Opr _d12_opr;
  static LIR_Opr _d14_opr;

  static LIR_Opr _a0_a1_opr;
  static LIR_Opr _a2_a3_opr;
  static LIR_Opr _v0_v1_opr;


  static LIR_Opr receiver_opr;
  static LIR_Opr _zero_oop_opr;
  static LIR_Opr _at_oop_opr;
  static LIR_Opr _v0_oop_opr;
  static LIR_Opr _v1_oop_opr;
  static LIR_Opr _a0_oop_opr;
  static LIR_Opr _a1_oop_opr;
  static LIR_Opr _a2_oop_opr;
  static LIR_Opr _a3_oop_opr;
  static LIR_Opr _t0_oop_opr;
  static LIR_Opr _t1_oop_opr;
  static LIR_Opr _t2_oop_opr;
  static LIR_Opr _t3_oop_opr;
#ifndef _LP64
  static LIR_Opr _t4_oop_opr;
  static LIR_Opr _t5_oop_opr;
  static LIR_Opr _t6_oop_opr;
  static LIR_Opr _t7_oop_opr;
#else
  static LIR_Opr _a4_oop_opr;
  static LIR_Opr _a5_oop_opr;
  static LIR_Opr _a6_oop_opr;
  static LIR_Opr _a7_oop_opr;
#endif
  static LIR_Opr _t8_oop_opr;
  static LIR_Opr _t9_oop_opr;
  static LIR_Opr _s0_oop_opr;
  static LIR_Opr _s1_oop_opr;
  static LIR_Opr _s2_oop_opr;
  static LIR_Opr _s3_oop_opr;
  static LIR_Opr _s4_oop_opr;
  static LIR_Opr _s5_oop_opr;
  static LIR_Opr _s6_oop_opr;
  static LIR_Opr _s7_oop_opr;
  static LIR_Opr _gp_oop_opr;
  static LIR_Opr _fp_oop_opr;
  static LIR_Opr _sp_oop_opr;
  static LIR_Opr _ra_oop_opr;
  static LIR_Opr _k0_oop_opr;
  static LIR_Opr _k1_oop_opr;

  static LIR_Opr _f0_oop_opr;
  static LIR_Opr _f12_oop_opr;
  static LIR_Opr _f14_oop_opr;
  static LIR_Opr _d0_oop_opr;
  static LIR_Opr _d12_oop_opr;
  static LIR_Opr _d14_oop_opr;

  static LIR_Opr _a0_a1_oop_opr;
  static LIR_Opr _a2_a3_oop_opr;
  static LIR_Opr _v0_v1_oop_opr;

//FIXME, needed under 64-bit? by aoqi
  static LIR_Opr _a0_a1_long_opr;
  static LIR_Opr _a2_a3_long_opr;
  static LIR_Opr _v0_v1_long_opr;
  static LIR_Opr _f0_float_opr;
  static LIR_Opr _f12_float_opr;
  static LIR_Opr _f14_float_opr;
  static LIR_Opr _d0_double_opr;
  static LIR_Opr _d12_double_opr;
  static LIR_Opr _d14_double_opr;


  static LIR_Opr _zero_metadata_opr;
  static LIR_Opr _at_metadata_opr;
  static LIR_Opr _v0_metadata_opr;
  static LIR_Opr _v1_metadata_opr;
  static LIR_Opr _a0_metadata_opr;
  static LIR_Opr _a1_metadata_opr;
  static LIR_Opr _a2_metadata_opr;
  static LIR_Opr _a3_metadata_opr;
  static LIR_Opr _t0_metadata_opr;
  static LIR_Opr _t1_metadata_opr;
  static LIR_Opr _t2_metadata_opr;
  static LIR_Opr _t3_metadata_opr;
  static LIR_Opr _a4_metadata_opr;
  static LIR_Opr _a5_metadata_opr;
  static LIR_Opr _a6_metadata_opr;
  static LIR_Opr _a7_metadata_opr;
  static LIR_Opr _t8_metadata_opr;
  static LIR_Opr _t9_metadata_opr;
  static LIR_Opr _s0_metadata_opr;
  static LIR_Opr _s1_metadata_opr;
  static LIR_Opr _s2_metadata_opr;
  static LIR_Opr _s3_metadata_opr;
  static LIR_Opr _s4_metadata_opr;
  static LIR_Opr _s5_metadata_opr;
  static LIR_Opr _s6_metadata_opr;
  static LIR_Opr _s7_metadata_opr;
  static LIR_Opr _gp_metadata_opr;
  static LIR_Opr _fp_metadata_opr;
  static LIR_Opr _sp_metadata_opr;
  static LIR_Opr _ra_metadata_opr;
  static LIR_Opr _k0_metadata_opr;
  static LIR_Opr _k1_metadata_opr;

//no implementation
  static LIR_Opr _f0_metadata_opr;
  static LIR_Opr _f12_metadata_opr;
  static LIR_Opr _f14_metadata_opr;
  static LIR_Opr _d0_metadata_opr;
  static LIR_Opr _d12_metadata_opr;
  static LIR_Opr _d14_metadata_opr;

  static LIR_Opr _a0_a1_metadata_opr;
  static LIR_Opr _a2_a3_metadata_opr;
  static LIR_Opr _v0_v1_metadata_opr;

static LIR_Opr as_long_opr(Register r, Register r2){
  return LIR_OprFact::double_cpu(cpu_reg2rnr(r), cpu_reg2rnr(r2));
}

static LIR_Opr as_float_opr(FloatRegister r) {
  return LIR_OprFact::single_fpu(r->encoding());
}


static bool is_caller_save_register (LIR_Opr  opr);
static bool is_caller_save_register (Register r);


// OptoReg name for spilled virtual FPU register n
//OptoReg::Name fpu_regname (int n);

static VMReg fpu_regname (int n);
static Register first_register();
static FloatRegister nr2floatreg (int rnr);
static int adjust_reg_range(int range) {
  // Reduce the number of available regs (to free r12) in case of compressed oops
  if (UseCompressedOops || UseCompressedClassPointers) return range - 1;
  return range;
}

static int nof_caller_save_cpu_regs() { return adjust_reg_range(pd_nof_caller_save_cpu_regs_frame_map); }
static int last_cpu_reg()             { return adjust_reg_range(pd_last_cpu_reg);  }
//static int last_byte_reg()            { return adjust_reg_range(pd_last_byte_reg); }

#endif // CPU_MIPS_VM_C1_FRAMEMAP_MIPS_HPP

