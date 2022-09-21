/*
 * Copyright (c) 1999, 2013, Oracle and/or its affiliates. All rights reserved.
 * Copyright (c) 2015, 2022, Loongson Technology. All rights reserved.
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
#include "asm/assembler.hpp"
#include "c1/c1_Defs.hpp"
#include "c1/c1_MacroAssembler.hpp"
#include "c1/c1_Runtime1.hpp"
#include "interpreter/interpreter.hpp"
#include "nativeInst_mips.hpp"
#include "oops/compiledICHolder.hpp"
#include "oops/oop.inline.hpp"
#include "prims/jvmtiExport.hpp"
#include "register_mips.hpp"
#include "runtime/sharedRuntime.hpp"
#include "runtime/signature.hpp"
#include "runtime/vframeArray.hpp"
#include "utilities/macros.hpp"
#include "vmreg_mips.inline.hpp"
#if INCLUDE_ALL_GCS
#include "gc_implementation/g1/g1SATBCardTableModRefBS.hpp"
#endif


// Implementation of StubAssembler
// this method will preserve the stack space for arguments as indicated by args_size
// for stack alignment consideration, you cannot call this with argument in stack.
// if you need >3 arguments, you must implement this method yourself.
int StubAssembler::call_RT(Register oop_result1, Register metadata_result, address entry, int args_size) {
  // setup registers
  const Register thread = TREG; // is callee-saved register (Visual C++ calling conventions)
  assert(!(oop_result1->is_valid() || metadata_result->is_valid()) || oop_result1 != metadata_result,                            "registers must be different");
  assert(oop_result1 != thread && metadata_result != thread, "registers must be different");
  assert(args_size >= 0, "illegal args_size");
  bool align_stack = false;
#ifdef _LP64
  // At a method handle call, the stack may not be properly aligned
  // when returning with an exception.
  align_stack = (stub_id() == Runtime1::handle_exception_from_callee_id);
#endif

  set_num_rt_args(1 + args_size);


  // push java thread (becomes first argument of C function)
  get_thread(thread);
  move(A0, thread);

  if(!align_stack) {
    set_last_Java_frame(thread, NOREG, FP, NULL);
  } else {
    address the_pc = pc();
    set_last_Java_frame(thread, NOREG, FP, the_pc);
    move(AT, -(StackAlignmentInBytes));
    andr(SP, SP, AT);
  }

  relocate(relocInfo::internal_pc_type);
  {
#ifndef _LP64
    int save_pc = (int)pc() +  12 + NativeCall::return_address_offset;
    lui(AT, Assembler::split_high(save_pc));
    addiu(AT, AT, Assembler::split_low(save_pc));
#else
    uintptr_t save_pc = (uintptr_t)pc() + NativeMovConstReg::instruction_size + 1 * BytesPerInstWord + NativeCall::return_address_offset_long;
    li48(AT, save_pc);
#endif
  }
  st_ptr(AT, thread, in_bytes(JavaThread::last_Java_pc_offset()));

  // do the call
#ifndef _LP64
  lui(T9, Assembler::split_high((int)entry));
  addiu(T9, T9, Assembler::split_low((int)entry));
#else
  li48(T9, (intptr_t)entry);
#endif
  jalr(T9);
  delayed()->nop();

  int call_offset = offset();

  // verify callee-saved register
#ifdef ASSERT
  guarantee(thread != V0, "change this code");
  push(V0);
  {
    Label L;
    get_thread(V0);
    beq(thread, V0, L);
    delayed()->nop();
    brk(17);
    stop("StubAssembler::call_RT: TREG not callee saved?");
    bind(L);
  }
  super_pop(V0);
#endif
  // discard thread and arguments
  ld_ptr(SP, thread, in_bytes(JavaThread::last_Java_sp_offset()));
  reset_last_Java_frame(thread, true);
  // check for pending exceptions
  {
    Label L;
    ld_ptr(AT, thread, in_bytes(Thread::pending_exception_offset()));
    beq(AT, R0, L);
    delayed()->nop();
    // exception pending => remove activation and forward to exception handler
    // make sure that the vm_results are cleared
    if (oop_result1->is_valid()) {
      st_ptr(R0, thread, in_bytes(JavaThread::vm_result_offset()));
    }
    if (metadata_result->is_valid()) {
      st_ptr(R0, thread, in_bytes(JavaThread::vm_result_2_offset()));
    }
    // the leave() in x86 just pops ebp and remains the return address on the top
    // of stack
    // the return address will be needed by forward_exception_entry()
    if (frame_size() == no_frame_size) {
      addiu(SP, FP, wordSize);
      ld_ptr(FP, SP, (-1) * wordSize);
      jmp(StubRoutines::forward_exception_entry(), relocInfo::runtime_call_type);
      delayed()->nop();
    } else if (_stub_id == Runtime1::forward_exception_id) {
      should_not_reach_here();
    } else {
      jmp(Runtime1::entry_for(Runtime1::forward_exception_id), relocInfo::runtime_call_type);
      delayed()->nop();
    }
    bind(L);
  }
  // get oop results if there are any and reset the values in the thread
  if (oop_result1->is_valid()) {
    ld_ptr(oop_result1, thread, in_bytes(JavaThread::vm_result_offset()));
    st_ptr(R0, thread, in_bytes(JavaThread::vm_result_offset()));
    verify_oop(oop_result1);
  }
  if (metadata_result->is_valid()) {
    ld_ptr(metadata_result, thread, in_bytes(JavaThread::vm_result_2_offset()));
    st_ptr(R0, thread, in_bytes(JavaThread::vm_result_2_offset()));
    verify_oop(metadata_result);
  }
  return call_offset;
}


int StubAssembler::call_RT(Register oop_result1, Register metadata_result, address entry, Register arg1) {
  if (arg1 != A1) move(A1, arg1);
  return call_RT(oop_result1, metadata_result, entry, 1);
}


int StubAssembler::call_RT(Register oop_result1, Register metadata_result, address entry, Register arg1, Register arg2) {
  if (arg1!=A1) move(A1, arg1);
  if (arg2!=A2) move(A2, arg2); assert(arg2 != A1, "smashed argument");
  return call_RT(oop_result1, metadata_result, entry, 2);
}


int StubAssembler::call_RT(Register oop_result1, Register metadata_result, address entry, Register arg1, Register arg2, Register arg3) {
  if (arg1!=A1) move(A1, arg1);
  if (arg2!=A2) move(A2, arg2); assert(arg2 != A1, "smashed argument");
  if (arg3!=A3) move(A3, arg3); assert(arg3 != A1 && arg3 != A2, "smashed argument");
  return call_RT(oop_result1, metadata_result, entry, 3);
}


// Implementation of StubFrame

class StubFrame: public StackObj {
 private:
  StubAssembler* _sasm;

 public:
  StubFrame(StubAssembler* sasm, const char* name, bool must_gc_arguments);
  void load_argument(int offset_in_words, Register reg);

  ~StubFrame();
};


#define __ _sasm->

StubFrame::StubFrame(StubAssembler* sasm, const char* name, bool must_gc_arguments) {
  _sasm = sasm;
  __ set_info(name, must_gc_arguments);
  __ enter();
}


//FIXME, I have no idea the frame architecture of mips
// load parameters that were stored with LIR_Assembler::store_parameter
// Note: offsets for store_parameter and load_argument must match
void StubFrame::load_argument(int offset_in_words, Register reg) {
  //fp, + 0: link
  //    + 1: return address
  //    + 2: argument with offset 0
  //    + 3: argument with offset 1
  //    + 4: ...
  __ ld_ptr(reg, Address(FP, (offset_in_words + 2) * BytesPerWord));
}


StubFrame::~StubFrame() {
  __ leave();
  __ jr(RA);
  __ delayed()->nop();
}

#undef __


// Implementation of Runtime1

#define __ sasm->

const int float_regs_as_doubles_size_in_words = 16;

//FIXME,
// Stack layout for saving/restoring  all the registers needed during a runtime
// call (this includes deoptimization)
// Note: note that users of this frame may well have arguments to some runtime
// while these values are on the stack. These positions neglect those arguments
// but the code in save_live_registers will take the argument count into
// account.
//
#ifdef _LP64
  #define SLOT2(x) x,
  #define SLOT_PER_WORD 2
#else
  #define SLOT2(x)
  #define SLOT_PER_WORD 1
#endif // _LP64

enum reg_save_layout {
#ifndef _LP64
  T0_off = 0,
  S0_off = T0_off + SLOT_PER_WORD * 8,
#else
  A4_off = 0,
  S0_off = A4_off + SLOT_PER_WORD * 8,
#endif
  FP_off = S0_off + SLOT_PER_WORD * 8, SLOT2(FPH_off)
  T8_off, SLOT2(T8H_off)
  T9_off, SLOT2(T9H_off)
  SP_off, SLOT2(SPH_off)
  V0_off, SLOT2(V0H_off)
  V1_off, SLOT2(V1H_off)
  A0_off, SLOT2(A0H_off)
  A1_off, SLOT2(A1H_off)
  A2_off, SLOT2(A2H_off)
  A3_off, SLOT2(A3H_off)

  // Float registers
  // FIXME: Jin: In MIPS64, F0~23 are all caller-saved registers
  F0_off, SLOT2( F0H_off)
  F1_off, SLOT2( F1H_off)
  F2_off, SLOT2( F2H_off)
  F3_off, SLOT2( F3H_off)
  F4_off, SLOT2( F4H_off)
  F5_off, SLOT2( F5H_off)
  F6_off, SLOT2( F6H_off)
  F7_off, SLOT2( F7H_off)
  F8_off, SLOT2( F8H_off)
  F9_off, SLOT2( F9H_off)
  F10_off, SLOT2( F10H_off)
  F11_off, SLOT2( F11H_off)
  F12_off, SLOT2( F12H_off)
  F13_off, SLOT2( F13H_off)
  F14_off, SLOT2( F14H_off)
  F15_off, SLOT2( F15H_off)
  F16_off, SLOT2( F16H_off)
  F17_off, SLOT2( F17H_off)
  F18_off, SLOT2( F18H_off)
  F19_off, SLOT2( F19H_off)

  GP_off, SLOT2( GPH_off)
  //temp_2_off,
  temp_1_off, SLOT2(temp_1H_off)
  saved_fp_off, SLOT2(saved_fpH_off)
  return_off, SLOT2(returnH_off)

  reg_save_frame_size,

  // illegal instruction handler
  continue_dest_off = temp_1_off,

  // deoptimization equates
  //deopt_type = temp_2_off,             // slot for type of deopt in progress
  ret_type = temp_1_off                // slot for return type
};



// Save off registers which might be killed by calls into the runtime.
// Tries to smart of about FP registers.  In particular we separate
// saving and describing the FPU registers for deoptimization since we
// have to save the FPU registers twice if we describe them and on P4
// saving FPU registers which don't contain anything appears
// expensive.  The deopt blob is the only thing which needs to
// describe FPU registers.  In all other cases it should be sufficient
// to simply save their current value.
static OopMap* generate_oop_map(StubAssembler* sasm, int num_rt_args,
                                bool save_fpu_registers = true, bool describe_fpu_registers = false) {

  LP64_ONLY(num_rt_args = 0);
  LP64_ONLY(assert((reg_save_frame_size * VMRegImpl::stack_slot_size) % 16 == 0, "must be 16 byte aligned");)
  int frame_size_in_slots = reg_save_frame_size + num_rt_args * wordSize / VMRegImpl::slots_per_word;   // args + thread
  sasm->set_frame_size(frame_size_in_slots / VMRegImpl::slots_per_word);

  // record saved value locations in an OopMap
  // locations are offsets from sp after runtime call; num_rt_args is number of arguments
  // in call, including thread
  OopMap* map = new OopMap(reg_save_frame_size, 0);

  map->set_callee_saved(VMRegImpl::stack2reg(V0_off + num_rt_args), V0->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(V1_off + num_rt_args), V1->as_VMReg());
#ifdef _LP64
  map->set_callee_saved(VMRegImpl::stack2reg(V0H_off + num_rt_args), V0->as_VMReg()->next());
  map->set_callee_saved(VMRegImpl::stack2reg(V1H_off + num_rt_args), V1->as_VMReg()->next());
#endif

  int i = 0;
#ifndef _LP64
  for (Register r = T0; r != T7->successor(); r = r->successor() ) {
    map->set_callee_saved(VMRegImpl::stack2reg(T0_off + num_rt_args + i++), r->as_VMReg());
  }
#else
  for (Register r = A4; r != T3->successor(); r = r->successor() ) {
    map->set_callee_saved(VMRegImpl::stack2reg(A4_off + num_rt_args + i++), r->as_VMReg());
    map->set_callee_saved(VMRegImpl::stack2reg(A4_off + num_rt_args + i++), r->as_VMReg()->next());
  }
#endif

  i = 0;
  for (Register r = S0; r != S7->successor(); r = r->successor() ) {
    map->set_callee_saved(VMRegImpl::stack2reg(S0_off + num_rt_args + i++), r->as_VMReg());
#ifdef _LP64
    map->set_callee_saved(VMRegImpl::stack2reg(S0_off + num_rt_args + i++), r->as_VMReg()->next());
#endif
  }

  map->set_callee_saved(VMRegImpl::stack2reg(FP_off + num_rt_args), FP->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(GP_off + num_rt_args), GP->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(T8_off + num_rt_args), T8->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(T9_off + num_rt_args), T9->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(A0_off + num_rt_args), A0->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(A1_off + num_rt_args), A1->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(A2_off + num_rt_args), A2->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(A3_off + num_rt_args), A3->as_VMReg());

  map->set_callee_saved(VMRegImpl::stack2reg(F0_off + num_rt_args), F0->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(F1_off + num_rt_args), F1->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(F2_off + num_rt_args), F2->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(F3_off + num_rt_args), F1->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(F4_off + num_rt_args), F4->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(F5_off + num_rt_args), F4->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(F6_off + num_rt_args), F4->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(F7_off + num_rt_args), F4->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(F8_off + num_rt_args), F4->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(F9_off + num_rt_args), F4->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(F10_off + num_rt_args), F4->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(F11_off + num_rt_args), F4->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(F12_off + num_rt_args), F12->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(F13_off + num_rt_args), F13->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(F14_off + num_rt_args), F14->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(F15_off + num_rt_args), F15->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(F16_off + num_rt_args), F16->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(F17_off + num_rt_args), F17->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(F18_off + num_rt_args), F18->as_VMReg());
  map->set_callee_saved(VMRegImpl::stack2reg(F19_off + num_rt_args), F19->as_VMReg());

#ifdef _LP64
  map->set_callee_saved(VMRegImpl::stack2reg(FPH_off + num_rt_args), FP->as_VMReg()->next());
  map->set_callee_saved(VMRegImpl::stack2reg(GPH_off + num_rt_args), GP->as_VMReg()->next());
  map->set_callee_saved(VMRegImpl::stack2reg(T8H_off + num_rt_args), T8->as_VMReg()->next());
  map->set_callee_saved(VMRegImpl::stack2reg(T9H_off + num_rt_args), T9->as_VMReg()->next());
  map->set_callee_saved(VMRegImpl::stack2reg(A0H_off + num_rt_args), A0->as_VMReg()->next());
  map->set_callee_saved(VMRegImpl::stack2reg(A1H_off + num_rt_args), A1->as_VMReg()->next());
  map->set_callee_saved(VMRegImpl::stack2reg(A2H_off + num_rt_args), A2->as_VMReg()->next());
  map->set_callee_saved(VMRegImpl::stack2reg(A3H_off + num_rt_args), A3->as_VMReg()->next());
#endif
  return map;
}

static OopMap* save_live_registers(StubAssembler* sasm, int num_rt_args,
                                   bool save_fpu_registers = true,
                                   bool describe_fpu_registers = false) {
  //const int reg_save_frame_size = return_off + 1 + num_rt_args;
  __ block_comment("save_live_registers");

  // save all register state - int, fpu
  __ addiu(SP, SP, -(reg_save_frame_size / SLOT_PER_WORD - 2)* wordSize);

#ifndef _LP64
  for (Register r = T0; r != T7->successor(); r = r->successor() ) {
    __ sw(r, SP, (r->encoding() - T0->encoding() + T0_off / SLOT_PER_WORD) * wordSize);
#else
  for (Register r = A4; r != T3->successor(); r = r->successor() ) {
    __ sd(r, SP, (r->encoding() - A4->encoding() + A4_off / SLOT_PER_WORD) * wordSize);
#endif
  }
  for (Register r = S0; r != S7->successor(); r = r->successor() ) {
    __ st_ptr(r, SP, (r->encoding() - S0->encoding() + S0_off / SLOT_PER_WORD) * wordSize);
  }
  __ st_ptr(FP, SP, FP_off * wordSize / SLOT_PER_WORD);
  __ st_ptr(GP, SP, GP_off * wordSize / SLOT_PER_WORD);
  __ st_ptr(T8, SP, T8_off * wordSize / SLOT_PER_WORD);
  __ st_ptr(T9, SP, T9_off * wordSize / SLOT_PER_WORD);
  __ st_ptr(A0, SP, A0_off * wordSize / SLOT_PER_WORD);
  __ st_ptr(A1, SP, A1_off * wordSize / SLOT_PER_WORD);
  __ st_ptr(A2, SP, A2_off * wordSize / SLOT_PER_WORD);
  __ st_ptr(A3, SP, A3_off * wordSize / SLOT_PER_WORD);
  __ st_ptr(V0, SP, V0_off * wordSize / SLOT_PER_WORD);
  __ st_ptr(V1, SP, V1_off * wordSize / SLOT_PER_WORD);

  __ sdc1(F0, SP, F0_off * wordSize / SLOT_PER_WORD);
  __ sdc1(F1, SP, F1_off * wordSize / SLOT_PER_WORD);
  __ sdc1(F2, SP, F2_off * wordSize / SLOT_PER_WORD);
  __ sdc1(F3, SP, F3_off * wordSize / SLOT_PER_WORD);
  __ sdc1(F4, SP, F4_off * wordSize / SLOT_PER_WORD);
  __ sdc1(F5, SP, F5_off * wordSize / SLOT_PER_WORD);
  __ sdc1(F6, SP, F6_off * wordSize / SLOT_PER_WORD);
  __ sdc1(F7, SP, F7_off * wordSize / SLOT_PER_WORD);
  __ sdc1(F8, SP, F8_off * wordSize / SLOT_PER_WORD);
  __ sdc1(F9, SP, F9_off * wordSize / SLOT_PER_WORD);
  __ sdc1(F10, SP, F10_off * wordSize / SLOT_PER_WORD);
  __ sdc1(F11, SP, F11_off * wordSize / SLOT_PER_WORD);
  __ sdc1(F12, SP, F12_off * wordSize / SLOT_PER_WORD);
  __ sdc1(F13, SP, F13_off * wordSize / SLOT_PER_WORD);
  __ sdc1(F14, SP, F14_off * wordSize / SLOT_PER_WORD);
  __ sdc1(F15, SP, F15_off * wordSize / SLOT_PER_WORD);
  __ sdc1(F16, SP, F16_off * wordSize / SLOT_PER_WORD);
  __ sdc1(F17, SP, F17_off * wordSize / SLOT_PER_WORD);
  __ sdc1(F18, SP, F18_off * wordSize / SLOT_PER_WORD);
  __ sdc1(F19, SP, F19_off * wordSize / SLOT_PER_WORD);

  return generate_oop_map(sasm, num_rt_args, save_fpu_registers, describe_fpu_registers);
}

static void restore_fpu(StubAssembler* sasm, bool restore_fpu_registers = true) {
  //static void restore_live_registers(MacroAssembler* sasm) {
#ifndef _LP64
  for (Register r = T0; r != T7->successor(); r = r->successor() ) {
    __ lw(r, SP, (r->encoding() - T0->encoding() + T0_off / SLOT_PER_WORD) * wordSize);
#else
  for (Register r = A4; r != T3->successor(); r = r->successor() ) {
    __ ld(r, SP, (r->encoding() - A4->encoding() + A4_off / SLOT_PER_WORD) * wordSize);
#endif
  }
  for (Register r = S0; r != S7->successor(); r = r->successor() ) {
    __ ld_ptr(r, SP, (r->encoding() - S0->encoding() + S0_off / SLOT_PER_WORD) * wordSize);
  }
  __ ld_ptr(FP, SP, FP_off * wordSize / SLOT_PER_WORD);
  __ ld_ptr(GP, SP, GP_off * wordSize / SLOT_PER_WORD);

  __ ld_ptr(T8, SP, T8_off * wordSize / SLOT_PER_WORD);
  __ ld_ptr(T9, SP, T9_off * wordSize / SLOT_PER_WORD);
  __ ld_ptr(A0, SP, A0_off * wordSize / SLOT_PER_WORD);
  __ ld_ptr(A1, SP, A1_off * wordSize / SLOT_PER_WORD);
  __ ld_ptr(A2, SP, A2_off * wordSize / SLOT_PER_WORD);
  __ ld_ptr(A3, SP, A3_off * wordSize / SLOT_PER_WORD);

  __ ld_ptr(V0, SP, V0_off * wordSize / SLOT_PER_WORD);
  __ ld_ptr(V1, SP, V1_off * wordSize / SLOT_PER_WORD);

  __ ldc1(F0, SP, F0_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F1, SP, F1_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F2, SP, F2_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F3, SP, F3_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F4, SP, F4_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F5, SP, F5_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F6, SP, F6_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F7, SP, F7_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F8, SP, F8_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F9, SP, F9_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F10, SP, F10_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F11, SP, F11_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F12, SP, F12_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F13, SP, F13_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F14, SP, F14_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F15, SP, F15_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F16, SP, F16_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F17, SP, F17_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F18, SP, F18_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F19, SP, F19_off * wordSize / SLOT_PER_WORD);

  __ addiu(SP, SP, (reg_save_frame_size / SLOT_PER_WORD - 2) * wordSize);
}

static void restore_live_registers(StubAssembler* sasm, bool restore_fpu_registers = true) {
  __ block_comment("restore_live_registers");
  restore_fpu(sasm, restore_fpu_registers);
}

static void restore_live_registers_except_V0(StubAssembler* sasm, bool restore_fpu_registers = true) {
  //static void restore_live_registers(MacroAssembler* sasm) {
  //FIXME , maybe V1 need to be saved too
  __ block_comment("restore_live_registers except V0");
#ifndef _LP64
  for (Register r = T0; r != T7->successor(); r = r->successor() ) {
    __ lw(r, SP, (r->encoding() - T0->encoding() + T0_off / SLOT_PER_WORD) * wordSize);
#else
  for (Register r = A4; r != T3->successor(); r = r->successor() ) {
    __ ld(r, SP, (r->encoding() - A4->encoding() + A4_off / SLOT_PER_WORD) * wordSize);
#endif
  }
  for (Register r = S0; r != S7->successor(); r = r->successor() ) {
    __ ld_ptr(r, SP, (r->encoding() - S0->encoding() + S0_off / SLOT_PER_WORD) * wordSize);
  }
  __ ld_ptr(FP, SP, FP_off * wordSize / SLOT_PER_WORD);
  __ ld_ptr(GP, SP, GP_off * wordSize / SLOT_PER_WORD);

  __ ld_ptr(T8, SP, T8_off * wordSize / SLOT_PER_WORD);
  __ ld_ptr(T9, SP, T9_off * wordSize / SLOT_PER_WORD);
  __ ld_ptr(A0, SP, A0_off * wordSize / SLOT_PER_WORD);
  __ ld_ptr(A1, SP, A1_off * wordSize / SLOT_PER_WORD);
  __ ld_ptr(A2, SP, A2_off * wordSize / SLOT_PER_WORD);
  __ ld_ptr(A3, SP, A3_off * wordSize / SLOT_PER_WORD);

#if 1
  __ ldc1(F0, SP, F0_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F1, SP, F1_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F2, SP, F2_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F3, SP, F3_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F4, SP, F4_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F5, SP, F5_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F6, SP, F6_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F7, SP, F7_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F8, SP, F8_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F9, SP, F9_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F10, SP, F10_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F11, SP, F11_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F12, SP, F12_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F13, SP, F13_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F14, SP, F14_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F15, SP, F15_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F16, SP, F16_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F17, SP, F17_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F18, SP, F18_off * wordSize / SLOT_PER_WORD);
  __ ldc1(F19, SP, F19_off * wordSize / SLOT_PER_WORD);
#endif

  __ ld_ptr(V1, SP, V1_off * wordSize / SLOT_PER_WORD);

  __ addiu(SP, SP, (reg_save_frame_size / SLOT_PER_WORD - 2) * wordSize);
}

void Runtime1::initialize_pd() {
  // nothing to do
}

// target: the entry point of the method that creates and posts the exception oop
// has_argument: true if the exception needs an argument (passed on stack because registers must be preserved)
OopMapSet* Runtime1::generate_exception_throw(StubAssembler* sasm, address target, bool has_argument) {
  // preserve all registers
  OopMap* oop_map = save_live_registers(sasm, 0);

  // now all registers are saved and can be used freely
  // verify that no old value is used accidentally
  //all reigster are saved , I think mips do not need this

  // registers used by this stub
  const Register temp_reg = T3;
  // load argument for exception that is passed as an argument into the stub
  if (has_argument) {
    __ ld_ptr(temp_reg, Address(FP, 2*BytesPerWord));
  }
  int call_offset;
  if (has_argument)
     call_offset = __ call_RT(noreg, noreg, target, temp_reg);
  else
     call_offset = __ call_RT(noreg, noreg, target);

  OopMapSet* oop_maps = new OopMapSet();
  oop_maps->add_gc_map(call_offset, oop_map);

  __ stop("should not reach here");

  return oop_maps;
}

OopMapSet* Runtime1::generate_handle_exception(StubID id, StubAssembler *sasm) {
  __ block_comment("generate_handle_exception");

  // incoming parameters
  const Register exception_oop = V0;
  const Register exception_pc = V1;
  // other registers used in this stub
  const Register thread = TREG;
#ifndef OPT_THREAD
  __ get_thread(thread);
#endif
  // Save registers, if required.
  OopMapSet* oop_maps = new OopMapSet();
  OopMap* oop_map = NULL;
  switch (id) {
  case forward_exception_id:
    // We're handling an exception in the context of a compiled frame.
    // The registers have been saved in the standard places.  Perform
    // an exception lookup in the caller and dispatch to the handler
    // if found.  Otherwise unwind and dispatch to the callers
    // exception handler.
    oop_map = generate_oop_map(sasm, 1 /*thread*/);

    // load and clear pending exception oop into V0
    __ ld_ptr(exception_oop, Address(thread, Thread::pending_exception_offset()));
    __ st_ptr(R0, Address(thread, Thread::pending_exception_offset()));

    // load issuing PC (the return address for this stub) into V1
    __ ld_ptr(exception_pc, Address(FP, 1*BytesPerWord));

    // make sure that the vm_results are cleared (may be unnecessary)
    __ st_ptr(R0, Address(thread, JavaThread::vm_result_offset()));
    __ st_ptr(R0, Address(thread, JavaThread::vm_result_2_offset()));
    break;
  case handle_exception_nofpu_id:
  case handle_exception_id:
    // At this point all registers MAY be live.
    oop_map = save_live_registers(sasm, 1 /*thread*/, id != handle_exception_nofpu_id);
    break;
  case handle_exception_from_callee_id: {
    // At this point all registers except exception oop (V0) and
    // exception pc (V1) are dead.
    const int frame_size = 2 /*BP, return address*/ NOT_LP64(+ 1 /*thread*/);
    oop_map = new OopMap(frame_size * VMRegImpl::slots_per_word, 0);
    sasm->set_frame_size(frame_size);
    break;
  }
  default:  ShouldNotReachHere();
  }

#ifdef TIERED
  // C2 can leave the fpu stack dirty
  __ empty_FPU_stack();
#endif // TIERED

  // verify that only V0 and V1 is valid at this time
  // verify that V0 contains a valid exception
  __ verify_not_null_oop(exception_oop);

  // load address of JavaThread object for thread-local data
  __ get_thread(thread);

#ifdef ASSERT
  // check that fields in JavaThread for exception oop and issuing pc are
  // empty before writing to them
  Label oop_empty;
  __ ld_ptr(AT, Address(thread, in_bytes(JavaThread::exception_oop_offset())));
  __ beq(AT, R0, oop_empty);
  __ delayed()->nop();
  __ stop("exception oop already set");
  __ bind(oop_empty);

  Label pc_empty;
  __ ld_ptr(AT, Address(thread, in_bytes(JavaThread::exception_pc_offset())));
  __ beq(AT, R0, pc_empty);
  __ delayed()->nop();
  __ stop("exception pc already set");
  __ bind(pc_empty);
#endif

  // save exception oop and issuing pc into JavaThread
  // (exception handler will load it from here)
  __ st_ptr(exception_oop, Address(thread, in_bytes(JavaThread::exception_oop_offset())));
  __ st_ptr(exception_pc, Address(thread, in_bytes(JavaThread::exception_pc_offset())));

  // patch throwing pc into return address (has bci & oop map)
  __ st_ptr(exception_pc, Address(FP, 1*BytesPerWord));

  // compute the exception handler.
  // the exception oop and the throwing pc are read from the fields in JavaThread
  __ block_comment(";; will call_RT exception_handler_for_pc");
  int call_offset = __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, exception_handler_for_pc));
  oop_maps->add_gc_map(call_offset, oop_map);
  __ block_comment(";; end of call_RT exception_handler_for_pc");

  // V0:  handler address or NULL if no handler exists
  //      will be the deopt blob if nmethod was deoptimized while we looked up
  //      handler regardless of whether handler existed in the nmethod.

  // only V0 is valid at this time, all other registers have been destroyed by the
  // runtime call

  // patch the return address -> the stub will directly return to the exception handler
  __ st_ptr(V0, Address(FP, 1 * BytesPerWord));

  switch (id) {
  case forward_exception_id:
  case handle_exception_nofpu_id:
  case handle_exception_id:
    // Restore the registers that were saved at the beginning.
    restore_live_registers(sasm, id != handle_exception_nofpu_id);
    break;
  case handle_exception_from_callee_id:
    // WIN64_ONLY: No need to add frame::arg_reg_save_area_bytes to SP
    // since we do a leave anyway.

    // Pop the return address since we are possibly changing SP (restoring from BP).
    __ move(SP, FP);
    __ pop(FP);
    __ pop(RA);
    __ jr(RA);  // jump to exception handler
    __ delayed()->nop();
    break;
   default:  ShouldNotReachHere();
  }

  return oop_maps;
}

void Runtime1::generate_unwind_exception(StubAssembler *sasm) {
  // incoming parameters
  const Register exception_oop = V0;
  // callee-saved copy of exception_oop during runtime call
  const Register exception_oop_callee_saved = S0;
  // other registers used in this stub
  const Register exception_pc = V1;
  const Register handler_addr = T3;
  const Register thread = TREG;

#ifdef ASSERT
  // check that fields in JavaThread for exception oop and issuing pc are empty
  __ get_thread(thread);
  Label oop_empty;
  __ ld_ptr(AT, thread, in_bytes(JavaThread::exception_oop_offset()));
  __ beq(AT, R0, oop_empty);
  __ delayed()->nop();
  __ stop("exception oop must be empty");
  __ bind(oop_empty);

  Label pc_empty;
  __ ld_ptr(AT, thread, in_bytes(JavaThread::exception_pc_offset()));
  __ beq(AT, R0, pc_empty);
  __ delayed()->nop();
  __ stop("exception pc must be empty");
  __ bind(pc_empty);
#endif
  // clear the FPU stack in case any FPU results are left behind
  __ empty_FPU_stack();

  // save exception_oop in callee-saved register to preserve it during runtime calls
  __ verify_not_null_oop(exception_oop);
  __ move(exception_oop_callee_saved, exception_oop);

#ifndef OPT_THREAD
  __ get_thread(thread);
#endif
  // Get return address (is in RA after leave).

  __ move(exception_pc, RA);
  __ push(RA);

  // search the exception handler address of the caller (using the return address)
  __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::exception_handler_for_return_address), thread, exception_pc);
  // V0: exception handler address of the caller

  // move result of call into correct register
  __ move(handler_addr, V0);

  // Restore exception oop to V0 (required convention of exception handler).
  __ move(exception_oop, exception_oop_callee_saved);

  // verify that there is really a valid exception in V0
  __ verify_oop(exception_oop);

  // get throwing pc (= return address).
  // V1 has been destroyed by the call, so it must be set again
  // the pop is also necessary to simulate the effect of a ret(0)
  __ super_pop(exception_pc);

  // continue at exception handler (return address removed)
  // note: do *not* remove arguments when unwinding the
  //       activation since the caller assumes having
  //       all arguments on the stack when entering the
  //       runtime to determine the exception handler
  //       (GC happens at call site with arguments!)
  // V0: exception oop
  // V1: throwing pc
  // T3: exception handler
  __ jr(handler_addr);
  __ delayed()->nop();
}

OopMapSet* Runtime1::generate_patching(StubAssembler* sasm, address target) {

  // use the maximum number of runtime-arguments here because it is difficult to
  // distinguish each RT-Call.
  // Note: This number affects also the RT-Call in generate_handle_exception because
  //       the oop-map is shared for all calls.

  DeoptimizationBlob* deopt_blob = SharedRuntime::deopt_blob();
  assert(deopt_blob != NULL, "deoptimization blob must have been created");
  // assert(deopt_with_exception_entry_for_patch != NULL,
  // "deoptimization blob must have been created");

  //OopMap* oop_map = save_live_registers(sasm, num_rt_args);
  OopMap* oop_map = save_live_registers(sasm, 0);
  const Register thread = T8;
  // push java thread (becomes first argument of C function)
  __ get_thread(thread);
  __ move(A0, thread);



  // NOTE: this frame should be compiled frame, but at this point, the pc in frame-anchor
  // is contained in interpreter. It should be wrong, and should be cleared but is not.
  // even if we cleared the wrong pc in anchor, the default way to get caller pc in class frame
  // is not right. It depends on that the caller pc is stored in *(sp - 1) but it's not the case

  __ set_last_Java_frame(thread, NOREG, FP, NULL);
  NOT_LP64(__ addiu(SP, SP, (-1) * wordSize));
  __ move(AT, -(StackAlignmentInBytes));
  __ andr(SP, SP, AT);
  __ relocate(relocInfo::internal_pc_type);
  {
#ifndef _LP64
    int save_pc = (int)__ pc() +  12 + NativeCall::return_address_offset;
    __ lui(AT, Assembler::split_high(save_pc));
    __ addiu(AT, AT, Assembler::split_low(save_pc));
#else
    uintptr_t save_pc = (uintptr_t)__ pc() + NativeMovConstReg::instruction_size + 1 * BytesPerInstWord + NativeCall::return_address_offset_long;
    __ li48(AT, save_pc);
#endif
  }
  __ st_ptr(AT, thread, in_bytes(JavaThread::last_Java_pc_offset()));

  // do the call
#ifndef _LP64
  __ lui(T9, Assembler::split_high((int)target));
  __ addiu(T9, T9, Assembler::split_low((int)target));
#else
  __ li48(T9, (intptr_t)target);
#endif
  __ jalr(T9);
  __ delayed()->nop();
  OopMapSet*  oop_maps = new OopMapSet();
  oop_maps->add_gc_map(__ offset(),  oop_map);

  __ get_thread(thread);

  __ ld_ptr (SP, thread, in_bytes(JavaThread::last_Java_sp_offset()));
  __ reset_last_Java_frame(thread, true);
  // discard thread arg
  // check for pending exceptions
  {
    Label L, skip;
    //Label no_deopt;
    __ ld_ptr(AT, thread, in_bytes(Thread::pending_exception_offset()));
    __ beq(AT, R0, L);
    __ delayed()->nop();
    // exception pending => remove activation and forward to exception handler

    __ bne(V0, R0, skip);
    __ delayed()->nop();
    __ jmp(Runtime1::entry_for(Runtime1::forward_exception_id),
        relocInfo::runtime_call_type);
    __ delayed()->nop();
    __ bind(skip);

    // the deopt blob expects exceptions in the special fields of
    // JavaThread, so copy and clear pending exception.

    // load and clear pending exception
    __ ld_ptr(V0, Address(thread,in_bytes(Thread::pending_exception_offset())));
    __ st_ptr(R0, Address(thread, in_bytes(Thread::pending_exception_offset())));

    // check that there is really a valid exception
    __ verify_not_null_oop(V0);

    // load throwing pc: this is the return address of the stub
    __ ld_ptr(V1, Address(SP, return_off * BytesPerWord));


#ifdef ASSERT
    // check that fields in JavaThread for exception oop and issuing pc are empty
    Label oop_empty;
    __ ld_ptr(AT, Address(thread, in_bytes(JavaThread::exception_oop_offset())));
    __ beq(AT,R0,oop_empty);
    __ delayed()->nop();
    __ stop("exception oop must be empty");
    __ bind(oop_empty);

    Label pc_empty;
    __ ld_ptr(AT, Address(thread, in_bytes(JavaThread::exception_pc_offset())));
    __ beq(AT,R0,pc_empty);
    __ delayed()->nop();
    __ stop("exception pc must be empty");
    __ bind(pc_empty);
#endif

    // store exception oop and throwing pc to JavaThread
    __ st_ptr(V0,Address(thread, in_bytes(JavaThread::exception_oop_offset())));
    __ st_ptr(V1,Address(thread, in_bytes(JavaThread::exception_pc_offset())));

    restore_live_registers(sasm);

    __ leave();

    // Forward the exception directly to deopt blob. We can blow no
    // registers and must leave throwing pc on the stack.  A patch may
    // have values live in registers so the entry point with the
    // exception in tls.
    __ jmp(deopt_blob->unpack_with_exception_in_tls(), relocInfo::runtime_call_type);
    __ delayed()->nop();

    __ bind(L);
  }

  // Runtime will return true if the nmethod has been deoptimized during
  // the patching process. In that case we must do a deopt reexecute instead.

  Label reexecuteEntry, cont;

  __ beq(V0, R0, cont);                              // have we deoptimized?
  __ delayed()->nop();

  // Will reexecute. Proper return address is already on the stack we just restore
  // registers, pop all of our frame but the return address and jump to the deopt blob
  restore_live_registers(sasm);

  __ leave();
  __ jmp(deopt_blob->unpack_with_reexecution(), relocInfo::runtime_call_type);
  __ delayed()->nop();

  __ bind(cont);
  restore_live_registers(sasm);

  __ leave();
  __ jr(RA);
  __ delayed()->nop();

  return oop_maps;
}


OopMapSet* Runtime1::generate_code_for(StubID id, StubAssembler* sasm) {
  // for better readability
  const bool must_gc_arguments = true;
  const bool dont_gc_arguments = false;


  // default value; overwritten for some optimized stubs that are called
  // from methods that do not use the fpu
  bool save_fpu_registers = true;


  // stub code & info for the different stubs
  OopMapSet* oop_maps = NULL;

  switch (id) {
    case forward_exception_id:
      {
        oop_maps = generate_handle_exception(id, sasm);
        __ leave();
        __ jr(RA);
        __ delayed()->nop();
      }
      break;

    case new_instance_id:
    case fast_new_instance_id:
    case fast_new_instance_init_check_id:
      {
        Register klass = A4; // Incoming
        Register obj   = V0; // Result

        if (id == new_instance_id) {
          __ set_info("new_instance", dont_gc_arguments);
        } else if (id == fast_new_instance_id) {
          __ set_info("fast new_instance", dont_gc_arguments);
        } else {
          assert(id == fast_new_instance_init_check_id, "bad StubID");
          __ set_info("fast new_instance init check", dont_gc_arguments);
        }

        if ((id == fast_new_instance_id || id == fast_new_instance_init_check_id)
             && UseTLAB && FastTLABRefill) {
          Label slow_path;
          Register obj_size = T0;
          Register t1       = T2;
          Register t2       = T3;
          assert_different_registers(klass, obj, obj_size, t1, t2);
          if (id == fast_new_instance_init_check_id) {
            // make sure the klass is initialized
            __ ld_ptr(AT, Address(klass, in_bytes(InstanceKlass::init_state_offset())));
            __ move(t1, InstanceKlass::fully_initialized);
            __ bne(AT, t1, slow_path);
            __ delayed()->nop();
          }
#ifdef ASSERT
          // assert object can be fast path allocated
          {
            Label ok, not_ok;
            __ lw(obj_size, klass, in_bytes(Klass::layout_helper_offset()));
            __ blez(obj_size, not_ok);
            __ delayed()->nop();
            __ andi(t1 , obj_size, Klass::_lh_instance_slow_path_bit);
            __ beq(t1, R0, ok);
            __ delayed()->nop();
            __ bind(not_ok);
            __ stop("assert(can be fast path allocated)");
            __ should_not_reach_here();
            __ bind(ok);
          }
#endif // ASSERT
          // if we got here then the TLAB allocation failed, so try
          // refilling the TLAB or allocating directly from eden.

          Label retry_tlab, try_eden;
          __ tlab_refill(retry_tlab, try_eden, slow_path); // does not destroy A4 (klass)

          __ bind(retry_tlab);

          // get the instance size
          __ lw(obj_size, klass, in_bytes(Klass::layout_helper_offset()));
          __ tlab_allocate(obj, obj_size, 0, t1, t2, slow_path);
          __ initialize_object(obj, klass, obj_size, 0, t1, t2);
          __ verify_oop(obj);
          __ jr(RA);
          __ delayed()->nop();

#ifndef OPT_THREAD
          const Register thread = T8;
          __ get_thread(thread);
#else
          const Register thread = TREG;
#endif

          __ bind(try_eden);

          // get the instance size
          __ lw(obj_size, klass, in_bytes(Klass::layout_helper_offset()));
          __ eden_allocate(obj, obj_size, 0, t1, t2, slow_path);
          __ incr_allocated_bytes(thread, obj_size, 0);

          __ initialize_object(obj, klass, obj_size, 0, t1, t2);
          __ verify_oop(obj);
          __ jr(RA);
          __ delayed()->nop();

          __ bind(slow_path);
        }
        __ enter();
        OopMap* map = save_live_registers(sasm, 0);
        int call_offset = __ call_RT(obj, noreg, CAST_FROM_FN_PTR(address, new_instance), klass);
        oop_maps = new OopMapSet();
        oop_maps->add_gc_map(call_offset, map);
        restore_live_registers_except_V0(sasm);
        __ verify_oop(obj);
        __ leave();
        __ jr(RA);
        __ delayed()->nop();

        // V0: new instance
      }
      break;


#ifdef TIERED
//FIXME, I hava no idea which register to use
    case counter_overflow_id:
      {
#ifndef _LP64
        Register bci = T5;
#else
        Register bci = A5;
#endif
        Register method = AT;
        __ enter();
        OopMap* map = save_live_registers(sasm, 0);
        // Retrieve bci
        __ lw(bci, Address(FP, 2*BytesPerWord));
        __ ld(method, Address(FP, 3*BytesPerWord));
        int call_offset = __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, counter_overflow), bci, method);
        oop_maps = new OopMapSet();
        oop_maps->add_gc_map(call_offset, map);
        restore_live_registers(sasm);
        __ leave();
        __ jr(RA);
        __ delayed()->nop();
      }
      break;
#endif // TIERED



    case new_type_array_id:
    case new_object_array_id:
      {
        // i use T2 as length register, T4 as klass register, V0 as result register.
        // MUST accord with NewTypeArrayStub::emit_code, NewObjectArrayStub::emit_code
        Register length   = T2; // Incoming
#ifndef _LP64
        Register klass    = T4; // Incoming
#else
        Register klass    = A4; // Incoming
#endif
        Register obj      = V0; // Result

        if (id == new_type_array_id) {
          __ set_info("new_type_array", dont_gc_arguments);
        } else {
          __ set_info("new_object_array", dont_gc_arguments);
        }

        if (UseTLAB && FastTLABRefill) {
          Register arr_size = T0;
          Register t1       = T1;
          Register t2       = T3;
          Label slow_path;
          assert_different_registers(length, klass, obj, arr_size, t1, t2);

          // check that array length is small enough for fast path
          __ move(AT, C1_MacroAssembler::max_array_allocation_length);
          __ sltu(AT, AT, length);
          __ bne(AT, R0, slow_path);
          __ delayed()->nop();

          // if we got here then the TLAB allocation failed, so try
          // refilling the TLAB or allocating directly from eden.
          Label retry_tlab, try_eden;
          //T0,T1,T5,T8 have changed!
          __ tlab_refill(retry_tlab, try_eden, slow_path); // preserves T2 & T4

          __ bind(retry_tlab);

          // get the allocation size: (length << (layout_helper & 0x1F)) + header_size
          __ lw(t1, klass, in_bytes(Klass::layout_helper_offset()));
          __ andi(AT, t1, 0x1f);
          __ sllv(arr_size, length, AT);
          __ srl(t1, t1, Klass::_lh_header_size_shift);
          __ andi(t1, t1, Klass::_lh_header_size_mask);
          __ addu(arr_size, t1, arr_size);
          __ addiu(arr_size, arr_size, MinObjAlignmentInBytesMask);  // align up
          __ move(AT, ~MinObjAlignmentInBytesMask);
          __ andr(arr_size, arr_size, AT);


          __ tlab_allocate(obj, arr_size, 0, t1, t2, slow_path);  // preserves arr_size
          __ initialize_header(obj, klass, length,t1,t2);
          __ lbu(t1, Address(klass, in_bytes(Klass::layout_helper_offset())
                                    + (Klass::_lh_header_size_shift / BitsPerByte)));
          assert(Klass::_lh_header_size_shift % BitsPerByte == 0, "bytewise");
          assert(Klass::_lh_header_size_mask <= 0xFF, "bytewise");
          __ andi(t1, t1, Klass::_lh_header_size_mask);
          __ subu(arr_size, arr_size, t1);  // body length
          __ addu(t1, t1, obj);             // body start
          __ initialize_body(t1, arr_size, 0, t2);
          __ verify_oop(obj);
          __ jr(RA);
          __ delayed()->nop();

#ifndef OPT_THREAD
          const Register thread = T8;
          __ get_thread(thread);
#else
          const Register thread = TREG;
#endif

          __ bind(try_eden);
          // get the allocation size: (length << (layout_helper & 0x1F)) + header_size
          __ lw(t1, klass, in_bytes(Klass::layout_helper_offset()));
          __ andi(AT, t1, 0x1f);
          __ sllv(arr_size, length, AT);
          __ srl(t1, t1, Klass::_lh_header_size_shift);
          __ andi(t1, t1, Klass::_lh_header_size_mask);
          __ addu(arr_size, t1, arr_size);
          __ addiu(arr_size, arr_size, MinObjAlignmentInBytesMask);  // align up
          __ move(AT, ~MinObjAlignmentInBytesMask);
          __ andr(arr_size, arr_size, AT);
          __ eden_allocate(obj, arr_size, 0, t1, t2, slow_path);  // preserves arr_size
          __ incr_allocated_bytes(thread, arr_size, 0);

          __ initialize_header(obj, klass, length,t1,t2);
          __ lbu(t1, Address(klass, in_bytes(Klass::layout_helper_offset())
                                    + (Klass::_lh_header_size_shift / BitsPerByte)));
          __ andi(t1, t1, Klass::_lh_header_size_mask);
          __ subu(arr_size, arr_size, t1);  // body length
          __ addu(t1, t1, obj);             // body start

          __ initialize_body(t1, arr_size, 0, t2);
          __ verify_oop(obj);
          __ jr(RA);
          __ delayed()->nop();
          __ bind(slow_path);
        }


        __ enter();
        OopMap* map = save_live_registers(sasm, 0);
        int call_offset;
        if (id == new_type_array_id) {
          call_offset = __ call_RT(obj, noreg,
                                    CAST_FROM_FN_PTR(address, new_type_array), klass, length);
        } else {
          call_offset = __ call_RT(obj, noreg,
                                   CAST_FROM_FN_PTR(address, new_object_array), klass, length);
        }

        oop_maps = new OopMapSet();
        oop_maps->add_gc_map(call_offset, map);
        restore_live_registers_except_V0(sasm);
        __ verify_oop(obj);
        __ leave();
        __ jr(RA);
        __ delayed()->nop();
      }
      break;

    case new_multi_array_id:
      {
        StubFrame f(sasm, "new_multi_array", dont_gc_arguments);
       //refer to c1_LIRGenerate_mips.cpp:do_NewmultiArray
        // V0: klass
        // T2: rank
        // T0: address of 1st dimension
        //__ call_RT(V0, noreg, CAST_FROM_FN_PTR(address, new_multi_array), A1, A2, A3);
        //OopMap* map = save_live_registers(sasm, 4);
        OopMap* map = save_live_registers(sasm, 0);
        int call_offset = __ call_RT(V0, noreg, CAST_FROM_FN_PTR(address, new_multi_array),
            V0,T2,T0);
        oop_maps = new OopMapSet();
        oop_maps->add_gc_map(call_offset, map);
        //FIXME
        restore_live_registers_except_V0(sasm);
        // V0: new multi array
        __ verify_oop(V0);
      }
      break;


    case register_finalizer_id:
      {
        __ set_info("register_finalizer", dont_gc_arguments);

        // The object is passed on the stack and we haven't pushed a
        // frame yet so it's one work away from top of stack.
        //reference to LIRGenerator::do_RegisterFinalizer, call_runtime
        __ move(V0, A0);
        __ verify_oop(V0);
        // load the klass and check the has finalizer flag
        Label register_finalizer;
#ifndef _LP64
        Register t = T5;
#else
        Register t = A5;
#endif
        //__ ld_ptr(t, Address(V0, oopDesc::klass_offset_in_bytes()));
        __ load_klass(t, V0);
        __ lw(t, Address(t, Klass::access_flags_offset()));
        __ move(AT, JVM_ACC_HAS_FINALIZER);
        __ andr(AT, AT, t);

        __ bne(AT, R0, register_finalizer);
        __ delayed()->nop();
        __ jr(RA);
        __ delayed()->nop();
        __ bind(register_finalizer);
        __ enter();
        OopMap* map = save_live_registers(sasm, 0 /*num_rt_args */);

        int call_offset = __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address,
              SharedRuntime::register_finalizer), V0);
        oop_maps = new OopMapSet();
        oop_maps->add_gc_map(call_offset, map);

        // Now restore all the live registers
        restore_live_registers(sasm);

        __ leave();
        __ jr(RA);
        __ delayed()->nop();
      }
      break;

//  case range_check_failed_id:
  case throw_range_check_failed_id:
      {
        StubFrame f(sasm, "range_check_failed", dont_gc_arguments);
        oop_maps = generate_exception_throw(sasm, CAST_FROM_FN_PTR(address,
              throw_range_check_exception),true);
      }
      break;

      case throw_index_exception_id:
      {
        // i use A1 as the index register, for this will be the first argument, see call_RT
        StubFrame f(sasm, "index_range_check_failed", dont_gc_arguments);
        oop_maps = generate_exception_throw(sasm, CAST_FROM_FN_PTR(address,
              throw_index_exception), true);
      }
      break;

  case throw_div0_exception_id:
      { StubFrame f(sasm, "throw_div0_exception", dont_gc_arguments);
        oop_maps = generate_exception_throw(sasm, CAST_FROM_FN_PTR(address,
              throw_div0_exception), false);
      }
      break;

  case throw_null_pointer_exception_id:
      {
        StubFrame f(sasm, "throw_null_pointer_exception", dont_gc_arguments);
        oop_maps = generate_exception_throw(sasm, CAST_FROM_FN_PTR(address,
              throw_null_pointer_exception),false);
      }
      break;

  case handle_exception_nofpu_id:
    save_fpu_registers = false;
     // fall through
  case handle_exception_id:
    {
      StubFrame f(sasm, "handle_exception", dont_gc_arguments);
      //OopMap* oop_map = save_live_registers(sasm, 1, save_fpu_registers);
      oop_maps = generate_handle_exception(id, sasm);
    }
    break;
  case handle_exception_from_callee_id:
    {
      StubFrame f(sasm, "handle_exception_from_callee", dont_gc_arguments);
      oop_maps = generate_handle_exception(id, sasm);
    }
    break;
  case unwind_exception_id:
    {
      __ set_info("unwind_exception", dont_gc_arguments);
      generate_unwind_exception(sasm);
    }
    break;


  case throw_array_store_exception_id:
    {
      StubFrame f(sasm, "throw_array_store_exception", dont_gc_arguments);
      // tos + 0: link
      //     + 1: return address
      oop_maps = generate_exception_throw(sasm, CAST_FROM_FN_PTR(address,
            throw_array_store_exception), true);
    }
    break;

  case throw_class_cast_exception_id:
    {
      StubFrame f(sasm, "throw_class_cast_exception", dont_gc_arguments);
      oop_maps = generate_exception_throw(sasm, CAST_FROM_FN_PTR(address,
            throw_class_cast_exception), true);
    }
    break;

  case throw_incompatible_class_change_error_id:
    {
      StubFrame f(sasm, "throw_incompatible_class_cast_exception", dont_gc_arguments);
      oop_maps = generate_exception_throw(sasm,
            CAST_FROM_FN_PTR(address, throw_incompatible_class_change_error), false);
    }
    break;

  case slow_subtype_check_id:
    {
    //actually , We do not use it
      // A0:klass_RInfo    sub
      // A1:k->encoding() super
      __ set_info("slow_subtype_check", dont_gc_arguments);
      __ st_ptr(T0, SP, (-1) * wordSize);
      __ st_ptr(T1, SP, (-2) * wordSize);
      __ addiu(SP, SP, (-2) * wordSize);

      Label miss;
      __ check_klass_subtype_slow_path(A0, A1, T0, T1, NULL, &miss);

      __ addiu(V0, R0, 1);
      __ addiu(SP, SP, 2 * wordSize);
      __ ld_ptr(T0, SP, (-1) * wordSize);
      __ ld_ptr(T1, SP, (-2) * wordSize);
      __ jr(RA);
      __ delayed()->nop();


      __ bind(miss);
      __ move(V0, R0);
      __ addiu(SP, SP, 2 * wordSize);
      __ ld_ptr(T0, SP, (-1) * wordSize);
      __ ld_ptr(T1, SP, (-2) * wordSize);
      __ jr(RA);
      __ delayed()->nop();
    }
    break;

  case monitorenter_nofpu_id:
    save_fpu_registers = false;// fall through

  case monitorenter_id:
    {
      StubFrame f(sasm, "monitorenter", dont_gc_arguments);
      OopMap* map = save_live_registers(sasm, 0, save_fpu_registers);

      f.load_argument(1, V0); // V0: object
#ifndef _LP64
      f.load_argument(0, T6); // T6: lock address
      int call_offset = __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address,
           monitorenter), V0, T6);
#else
      f.load_argument(0, A6); // A6: lock address
      int call_offset = __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address,
           monitorenter), V0, A6);
#endif

      oop_maps = new OopMapSet();
      oop_maps->add_gc_map(call_offset, map);
      restore_live_registers(sasm, save_fpu_registers);
    }
    break;

  case monitorexit_nofpu_id:
    save_fpu_registers = false;
        // fall through
  case monitorexit_id:
    {
      StubFrame f(sasm, "monitorexit", dont_gc_arguments);
      OopMap* map = save_live_registers(sasm, 0, save_fpu_registers);

#ifndef _LP64
      f.load_argument(0, T6); // T6: lock address
#else
      f.load_argument(0, A6); // A6: lock address
#endif
      // note: really a leaf routine but must setup last java sp
      //       => use call_RT for now (speed can be improved by
      //       doing last java sp setup manually)
#ifndef _LP64
      int call_offset = __ call_RT(noreg, noreg,
                                    CAST_FROM_FN_PTR(address, monitorexit), T6);
#else
      int call_offset = __ call_RT(noreg, noreg,
                                    CAST_FROM_FN_PTR(address, monitorexit), A6);
#endif
      oop_maps = new OopMapSet();
      oop_maps->add_gc_map(call_offset, map);
      restore_live_registers(sasm, save_fpu_registers);

    }
    break;
        //  case init_check_patching_id:
  case access_field_patching_id:
    {
      StubFrame f(sasm, "access_field_patching", dont_gc_arguments);
      // we should set up register map
      oop_maps = generate_patching(sasm, CAST_FROM_FN_PTR(address, access_field_patching));

    }
    break;

  case load_klass_patching_id:
    {
      StubFrame f(sasm, "load_klass_patching", dont_gc_arguments);
      // we should set up register map
      oop_maps = generate_patching(sasm, CAST_FROM_FN_PTR(address,
            move_klass_patching));
    }
    break;
  case load_mirror_patching_id:
    {
      StubFrame f(sasm, "load_mirror_patching" , dont_gc_arguments);
      oop_maps = generate_patching(sasm, CAST_FROM_FN_PTR(address, move_mirror_patching));
    }
    break;

  case load_appendix_patching_id:
    {
      StubFrame f(sasm, "load_appendix_patching", dont_gc_arguments);
      // we should set up register map
      oop_maps = generate_patching(sasm, CAST_FROM_FN_PTR(address, move_appendix_patching));
    }
    break;

  case dtrace_object_alloc_id:
    {
      // V0:object
      StubFrame f(sasm, "dtrace_object_alloc", dont_gc_arguments);
      // we can't gc here so skip the oopmap but make sure that all
      // the live registers get saved.
      save_live_registers(sasm, 0);

      __ push_reg(V0);
      __ move(A0, V0);
      __ call(CAST_FROM_FN_PTR(address, SharedRuntime::dtrace_object_alloc),
          relocInfo::runtime_call_type);
      __ delayed()->nop();
      __ super_pop(V0);

      restore_live_registers(sasm);
    }
    break;

  case fpu2long_stub_id:
    {
      //FIXME, I hava no idea how to port this
      //tty->print_cr("fpu2long_stub_id unimplemented yet!");
    }
    break;

  case deoptimize_id:
    {
      StubFrame f(sasm, "deoptimize", dont_gc_arguments);
      const int num_rt_args = 1;  // thread
      OopMap* oop_map = save_live_registers(sasm, num_rt_args);
      int call_offset = __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, deoptimize));
      oop_maps = new OopMapSet();
      oop_maps->add_gc_map(call_offset, oop_map);
      restore_live_registers(sasm);
      DeoptimizationBlob* deopt_blob = SharedRuntime::deopt_blob();
      assert(deopt_blob != NULL, "deoptimization blob must have been created");
      __ leave();
      __ jmp(deopt_blob->unpack_with_reexecution(), relocInfo::runtime_call_type);
      __ delayed()->nop();
    }
   break;

  case predicate_failed_trap_id:
    {
      StubFrame f(sasm, "predicate_failed_trap", dont_gc_arguments);

      OopMap* map = save_live_registers(sasm, 1);

      int call_offset = __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, predicate_failed_trap));
      oop_maps = new OopMapSet();
      oop_maps->add_gc_map(call_offset, map);
      restore_live_registers(sasm);
      __ leave();
      DeoptimizationBlob* deopt_blob = SharedRuntime::deopt_blob();
      assert(deopt_blob != NULL, "deoptimization blob must have been created");

      __ jmp(deopt_blob->unpack_with_reexecution(), relocInfo::runtime_call_type);
      __ delayed()->nop();
    }
   break;

  default:
    {
      StubFrame f(sasm, "unimplemented entry", dont_gc_arguments);
      __ move(A1, (int)id);
      __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, unimplemented_entry), A1);
      __ should_not_reach_here();
    }
    break;
  }
  return oop_maps;
}

#undef __

const char *Runtime1::pd_name_for_address(address entry) {
  return "<unknown function>";
}
