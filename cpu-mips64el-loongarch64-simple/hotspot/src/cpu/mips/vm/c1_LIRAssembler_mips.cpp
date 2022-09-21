/*
 * Copyright (c) 2000, 2013, Oracle and/or its affiliates. All rights reserved.
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

#include "precompiled.hpp"
#include "asm/macroAssembler.hpp"
#include "asm/macroAssembler.inline.hpp"
#include "c1/c1_Compilation.hpp"
#include "c1/c1_LIRAssembler.hpp"
#include "c1/c1_MacroAssembler.hpp"
#include "c1/c1_Runtime1.hpp"
#include "c1/c1_ValueStack.hpp"
#include "ci/ciArrayKlass.hpp"
#include "ci/ciInstance.hpp"
#include "gc_interface/collectedHeap.hpp"
#include "memory/barrierSet.hpp"
#include "memory/cardTableModRefBS.hpp"
#include "nativeInst_mips.hpp"
#include "oops/objArrayKlass.hpp"
#include "runtime/sharedRuntime.hpp"
#define __ _masm->

static void select_different_registers(Register preserve,
                                       Register extra,
                                       Register &tmp1,
                                       Register &tmp2) {
  if (tmp1 == preserve) {
    assert_different_registers(tmp1, tmp2, extra);
    tmp1 = extra;
  } else if (tmp2 == preserve) {
    assert_different_registers(tmp1, tmp2, extra);
    tmp2 = extra;
  }
  assert_different_registers(preserve, tmp1, tmp2);
}



static void select_different_registers(Register preserve,
                                       Register extra,
                                       Register &tmp1,
                                       Register &tmp2,
                                       Register &tmp3) {
  if (tmp1 == preserve) {
    assert_different_registers(tmp1, tmp2, tmp3, extra);
    tmp1 = extra;
  } else if (tmp2 == preserve) {
    tmp2 = extra;
  } else if (tmp3 == preserve) {
    assert_different_registers(tmp1, tmp2, tmp3, extra);
    tmp3 = extra;
  }
  assert_different_registers(preserve, tmp1, tmp2, tmp3);
}

// need add method Assembler::is_simm16 in assembler_gs2.hpp
bool LIR_Assembler::is_small_constant(LIR_Opr opr) {
  if (opr->is_constant()) {
    LIR_Const* constant = opr->as_constant_ptr();
    switch (constant->type()) {
      case T_INT: {
        jint value = constant->as_jint();
        return Assembler::is_simm16(value);
      }
      default:
        return false;
    }
  }
  return false;
}

//FIXME, which register should be used?
LIR_Opr LIR_Assembler::receiverOpr() {
  return FrameMap::_t0_oop_opr;
}

LIR_Opr LIR_Assembler::osrBufferPointer() {
#ifdef _LP64
  Register r = receiverOpr()->as_register();
  return FrameMap::as_long_opr(r, r);
#else
  return FrameMap::as_opr(receiverOpr()->as_register());
#endif
}

//--------------fpu register translations-----------------------
// FIXME:I do not know what's to do for mips fpu

address LIR_Assembler::float_constant(float f) {
  address const_addr = __ float_constant(f);
  if (const_addr == NULL) {
    bailout("const section overflow");
    return __ code()->consts()->start();
  } else {
    return const_addr;
  }
}


address LIR_Assembler::double_constant(double d) {
  address const_addr = __ double_constant(d);
  if (const_addr == NULL) {
    bailout("const section overflow");
    return __ code()->consts()->start();
  } else {
    return const_addr;
  }
}





void LIR_Assembler::reset_FPU() {
  Unimplemented();
}


void LIR_Assembler::set_24bit_FPU() {
  Unimplemented();
}

//FIXME.
void LIR_Assembler::fpop() {
  // do nothing
}
void LIR_Assembler::fxch(int i) {
  // do nothing
}
void LIR_Assembler::fld(int i) {
  // do nothing
}
void LIR_Assembler::ffree(int i) {
  // do nothing
}

void LIR_Assembler::breakpoint() {
  __ brk(17);
}
//FIXME, opr can not be float?
void LIR_Assembler::push(LIR_Opr opr) {
  if (opr->is_single_cpu()) {
    __ push_reg(opr->as_register());
  } else if (opr->is_double_cpu()) {
    __ push_reg(opr->as_register_hi());
    __ push_reg(opr->as_register_lo());
  } else if (opr->is_stack()) {
    __ push_addr(frame_map()->address_for_slot(opr->single_stack_ix()));
  } else if (opr->is_constant()) {
    LIR_Const* const_opr = opr->as_constant_ptr();
    if (const_opr->type() == T_OBJECT) {
      __ push_oop(const_opr->as_jobject());
    } else if (const_opr->type() == T_INT) {
      __ push_jint(const_opr->as_jint());
    } else {
      ShouldNotReachHere();
    }
  } else {
    ShouldNotReachHere();
  }
}

void LIR_Assembler::pop(LIR_Opr opr) {
  if (opr->is_single_cpu() ) {
    __ pop(opr->as_register());
  } else {
    assert(false, "Must be single word register or floating-point register");
  }
}


Address LIR_Assembler::as_Address(LIR_Address* addr) {
  Unimplemented();
}

// Reset base register
Address LIR_Assembler::rebase_Address(LIR_Address* addr, Register disp) {
  bool disp_rebased = false;

  if (addr->base()->is_illegal()) {
    Unimplemented();
    assert(addr->index()->is_illegal(), "must be illegal too");
    AddressLiteral laddr((address)addr->disp(), relocInfo::none);
  }

  Register base = addr->base()->as_pointer_register();
  if (disp != NOREG) {
    __ addu(disp, base, disp);
    disp_rebased = true;
  }

  if (addr->index()->is_illegal()) {
    return Address(disp_rebased ? disp : base, disp_rebased ? 0 : addr->disp());
  } else if (addr->index()->is_cpu_register()) {
    Register index = addr->index()->as_pointer_register();
    return Address(disp_rebased ? disp : base, index, (Address::ScaleFactor) addr->scale(), disp_rebased ? 0 : addr->disp());
  } else if (addr->index()->is_constant()) {
    intptr_t addr_offset = (addr->index()->as_constant_ptr()->as_jint() << addr->scale());
    if (!disp_rebased) {
      addr_offset += addr->disp();
    }
    assert(Assembler::is_simm32(addr_offset), "must be");
    return Address(disp_rebased ? disp : base, addr_offset);
  } else {
    Unimplemented();
  }

  // fall through
  ShouldNotReachHere();
  return Address();
}


Address LIR_Assembler::as_Address_lo(LIR_Address* addr) {
  Unimplemented();
}


Address LIR_Assembler::as_Address_hi(LIR_Address* addr) {
  Unimplemented();
}


//void LIR_Assembler::osr_entry(IRScope* scope, int number_of_locks, Label* continuation, int osr_bci) {
void LIR_Assembler::osr_entry() {
  //  assert(scope->is_top_scope(), "inlined OSR not yet implemented");
  offsets()->set_value(CodeOffsets::OSR_Entry, code_offset());
  BlockBegin* osr_entry = compilation()->hir()->osr_entry();
  ValueStack* entry_state = osr_entry->state();
  int number_of_locks = entry_state->locks_size();

  // we jump here if osr happens with the interpreter
  // state set up to continue at the beginning of the
  // loop that triggered osr - in particular, we have
  // the following registers setup:
  //
  // S7: interpreter locals pointer
  // V1: interpreter locks pointer
  // RA: return address
  //T0: OSR buffer
  // build frame
  // ciMethod* m = scope->method();
  ciMethod* m = compilation()->method();
  __ build_frame(initial_frame_size_in_bytes(), bang_size_in_bytes());

  // OSR buffer is
  //
  // locals[nlocals-1..0]
  // monitors[0..number_of_locks]
  //
  // locals is a direct copy of the interpreter frame so in the osr buffer
  // so first slot in the local array is the last local from the interpreter
  // and last slot is local[0] (receiver) from the interpreter
  //
  // Similarly with locks. The first lock slot in the osr buffer is the nth lock
  // from the interpreter frame, the nth lock slot in the osr buffer is 0th lock
  // in the interpreter frame (the method lock if a sync method)

  // Initialize monitors in the compiled activation.
  //   T0: pointer to osr buffer
  //
  // All other registers are dead at this point and the locals will be
  // copied into place by code emitted in the IR.

  Register OSR_buf = osrBufferPointer()->as_pointer_register();


  // note: we do osr only if the expression stack at the loop beginning is empty,
  //       in which case the spill area is empty too and we don't have to setup
  //       spilled locals
  //
  // copy monitors
  // V1: pointer to locks
  {
    assert(frame::interpreter_frame_monitor_size() == BasicObjectLock::size(), "adjust code below");
    int monitor_offset = BytesPerWord * method()->max_locals()+
      (BasicObjectLock::size() * BytesPerWord) * (number_of_locks - 1);
    for (int i = 0; i < number_of_locks; i++) {
      int slot_offset =monitor_offset - (i * BasicObjectLock::size())*BytesPerWord;
#ifdef ASSERT
      {
        Label L;
        //__ lw(AT, V1, slot_offset * BytesPerWord + BasicObjectLock::obj_offset_in_bytes());
        __ ld_ptr(AT, OSR_buf, slot_offset + BasicObjectLock::obj_offset_in_bytes());
        __ bne(AT, R0, L);
        __ delayed()->nop();
        __ stop("locked object is NULL");
        __ bind(L);
      }
#endif
      __ ld_ptr(AT, OSR_buf, slot_offset + BasicObjectLock::lock_offset_in_bytes());
      __ st_ptr(AT, frame_map()->address_for_monitor_lock(i));
      __ ld_ptr(AT, OSR_buf, slot_offset + BasicObjectLock::obj_offset_in_bytes());
      __ st_ptr(AT, frame_map()->address_for_monitor_object(i));
    }
  }
}


int LIR_Assembler::check_icache() {
  Register receiver = FrameMap::receiver_opr->as_register();
  Register ic_klass = IC_Klass;

  int offset = __ offset();
  __ inline_cache_check(receiver, IC_Klass);
  __ align(CodeEntryAlignment);
  return offset;


}

void LIR_Assembler::jobject2reg_with_patching(Register reg, CodeEmitInfo* info) {
  jobject o = NULL;
  int oop_index = __ oop_recorder()->allocate_oop_index(o);
  PatchingStub* patch = new PatchingStub(_masm, patching_id(info), oop_index);
  RelocationHolder rspec = oop_Relocation::spec(oop_index);
  __ relocate(rspec);
#ifndef _LP64
  __ lui(reg, Assembler::split_high((int)o));
  __ addiu(reg, reg, Assembler::split_low((int)o));
#else
//li may not pass NativeMovConstReg::verify. see nativeMovConstReg_at(pc_start()); in PatchingStub::install.
  __ li48(reg, (long)o);
#endif
  // patching_epilog(patch, LIR_Op1::patch_normal, noreg, info);
  patching_epilog(patch, lir_patch_normal, reg, info);
}

void LIR_Assembler::klass2reg_with_patching(Register reg, CodeEmitInfo* info) {
  Metadata *o = NULL;
  int index = __ oop_recorder()->allocate_metadata_index(o);
  PatchingStub* patch = new PatchingStub(_masm, PatchingStub::load_klass_id, index);
  RelocationHolder rspec = metadata_Relocation::spec(index);
  __ relocate(rspec);
  __ li48(reg, (long)o);
  patching_epilog(patch, lir_patch_normal, reg, info);
}

int LIR_Assembler::initial_frame_size_in_bytes() const {
  // if rounding, must let FrameMap know!
  return (frame_map()->framesize() - (2*VMRegImpl::slots_per_word))  * VMRegImpl::stack_slot_size;
}

int LIR_Assembler::emit_exception_handler() {
  // if the last instruction is a call (typically to do a throw which
  // is coming at the end after block reordering) the return address
  // must still point into the code area in order to avoid assertion
  // failures when searching for the corresponding bci => add a nop
  // (was bug 5/14/1999 - gri)
  // Lazy deopt bug 4932387. If last instruction is a call then we
  // need an area to patch where we won't overwrite the exception
  // handler. This means we need 5 bytes. Could use a fat_nop
  // but since this never gets executed it doesn't really make
  // much difference.
  //
  for (int i = 0; i < (NativeCall::instruction_size/BytesPerInstWord + 1) ; i++ ) {
    __ nop();
  }

  // generate code for exception handler
  address handler_base = __ start_a_stub(exception_handler_size);
  if (handler_base == NULL) {
    // no enough space
    bailout("exception handler overflow");
    return -1;
  }

  int offset = code_offset();

  // the exception oop and pc are in V0, and V1
  // no other registers need to be preserved, so invalidate them
  //__ invalidate_registers(false, true, true, false, true, true);

  // check that there is really an exception
  __ verify_not_null_oop(V0);

  // search an exception handler (V0: exception oop, V1: throwing pc)
  __ call(Runtime1::entry_for(Runtime1::handle_exception_from_callee_id));
  __ delayed()->nop();
  __ should_not_reach_here();
  guarantee(code_offset() - offset <= exception_handler_size, "overflow");
  __ end_a_stub();

  return offset;
}

// Emit the code to remove the frame from the stack in the exception
// unwind path.
int LIR_Assembler::emit_unwind_handler() {
#ifndef PRODUCT
  if (CommentedAssembly) {
    _masm->block_comment("Unwind handler");
  }
#endif

  int offset = code_offset();
  // Fetch the exception from TLS and clear out exception related thread state
  Register thread = TREG;
#ifndef OPT_THREAD
  __ get_thread(thread);
#endif
  __ ld_ptr(V0, Address(thread, JavaThread::exception_oop_offset()));
  __ st_ptr(R0, Address(thread, JavaThread::exception_oop_offset()));
  __ st_ptr(R0, Address(thread, JavaThread::exception_pc_offset()));

  __ bind(_unwind_handler_entry);
  __ verify_not_null_oop(V0);
  if (method()->is_synchronized() || compilation()->env()->dtrace_method_probes()) {
    __ move(S0, V0);  // Preserve the exception (S0 is always callee-saved)
  }

  // Preform needed unlocking
  MonitorExitStub* stub = NULL;
  if (method()->is_synchronized()) {
    monitor_address(0, FrameMap::_v0_opr);
    stub = new MonitorExitStub(FrameMap::_v0_opr, true, 0);
    __ unlock_object(A0, A1, V0, *stub->entry());
    __ bind(*stub->continuation());
  }

  if (compilation()->env()->dtrace_method_probes()) {
    __ move(A0, thread);
    __ mov_metadata(A1, method()->constant_encoding());
    __ patchable_call(CAST_FROM_FN_PTR(address, SharedRuntime::dtrace_method_exit));
  }

  if (method()->is_synchronized() || compilation()->env()->dtrace_method_probes()) {
    __ move(V0, S0);  // Restore the exception
  }

  // remove the activation and dispatch to the unwind handler
  // leave activation of nmethod
  __ remove_frame(initial_frame_size_in_bytes());

  __ jmp(Runtime1::entry_for(Runtime1::unwind_exception_id));
  __ delayed()->nop();

  // Emit the slow path assembly
  if (stub != NULL) {
    stub->emit_code(this);
  }

  return offset;
}


int LIR_Assembler::emit_deopt_handler() {
  // if the last instruction is a call (typically to do a throw which
  // is coming at the end after block reordering) the return address
  // must still point into the code area in order to avoid assertion
  // failures when searching for the corresponding bci => add a nop
  // (was bug 5/14/1999 - gri)

   __ nop();

   // generate code for exception handler
  address handler_base = __ start_a_stub(deopt_handler_size);
  if (handler_base == NULL) {
    // not enough space left for the handler
    bailout("deopt handler overflow");
    return -1;
  }
  int offset = code_offset();

//  compilation()->offsets()->set_value(CodeOffsets::Deopt, code_offset());

  __ call(SharedRuntime::deopt_blob()->unpack());
  __ delayed()->nop();

  guarantee(code_offset() - offset <= deopt_handler_size, "overflow");
  __ end_a_stub();

  return offset;

}


// Optimized Library calls
// This is the fast version of java.lang.String.compare; it has not
// OSR-entry and therefore, we generate a slow version for OSR's
//void LIR_Assembler::emit_string_compare(IRScope* scope) {
void LIR_Assembler::emit_string_compare(LIR_Opr arg0, LIR_Opr arg1, LIR_Opr dst, CodeEmitInfo* info) {
  // get two string object in T0&T1
  //receiver already in T0
  __ ld_ptr(T1, arg1->as_register());
  //__ ld_ptr(T2, T0, java_lang_String::value_offset_in_bytes());  //value, T_CHAR array
  __ load_heap_oop(T2, Address(T0, java_lang_String::value_offset_in_bytes()));
  __ ld_ptr(AT, T0, java_lang_String::offset_offset_in_bytes());  //offset
  __ shl(AT, 1);
  __ addu(T2, T2, AT);
  __ addiu(T2, T2, arrayOopDesc::base_offset_in_bytes(T_CHAR));
  // Now T2 is the address of the first char in first string(T0)

  add_debug_info_for_null_check_here(info);
  //__ ld_ptr(T3, T1, java_lang_String::value_offset_in_bytes());
  __ load_heap_oop(T3, Address(T1, java_lang_String::value_offset_in_bytes()));
  __ ld_ptr(AT, T1, java_lang_String::offset_offset_in_bytes());
  __ shl(AT, 1);
  __ addu(T3, T3, AT);
  __ addiu(T3, T3, arrayOopDesc::base_offset_in_bytes(T_CHAR));
  // Now T3 is the address of the first char in second string(T1)

#ifndef _LP64
  // compute minimum length (in T4) and difference of lengths (V0)
  Label L;
  __ lw (T4, Address(T0, java_lang_String::count_offset_in_bytes()));
  // the length of the first string(T0)
  __ lw (T5, Address(T1, java_lang_String::count_offset_in_bytes()));
  // the length of the second string(T1)

  __ subu(V0, T4, T5);
  __ blez(V0, L);
  __ delayed()->nop();
  __ move (T4, T5);
  __ bind (L);

  Label Loop, haveResult, LoopEnd;
  __ bind(Loop);
  __ beq(T4, R0, LoopEnd);
  __ delayed();

  __ addiu(T2, T2, 2);

  // compare current character
  __ lhu(T5, T2, -2);
  __ lhu(T6, T3, 0);
  __ bne(T5, T6, haveResult);
  __ delayed();

  __ addiu(T3, T3, 2);

  __ b(Loop);
  __ delayed()->addiu(T4, T4, -1);

  __ bind(haveResult);
  __ subu(V0, T5, T6);

  __ bind(LoopEnd);
#else
  // compute minimum length (in T4) and difference of lengths (V0)
  Label L;
  __ lw (A4, Address(T0, java_lang_String::count_offset_in_bytes()));
  // the length of the first string(T0)
  __ lw (A5, Address(T1, java_lang_String::count_offset_in_bytes()));
  // the length of the second string(T1)

  __ dsubu(V0, A4, A5);
  __ blez(V0, L);
  __ delayed()->nop();
  __ move (A4, A5);
  __ bind (L);

  Label Loop, haveResult, LoopEnd;
  __ bind(Loop);
  __ beq(A4, R0, LoopEnd);
  __ delayed();

  __ daddiu(T2, T2, 2);

  // compare current character
  __ lhu(A5, T2, -2);
  __ lhu(A6, T3, 0);
  __ bne(A5, A6, haveResult);
  __ delayed();

  __ daddiu(T3, T3, 2);

  __ b(Loop);
  __ delayed()->addiu(A4, A4, -1);

  __ bind(haveResult);
  __ dsubu(V0, A5, A6);

  __ bind(LoopEnd);
#endif
  return_op(FrameMap::_v0_opr);
}


void LIR_Assembler::return_op(LIR_Opr result) {
  assert(result->is_illegal() || !result->is_single_cpu() || result->as_register() == V0, "word returns are in V0");
  // Pop the stack before the safepoint code
  __ remove_frame(initial_frame_size_in_bytes());
#ifndef _LP64
  __ lui(AT, Assembler::split_high((intptr_t)os::get_polling_page()
  + (SafepointPollOffset % os::vm_page_size())));
  __ relocate(relocInfo::poll_return_type);
  __ lw(AT, AT, Assembler::split_low((intptr_t)os::get_polling_page()
  + (SafepointPollOffset % os::vm_page_size())));
#else
  #ifndef OPT_SAFEPOINT
  __ li48(AT, (intptr_t)os::get_polling_page() + (SafepointPollOffset % os::vm_page_size()));
  __ relocate(relocInfo::poll_return_type);
  __ lw(AT, AT, 0);
  #else
  __ lui(AT, Assembler::split_high((intptr_t)os::get_polling_page() + (SafepointPollOffset % os::vm_page_size())));
  __ relocate(relocInfo::poll_return_type);
  __ lw(AT, AT, Assembler::split_low((intptr_t)os::get_polling_page() + (SafepointPollOffset % os::vm_page_size())));
  #endif
#endif

  __ jr(RA);
  __ delayed()->nop();
}

//read protect mem to R0 won't cause the exception only in godson-2e, So I modify R0 to AT.
int LIR_Assembler::safepoint_poll(LIR_Opr tmp, CodeEmitInfo* info) {
  assert(info != NULL, "info must not be null for safepoint poll");
  int offset = __ offset();
  Register r = tmp->as_register();
#ifndef _LP64
  __ lui(r, Assembler::split_high((intptr_t)os::get_polling_page() + (SafepointPollOffset % os::vm_page_size())));
  add_debug_info_for_branch(info);
  __ relocate(relocInfo::poll_type);
  __ lw(AT, r, Assembler::split_low((intptr_t)os::get_polling_page() + (SafepointPollOffset % os::vm_page_size())));
#else
  #ifndef OPT_SAFEPOINT
  __ li48(r, (intptr_t)os::get_polling_page() + (SafepointPollOffset % os::vm_page_size()));
  add_debug_info_for_branch(info);
  __ relocate(relocInfo::poll_type);
  __ lw(AT, r, 0);
  #else
  __ lui(r, Assembler::split_high((intptr_t)os::get_polling_page() + (SafepointPollOffset % os::vm_page_size())));
  add_debug_info_for_branch(info);
  __ relocate(relocInfo::poll_type);
  __ lw(AT, r, Assembler::split_low((intptr_t)os::get_polling_page() + (SafepointPollOffset % os::vm_page_size())));
  #endif
#endif
  return offset;
}

void LIR_Assembler::move_regs(Register from_reg, Register to_reg) {
  if (from_reg != to_reg) __ move(to_reg, from_reg);
}


void LIR_Assembler::swap_reg(Register a, Register b) {
  __ xorr(a, a, b);
  __ xorr(b, a, b);
  __ xorr(a, a, b);
}

void LIR_Assembler::const2reg(LIR_Opr src, LIR_Opr dest, LIR_PatchCode patch_code, CodeEmitInfo* info) {
  assert(src->is_constant(), "should not call otherwise");
  assert(dest->is_register(), "should not call otherwise");
  LIR_Const* c = src->as_constant_ptr();
  switch (c->type()) {
    case T_ADDRESS: {
      assert(patch_code == lir_patch_none, "no patching handled here");
      Unimplemented();
      __ move(dest->as_register(), c->as_jint()); // FIXME
      break;
    }

    case T_INT: {
      assert(patch_code == lir_patch_none, "no patching handled here");
      __ move(dest->as_register(), c->as_jint());
      break;
    }

    case T_LONG: {
#ifndef _LP64
      jlong con = c->as_jlong();
      jint* conhi = (jint*)&con + 1;
      jint* conlow = (jint*)&con;

      if (dest->is_double_cpu()) {
        __ move(dest->as_register_lo(), *conlow);
        __ move(dest->as_register_hi(), *conhi);
      } else {
        //  assert(dest->is_double(), "wrong register kind");
        __ move(AT, *conlow);
        __ mtc1(AT, dest->as_double_reg());
        __ move(AT, *conhi);
        __ mtc1(AT, dest->as_double_reg()+1);
      }
#else
      if (dest->is_double_cpu()) {
        __ li(dest->as_register_lo(), c->as_jlong());
      } else {
        __ li(dest->as_register(), c->as_jlong());
      }
#endif
      break;
    }

    case T_OBJECT: {
      if (patch_code == lir_patch_none) {
        jobject2reg(c->as_jobject(), dest->as_register());
      } else {
        jobject2reg_with_patching(dest->as_register(), info);
      }
      break;
    }

    case T_METADATA: {
      if (patch_code != lir_patch_none) {
        klass2reg_with_patching(dest->as_register(), info);
      } else {
        __ mov_metadata(dest->as_register(), c->as_metadata());
      }
      break;
    }

    case T_FLOAT: {
      address const_addr = float_constant(c->as_jfloat());
      assert (const_addr != NULL, "must create float constant in the constant table");

      if (dest->is_single_fpu()) {
        __ relocate(relocInfo::internal_pc_type);
#ifndef _LP64
        __ lui(AT, Assembler::split_high((int)const_addr));
        __ addiu(AT, AT, Assembler::split_low((int)const_addr));
#else
        __ li48(AT, (long)const_addr);
#endif
        __ lwc1(dest->as_float_reg(), AT, 0);

      } else {
        assert(dest->is_single_cpu(), "Must be a cpu register.");
        assert(dest->as_register() != AT, "AT can not be allocated.");

        __ relocate(relocInfo::internal_pc_type);
#ifndef _LP64
        __ lui(AT, Assembler::split_high((int)const_addr));
        __ addiu(AT, AT, Assembler::split_low((int)const_addr));
#else
        __ li48(AT, (long)const_addr);
#endif
        __ lw(dest->as_register(), AT, 0);
      }
      break;
    }

    case T_DOUBLE: {
      address const_addr = double_constant(c->as_jdouble());
      assert (const_addr != NULL, "must create double constant in the constant table");

      if (dest->is_double_fpu()) {
        __ relocate(relocInfo::internal_pc_type);
#ifndef _LP64
        __ lui(AT, Assembler::split_high((int)const_addr));
        __ addiu(AT, AT, Assembler::split_low((int)const_addr));
        __ lwc1(dest->as_double_reg(), AT, 0);
        __ lwc1(dest->as_double_reg()+1, AT, 4);
#else
        __ li48(AT, (long)const_addr);
        __ ldc1(dest->as_double_reg(), AT, 0);
#endif
      } else {
        assert(dest->as_register_lo() != AT, "AT can not be allocated.");
        assert(dest->as_register_hi() != AT, "AT can not be allocated.");

        __ relocate(relocInfo::internal_pc_type);
#ifndef _LP64
        __ lui(AT, Assembler::split_high((int)const_addr));
        __ addiu(AT, AT, Assembler::split_low((int)const_addr));
        __ lw(dest->as_register_lo(), AT, 0);
        __ lw(dest->as_register_hi(), AT, 4);
#else
        __ li48(AT, (long)const_addr);
        __ ld(dest->as_register_lo(), AT, 0);
#endif
      }
      break;
    }

    default:
      ShouldNotReachHere();
  }
}

void LIR_Assembler::const2stack(LIR_Opr src, LIR_Opr dest) {
  assert(src->is_constant(), "should not call otherwise");
  assert(dest->is_stack(), "should not call otherwise");
  LIR_Const* c = src->as_constant_ptr();
  switch (c->type()) {
    case T_INT:  // fall through
      __ move(AT, c->as_jint_bits());
      __ sw(AT, frame_map()->address_for_slot(dest->single_stack_ix()));
      break;

    case T_FLOAT:
      Unimplemented();
      break;

    case T_ADDRESS:
      Unimplemented();
      __ move(AT, c->as_jint_bits());
      __ st_ptr(AT, frame_map()->address_for_slot(dest->single_stack_ix()));
      break;

    case T_OBJECT:
      if (c->as_jobject() == NULL) {
        __ st_ptr(R0, frame_map()->address_for_slot(dest->single_stack_ix()));
      } else {
        int oop_index = __ oop_recorder()->find_index(c->as_jobject());
        RelocationHolder rspec = oop_Relocation::spec(oop_index);
        __ relocate(rspec);
#ifndef _LP64
        __ lui(AT, Assembler::split_high((int)c->as_jobject()));
        __ addiu(AT, AT, Assembler::split_low((int)c->as_jobject()));
#else
        __ li48(AT, (long)c->as_jobject());
#endif
        __ st_ptr(AT, frame_map()->address_for_slot(dest->single_stack_ix()));
      }
      break;
    case T_LONG:  // fall through
    case T_DOUBLE:
#ifndef _LP64
      __ move(AT, c->as_jint_lo_bits());
      __ sw(AT, frame_map()->address_for_slot(dest->double_stack_ix(),
      lo_word_offset_in_bytes));
      __ move(AT, c->as_jint_hi_bits());
      __ sw(AT, frame_map()->address_for_slot(dest->double_stack_ix(),
      hi_word_offset_in_bytes));
#else
      __ move(AT, c->as_jlong_bits());
      __ sd(AT, frame_map()->address_for_slot(dest->double_stack_ix(),
      lo_word_offset_in_bytes));
#endif
      break;
    default:
      ShouldNotReachHere();
  }
}

void LIR_Assembler::const2mem(LIR_Opr src, LIR_Opr dest, BasicType type, CodeEmitInfo* info, bool wide) {
  assert(src->is_constant(), "should not call otherwise");
  assert(dest->is_address(), "should not call otherwise");
  LIR_Const* c = src->as_constant_ptr();
  LIR_Address* addr = dest->as_address_ptr();

  int null_check_here = code_offset();
  switch (type) {
    case T_LONG: // fall through
    case T_DOUBLE:
#ifndef _LP64
      Unimplemented();
      __ move(AT, c->as_jint_hi_bits());
      __ sw(AT, as_Address_hi(addr));
      __ move(AT, c->as_jint_lo_bits());
      __ sw(AT, as_Address_lo(addr));
#else
      if (c->as_jlong_bits() != 0) {
        // DoublePrint: -0.0
        //   (gdb) print /x -9223372036854775808
        //   $1 = 0x8000000000000000
        //
        __ li64(AT, c->as_jlong_bits());
        __ store_for_type(AT, rebase_Address(addr), T_LONG);
      } else {
        __ store_for_type(R0, rebase_Address(addr), T_LONG);
      }
#endif
      break;
    case T_OBJECT:  // fall through
    case T_ARRAY:
      if (c->as_jobject() == NULL){
        __ store_for_type(R0, rebase_Address(addr), T_OBJECT, wide);
      } else {
        int oop_index = __ oop_recorder()->find_index(c->as_jobject());
        RelocationHolder rspec = oop_Relocation::spec(oop_index);
        __ relocate(rspec);
#ifndef _LP64
        Unimplemented();
        __ lui(AT, Assembler::split_high((int)c->as_jobject()));
        __ addiu(AT, AT, Assembler::split_low((int)c->as_jobject()));
        __ st_ptr(AT, as_Address(addr));
        null_check_here = code_offset();
#else
        __ li64(AT, (long)c->as_jobject());
        if (UseCompressedOops && !wide) {
          __ encode_heap_oop(AT);
          null_check_here = code_offset();
        }
        __ store_for_type(AT, rebase_Address(addr), T_OBJECT, wide);
#endif
      }
      break;
    case T_INT:     // fall through
    case T_FLOAT:
      if(c->as_jint_bits() != 0) {
        __ move(AT, c->as_jint_bits());
        __ store_for_type(AT, rebase_Address(addr));
      } else
        __ store_for_type(R0, rebase_Address(addr));
      break;
    case T_ADDRESS:
      __ move(AT, c->as_jint_bits());
      __ store_for_type(AT, rebase_Address(addr), T_ADDRESS);
      break;
    case T_BOOLEAN: // fall through
    case T_BYTE:
      if(c->as_jint() != 0) {
        __ move(AT, c->as_jint());
        __ store_for_type(AT, rebase_Address(addr), T_BYTE);
      } else {
        __ store_for_type(R0, rebase_Address(addr), T_BYTE);
      }
      break;
    case T_CHAR:    // fall through
    case T_SHORT:
      if(c->as_jint() != 0) {
        __ move(AT, c->as_jint());
        __ store_for_type(AT, rebase_Address(addr), T_SHORT);
      } else {
        __ store_for_type(R0, rebase_Address(addr), T_SHORT);
      }
      break;
    default: ShouldNotReachHere();
  };
  if (info != NULL) add_debug_info_for_null_check(null_check_here, info);
}

void LIR_Assembler::reg2reg(LIR_Opr src, LIR_Opr dest) {
  assert(src->is_register(), "should not call otherwise");
  assert(dest->is_register(), "should not call otherwise");
  if (dest->is_float_kind() && src->is_float_kind()) {
  // float to float moves
    if (dest->is_single_fpu()) {
      assert(src->is_single_fpu(), "must both be float");
      __ mov_s(dest->as_float_reg(), src->as_float_reg());
    } else {
      assert(src->is_double_fpu(), "must bothe be double");
      __ mov_d( dest->as_double_reg(),src->as_double_reg());
    }
  } else if (!dest->is_float_kind() && !src->is_float_kind()) {
    // int to int moves
    if (dest->is_single_cpu()) {
#ifdef _LP64
      if (src->type() == T_LONG) {
        // Can do LONG -> OBJECT
        move_regs(src->as_register_lo(), dest->as_register());
        return;
      }
#endif
      assert(src->is_single_cpu(), "must match");
      if (src->type() == T_OBJECT) {
        __ verify_oop(src->as_register());
      }
      move_regs(src->as_register(), dest->as_register());

    } else if (dest->is_double_cpu()) {
#ifdef _LP64
      if (src->type() == T_OBJECT || src->type() == T_ARRAY) {
        // Surprising to me but we can see move of a long to t_object
        __ verify_oop(src->as_register());
        move_regs(src->as_register(), dest->as_register_lo());
        return;
      }
#endif
      Register f_lo;
      Register f_hi;
      Register t_lo;
      Register t_hi;

      if (src->is_single_cpu()) {
        f_lo = src->as_register();
        t_lo = dest->as_register_lo();
      } else {
        f_lo = src->as_register_lo();
        f_hi = src->as_register_hi();
        t_lo = dest->as_register_lo();
        t_hi = dest->as_register_hi();
        assert(f_hi == f_lo, "must be same");
        assert(t_hi == t_lo, "must be same");
      }
#ifdef _LP64
      move_regs(f_lo, t_lo);
#else
      assert(f_lo != f_hi && t_lo != t_hi, "invalid register allocation");

      if (f_lo == t_hi && f_hi == t_lo) {
        swap_reg(f_lo, f_hi);
      } else if (f_hi == t_lo) {
        assert(f_lo != t_hi, "overwriting register");
        move_regs(f_hi, t_hi);
        move_regs(f_lo, t_lo);
      } else {
        assert(f_hi != t_lo, "overwriting register");
        move_regs(f_lo, t_lo);
        move_regs(f_hi, t_hi);
      }
#endif // LP64
    }
  } else {
    // float to int or int to float moves
    if (dest->is_double_cpu()) {
      assert(src->is_double_fpu(), "must match");
      __ mfc1(dest->as_register_lo(), src->as_double_reg());
#ifndef _LP64
      __ mfc1(dest->as_register_hi(), src->as_double_reg() + 1);
#endif
    } else if (dest->is_single_cpu()) {
      assert(src->is_single_fpu(), "must match");
      __ mfc1(dest->as_register(), src->as_float_reg());
    } else if (dest->is_double_fpu()) {
      assert(src->is_double_cpu(), "must match");
      __ mtc1(src->as_register_lo(), dest->as_double_reg());
#ifndef _LP64
      __ mtc1(src->as_register_hi(), dest->as_double_reg() + 1);
#endif
    } else if (dest->is_single_fpu()) {
      assert(src->is_single_cpu(), "must match");
      __ mtc1(src->as_register(), dest->as_float_reg());
    }
  }
}


void LIR_Assembler::reg2stack(LIR_Opr src, LIR_Opr dest, BasicType type, bool pop_fpu_stack) {
  assert(src->is_register(), "should not call otherwise");
  assert(dest->is_stack(), "should not call otherwise");

  if (src->is_single_cpu()) {
    Address dst = frame_map()->address_for_slot(dest->single_stack_ix());
    if (type == T_ARRAY || type == T_OBJECT) {
      __ verify_oop(src->as_register());
      __ st_ptr(src->as_register(), dst);
    } else if (type == T_METADATA || type == T_DOUBLE) {
      __ st_ptr(src->as_register(), dst);
    } else {
      __ sw(src->as_register(), dst);
    }
  } else if (src->is_double_cpu()) {
    Address dstLO = frame_map()->address_for_slot(dest->double_stack_ix(), lo_word_offset_in_bytes);
    Address dstHI = frame_map()->address_for_slot(dest->double_stack_ix(), hi_word_offset_in_bytes);
     __ st_ptr(src->as_register_lo(), dstLO);
     NOT_LP64(__ st_ptr(src->as_register_hi(), dstHI));
  } else if (src->is_single_fpu()) {
    Address dst_addr = frame_map()->address_for_slot(dest->single_stack_ix());
    __ swc1(src->as_float_reg(), dst_addr);

  } else if (src->is_double_fpu()) {
    Address dst_addr = frame_map()->address_for_slot(dest->double_stack_ix());
#ifndef _LP64
    __ swc1(src->as_double_reg(), dst_addr);
    __ swc1(src->as_double_reg() + 1, dst_addr.base(), dst_addr.disp() + 4);
#else
    __ sdc1(src->as_double_reg(), dst_addr);
#endif

  } else {
    ShouldNotReachHere();
  }
}


void LIR_Assembler::reg2mem(LIR_Opr src, LIR_Opr dest, BasicType type, LIR_PatchCode patch_code, CodeEmitInfo* info,bool pop_fpu_stack, bool wide,  bool/*unaliged*/) {
  LIR_Address* to_addr = dest->as_address_ptr();

  Register base_reg = to_addr->base()->as_pointer_register();

  PatchingStub* patch = NULL;
  bool needs_patching = (patch_code != lir_patch_none);
  Register disp_reg = NOREG;
  int disp_value = to_addr->disp();
  //
  //  the start position of patch template is labeled by "new PatchingStub(...)"
  //  during patch, T9 will be changed and not restore
  //  that's why we use S7 but not T9 as compressed_src here
  //
  Register compressed_src = S7;

  if (type == T_ARRAY || type == T_OBJECT) {
    __ verify_oop(src->as_register());
#ifdef _LP64
    if (UseCompressedOops && !wide) {
      __ move(compressed_src, src->as_register());
      __ encode_heap_oop(compressed_src);
    }
#endif
  }

  if (needs_patching) {
    patch = new PatchingStub(_masm, PatchingStub::access_field_id);
    assert(!src->is_double_cpu() ||
        patch_code == lir_patch_none ||
        patch_code == lir_patch_normal,
        "patching doesn't match register");
    Address toa = rebase_Address(to_addr);
    assert(toa.disp() != 0, "must have");
  }

  if (info != NULL) {
    add_debug_info_for_null_check_here(info);
  }
  if (needs_patching) {
    disp_reg = T9;
    __ lui(disp_reg, Assembler::split_high(disp_value));
    __ ori(disp_reg, disp_reg, Assembler::split_low(disp_value));
  }
  int offset = code_offset();

  switch(type) {
    case T_DOUBLE:
      assert(src->is_double_fpu(), "just check");
#ifndef _LP64
      Unimplemented();
      if (disp_reg == noreg) {
        __ swc1(src->as_double_reg(), base_reg, disp_value);
        __ swc1(src->as_double_reg() + 1, base_reg, disp_value + 4);
      } else if (needs_patching) {
        __ addu(AT, base_reg, disp_reg);
        __ swc1(src->as_double_reg(), AT, 0);
        __ swc1(src->as_double_reg() + 1, AT, 4);
#else
      if (needs_patching) {
        __ store_for_type(src->as_double_reg(), rebase_Address(to_addr, disp_reg), T_DOUBLE);
#endif
      } else {
#ifndef _LP64
        Unimplemented();
        __ addu(AT, base_reg, disp_reg);
        __ swc1(src->as_double_reg(), AT, Assembler::split_low(disp_value));
        __ swc1(src->as_double_reg() + 1, AT, Assembler::split_low(disp_value) + 4);
#else
        __ store_for_type(src->as_double_reg(), rebase_Address(to_addr), T_DOUBLE);
#endif
      }
      break;

    case T_FLOAT:
      if (needs_patching) {
        __ store_for_type(src->as_float_reg(), rebase_Address(to_addr, disp_reg), T_FLOAT);
      } else {
        __ store_for_type(src->as_float_reg(), rebase_Address(to_addr), T_FLOAT);
      }
      break;

    case T_LONG: {
      Register from_lo = src->as_register_lo();
      Register from_hi = src->as_register_hi();
#ifdef _LP64
      if (needs_patching) {
        __ store_for_type(from_lo, rebase_Address(to_addr, disp_reg), T_LONG);
      } else {
        __ store_for_type(from_lo, rebase_Address(to_addr), T_LONG);
      }
#else
      Unimplemented();
      Register base = to_addr->base()->as_register();
      Register index = noreg;
      if (to_addr->index()->is_register()) {
        index = to_addr->index()->as_register();
      }
      if (base == from_lo || index == from_lo) {
        assert(base != from_hi, "can't be");
        assert(index == noreg || (index != base && index != from_hi), "can't handle this");
        if (needs_patching) {
          __ addu(AT, base_reg, disp_reg);
          NOT_LP64(__ st_ptr(from_hi, AT, longSize/2);)
            __ st_ptr(from_lo, AT, 0);
        } else {
          __ st_ptr(from_hi, as_Address_hi(to_addr));
          __ st_ptr(from_lo, as_Address_lo(to_addr));
         }
      } else {
        assert(index == noreg || (index != base && index != from_lo), "can't handle this");
        if (needs_patching) {
          __ addu(AT, base_reg, disp_reg);
          __ st_ptr(from_lo, AT, 0);
          __ st_ptr(from_hi, AT, longSize/2);
        } else {
          __ st_ptr(from_lo, as_Address_lo(to_addr));
          __ st_ptr(from_hi, as_Address_hi(to_addr));
        }
      }
#endif
      break;
    }
    case T_ARRAY:
    case T_OBJECT:
#ifdef _LP64
      if (UseCompressedOops && !wide) {
        if (needs_patching) {
          __ store_for_type(compressed_src, rebase_Address(to_addr, disp_reg), T_OBJECT, wide);
        } else {
          __ store_for_type(compressed_src, rebase_Address(to_addr), T_OBJECT, wide);
        }
      } else {
        if (needs_patching) {
          __ store_for_type(src->as_register(), rebase_Address(to_addr, disp_reg), T_OBJECT, wide);
        } else {
          __ store_for_type(src->as_register(), rebase_Address(to_addr), T_OBJECT, wide);
        }
      }
      break;
#endif
    case T_ADDRESS:
#ifdef _LP64
      if (needs_patching) {
        __ store_for_type(src->as_register(), rebase_Address(to_addr, disp_reg), T_ADDRESS);
      } else {
        __ store_for_type(src->as_register(), rebase_Address(to_addr), T_ADDRESS);
      }
      break;
#endif
    case T_INT:
      if (needs_patching) {
        __ store_for_type(src->as_register(), rebase_Address(to_addr, disp_reg));
      } else {
        __ store_for_type(src->as_register(), rebase_Address(to_addr));
      }
      break;

    case T_CHAR:
    case T_SHORT:
       if (needs_patching) {
         __ store_for_type(src->as_register(), rebase_Address(to_addr, disp_reg), T_SHORT);
       } else {
         __ store_for_type(src->as_register(), rebase_Address(to_addr), T_SHORT);
       }
       break;

    case T_BYTE:
    case T_BOOLEAN:
      assert(src->is_single_cpu(), "just check");

      if (needs_patching) {
        __ store_for_type(src->as_register(), rebase_Address(to_addr, disp_reg), T_BYTE);
      } else {
        __ store_for_type(src->as_register(), rebase_Address(to_addr), T_BYTE);
      }
      break;

    default:
      ShouldNotReachHere();
  }


  if (needs_patching) {
    patching_epilog(patch, patch_code, to_addr->base()->as_register(), info);
  }
}



void LIR_Assembler::stack2reg(LIR_Opr src, LIR_Opr dest, BasicType type) {
  assert(src->is_stack(), "should not call otherwise");
  assert(dest->is_register(), "should not call otherwise");

  if (dest->is_single_cpu()) {
#ifdef _LP64
    if (type == T_INT)
      __ lw(dest->as_register(), frame_map()->address_for_slot(src->single_stack_ix()));
    else
#endif
    __ ld_ptr(dest->as_register(), frame_map()->address_for_slot(src->single_stack_ix()));
    if (type == T_ARRAY || type == T_OBJECT) {
      __ verify_oop(dest->as_register());
    }
  } else if (dest->is_double_cpu()) {
#ifdef _LP64
  // java.util.concurrent.locks.ReentrantReadWriteLock$Sync::tryAcquire
  //
  //   88 move [stack:2|L] [a5a5|J]
  //   OpenJDK 64-Bit Client VM warning: /mnt/openjdk6-mips/hotspot/src/share/c1/c1_LIR.hpp, 397 , assert(is_double_stack() && !is_virtual(),"type check")
  //   OpenJDK 64-Bit Client VM warning: /mnt/openjdk6-mips/hotspot/src/share/c1/c1_LIR.hpp, 397 , assert(is_double_stack() && !is_virtual(),"type check")
  //   0x000000556197af8c: ld a5, 0x50(sp)

    Address src_addr_LO;
    if (src->is_single_stack())
       src_addr_LO = frame_map()->address_for_slot(src->single_stack_ix(),lo_word_offset_in_bytes);
    else if (src->is_double_stack())
       src_addr_LO = frame_map()->address_for_slot(src->double_stack_ix(),lo_word_offset_in_bytes);
    else
       ShouldNotReachHere();
#else
    Address src_addr_LO = frame_map()->address_for_slot(src->double_stack_ix(),lo_word_offset_in_bytes);
    Address src_addr_HI = frame_map()->address_for_slot(src->double_stack_ix(), hi_word_offset_in_bytes);
#endif
#ifdef _LP64
    if (src->type() == T_INT)
      __ lw(dest->as_register_lo(), src_addr_LO);
    else
#endif
    __ ld_ptr(dest->as_register_lo(), src_addr_LO);
    NOT_LP64(__ ld_ptr(dest->as_register_hi(), src_addr_HI));
  } else if (dest->is_single_fpu()) {
    Address addr = frame_map()->address_for_slot(src->single_stack_ix());
    __ lwc1(dest->as_float_reg(), addr);
  } else if (dest->is_double_fpu())  {
    Address src_addr_LO = frame_map()->address_for_slot(src->double_stack_ix(),lo_word_offset_in_bytes);
#ifndef _LP64
    Address src_addr_HI = frame_map()->address_for_slot(src->double_stack_ix(), hi_word_offset_in_bytes);
    __ lwc1(dest->as_double_reg(), src_addr_LO);
    __ lwc1(dest->as_double_reg()+1, src_addr_HI);
#else
    __ ldc1(dest->as_double_reg(), src_addr_LO);
#endif
  } else {
    ShouldNotReachHere();
    /*
    assert(dest->is_single_cpu(), "cannot be anything else but a single cpu");
    assert(type!= T_ILLEGAL, "Bad type in stack2reg")
    Address addr = frame_map()->address_for_slot(src->single_stack_ix());
    __ lw(dest->as_register(), addr);
    */
  }
}

void LIR_Assembler::stack2stack(LIR_Opr src, LIR_Opr dest, BasicType type) {
  if (src->is_single_stack()) {
    //
    //      [b.q.e.a.z::bw()]
    //      move [stack:15|L] [stack:17|L]
    //         0x00000055584e7cf4: lw at, 0x78(sp)  <--- error!
    //         0x00000055584e7cf8: sw at, 0x88(sp)
    //
    if (type == T_OBJECT) {
      __ ld(AT, frame_map()->address_for_slot(src ->single_stack_ix()));
      __ sd(AT, frame_map()->address_for_slot(dest->single_stack_ix()));
    } else {
      __ lw(AT, frame_map()->address_for_slot(src ->single_stack_ix()));
      __ sw(AT, frame_map()->address_for_slot(dest->single_stack_ix()));
    }
  } else if (src->is_double_stack()) {
#ifndef _LP64
    __ lw(AT, frame_map()->address_for_slot(src ->double_stack_ix()));
    __ sw(AT, frame_map()->address_for_slot(dest->double_stack_ix()));
    __ lw(AT, frame_map()->address_for_slot(src ->double_stack_ix(),4));
    __ sw(AT, frame_map()->address_for_slot(dest ->double_stack_ix(),4));
#else
    __ ld_ptr(AT, frame_map()->address_for_slot(src ->double_stack_ix()));
    __ st_ptr(AT, frame_map()->address_for_slot(dest->double_stack_ix()));
#endif
  } else {
    ShouldNotReachHere();
  }
}

// if patching needed, be sure the instruction at offset is a MoveMemReg
void LIR_Assembler::mem2reg(LIR_Opr src, LIR_Opr dest, BasicType type, LIR_PatchCode patch_code, CodeEmitInfo* info, bool wide, bool) {
  assert(src->is_address(), "should not call otherwise");
  assert(dest->is_register(), "should not call otherwise");
  LIR_Address* addr = src->as_address_ptr();

  Register src_reg = addr->base()->as_pointer_register();
  Register disp_reg = noreg;
  int disp_value = addr->disp();
  bool needs_patching = (patch_code != lir_patch_none);

  PatchingStub* patch = NULL;
  if (needs_patching) {
    patch = new PatchingStub(_masm, PatchingStub::access_field_id);
  }

  // we must use lui&addiu,
  if (needs_patching) {
    disp_reg = T9;
    __ lui(disp_reg, Assembler::split_high(disp_value));
    __ ori(disp_reg, disp_reg, Assembler::split_low(disp_value));
  }

  // remember the offset of the load.  The patching_epilog must be done
  // before the call to add_debug_info, otherwise the PcDescs don't get
  // entered in increasing order.
  int offset = code_offset();

  switch(type) {
    case T_BOOLEAN:
    case T_BYTE: {
      if (needs_patching) {
        offset = __ load_for_type(dest->as_register(), rebase_Address(addr, disp_reg), T_BYTE);
      } else {
        offset = __ load_for_type(dest->as_register(), rebase_Address(addr), T_BYTE);
      }
    }
    break;

    case T_CHAR: {
      if (needs_patching) {
        offset = __ load_for_type(dest->as_register(), rebase_Address(addr, disp_reg), T_CHAR);
      } else {
        offset = __ load_for_type(dest->as_register(), rebase_Address(addr), T_CHAR);
      }
    }
    break;

    case T_SHORT: {
      if (needs_patching) {
        offset = __ load_for_type(dest->as_register(), rebase_Address(addr, disp_reg), T_SHORT);
      } else {
        offset = __ load_for_type(dest->as_register(), rebase_Address(addr), T_SHORT);
      }
    }
    break;

    case T_OBJECT:
    case T_ARRAY:
      if (UseCompressedOops && !wide) {
        if (needs_patching) {
          offset = __ load_for_type(dest->as_register(), rebase_Address(addr, disp_reg), T_OBJECT, wide);
        } else {
          offset = __ load_for_type(dest->as_register(), rebase_Address(addr), T_OBJECT, wide);
        }
      } else {
        if (needs_patching) {
          offset = __ load_for_type(dest->as_register(), rebase_Address(addr, disp_reg), T_OBJECT, wide);
        } else {
          offset = __ load_for_type(dest->as_register(), rebase_Address(addr), T_OBJECT, wide);
        }
      }
      break;
    case T_ADDRESS:
      if (UseCompressedClassPointers && addr->disp() == oopDesc::klass_offset_in_bytes()) {
        if (needs_patching) {
          offset = __ load_for_type(dest->as_register(), rebase_Address(addr, disp_reg), T_ADDRESS);
        } else {
          offset = __ load_for_type(dest->as_register(), rebase_Address(addr), T_ADDRESS);
        }
      } else {
        if (needs_patching) {
          offset = __ load_for_type(dest->as_register(), rebase_Address(addr, disp_reg), T_ADDRESS);
        } else {
          offset = __ load_for_type(dest->as_register(), rebase_Address(addr), T_ADDRESS);
        }
      }
      break;
    case T_INT:  {
      if (needs_patching) {
        offset = __ load_for_type(dest->as_register(), rebase_Address(addr, disp_reg), T_INT);
      } else {
        offset = __ load_for_type(dest->as_register(), rebase_Address(addr), T_INT);
      }
    }
    break;

    case T_LONG: {
      Register to_lo = dest->as_register_lo();
      Register to_hi = dest->as_register_hi();
#ifdef _LP64
      if (needs_patching) {
        __ load_for_type(to_lo, rebase_Address(addr, disp_reg), T_LONG);
      } else {
        __ load_for_type(to_lo, rebase_Address(addr), T_LONG);
      }
#else
      Unimplemented();
      Register base = addr->base()->as_register();
      Register index = noreg;
      if (addr->index()->is_register()) {
        index = addr->index()->as_register();
      }
      if ((base == to_lo && index == to_hi) ||(base == to_hi && index == to_lo)) {
        // addresses with 2 registers are only formed as a result of
        // array access so this code will never have to deal with
        // patches or null checks.
        assert(info == NULL && patch == NULL, "must be");
        __ lea(to_hi, as_Address(addr));
        __ lw(to_lo, Address(to_hi));
        __ lw(to_hi, Address(to_hi, BytesPerWord));
      } else if (base == to_lo || index == to_lo) {
        assert(base != to_hi, "can't be");
        assert(index == noreg || (index != base && index != to_hi), "can't handle this");
        if (needs_patching) {
          __ addu(AT, src_reg, disp_reg);
          offset = code_offset();
          __ lw(to_hi, AT, longSize/2);
          __ lw(to_lo, AT, 0);
        } else {
          __ lw(to_hi, as_Address_hi(addr));
          __ lw(to_lo, as_Address_lo(addr));
        }
      } else {
        assert(index == noreg || (index != base && index != to_lo), "can't handle this");
        if (needs_patching) {
          __ addu(AT, src_reg, disp_reg);
          offset = code_offset();
          __ lw(to_lo, AT, 0);
          __ lw(to_hi, AT, longSize/2);
        } else {
          __ lw(to_lo, as_Address_lo(addr));
          __ lw(to_hi, as_Address_hi(addr));
        }
      }
#endif
    }
    break;

    case T_FLOAT: {
      if (needs_patching) {
        offset = __ load_for_type(dest->as_float_reg(), rebase_Address(addr, disp_reg), T_FLOAT);
      } else {
        offset = __ load_for_type(dest->as_float_reg(), rebase_Address(addr), T_FLOAT);
      }
    }
    break;

    case T_DOUBLE: {
#ifndef _LP64
      Unimplemented();
      if (disp_reg == noreg) {
        __ lwc1(dest->as_double_reg(), src_reg, disp_value);
        __ lwc1(dest->as_double_reg() + 1, src_reg, disp_value + 4);
      } else if (needs_patching) {
        offset = code_offset();
        __ addu(AT, src_reg, disp_reg);
        __ lwc1(dest->as_double_reg(), AT, 0);
        __ lwc1(dest->as_double_reg() + 1, AT, 4);
#else
      if (needs_patching) {
        offset = __ load_for_type(dest->as_double_reg(), rebase_Address(addr, disp_reg), T_DOUBLE);
#endif
      } else {
#ifndef _LP64
        Unimplemented();
        __ addu(AT, src_reg, disp_reg);
        __ lwc1(dest->as_double_reg(), AT, Assembler::split_low(disp_value));
        __ lwc1(dest->as_double_reg() + 1, AT, Assembler::split_low(disp_value) + 4);
#else
        offset = __ load_for_type(dest->as_double_reg(), rebase_Address(addr), T_DOUBLE);
#endif
      }
    }
    break;

    default:
      ShouldNotReachHere();
  }

  if (needs_patching) {
    patching_epilog(patch, patch_code, src_reg, info);
  }

  if (type == T_ARRAY || type == T_OBJECT) {
#ifdef _LP64
    if (UseCompressedOops && !wide) {
      __ decode_heap_oop(dest->as_register());
    }
#endif
    __ verify_oop(dest->as_register());
  } else if (type == T_ADDRESS && addr->disp() == oopDesc::klass_offset_in_bytes()) {
    if (UseCompressedClassPointers) {
      __ decode_klass_not_null(dest->as_register());
    }
  }
  if (info != NULL) add_debug_info_for_null_check(offset, info);
}


void LIR_Assembler::prefetchr(LIR_Opr src) {
  LIR_Address* addr = src->as_address_ptr();
  Address from_addr = rebase_Address(addr);
}


void LIR_Assembler::prefetchw(LIR_Opr src) {
}

NEEDS_CLEANUP; // This could be static?
Address::ScaleFactor LIR_Assembler::array_element_size(BasicType type) const {
  int elem_size = type2aelembytes(type);
  switch (elem_size) {
    case 1: return Address::times_1;
    case 2: return Address::times_2;
    case 4: return Address::times_4;
    case 8: return Address::times_8;
  }
  ShouldNotReachHere();
  return Address::no_scale;
}


void LIR_Assembler::emit_op3(LIR_Op3* op) {
 switch (op->code()) {
    case lir_frem:
      arithmetic_frem(
        op->code(),
        op->in_opr1(),
        op->in_opr2(),
        op->in_opr3(),
        op->result_opr(),
        op->info());
      break;

    case lir_idiv:
    case lir_irem:
      arithmetic_idiv(
        op->code(),
        op->in_opr1(),
        op->in_opr2(),
        op->in_opr3(),
        op->result_opr(),
        op->info());
      break;
    default:      ShouldNotReachHere(); break;
  }
}

void LIR_Assembler::emit_op4(LIR_Op4* op) {
 switch (op->code()) {
    case lir_cmove_mips:
      emit_cmove_mips(op);
      break;
    default:      ShouldNotReachHere(); break;
  }
}

void LIR_Assembler::emit_cmove_mips(LIR_Op4* op) {
  LIR_Opr cmp1 = op->in_opr1();
  LIR_Opr cmp2 = op->in_opr2();
  LIR_Opr src1 = op->in_opr3();
  LIR_Opr src2 = op->in_opr4();
  LIR_Condition condition = op->cond();
  LIR_Opr dst  = op->result_opr();

  if (src1->is_constant() && src2->is_constant() && (dst->is_single_cpu() || dst->is_double_cpu())) {
    jlong value = (jlong)(src2->pointer()->as_constant()->as_pointer());
    Register dst_reg = dst->as_register_lo();
    __ set64(dst_reg, value);
  } else {
    Unimplemented();
  }
/*
  if (opr1->is_single_cpu()) {
    Register reg_op1 = opr1->as_register();
    if (opr2->is_single_cpu()) {
      Register reg_op2 = opr2->as_register();
      switch (condition) {
        case lir_cond_equal:
          __ beq_far(reg_op1, reg_op2, *op->label());
          break;
        case lir_cond_notEqual:
          if(op->label()==NULL)
            __ bne_far(reg_op1, reg_op2, *op->label());
          else
            __ bne_far(reg_op1, reg_op2, *op->label());
          break;
        case lir_cond_less:
          __ slt(AT, reg_op1, reg_op2);
          __ bne_far(AT, R0, *op->label());
          break;
        case lir_cond_lessEqual:
          __ slt(AT, reg_op2, reg_op1);
          __ beq_far(AT, R0, *op->label());
          break;
        case lir_cond_belowEqual:
          __ sltu(AT, reg_op2, reg_op1);
          __ beq_far(AT, R0, *op->label());
          break;
        case lir_cond_greaterEqual:
          __ slt(AT, reg_op1, reg_op2);
          __ beq_far(AT, R0, *op->label());
          break;
        case lir_cond_aboveEqual:
          __ sltu(AT, reg_op1, reg_op2);
          __ beq_far(AT, R0, *op->label());
          break;
        case lir_cond_greater:
          __ slt(AT, reg_op2, reg_op1);
          __ bne_far(AT, R0, *op->label());
          break;
        default: ShouldNotReachHere();
      }
    } else if (opr2->is_constant()) {
      NOT_LP64(jint) LP64_ONLY(jlong) temp_value;
      bool is_object = false;
      if (opr2->pointer()->as_constant()->type() == T_INT) {
        temp_value = (jint)(opr2->as_jint());
      } else if (opr2->pointer()->as_constant()->type() == T_LONG) {
        temp_value = (jlong)(opr2->as_jlong());
      } else if (opr2->pointer()->as_constant()->type() == T_OBJECT) {
        is_object = true;
        temp_value = NOT_LP64((jint)) LP64_ONLY((jlong))(opr2->as_jobject());
      } else {
        ShouldNotReachHere();
      }

      switch (condition) {
        case lir_cond_equal:
          if (temp_value) {
            if (is_object) {
              int oop_index = __ oop_recorder()->allocate_oop_index((jobject)temp_value);
              RelocationHolder rspec = oop_Relocation::spec(oop_index);
              __ relocate(rspec);
            }
            __ li(AT, temp_value);
            __ beq_far(reg_op1, AT, *op->label());
          } else {
            __ beq_far(reg_op1, R0, *op->label());
          }
          break;

        case lir_cond_notEqual:
          if (temp_value) {
            if (is_object) {
              int oop_index = __ oop_recorder()->allocate_oop_index((jobject)temp_value);
              RelocationHolder rspec = oop_Relocation::spec(oop_index);
              __ relocate(rspec);
            }
            __ li(AT, temp_value);
            __ bne_far(reg_op1, AT, *op->label());
          } else {
            __ bne_far(reg_op1, R0, *op->label());
          }
          break;

        case lir_cond_less:
          if (Assembler::is_simm16(temp_value)) {
            __ slti(AT, reg_op1, temp_value);
          } else {
            __ move(AT, temp_value);
            __ slt(AT, reg_op1, AT);
          }
          __ bne_far(AT, R0, *op->label());
          break;

        case lir_cond_lessEqual:
          __ li(AT, temp_value);
          __ slt(AT, AT, reg_op1);
          __ beq_far(AT, R0, *op->label());
          break;

        case lir_cond_belowEqual:
#ifdef OPT_RANGECHECK
          if (op->check()) {
            __ li(AT, temp_value);
            add_debug_info_for_range_check_here(op->info(), temp_value);
            __ tgeu(AT, reg_op1, 29);
          } else {
#endif
            __ li(AT, temp_value);
            __ sltu(AT, AT, reg_op1);
            __ beq_far(AT, R0, *op->label());
#ifdef OPT_RANGECHECK
          }
#endif
          break;

        case lir_cond_greaterEqual:
          if (Assembler::is_simm16(temp_value)) {
            __ slti(AT, reg_op1, temp_value);
          } else {
            __ li(AT, temp_value);
            __ slt(AT, reg_op1, AT);
          }
          __ beq_far(AT, R0, *op->label());
          break;

        case lir_cond_aboveEqual:
          if (Assembler::is_simm16(temp_value)) {
            __ sltiu(AT, reg_op1, temp_value);
          } else {
            __ li(AT, temp_value);
            __ sltu(AT, reg_op1, AT);
          }
          __ beq_far(AT, R0, *op->label());
          break;

        case lir_cond_greater:
          __ li(AT, temp_value);
          __ slt(AT, AT, reg_op1);
          __ bne_far(AT, R0, *op->label());
          break;

        default: ShouldNotReachHere();
      }

    } else {
      if (opr2->is_address()) {
        if (op->type() == T_INT)
          __ lw(AT, as_Address(opr2->pointer()->as_address()));
        else
          __ ld_ptr(AT, as_Address(opr2->pointer()->as_address()));
      } else if (opr2->is_stack()) {
        __ ld_ptr(AT, frame_map()->address_for_slot(opr2->single_stack_ix()));
      } else {
        ShouldNotReachHere();
      }
      switch (condition) {
        case lir_cond_equal:
          __ beq_far(reg_op1, AT, *op->label());
          break;
        case lir_cond_notEqual:
          __ bne_far(reg_op1, AT, *op->label());
          break;
        case lir_cond_less:
          __ slt(AT, reg_op1, AT);
          __ bne_far(AT, R0, *op->label());
          break;
        case lir_cond_lessEqual:
          __ slt(AT, AT, reg_op1);
          __ beq_far(AT, R0, *op->label());
          break;
        case lir_cond_belowEqual:
          __ sltu(AT, AT, reg_op1);
          __ beq_far(AT, R0, *op->label());
          break;
        case lir_cond_greaterEqual:
          __ slt(AT, reg_op1, AT);
          __ beq_far(AT, R0, *op->label());
          break;
        case lir_cond_aboveEqual:
#ifdef OPT_RANGECHECK
          if (op->check()) {
            add_debug_info_for_range_check_here(op->info(), opr1->rinfo());
            __ tgeu(reg_op1, AT, 29);
          } else {
#endif
            __ sltu(AT, reg_op1, AT);
            __ beq_far(AT, R0, *op->label());
#ifdef OPT_RANGECHECK
          }
#endif
          break;
        case lir_cond_greater:
          __ slt(AT, AT, reg_op1);
          __ bne_far(AT, R0, *op->label());
          break;
        default: ShouldNotReachHere();
      }
    }
      __ delayed()->nop();

  } else if(opr1->is_address() || opr1->is_stack()) {
    if (opr2->is_constant()) {
      NOT_LP64(jint) LP64_ONLY(jlong) temp_value;
      if (opr2->as_constant_ptr()->type() == T_INT) {
        temp_value = (jint)opr2->as_constant_ptr()->as_jint();
      } else if (opr2->as_constant_ptr()->type() == T_OBJECT) {
        temp_value = NOT_LP64((jint)) LP64_ONLY((jlong))(opr2->as_constant_ptr()->as_jobject());
      } else {
        ShouldNotReachHere();
      }

      if (Assembler::is_simm16(temp_value)) {
        if (opr1->is_address()) {
          __ lw(AT, as_Address(opr1->pointer()->as_address()));
        } else {
          __ lw(AT, frame_map()->address_for_slot(opr1->single_stack_ix()));
        }

        switch(condition) {

          case lir_cond_equal:
            __ addiu(AT, AT, -(int)temp_value);
            __ beq_far(AT, R0, *op->label());
            break;
          case lir_cond_notEqual:
            __ addiu(AT, AT, -(int)temp_value);
            __ bne_far(AT, R0, *op->label());
            break;
          case lir_cond_less:
            __ slti(AT, AT, temp_value);
            __ bne_far(AT, R0, *op->label());
            break;
          case lir_cond_lessEqual:
            __ addiu(AT, AT, -temp_value);
            __ slt(AT, R0, AT);
            __ beq_far(AT, R0, *op->label());
            break;
          case lir_cond_belowEqual:
            __ addiu(AT, AT, -temp_value);
            __ sltu(AT, R0, AT);
            __ beq_far(AT, R0, *op->label());
            break;
          case lir_cond_greaterEqual:
            __ slti(AT, AT, temp_value);
            __ beq_far(AT, R0, *op->label());
            break;
          case lir_cond_aboveEqual:
            __ sltiu(AT, AT, temp_value);
            __ beq_far(AT, R0, *op->label());
            break;
          case lir_cond_greater:
            __ addiu(AT, AT, -temp_value);
            __ slt(AT, R0, AT);
            __ bne_far(AT, R0, *op->label());
            break;

          default:
            Unimplemented();
        }
      } else {
        Unimplemented();
      }
    } else {
      Unimplemented();
    }
    __ delayed()->nop();

  } else if(opr1->is_double_cpu()) {
    Register opr1_lo = opr1->as_register_lo();
    Register opr1_hi = opr1->as_register_hi();

    if (opr2->is_double_cpu()) {
      Register opr2_lo = opr2->as_register_lo();
      Register opr2_hi = opr2->as_register_hi();
      switch (condition) {
        case lir_cond_equal: {
          Label L;
#ifndef _LP64
          Unimplemented();
#else
          __ beq_far(opr1_lo, opr2_lo, *op->label());
#endif
          __ delayed()->nop();
          __ bind(L);
        }
        break;

        case lir_cond_notEqual:
          if (op->label()==NULL)
            __ bne_far(opr1_lo, opr2_lo, *op->label());
          else
            __ bne_far(opr1_lo, opr2_lo, *op->label());
            __ delayed()->nop();
          if (op->label()==NULL)
            NOT_LP64(__ bne(opr1_hi, opr2_hi, *op->label()));
          else
            NOT_LP64(__ bne_far(opr1_hi, opr2_hi, *op->label()));
            NOT_LP64(__ delayed()->nop());
          break;

        case lir_cond_less: {
#ifdef _LP64
          __ slt(AT, opr1_lo, opr2_lo);
          __ bne_far(AT, R0, *op->label());
          __ delayed()->nop();
#else
          Unimplemented();
#endif
        }
          break;

        case lir_cond_lessEqual: {
#ifdef _LP64
          __ slt(AT, opr2_lo, opr1_lo);
          __ beq_far(AT, R0, *op->label());
          __ delayed()->nop();
#else
          Unimplemented();
#endif
          }
          break;

        case lir_cond_belowEqual: {
#ifdef _LP64
          __ sltu(AT, opr2_lo, opr1_lo);
          __ beq_far(AT, R0, *op->label());
          __ delayed()->nop();
#else
          Unimplemented();
#endif
        }
          break;

        case lir_cond_greaterEqual: {
#ifdef _LP64
          __ slt(AT, opr1_lo, opr2_lo);
          __ beq_far(AT, R0, *op->label());
          __ delayed()->nop();
#else
          Unimplemented();
#endif
        }
        break;

        case lir_cond_aboveEqual: {
#ifdef _LP64
          __ sltu(AT, opr1_lo, opr2_lo);
          __ beq_far(AT, R0, *op->label());
          __ delayed()->nop();
#else
          Unimplemented();
#endif
        }
        break;

        case lir_cond_greater: {
#ifdef _LP64
           __ slt(AT, opr2_lo, opr1_lo);
           __ bne_far(AT, R0, *op->label());
           __ delayed()->nop();
#else
          Unimplemented();
#endif
         }
         break;

        default: ShouldNotReachHere();
      }

    } else if(opr2->is_constant()) {
      jlong lv = opr2->as_jlong();
      switch (condition) {
        case lir_cond_equal:
#ifdef _LP64
          __ li(T8, lv);
          __ beq_far(opr1_lo, T8, *op->label());
          __ delayed()->nop();
#else
          Unimplemented();
#endif
          break;

        case lir_cond_notEqual:
#ifdef _LP64
          __ li(T8, lv);
          __ bne_far(opr1_lo, T8, *op->label());
          __ delayed()->nop();
#else
          Unimplemented();
#endif
          break;

        case lir_cond_less:
#ifdef _LP64
          __ li(T8, lv);
          __ slt(AT, opr1_lo, T8);
          __ bne_far(AT, R0, *op->label());
          __ delayed()->nop();
#else
          Unimplemented();
#endif
          break;

        case lir_cond_lessEqual:
#ifdef _LP64
          __ li(T8, lv);
          __ slt(AT, T8, opr1_lo);
          __ beq_far(AT, R0, *op->label());
          __ delayed()->nop();
#else
          Unimplemented();
#endif
          break;

        case lir_cond_belowEqual:
#ifdef _LP64
          __ li(T8, lv);
          __ sltu(AT, T8, opr1_lo);
          __ beq_far(AT, R0, *op->label());
          __ delayed()->nop();
#else
          Unimplemented();
#endif
          break;

        case lir_cond_greaterEqual:
#ifdef _LP64
          __ li(T8, lv);
          __ slt(AT, opr1_lo, T8);
          __ beq_far(AT, R0, *op->label());
          __ delayed()->nop();
#else
          Unimplemented();
#endif
          break;

        case lir_cond_aboveEqual:
#ifdef _LP64
          __ li(T8, lv);
          __ sltu(AT, opr1_lo, T8);
          __ beq_far(AT, R0, *op->label());
          __ delayed()->nop();
#else
          Unimplemented();
#endif
          break;

        case lir_cond_greater:
#ifdef _LP64
          __ li(T8, lv);
          __ slt(AT, T8, opr1_lo);
          __ bne_far(AT, R0, *op->label());
          __ delayed()->nop();
#else
          Unimplemented();
#endif
          break;

        default:
          ShouldNotReachHere();
      }
    } else {
      Unimplemented();
    }
  } else if (opr1->is_single_fpu()) {
    assert(opr2->is_single_fpu(), "change the code");

    FloatRegister reg_op1 = opr1->as_float_reg();
    FloatRegister reg_op2 = opr2->as_float_reg();
    bool un_jump = (op->ublock()->label()==op->label());

    Label& L = *op->label();

    switch (condition) {
      case lir_cond_equal:
        if (un_jump)
          __ c_ueq_s(reg_op1, reg_op2);
        else
          __ c_eq_s(reg_op1, reg_op2);
        __ bc1t(L);

        break;

      case lir_cond_notEqual:
        if (un_jump)
          __ c_eq_s(reg_op1, reg_op2);
        else
          __ c_ueq_s(reg_op1, reg_op2);
        __ bc1f(L);

        break;

      case lir_cond_less:
        if (un_jump)
          __ c_ult_s(reg_op1, reg_op2);
        else
          __ c_olt_s(reg_op1, reg_op2);
        __ bc1t(L);

        break;

      case lir_cond_lessEqual:
      case lir_cond_belowEqual:
        if (un_jump)
          __ c_ule_s(reg_op1, reg_op2);
        else
          __ c_ole_s(reg_op1, reg_op2);
        __ bc1t(L);

        break;

      case lir_cond_greaterEqual:
      case lir_cond_aboveEqual:
        if (un_jump)
          __ c_olt_s(reg_op1, reg_op2);
        else
          __ c_ult_s(reg_op1, reg_op2);
        __ bc1f(L);

        break;

      case lir_cond_greater:
        if (un_jump)
          __ c_ole_s(reg_op1, reg_op2);
        else
          __ c_ule_s(reg_op1, reg_op2);
        __ bc1f(L);

        break;

      default:
        ShouldNotReachHere();
    }
    __ delayed()->nop();
  } else if (opr1->is_double_fpu()) {
    assert(opr2->is_double_fpu(), "change the code");

    FloatRegister reg_op1 = opr1->as_double_reg();
    FloatRegister reg_op2 = opr2->as_double_reg();
    bool un_jump = (op->ublock()->label()==op->label());
    Label& L = *op->label();

    switch (condition) {
      case lir_cond_equal:
        if (un_jump)
          __ c_ueq_d(reg_op1, reg_op2);
        else
          __ c_eq_d(reg_op1, reg_op2);
        __ bc1t(L);

        break;

      case lir_cond_notEqual:
        if (un_jump)
          __ c_eq_d(reg_op1, reg_op2);
        else
          __ c_ueq_d(reg_op1, reg_op2);
        __ bc1f(L);

        break;

      case lir_cond_less:
        if (un_jump)
          __ c_ult_d(reg_op1, reg_op2);
        else
          __ c_olt_d(reg_op1, reg_op2);
        __ bc1t(L);

        break;

      case lir_cond_lessEqual:
      case lir_cond_belowEqual:
        if (un_jump)
          __ c_ule_d(reg_op1, reg_op2);
        else
          __ c_ole_d(reg_op1, reg_op2);
        __ bc1t(L);

        break;

      case lir_cond_greaterEqual:
      case lir_cond_aboveEqual:
        if (un_jump)
          __ c_olt_d(reg_op1, reg_op2);
        else
          __ c_ult_d(reg_op1, reg_op2);
        __ bc1f(L);

        break;

      case lir_cond_greater:
        if (un_jump)
          __ c_ole_d(reg_op1, reg_op2);
        else
          __ c_ule_d(reg_op1, reg_op2);
        __ bc1f(L);

        break;

      default:
        ShouldNotReachHere();
    }
    __ delayed()->nop();
  } else {
    Unimplemented();
  }
*/
}

void LIR_Assembler::emit_opBranch(LIR_OpBranch* op) {
  LIR_Opr opr1 = op->left();
  LIR_Opr opr2 = op->right();
  LIR_Condition condition = op->cond();
#ifdef ASSERT
  assert(op->block() == NULL || op->block()->label() == op->label(), "wrong label");
  if (op->block() != NULL)  _branch_target_blocks.append(op->block());
  if (op->ublock() != NULL) _branch_target_blocks.append(op->ublock());
#endif
  if (op->cond() == lir_cond_always) {
    if(op->label()==NULL)
      __ b_far(*op->label());
    else
      __ b_far(*op->label());
    __ delayed()->nop();
    return;
  }
  if (opr1->is_single_cpu()) {
    Register reg_op1 = opr1->as_register();
    if (opr2->is_single_cpu()) {
#ifdef OPT_RANGECHECK
      assert(!op->check(), "just check");
#endif
      Register reg_op2 = opr2->as_register();
      switch (condition) {
        case lir_cond_equal:
          __ beq_far(reg_op1, reg_op2, *op->label());
          break;
        case lir_cond_notEqual:
          if(op->label()==NULL)
            __ bne_far(reg_op1, reg_op2, *op->label());
          else
            __ bne_far(reg_op1, reg_op2, *op->label());
          break;
        case lir_cond_less:
          // AT = 1 TRUE
          __ slt(AT, reg_op1, reg_op2);
          __ bne_far(AT, R0, *op->label());
          break;
        case lir_cond_lessEqual:
          // AT = 0 TRUE
          __ slt(AT, reg_op2, reg_op1);
          __ beq_far(AT, R0, *op->label());
          break;
        case lir_cond_belowEqual:
          // AT = 0 TRUE
          __ sltu(AT, reg_op2, reg_op1);
          __ beq_far(AT, R0, *op->label());
          break;
        case lir_cond_greaterEqual:
          // AT = 0 TRUE
          __ slt(AT, reg_op1, reg_op2);
          __ beq_far(AT, R0, *op->label());
          break;
        case lir_cond_aboveEqual:
          // AT = 0 TRUE
          __ sltu(AT, reg_op1, reg_op2);
          __ beq_far(AT, R0, *op->label());
          break;
        case lir_cond_greater:
          // AT = 1 TRUE
          __ slt(AT, reg_op2, reg_op1);
          __ bne_far(AT, R0, *op->label());
          break;
        default: ShouldNotReachHere();
      }
    } else if (opr2->is_constant()) {
      NOT_LP64(jint) LP64_ONLY(jlong) temp_value;
      bool is_object = false;
      if (opr2->pointer()->as_constant()->type() == T_INT) {
        temp_value = (jint)(opr2->as_jint());
      } else if (opr2->pointer()->as_constant()->type() == T_LONG) {
        temp_value = (jlong)(opr2->as_jlong());
      } else if (opr2->pointer()->as_constant()->type() == T_OBJECT) {
        is_object = true;
        temp_value = NOT_LP64((jint)) LP64_ONLY((jlong))(opr2->as_jobject());
      } else {
        ShouldNotReachHere();
      }

      switch (condition) {
        case lir_cond_equal:
#ifdef OPT_RANGECHECK
          assert(!op->check(), "just check");
#endif
          if (temp_value) {
            if (is_object) {
              int oop_index = __ oop_recorder()->allocate_oop_index((jobject)temp_value);
              RelocationHolder rspec = oop_Relocation::spec(oop_index);
              __ relocate(rspec);
            }
            __ li(AT, temp_value);
            __ beq_far(reg_op1, AT, *op->label());
          } else {
            __ beq_far(reg_op1, R0, *op->label());
          }
          break;

        case lir_cond_notEqual:
#ifdef OPT_RANGECHECK
          assert(!op->check(), "just check");
#endif
          if (temp_value) {
            if (is_object) {
              int oop_index = __ oop_recorder()->allocate_oop_index((jobject)temp_value);
              RelocationHolder rspec = oop_Relocation::spec(oop_index);
              __ relocate(rspec);
            }
            __ li(AT, temp_value);
            __ bne_far(reg_op1, AT, *op->label());
          } else {
            __ bne_far(reg_op1, R0, *op->label());
          }
          break;

        case lir_cond_less:
#ifdef OPT_RANGECHECK
          assert(!op->check(), "just check");
#endif
          // AT = 1 TRUE
          if (Assembler::is_simm16(temp_value)) {
            __ slti(AT, reg_op1, temp_value);
          } else {
            __ move(AT, temp_value);
            __ slt(AT, reg_op1, AT);
          }
          __ bne_far(AT, R0, *op->label());
          break;

        case lir_cond_lessEqual:
#ifdef OPT_RANGECHECK
          assert(!op->check(), "just check");
#endif
          // AT = 0 TRUE
          __ li(AT, temp_value);
          __ slt(AT, AT, reg_op1);
          __ beq_far(AT, R0, *op->label());
          break;

        case lir_cond_belowEqual:
          // AT = 0 TRUE
#ifdef OPT_RANGECHECK
          if (op->check()) {
            __ li(AT, temp_value);
            add_debug_info_for_range_check_here(op->info(), temp_value);
            __ tgeu(AT, reg_op1, 29);
          } else {
#endif
            __ li(AT, temp_value);
            __ sltu(AT, AT, reg_op1);
            __ beq_far(AT, R0, *op->label());
#ifdef OPT_RANGECHECK
          }
#endif
          break;

        case lir_cond_greaterEqual:
#ifdef OPT_RANGECHECK
          assert(!op->check(), "just check");
#endif
          // AT = 0 TRUE
          if (Assembler::is_simm16(temp_value)) {
            __ slti(AT, reg_op1, temp_value);
          } else {
            __ li(AT, temp_value);
            __ slt(AT, reg_op1, AT);
          }
          __ beq_far(AT, R0, *op->label());
          break;

        case lir_cond_aboveEqual:
#ifdef OPT_RANGECHECK
          assert(!op->check(), "just check");
#endif
          // AT = 0 TRUE
          if (Assembler::is_simm16(temp_value)) {
            __ sltiu(AT, reg_op1, temp_value);
          } else {
            __ li(AT, temp_value);
            __ sltu(AT, reg_op1, AT);
          }
          __ beq_far(AT, R0, *op->label());
          break;

        case lir_cond_greater:
#ifdef OPT_RANGECHECK
          assert(!op->check(), "just check");
#endif
          // AT = 1 TRUE
          __ li(AT, temp_value);
          __ slt(AT, AT, reg_op1);
          __ bne_far(AT, R0, *op->label());
          break;

        default: ShouldNotReachHere();
      }

    } else {
      if (opr2->is_address()) {
        if (op->type() == T_INT)
          __ lw(AT, rebase_Address(opr2->pointer()->as_address()));
        else
          __ ld_ptr(AT, rebase_Address(opr2->pointer()->as_address()));
      } else if (opr2->is_stack()) {
        __ ld_ptr(AT, frame_map()->address_for_slot(opr2->single_stack_ix()));
      } else {
        ShouldNotReachHere();
      }
      switch (condition) {
        case lir_cond_equal:
#ifdef OPT_RANGECHECK
          assert(!op->check(), "just check");
#endif
          __ beq_far(reg_op1, AT, *op->label());
          break;
        case lir_cond_notEqual:
#ifdef OPT_RANGECHECK
          assert(!op->check(), "just check");
#endif
          __ bne_far(reg_op1, AT, *op->label());
          break;
        case lir_cond_less:
#ifdef OPT_RANGECHECK
          assert(!op->check(), "just check");
#endif
          // AT = 1 TRUE
          __ slt(AT, reg_op1, AT);
          __ bne_far(AT, R0, *op->label());
          break;
        case lir_cond_lessEqual:
#ifdef OPT_RANGECHECK
          assert(!op->check(), "just check");
#endif
          // AT = 0 TRUE
          __ slt(AT, AT, reg_op1);
          __ beq_far(AT, R0, *op->label());
          break;
        case lir_cond_belowEqual:
#ifdef OPT_RANGECHECK
          assert(!op->check(), "just check");
#endif
          // AT = 0 TRUE
          __ sltu(AT, AT, reg_op1);
          __ beq_far(AT, R0, *op->label());
          break;
        case lir_cond_greaterEqual:
#ifdef OPT_RANGECHECK
          assert(!op->check(), "just check");
#endif
          // AT = 0 TRUE
          __ slt(AT, reg_op1, AT);
          __ beq_far(AT, R0, *op->label());
          break;
        case lir_cond_aboveEqual:
          // AT = 0 TRUE
#ifdef OPT_RANGECHECK
          if (op->check()) {
            add_debug_info_for_range_check_here(op->info(), opr1->rinfo());
            __ tgeu(reg_op1, AT, 29);
          } else {
#endif
            __ sltu(AT, reg_op1, AT);
            __ beq_far(AT, R0, *op->label());
#ifdef OPT_RANGECHECK
          }
#endif
          break;
        case lir_cond_greater:
#ifdef OPT_RANGECHECK
          assert(!op->check(), "just check");
#endif
          // AT = 1 TRUE
          __ slt(AT, AT, reg_op1);
          __ bne_far(AT, R0, *op->label());
          break;
        default: ShouldNotReachHere();
      }
    }
#ifdef OPT_RANGECHECK
    if (!op->check())
#endif
      __ delayed()->nop();

  } else if(opr1->is_address() || opr1->is_stack()) {
#ifdef OPT_RANGECHECK
    assert(!op->check(), "just check");
#endif
    if (opr2->is_constant()) {
      NOT_LP64(jint) LP64_ONLY(jlong) temp_value;
      if (opr2->as_constant_ptr()->type() == T_INT) {
        temp_value = (jint)opr2->as_constant_ptr()->as_jint();
      } else if (opr2->as_constant_ptr()->type() == T_OBJECT) {
        temp_value = NOT_LP64((jint)) LP64_ONLY((jlong))(opr2->as_constant_ptr()->as_jobject());
      } else {
        ShouldNotReachHere();
      }

      if (Assembler::is_simm16(temp_value)) {
        if (opr1->is_address()) {
          __ lw(AT, rebase_Address(opr1->pointer()->as_address()));
        } else {
          __ lw(AT, frame_map()->address_for_slot(opr1->single_stack_ix()));
        }

        switch(condition) {

          case lir_cond_equal:
            __ addiu(AT, AT, -(int)temp_value);
            __ beq_far(AT, R0, *op->label());
            break;
          case lir_cond_notEqual:
            __ addiu(AT, AT, -(int)temp_value);
            __ bne_far(AT, R0, *op->label());
            break;
          case lir_cond_less:
            // AT = 1 TRUE
            __ slti(AT, AT, temp_value);
            __ bne_far(AT, R0, *op->label());
            break;
          case lir_cond_lessEqual:
            // AT = 0 TRUE
            __ addiu(AT, AT, -temp_value);
            __ slt(AT, R0, AT);
            __ beq_far(AT, R0, *op->label());
            break;
          case lir_cond_belowEqual:
            // AT = 0 TRUE
            __ li(T9, temp_value);
            __ sltu(AT, T9, AT);
            __ beq_far(AT, R0, *op->label());
            break;
          case lir_cond_greaterEqual:
            // AT = 0 TRUE
            __ slti(AT, AT, temp_value);
            __ beq_far(AT, R0, *op->label());
            break;
          case lir_cond_aboveEqual:
            // AT = 0 TRUE
            __ sltiu(AT, AT, temp_value);
            __ beq_far(AT, R0, *op->label());
            break;
          case lir_cond_greater:
            // AT = 1 TRUE
            __ addiu(AT, AT, -temp_value);
            __ slt(AT, R0, AT);
            __ bne_far(AT, R0, *op->label());
            break;

          default:
            Unimplemented();
        }
      } else {
        Unimplemented();
      }
    } else {
      Unimplemented();
    }
    __ delayed()->nop();

  } else if(opr1->is_double_cpu()) {
#ifdef OPT_RANGECHECK
    assert(!op->check(), "just check");
#endif
    Register opr1_lo = opr1->as_register_lo();
    Register opr1_hi = opr1->as_register_hi();

    if (opr2->is_double_cpu()) {
      Register opr2_lo = opr2->as_register_lo();
      Register opr2_hi = opr2->as_register_hi();
      switch (condition) {
        case lir_cond_equal: {
                               Label L;
#ifndef _LP64
          __ bne(opr1_lo, opr2_lo, L);
          __ delayed()->nop();
          __ beq(opr1_hi, opr2_hi, *op->label());
#else
          // static jobject java.lang.Long.toString(jlong)
          //
          //   10 move [t0t0|J] [a4a4|J]
          //   12 move [lng:-9223372036854775808|J] [a6a6|J]
          //   14 branch [EQ] [a4a4|J] [a6a6|J] [B1]
          // 0x000000555e8532e4: bne a4, a6, 0x000000555e8532e4  <-- error
          // 0x000000555e8532e8: sll zero, zero, 0
          //
          __ beq_far(opr1_lo, opr2_lo, *op->label());
#endif
          __ delayed()->nop();
          __ bind(L);
        }
        break;

        case lir_cond_notEqual:
          if (op->label()==NULL)
            __ bne_far(opr1_lo, opr2_lo, *op->label());
          else
            __ bne_far(opr1_lo, opr2_lo, *op->label());
            __ delayed()->nop();
          if (op->label()==NULL)
            NOT_LP64(__ bne(opr1_hi, opr2_hi, *op->label()));
          else
            NOT_LP64(__ bne_far(opr1_hi, opr2_hi, *op->label()));
            NOT_LP64(__ delayed()->nop());
          break;

        case lir_cond_less: {
#ifdef _LP64
          __ slt(AT, opr1_lo, opr2_lo);
          __ bne_far(AT, R0, *op->label());
          __ delayed()->nop();
#else
          Label L;

          // if hi less then jump
          __ slt(AT, opr1_hi, opr2_hi);
          __ bne(AT, R0, *op->label());
          __ delayed()->nop();

          // if hi great then fail
          __ bne(opr1_hi, opr2_hi, L);
          __ delayed();

          // now just comp lo as unsigned
          __ sltu(AT, opr1_lo, opr2_lo);
          __ bne_far(AT, R0, *op->label());
          __ delayed()->nop();

          __ bind(L);
#endif
        }
          break;

        case lir_cond_lessEqual: {
#ifdef _LP64
          __ slt(AT, opr2_lo, opr1_lo);
          __ beq_far(AT, R0, *op->label());
          __ delayed()->nop();
#else
          Label L;

          // if hi great then fail
          __ slt(AT, opr2_hi, opr1_hi);
          __ bne(AT, R0, L);
          __ delayed()->nop();

          // if hi less then jump
          if(op->label()==NULL)
            __ bne(opr2_hi, opr1_hi, *op->label());
          else
            __ bne_far(opr2_hi, opr1_hi, *op->label());
          __ delayed();

          // now just comp lo as unsigned
          __ sltu(AT, opr2_lo, opr1_lo);
          __ beq(AT, R0, *op->label());
          __ delayed()->nop();

          __ bind(L);
#endif
          }
          break;

        case lir_cond_belowEqual: {
#ifdef _LP64
          __ sltu(AT, opr2_lo, opr1_lo);
          __ beq_far(AT, R0, *op->label());
          __ delayed()->nop();
#else
          Label L;

          // if hi great then fail
          __ sltu(AT, opr2_hi, opr1_hi);
          __ bne_far(AT, R0, L);
          __ delayed()->nop();

          // if hi less then jump
          if(op->label()==NULL)
            __ bne(opr2_hi, opr1_hi, *op->label());
          else
            __ bne_far(opr2_hi, opr1_hi, *op->label());
          __ delayed();

          // now just comp lo as unsigned
          __ sltu(AT, opr2_lo, opr1_lo);
          __ beq(AT, R0, *op->label());
          __ delayed()->nop();

          __ bind(L);
#endif
        }
          break;

        case lir_cond_greaterEqual: {
#ifdef _LP64
          __ slt(AT, opr1_lo, opr2_lo);
          __ beq_far(AT, R0, *op->label());
          __ delayed()->nop();
#else
          Label L;

          // if hi less then fail
          __ slt(AT, opr1_hi, opr2_hi);
          __ bne_far(AT, R0, L);
          __ delayed()->nop();

          // if hi great then jump
          if(op->label()==NULL)
            __ bne(opr2_hi, opr1_hi, *op->label());
          else
            __ bne_far(opr2_hi, opr1_hi, *op->label());
          __ delayed();

          // now just comp lo as unsigned
          __ sltu(AT, opr1_lo, opr2_lo);
          __ beq(AT, R0, *op->label());
          __ delayed()->nop();

          __ bind(L);
#endif
        }
        break;

        case lir_cond_aboveEqual: {
#ifdef _LP64
          __ sltu(AT, opr1_lo, opr2_lo);
          __ beq_far(AT, R0, *op->label());
          __ delayed()->nop();
#else
          Label L;

          // if hi less then fail
          __ sltu(AT, opr1_hi, opr2_hi);
          __ bne(AT, R0, L);
          __ delayed()->nop();

          // if hi great then jump
          if(op->label()==NULL)
            __ bne(opr2_hi, opr1_hi, *op->label());
          else
            __ bne_far(opr2_hi, opr1_hi, *op->label());
          __ delayed();

          // now just comp lo as unsigned
          __ sltu(AT, opr1_lo, opr2_lo);
          __ beq(AT, R0, *op->label());
          __ delayed()->nop();

          __ bind(L);
#endif
        }
        break;

        case lir_cond_greater: {
#ifdef _LP64
           __ slt(AT, opr2_lo, opr1_lo);
           __ bne_far(AT, R0, *op->label());
           __ delayed()->nop();
#else
           Label L;

           // if hi great then jump
           __ slt(AT, opr2_hi, opr1_hi);
           __ bne(AT, R0, *op->label());
           __ delayed()->nop();

           // if hi less then fail
           __ bne(opr2_hi, opr1_hi, L);
           __ delayed();

           // now just comp lo as unsigned
           __ sltu(AT, opr2_lo, opr1_lo);
           __ bne(AT, R0, *op->label());
           __ delayed()->nop();

           __ bind(L);
#endif
         }
         break;

        default: ShouldNotReachHere();
      }

    } else if(opr2->is_constant()) {
      jlong lv = opr2->as_jlong();
#ifndef _LP64
      jint iv_lo = (jint)lv;
      jint iv_hi = (jint)(lv>>32);
      bool is_zero = (lv==0);
#endif

      switch (condition) {
        case lir_cond_equal:
#ifdef _LP64
          __ li(T8, lv);
          __ beq_far(opr1_lo, T8, *op->label());
          __ delayed()->nop();
#else
          if (is_zero) {
            __ orr(AT, opr1_lo, opr1_hi);
            __ beq(AT, R0, *op->label());
            __ delayed()->nop();
          } else {
            Label L;
            __ move(T8, iv_lo);
            __ bne(opr1_lo, T8, L);
            __ delayed();
            __ move(T8, iv_hi);
            __ beq(opr1_hi, T8, *op->label());
            __ delayed()->nop();
            __ bind(L);
          }
#endif
          break;

        case lir_cond_notEqual:
#ifdef _LP64
          __ li(T8, lv);
          __ bne_far(opr1_lo, T8, *op->label());
          __ delayed()->nop();
#else
          if (is_zero) {
            __ orr(AT, opr1_lo, opr1_hi);
            __ bne(AT, R0, *op->label());
            __ delayed()->nop();
          } else {
            __ move(T8, iv_lo);
            __ bne(opr1_lo, T8, *op->label());
            __ delayed();
            __ move(T8, iv_hi);
            __ bne(opr1_hi, T8, *op->label());
            __ delayed()->nop();
          }
#endif
          break;

        case lir_cond_less:
#ifdef _LP64
          __ li(T8, lv);
          __ slt(AT, opr1_lo, T8);
          __ bne_far(AT, R0, *op->label());
          __ delayed()->nop();
#else
          if (is_zero) {
            __ bltz(opr1_hi, *op->label());
            __ delayed()->nop();
            __ bltz(opr1_lo, *op->label());
            __ delayed()->nop();
          } else {
            Label L;

            // if hi less then jump
            __ move(T8, iv_hi);
            __ slt(AT, opr1_hi, T8);
            __ bne_far(AT, R0, *op->label());
            __ delayed()->nop();

            // if hi great then fail
            __ bne(opr1_hi, T8, L);
            __ delayed();

            // now just comp lo as unsigned
            if (Assembler::is_simm16(iv_lo)) {
              __ sltiu(AT, opr1_lo, iv_lo);
            } else {
              __ move(T8, iv_lo);
              __ sltu(AT, opr1_lo, T8);
            }
            __ bne(AT, R0, *op->label());
            __ delayed()->nop();

            __ bind(L);
          }
#endif
          break;

        case lir_cond_lessEqual:
#ifdef _LP64
          __ li(T8, lv);
          __ slt(AT, T8, opr1_lo);
          __ beq_far(AT, R0, *op->label());
          __ delayed()->nop();
#else
          if (is_zero) {
            __ bltz(opr1_hi, *op->label());
            __ delayed()->nop();
            __ orr(AT, opr1_hi, opr1_lo);
            __ beq(AT, R0, *op->label());
            __ delayed();
          } else {
            Label L;

            // if hi great then fail
            __ move(T8, iv_hi);
            __ slt(AT, T8, opr1_hi);
            __ bne(AT, R0, L);
            __ delayed()->nop();

            // if hi less then jump
            __ bne(T8, opr1_hi, *op->label());
            __ delayed();

            // now just comp lo as unsigned
            __ move(T8, iv_lo);
            __ sltu(AT, T8, opr1_lo);
            __ beq(AT, R0, *op->label());
            __ delayed()->nop();

            __ bind(L);
          }
#endif
          break;

        case lir_cond_belowEqual:
#ifdef _LP64
          __ li(T8, lv);
          __ sltu(AT, T8, opr1_lo);
          __ beq_far(AT, R0, *op->label());
          __ delayed()->nop();
#else
          if (is_zero) {
            __ orr(AT, opr1_hi, opr1_lo);
            __ beq(AT, R0, *op->label());
            __ delayed()->nop();
          } else {
            Label L;

            // if hi great then fail
            __ move(T8, iv_hi);
            __ sltu(AT, T8, opr1_hi);
            __ bne(AT, R0, L);
            __ delayed()->nop();

            // if hi less then jump
            __ bne(T8, opr1_hi, *op->label());
            __ delayed();

            // now just comp lo as unsigned
            __ move(T8, iv_lo);
            __ sltu(AT, T8, opr1_lo);
            __ beq(AT, R0, *op->label());
            __ delayed()->nop();

            __ bind(L);
          }
#endif
          break;

        case lir_cond_greaterEqual:
#ifdef _LP64
          __ li(T8, lv);
          __ slt(AT, opr1_lo, T8);
          __ beq_far(AT, R0, *op->label());
          __ delayed()->nop();
#else
          if (is_zero) {
            __ bgez(opr1_hi, *op->label());
            __ delayed()->nop();
          } else {
            Label L;

            // if hi less then fail
            __ move(T8, iv_hi);
            __ slt(AT, opr1_hi, T8);
            __ bne(AT, R0, L);
            __ delayed()->nop();

            // if hi great then jump
            __ bne(T8, opr1_hi, *op->label());
            __ delayed();

            // now just comp lo as unsigned
            if (Assembler::is_simm16(iv_lo)) {
              __ sltiu(AT, opr1_lo, iv_lo);
            } else {
              __ move(T8, iv_lo);
              __ sltu(AT, opr1_lo, T8);
            }
            __ beq(AT, R0, *op->label());
            __ delayed()->nop();

            __ bind(L);
          }
#endif
          break;

        case lir_cond_aboveEqual:
#ifdef _LP64
          __ li(T8, lv);
          __ sltu(AT, opr1_lo, T8);
          __ beq_far(AT, R0, *op->label());
          __ delayed()->nop();
#else
          if (is_zero) {
            if(op->label()==NULL)
              __ b(*op->label());
            else
              __ b_far(*op->label());
            __ delayed()->nop();
          } else {
            Label L;

            // if hi less then fail
            __ move(T8, iv_hi);
            __ sltu(AT, opr1_hi, T8);
            __ bne(AT, R0, L);
            __ delayed()->nop();

            // if hi great then jump
            __ bne(T8, opr1_hi, *op->label());
            __ delayed();

            // now just comp lo as unsigned
            if (Assembler::is_simm16(iv_lo)) {
              __ sltiu(AT, opr1_lo, iv_lo);
            } else {
              __ move(T8, iv_lo);
              __ sltu(AT, opr1_lo, T8);
            }
            __ beq(AT, R0, *op->label());
            __ delayed()->nop();

            __ bind(L);
          }
#endif
          break;

        case lir_cond_greater:
#ifdef _LP64
          __ li(T8, lv);
          __ slt(AT, T8, opr1_lo);
          __ bne_far(AT, R0, *op->label());
          __ delayed()->nop();
#else
          if (is_zero) {
            Label L;
            __ bgtz(opr1_hi, *op->label());
            __ delayed()->nop();
            __ bne(opr1_hi, R0, L);
            __ delayed()->nop();
            __ bne(opr1_lo, R0, *op->label());
            __ delayed()->nop();
            __ bind(L);
          } else {
            Label L;

            // if hi great then jump
            __ move(T8, iv_hi);
            __ slt(AT, T8, opr1_hi);
            __ bne(AT, R0, *op->label());
            __ delayed()->nop();

            // if hi less then fail
            __ bne(T8, opr1_hi, L);
            __ delayed();

            // now just comp lo as unsigned
            __ move(T8, iv_lo);
            __ sltu(AT, T8, opr1_lo);
            __ bne(AT, R0, *op->label());
            __ delayed()->nop();

            __ bind(L);
          }
#endif
          break;

        default:
          ShouldNotReachHere();
      }
    } else {
      Unimplemented();
    }
  } else if (opr1->is_single_fpu()) {
#ifdef OPT_RANGECHECK
    assert(!op->check(), "just check");
#endif
    assert(opr2->is_single_fpu(), "change the code");

    FloatRegister reg_op1 = opr1->as_float_reg();
    FloatRegister reg_op2 = opr2->as_float_reg();
    //  bool un_ls
    bool un_jump = (op->ublock()->label()==op->label());

    Label& L = *op->label();

    switch (condition) {
      case lir_cond_equal:
        if (un_jump)
          __ c_ueq_s(reg_op1, reg_op2);
        else
          __ c_eq_s(reg_op1, reg_op2);
        __ bc1t(L);

        break;

      case lir_cond_notEqual:
        if (un_jump)
          __ c_eq_s(reg_op1, reg_op2);
        else
          __ c_ueq_s(reg_op1, reg_op2);
        __ bc1f(L);

        break;

      case lir_cond_less:
        if (un_jump)
          __ c_ult_s(reg_op1, reg_op2);
        else
          __ c_olt_s(reg_op1, reg_op2);
        __ bc1t(L);

        break;

      case lir_cond_lessEqual:
      case lir_cond_belowEqual:
        if (un_jump)
          __ c_ule_s(reg_op1, reg_op2);
        else
          __ c_ole_s(reg_op1, reg_op2);
        __ bc1t(L);

        break;

      case lir_cond_greaterEqual:
      case lir_cond_aboveEqual:
        if (un_jump)
          __ c_olt_s(reg_op1, reg_op2);
        else
          __ c_ult_s(reg_op1, reg_op2);
        __ bc1f(L);

        break;

      case lir_cond_greater:
        if (un_jump)
          __ c_ole_s(reg_op1, reg_op2);
        else
          __ c_ule_s(reg_op1, reg_op2);
        __ bc1f(L);

        break;

      default:
        ShouldNotReachHere();
    }
    __ delayed()->nop();
  } else if (opr1->is_double_fpu()) {
#ifdef OPT_RANGECHECK
    assert(!op->check(), "just check");
#endif
    assert(opr2->is_double_fpu(), "change the code");

    FloatRegister reg_op1 = opr1->as_double_reg();
    FloatRegister reg_op2 = opr2->as_double_reg();
    bool un_jump = (op->ublock()->label()==op->label());
    Label& L = *op->label();

    switch (condition) {
      case lir_cond_equal:
        if (un_jump)
          __ c_ueq_d(reg_op1, reg_op2);
        else
          __ c_eq_d(reg_op1, reg_op2);
        __ bc1t(L);

        break;

      case lir_cond_notEqual:
        if (un_jump)
          __ c_eq_d(reg_op1, reg_op2);
        else
          __ c_ueq_d(reg_op1, reg_op2);
        __ bc1f(L);

        break;

      case lir_cond_less:
        if (un_jump)
          __ c_ult_d(reg_op1, reg_op2);
        else
          __ c_olt_d(reg_op1, reg_op2);
        __ bc1t(L);

        break;

      case lir_cond_lessEqual:
      case lir_cond_belowEqual:
        if (un_jump)
          __ c_ule_d(reg_op1, reg_op2);
        else
          __ c_ole_d(reg_op1, reg_op2);
        __ bc1t(L);

        break;

      case lir_cond_greaterEqual:
      case lir_cond_aboveEqual:
        if (un_jump)
          __ c_olt_d(reg_op1, reg_op2);
        else
          __ c_ult_d(reg_op1, reg_op2);
        __ bc1f(L);

        break;

      case lir_cond_greater:
        if (un_jump)
          __ c_ole_d(reg_op1, reg_op2);
        else
          __ c_ule_d(reg_op1, reg_op2);
        __ bc1f(L);

        break;

      default:
        ShouldNotReachHere();
    }
    __ delayed()->nop();
  } else {
    Unimplemented();
  }
}

void LIR_Assembler::emit_opCmpBranch(LIR_OpCmpBranch* op) {
  ShouldNotReachHere();
}

void LIR_Assembler::emit_opConvert(LIR_OpConvert* op) {
  LIR_Opr value        = op->in_opr();
  LIR_Opr src          = op->in_opr();
  LIR_Opr dest         = op->result_opr();
  Bytecodes::Code code = op->bytecode();

  switch (code) {
    case Bytecodes::_i2l:
      move_regs(src->as_register(), dest->as_register_lo());
      NOT_LP64(__ sra (dest->as_register_hi(), dest->as_register_lo(), 31));
      break;

    case Bytecodes::_l2i:
#ifndef _LP64
      move_regs (src->as_register_lo(), dest->as_register());
#else
      __ dsll32(dest->as_register(), src->as_register_lo(), 0);
      __ dsra32(dest->as_register(), dest->as_register(), 0);
#endif
      break;

    case Bytecodes::_i2b:
#ifndef _LP64
      move_regs (src->as_register(), dest->as_register());
      __ sign_extend_byte(dest->as_register());
#else
      __ dsll32(dest->as_register(), src->as_register(), 24);
      __ dsra32(dest->as_register(), dest->as_register(), 24);
#endif
      break;

    case Bytecodes::_i2c:
      __ andi(dest->as_register(), src->as_register(), 0xFFFF);
      break;

    case Bytecodes::_i2s:
#ifndef _LP64
      move_regs (src->as_register(), dest->as_register());
      __ sign_extend_short(dest->as_register());
#else
      __ dsll32(dest->as_register(), src->as_register(), 16);
      __ dsra32(dest->as_register(), dest->as_register(), 16);
#endif
      break;

    case Bytecodes::_f2d:
      __ cvt_d_s(dest->as_double_reg(), src->as_float_reg());
      break;

    case Bytecodes::_d2f:
      __ cvt_s_d(dest->as_float_reg(), src->as_double_reg());
      break;
    case Bytecodes::_i2f: {
          FloatRegister df = dest->as_float_reg();
          if(src->is_single_cpu()) {
            __ mtc1(src->as_register(), df);
            __ cvt_s_w(df, df);
          } else if (src->is_stack()) {
            Address src_addr = src->is_single_stack()
        ? frame_map()->address_for_slot(src->single_stack_ix())
        : frame_map()->address_for_slot(src->double_stack_ix());
            __ lw(AT, src_addr);
            __ mtc1(AT, df);
            __ cvt_s_w(df, df);
          } else {
            Unimplemented();
          }
          break;
        }
    case Bytecodes::_i2d: {
          FloatRegister dd = dest->as_double_reg();
          if (src->is_single_cpu()) {
            __ mtc1(src->as_register(), dd);
            __ cvt_d_w(dd, dd);
          } else if (src->is_stack()) {
            Address src_addr = src->is_single_stack()
        ? frame_map()->address_for_slot(value->single_stack_ix())
        : frame_map()->address_for_slot(value->double_stack_ix());
            __ lw(AT, src_addr);
            __ mtc1(AT, dd);
            __ cvt_d_w(dd, dd);
          } else {
            Unimplemented();
          }
          break;
        }
    case Bytecodes::_f2i: {
          FloatRegister fval = src->as_float_reg();
          Register dreg = dest->as_register();

          Label L;
          __ c_un_s(fval, fval);    //NaN?
          __ bc1t(L);
          __ delayed();
          __ move(dreg, R0);

          __ trunc_w_s(F30, fval);

          // Call SharedRuntime:f2i() to do valid convention
          __ cfc1(AT, 31);
          __ li(T9, 0x10000);
          __ andr(AT, AT, T9);
          __ beq(AT, R0, L);
          __ delayed()->mfc1(dreg, F30);

          __ mov_s(F12, fval);
          __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::f2i), 1);
          __ move(dreg, V0);
          __ bind(L);
          break;
        }
    case Bytecodes::_d2i: {
          FloatRegister dval = src->as_double_reg();
          Register dreg = dest->as_register();

          Label L;
#ifndef _LP64
          __ c_un_d(dval, dval);    //NaN?
          __ bc1t(L);
          __ delayed();
          __ move(dreg, R0);
#endif

          __ trunc_w_d(F30, dval);
          __ cfc1(AT, 31);
          __ li(T9, 0x10000);
          __ andr(AT, AT, T9);
          __ beq(AT, R0, L);
          __ delayed()->mfc1(dreg, F30);

          __ mov_d(F12, dval);
          __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::d2i), 1);
          __ move(dreg, V0);
          __ bind(L);
          break;
        }
    case Bytecodes::_l2f: {
          FloatRegister ldf = dest->as_float_reg();
          if (src->is_double_cpu()) {
#ifndef _LP64
            __ mtc1(src->as_register_lo(), ldf);
            __ mtc1(src->as_register_hi(), ldf + 1);
            __ cvt_s_l(ldf, ldf);
#else
            __ dmtc1(src->as_register_lo(), ldf);
            __ cvt_s_l(ldf, ldf);
#endif
          } else if (src->is_double_stack()) {
            Address src_addr=frame_map()->address_for_slot(value->double_stack_ix());
#ifndef _LP64
            __ lw(AT, src_addr);
            __ mtc1(AT, ldf);
            __ lw(AT, src_addr.base(), src_addr.disp() + 4);
            __ mtc1(AT, ldf + 1);
            __ cvt_s_l(ldf, ldf);
#else
            __ ld(AT, src_addr);
            __ dmtc1(AT, ldf);
            __ cvt_s_l(ldf, ldf);
#endif
          } else {
            Unimplemented();
          }
          break;
        }
    case Bytecodes::_l2d: {
          FloatRegister ldd = dest->as_double_reg();
          if (src->is_double_cpu()) {
#ifndef _LP64
            __ mtc1(src->as_register_lo(), ldd);
            __ mtc1(src->as_register_hi(), ldd + 1);
            __ cvt_d_l(ldd, ldd);
#else
            __ dmtc1(src->as_register_lo(), ldd);
            __ cvt_d_l(ldd, ldd);
#endif
          } else if (src->is_double_stack()) {
            Address src_addr = frame_map()->address_for_slot(src->double_stack_ix());
#ifndef _LP64
            __ lw(AT, src_addr);
            __ mtc1(AT, ldd);
            __ lw(AT, src_addr.base(), src_addr.disp() + 4);
            __ mtc1(AT, ldd + 1);
            __ cvt_d_l(ldd, ldd);
#else
            __ ld(AT, src_addr);
            __ dmtc1(AT, ldd);
            __ cvt_d_l(ldd, ldd);
#endif
          } else {
            Unimplemented();
          }
          break;
        }

    case Bytecodes::_f2l: {
          FloatRegister fval = src->as_float_reg();
          Register dlo = dest->as_register_lo();
          Register dhi = dest->as_register_hi();

          Label L;
          __ move(dhi, R0);
          __ c_un_s(fval, fval);    //NaN?
          __ bc1t(L);
          __ delayed();
          __ move(dlo, R0);

          __ trunc_l_s(F30, fval);
#ifdef _LP64
          __ cfc1(AT, 31);
          __ li(T9, 0x10000);
          __ andr(AT, AT, T9);
          __ beq(AT, R0, L);
          __ delayed()->dmfc1(dlo, F30);

          __ mov_s(F12, fval);
          __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::f2l), 1);
          __ move(dlo, V0);
#else
          __ mfc1(dlo, F30);
#endif
          NOT_LP64(__ mfc1(dhi, F31));
          __ bind(L);
          break;
        }
    case Bytecodes::_d2l: {
          FloatRegister dval = src->as_double_reg();
          Register dlo = dest->as_register_lo();
          Register dhi = dest->as_register_hi();

          Label L;
          __ move(dhi, R0);
          __ c_un_d(dval, dval);    //NaN?
          __ bc1t(L);
          __ delayed();
          __ move(dlo, R0);

          __ trunc_l_d(F30, dval);
#ifdef _LP64
          __ cfc1(AT, 31);
          __ li(T9, 0x10000);
          __ andr(AT, AT, T9);
          __ beq(AT, R0, L);
          __ delayed()->dmfc1(dlo, F30);

          __ mov_d(F12, dval);
          __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::d2l), 1);
          __ move(dlo, V0);
#else
          __ mfc1(dlo, F30);
          __ mfc1(dhi, F31);
#endif
          __ bind(L);
          break;
        }

    default: ShouldNotReachHere();
  }
}

void LIR_Assembler::emit_alloc_obj(LIR_OpAllocObj* op) {
  if (op->init_check()) {
    add_debug_info_for_null_check_here(op->stub()->info());
    __ lw(AT,Address(op->klass()->as_register(),
    InstanceKlass::init_state_offset()));
    __ addiu(AT, AT, -InstanceKlass::fully_initialized);
    __ bne_far(AT, R0,*op->stub()->entry());
    __ delayed()->nop();
  }
  __ allocate_object(
      op->obj()->as_register(),
      op->tmp1()->as_register(),
      op->tmp2()->as_register(),
      op->header_size(),
      op->object_size(),
      op->klass()->as_register(),
      *op->stub()->entry());

  __ bind(*op->stub()->continuation());
}

void LIR_Assembler::emit_alloc_array(LIR_OpAllocArray* op) {
  if (UseSlowPath ||
    (!UseFastNewObjectArray && (op->type() == T_OBJECT || op->type() == T_ARRAY)) ||
    (!UseFastNewTypeArray   && (op->type() != T_OBJECT && op->type() != T_ARRAY))) {
    __ b_far(*op->stub()->entry());
    __ delayed()->nop();
  } else {
    Register len =  op->len()->as_register();
    Register tmp1 = op->tmp1()->as_register();
    Register tmp2 = op->tmp2()->as_register();
    Register tmp3 = op->tmp3()->as_register();
    __ allocate_array(op->obj()->as_register(),
        len,
        tmp1,
        tmp2,
        tmp3,
        arrayOopDesc::header_size(op->type()),
        array_element_size(op->type()),
        op->klass()->as_register(),
        *op->stub()->entry());
  }
  __ bind(*op->stub()->continuation());
}

void LIR_Assembler::type_profile_helper(Register mdo,
                                        ciMethodData *md, ciProfileData *data,
                                        Register recv, Label* update_done) {
  Register tmp = T9;

  for (uint i = 0; i < ReceiverTypeData::row_limit(); i++) {
    Label next_test;
    // See if the receiver is receiver[n].
    __ ld_ptr(AT, Address(mdo, md->byte_offset_of_slot(data, ReceiverTypeData::receiver_offset(i))));
    __ bne(AT, recv, next_test);
    __ delayed()->nop();
    Address data_addr(mdo, md->byte_offset_of_slot(data, ReceiverTypeData::receiver_count_offset(i)));
    __ ld_ptr(tmp, data_addr);
    __ addiu(tmp, tmp, DataLayout::counter_increment);
    __ st_ptr(tmp, data_addr);
    __ b(*update_done);
    __ delayed()->nop();
    __ bind(next_test);
  }

  // Didn't find receiver; find next empty slot and fill it in
  for (uint i = 0; i < ReceiverTypeData::row_limit(); i++) {
    Label next_test;
    Address recv_addr(mdo, md->byte_offset_of_slot(data, ReceiverTypeData::receiver_offset(i)));
    __ ld_ptr(AT, recv_addr);
    __ bne(AT, R0, next_test);
    __ delayed()->nop();
    __ st_ptr(recv, recv_addr);
    __ move(tmp, DataLayout::counter_increment);
    __ st_ptr(tmp, Address(mdo, md->byte_offset_of_slot(data, ReceiverTypeData::receiver_count_offset(i))));
    __ b(*update_done);
    __ delayed()->nop();
    __ bind(next_test);
  }
}

void LIR_Assembler::emit_typecheck_helper(LIR_OpTypeCheck *op, Label* success, Label* failure, Label* obj_is_null) {
  // we always need a stub for the failure case.
  CodeStub* stub = op->stub();
  Register obj = op->object()->as_register();
  Register k_RInfo = op->tmp1()->as_register();
  Register klass_RInfo = op->tmp2()->as_register();
  Register dst = op->result_opr()->as_register();
  ciKlass* k = op->klass();
  Register Rtmp1 = noreg;

  // check if it needs to be profiled
  ciMethodData* md = NULL;
  ciProfileData* data = NULL;

  if (op->should_profile()) {
    ciMethod* method = op->profiled_method();
    assert(method != NULL, "Should have method");
    int bci = op->profiled_bci();
    md = method->method_data_or_null();
    assert(md != NULL, "Sanity");
    data = md->bci_to_data(bci);
    assert(data != NULL,                "need data for type check");
    assert(data->is_ReceiverTypeData(), "need ReceiverTypeData for type check");
  }
  Label profile_cast_success, profile_cast_failure;
  Label *success_target = op->should_profile() ? &profile_cast_success : success;
  Label *failure_target = op->should_profile() ? &profile_cast_failure : failure;

  if (obj == k_RInfo) {
    k_RInfo = dst;
  } else if (obj == klass_RInfo) {
    klass_RInfo = dst;
  }
  if (k->is_loaded() && !UseCompressedClassPointers) {
    select_different_registers(obj, dst, k_RInfo, klass_RInfo);
  } else {
    Rtmp1 = op->tmp3()->as_register();
    select_different_registers(obj, dst, k_RInfo, klass_RInfo, Rtmp1);
  }

  assert_different_registers(obj, k_RInfo, klass_RInfo);

  if (op->should_profile()) {
    Label not_null;
    __ bne(obj, R0, not_null);
    __ delayed()->nop();
    // Object is null; update MDO and exit
    Register mdo  = klass_RInfo;
    __ mov_metadata(mdo, md->constant_encoding());
    Address data_addr(mdo, md->byte_offset_of_slot(data, DataLayout::header_offset()));
    int header_bits = DataLayout::flag_mask_to_header_mask(BitData::null_seen_byte_constant());
    __ lw(T9, data_addr);
    __ ori(T9, T9, header_bits);
    __ sw(T9, data_addr);
    __ b(*obj_is_null);
    __ delayed()->nop();
    __ bind(not_null);
  } else {
    __ beq(obj, R0, *obj_is_null);
    __ delayed()->nop();
  }

  if (!k->is_loaded()) {
    klass2reg_with_patching(k_RInfo, op->info_for_patch());
  } else {
#ifdef _LP64
    __ mov_metadata(k_RInfo, k->constant_encoding());
#endif // _LP64
  }
  __ verify_oop(obj);

  if (op->fast_check()) {
    // get object class
    // not a safepoint as obj null check happens earlier
    if (UseCompressedClassPointers) {
      __ load_klass(Rtmp1, obj);
      __ bne_far(k_RInfo, Rtmp1, *failure_target);
      __ delayed()->nop();
    } else {
      __ ld(AT, Address(obj, oopDesc::klass_offset_in_bytes()));
      __ bne_far(k_RInfo, AT, *failure_target);
      __ delayed()->nop();
    }
    // successful cast, fall through to profile or jump
  } else {
    // get object class
    // not a safepoint as obj null check happens earlier
    __ load_klass(klass_RInfo, obj);
    if (k->is_loaded()) {
      // See if we get an immediate positive hit
      __ ld(AT, Address(klass_RInfo, k->super_check_offset()));
      if ((juint)in_bytes(Klass::secondary_super_cache_offset()) != k->super_check_offset()) {
        __ bne_far(k_RInfo, AT, *failure_target);
        __ delayed()->nop();
        // successful cast, fall through to profile or jump
      } else {
        // See if we get an immediate positive hit
        __ beq(k_RInfo, AT, *success_target);
        __ delayed()->nop();
        // check for self
        __ beq(k_RInfo, klass_RInfo, *success_target);
        __ delayed()->nop();

        if (A0 != klass_RInfo) __ push(A0);
        if (A1 != k_RInfo)     __ push(A1);
        if (A0 != klass_RInfo) __ move(A0, klass_RInfo);
        if (A1 != k_RInfo)     __ move(A1, k_RInfo);
        __ call(Runtime1::entry_for(Runtime1::slow_subtype_check_id), relocInfo::runtime_call_type);
        __ delayed()->nop();
        if (A1 != k_RInfo)     __ pop(A1);
        if (A0 != klass_RInfo) __ pop(A0);
        // result is a boolean
        __ beq_far(V0, R0, *failure_target);
        __ delayed()->nop();
        // successful cast, fall through to profile or jump
      }
    } else {
      // perform the fast part of the checking logic
      __ check_klass_subtype_fast_path(klass_RInfo, k_RInfo, Rtmp1, success_target, failure_target, NULL);
      // call out-of-line instance of __ check_klass_subtype_slow_path(...):
      if (A0 != klass_RInfo) __ push(A0);
      if (A1 != k_RInfo)     __ push(A1);
      if (A0 != klass_RInfo) __ move(A0, klass_RInfo);
      if (A1 != k_RInfo)     __ move(A1, k_RInfo);
      __ call(Runtime1::entry_for(Runtime1::slow_subtype_check_id), relocInfo::runtime_call_type);
      __ delayed()->nop();
      if (A1 != k_RInfo)     __ pop(A1);
      if (A0 != klass_RInfo) __ pop(A0);
      // result is a boolean
      __ beq_far(V0, R0, *failure_target);
      __ delayed()->nop();
      // successful cast, fall through to profile or jump
    }
  }
  if (op->should_profile()) {
    Register mdo  = klass_RInfo, recv = k_RInfo;
    __ bind(profile_cast_success);
    __ mov_metadata(mdo, md->constant_encoding());
    __ load_klass(recv, obj);
    Label update_done;
    type_profile_helper(mdo, md, data, recv, success);
    __ b_far(*success);
    __ delayed()->nop();

    __ bind(profile_cast_failure);
    __ mov_metadata(mdo, md->constant_encoding());
    Address counter_addr(mdo, md->byte_offset_of_slot(data, CounterData::count_offset()));
    __ ld_ptr(T9, counter_addr);
    __ addiu(T9, T9, -DataLayout::counter_increment);
    __ st_ptr(T9, counter_addr);

    __ b_far(*failure);
    __ delayed()->nop();
  }
  __ b_far(*success);
  __ delayed()->nop();
}



void LIR_Assembler::emit_opTypeCheck(LIR_OpTypeCheck* op) {
  LIR_Code code = op->code();
  if (code == lir_store_check) {
    Register value = op->object()->as_register();
    Register array = op->array()->as_register();
    Register k_RInfo = op->tmp1()->as_register();
    Register klass_RInfo = op->tmp2()->as_register();
    Register tmp = op->tmp3()->as_register();

    CodeStub* stub = op->stub();

    //check if it needs to be profiled
    ciMethodData* md;
    ciProfileData* data;

    if (op->should_profile()) {
      ciMethod* method = op->profiled_method();
      assert(method != NULL, "Should have method");
      int bci = op->profiled_bci();
      md = method->method_data_or_null();
      assert(md != NULL, "Sanity");
      data = md->bci_to_data(bci);
      assert(data != NULL,                "need data for type check");
      assert(data->is_ReceiverTypeData(), "need ReceiverTypeData for type check");
      }
      Label profile_cast_success, profile_cast_failure, done;
      Label *success_target = op->should_profile() ? &profile_cast_success : &done;
      Label *failure_target = op->should_profile() ? &profile_cast_failure : stub->entry();

      if(op->should_profile()) {
        Label not_null;
        __ bne(value, R0, not_null);
        __ delayed()->nop();

        Register mdo  = klass_RInfo;
        __ mov_metadata(mdo, md->constant_encoding());
        Address data_addr(mdo, md->byte_offset_of_slot(data, DataLayout::header_offset()));
        int header_bits = DataLayout::flag_mask_to_header_mask(BitData::null_seen_byte_constant());
        __ lw(T9, data_addr);
        __ ori(T9, T9, header_bits);
        __ sw(T9, data_addr);
        __ b(done);
        __ delayed()->nop();
        __ bind(not_null);
      } else {
        __ beq(value, R0, done);
        __ delayed()->nop();
      }

    add_debug_info_for_null_check_here(op->info_for_exception());
    __ load_klass(k_RInfo, array);
    __ load_klass(klass_RInfo, value);
    // get instance klass (it's already uncompressed)
    __ ld_ptr(k_RInfo, Address(k_RInfo, ObjArrayKlass::element_klass_offset()));
    // perform the fast part of the checking logic
    __ check_klass_subtype_fast_path(klass_RInfo, k_RInfo, tmp, success_target, failure_target, NULL);
    if (A0 != klass_RInfo) __ push(A0);
    if (A1 != k_RInfo)     __ push(A1);
    if (A0 != klass_RInfo) __ move(A0, klass_RInfo);
    if (A1 != k_RInfo)     __ move(A1, k_RInfo);
    __ call(Runtime1::entry_for(Runtime1::slow_subtype_check_id), relocInfo::runtime_call_type);
    __ delayed()->nop();
    if (A1 != k_RInfo)     __ pop(A1);
    if (A0 != klass_RInfo) __ pop(A0);
    // result is a boolean
    __ beq_far(V0, R0, *failure_target);
    __ delayed()->nop();
    // fall through to the success case

    if (op->should_profile()) {
      Register mdo = klass_RInfo, recv = k_RInfo;
      __ bind(profile_cast_success);
      __ mov_metadata(mdo, md->constant_encoding());
      __ load_klass(recv, value);
      Label update_done;
      type_profile_helper(mdo, md, data, recv, &done);
      __ b_far(done);
      __ delayed()->nop();

      __ bind(profile_cast_failure);
      __ mov_metadata(mdo, md->constant_encoding());
      Address counter_addr(mdo, md->byte_offset_of_slot(data, CounterData::count_offset()));
      __ ld_ptr(T9, counter_addr);
      __ addiu(T9, T9, -DataLayout::counter_increment);
      __ st_ptr(T9, counter_addr);
      __ b_far(*stub->entry());
      __ delayed()->nop();
    }

    __ bind(done);
  } else if (code == lir_checkcast) {
    Register obj = op->object()->as_register();
    Register dst = op->result_opr()->as_register();
    Label success;
    emit_typecheck_helper(op, &success, op->stub()->entry(), &success);
    __ bind(success);
    if (dst != obj) {
      __ move(dst, obj);
    }
  } else if (code == lir_instanceof) {
    Register obj = op->object()->as_register();
    Register dst = op->result_opr()->as_register();
    Label success, failure, done;
    emit_typecheck_helper(op, &success, &failure, &failure);
    __ bind(failure);
    __ move(dst, R0);
    __ b(done);
    __ delayed()->nop();
    __ bind(success);
    __ addiu(dst, R0, 1);
    __ bind(done);
  } else {
    ShouldNotReachHere();
  }
}

void LIR_Assembler::emit_compare_and_swap(LIR_OpCompareAndSwap* op) {
  if (op->code() == lir_cas_long) {
#ifdef _LP64
    Register addr = (op->addr()->is_single_cpu() ? op->addr()->as_register() : op->addr()->as_register_lo());
    Register newval = (op->new_value()->is_single_cpu() ? op->new_value()->as_register() : op->new_value()->as_register_lo());
    Register cmpval = (op->cmp_value()->is_single_cpu() ? op->cmp_value()->as_register() : op->cmp_value()->as_register_lo());
    assert(newval != NULL, "new val must be register");
    assert(cmpval != newval, "cmp and new values must be in different registers");
    assert(cmpval != addr, "cmp and addr must be in different registers");
    assert(newval != addr, "new value and addr must be in different registers");
    __ cmpxchg(newval, addr, cmpval);    // 64-bit test-and-set
#else
    Register addr = op->addr()->as_register();
    __ cmpxchg8(op->new_value()->as_register_lo(),
    op->new_value()->as_register_hi(),
    addr,
    op->cmp_value()->as_register_lo(),
    op->cmp_value()->as_register_hi())
#endif
  } else if (op->code() == lir_cas_int || op->code() == lir_cas_obj) {
    Register addr = op->addr()->as_pointer_register();
    Register cmp_value = op->cmp_value()->as_register();
    Register new_value = op->new_value()->as_register();
    Register tmp1 = op->tmp1()->as_register();
    Register tmp2 = op->tmp2()->as_register();

    assert_different_registers(addr, cmp_value, new_value, tmp1, tmp2);

    if (op->code() == lir_cas_obj) {
#ifdef _LP64
      if (UseCompressedOops) {
          __ move(tmp1, cmp_value);
          __ encode_heap_oop(tmp1);
          __ sll(tmp1, tmp1, 0);
          __ move(tmp2, new_value);
          __ encode_heap_oop(tmp2);
          __ sll(tmp2, tmp2, 0);
          __ cmpxchg32(tmp2, addr, tmp1);  // 32-bit test-and-set
      } else {
          __ cmpxchg(new_value, addr, cmp_value);    // 64-bit test-and-set
      }
    } else
#endif
    {
      __ cmpxchg32(new_value, addr, cmp_value);  // 32-bit test-and-set
    }
  } else {
    Unimplemented();
  }
}

#ifndef MIPS64
void LIR_Assembler::cmove(LIR_Condition condition, LIR_Opr opr1, LIR_Opr opr2, LIR_Opr result) {
    Unimplemented();
}
#endif


void LIR_Assembler::arith_op(LIR_Code code, LIR_Opr left, LIR_Opr right, LIR_Opr dest, CodeEmitInfo *info, bool pop_fpu_stack) {
  assert(info == NULL || ((code == lir_rem || code == lir_div || code == lir_sub) && right->is_double_cpu()), "info is only for ldiv/lrem");
  if (left->is_single_cpu()) {
    Register lreg = left->as_register();

    if (right->is_cpu_register()) {
      Register rreg, res;
      if (right->is_single_cpu()) {
        rreg = right->as_register();
#ifdef _LP64
        if (dest->is_double_cpu())
          res = dest->as_register_lo();
        else
#endif
          res = dest->as_register();
      } else if (right->is_double_cpu()) {
        assert(right->is_double_cpu(), "right must be long");
        rreg = right->as_register_lo();
#ifdef _LP64
        if (dest->is_double_cpu())
          res = dest->as_register_lo();
        else
#endif
          res = dest->as_register();
      } else {
        ShouldNotReachHere();
      }

      switch (code) {
      case lir_add:
#ifdef _LP64
        if (dest->is_single_cpu())
          __ addu32(res, lreg, rreg);
        else
#endif
          __ addu(res, lreg, rreg);
        break;

      case lir_mul:
#ifndef _LP64
        __ mult(lreg, rreg);
        __ mflo(res);
#else
        if (dest->is_single_cpu()) {
          __ mul(res, lreg, rreg);
        } else {
          if (UseLEXT1) {
            __ gsdmult(res, lreg, rreg);
          } else {
            __ dmult(lreg, rreg);
            __ mflo(res);
          }
        }
#endif
        break;

      case lir_sub:
#ifdef _LP64
        if (dest->is_single_cpu())
          __ subu32(res, lreg, rreg);
        else
#endif
          __ subu(res, lreg, rreg);
        break;

      default:
        ShouldNotReachHere();
      }
    } else if (right->is_stack()) {
      // cpu register - stack
      Unimplemented();
    } else if (right->is_constant()) {
      // cpu register - constant
      Register res;
      if (dest->is_double_cpu()) {
        res = dest->as_register_lo();
      } else {
        res = dest->as_register();
      }
      jint c;
      if (right->type() == T_INT) {
        c = right->as_constant_ptr()->as_jint();
      } else {
        c = right->as_constant_ptr()->as_jlong();
      }

      switch (code) {
      case lir_mul_strictfp:
      case lir_mul:
#ifndef _LP64
        __ move(AT, c);
        __ mult(lreg, AT);
        __ mflo(res);
#else
        if (dest->is_single_cpu()) {
          __ move(AT, c);
          __ mul(res, lreg, AT);
        } else {
          __ set64(AT, c);
          if (UseLEXT1) {
            __ gsmult(res, lreg, AT);
          } else {
            __ dmult(lreg, AT);
            __ mflo(res);
          }
        }
#endif
        break;

      case lir_add:
        if (Assembler::is_simm16(c)) {
          if (dest->is_single_cpu()) {
            __ addiu32(res, lreg, c);
          } else {
            __ addiu(res, lreg, c);
          }
        } else {
          if (dest->is_single_cpu()) {
            __ move(AT, c);
            __ addu32(res, lreg, AT);
          } else {
            __ set64(AT, c);
            __ addu(res, lreg, AT);
          }
        }
        break;

      case lir_sub:
        if (Assembler::is_simm16(-c)) {
          if (dest->is_single_cpu()) {
            __ addiu32(res, lreg, -c);
          } else {
            __ addiu(res, lreg, -c);
          }
        } else {
          if (dest->is_single_cpu()) {
            __ move(AT, c);
            __ subu32(res, lreg, AT);
          } else {
            __ set64(AT, c);
            __ subu(res, lreg, AT);
          }
        }
        break;

      default:
        ShouldNotReachHere();
      }
    } else {
      ShouldNotReachHere();
    }

  } else if (left->is_double_cpu()) {
    Register op1_lo = left->as_register_lo();
    Register op1_hi = left->as_register_hi();
    Register op2_lo;
    Register op2_hi;
    Register dst_lo;
    Register dst_hi;

    if (dest->is_single_cpu()) {
      ShouldNotReachHere();
      dst_lo = dest->as_register();
    } else {
#ifdef _LP64
      dst_lo = dest->as_register_lo();
#else
      dst_lo = dest->as_register_lo();
      dst_hi = dest->as_register_hi();
#endif
    }
    if (right->is_constant()) {
      op2_lo = AT;
      op2_hi = R0;
#ifndef _LP64
      __ li(AT, right->as_constant_ptr()->as_jint());
#else
      __ li(AT, right->as_constant_ptr()->as_jlong_bits());
#endif
    } else if (right->is_double_cpu()) { // Double cpu
      assert(right->is_double_cpu(), "right must be long");
      assert(dest->is_double_cpu(), "dest must be long");
      op2_lo = right->as_register_lo();
      op2_hi = right->as_register_hi();
    } else {
#ifdef _LP64
      op2_lo = right->as_register();
#else
      ShouldNotReachHere();
#endif
    }

    NOT_LP64(assert_different_registers(op1_lo, op1_hi, op2_lo, op2_hi));

    switch (code) {
    case lir_add:
#ifndef _LP64
      __ addu(dst_lo, op1_lo, op2_lo);
      __ sltu(AT, dst_lo, op2_lo);
      __ addu(dst_hi, op1_hi, op2_hi);
      __ addu(dst_hi, dst_hi, AT);
#else
      __ addu(dst_lo, op1_lo, op2_lo);
#endif
      break;

    case lir_sub:
#ifndef _LP64
      __ subu(dst_lo, op1_lo, op2_lo);
      __ sltu(AT, op1_lo, dst_lo);
      __ subu(dst_hi, op1_hi, op2_hi);
      __ subu(dst_hi, dst_hi, AT);
#else
      __ subu(dst_lo, op1_lo, op2_lo);
#endif
      break;

    case lir_mul: {

#ifndef _LP64
      Label zero, quick, done;
      // zero?
      __ orr(AT, op2_lo, op1_lo);
      __ beq(AT, R0, zero);
      __ delayed();
      __ move(dst_hi, R0);

      // quick?
      __ orr(AT, op2_hi, op1_hi);
      __ beq(AT, R0, quick);
      __ delayed()->nop();

      __ multu(op2_lo, op1_hi);
      __ nop();
      __ nop();
      __ mflo(dst_hi);
      __ multu(op2_hi, op1_lo);
      __ nop();
      __ nop();
      __ mflo(AT);

      __ bind(quick);
      __ multu(op2_lo, op1_lo);
      __ addu(dst_hi, dst_hi, AT);
      __ nop();
      __ mflo(dst_lo);
      __ mfhi(AT);
      __ b(done);
      __ delayed()->addu(dst_hi, dst_hi, AT);

      __ bind(zero);
      __ move(dst_lo, R0);
      __ bind(done);
#else
      if (UseLEXT1) {
        __ gsdmult(dst_lo, op2_lo, op1_lo);
      } else {
        __ dmult(op2_lo, op1_lo);
        __ mflo(dst_lo);
      }
#endif //_LP64
    } break;

    default:
      ShouldNotReachHere();
    }

  } else if (left->is_single_fpu()) {
    assert(right->is_single_fpu(), "right must be float");
    assert(dest->is_single_fpu(), "dest must be float");

    FloatRegister lreg = left->as_float_reg();
    FloatRegister rreg = right->as_float_reg();
    FloatRegister res = dest->as_float_reg();

    switch (code) {
    case lir_add:
      __ add_s(res, lreg, rreg);
      break;
    case lir_sub:
      __ sub_s(res, lreg, rreg);
      break;
    case lir_mul:
    case lir_mul_strictfp:
      __ mul_s(res, lreg, rreg);
      break;
    case lir_div:
    case lir_div_strictfp:
      __ div_s(res, lreg, rreg);
      break;
    default:
      ShouldNotReachHere();
    }
  } else if (left->is_double_fpu()) {
    assert(right->is_double_fpu(), "right must be double");
    assert(dest->is_double_fpu(), "dest must be double");

    FloatRegister lreg = left->as_double_reg();
    FloatRegister rreg = right->as_double_reg();
    FloatRegister res = dest->as_double_reg();

    switch (code) {
    case lir_add:
      __ add_d(res, lreg, rreg);
      break;
    case lir_sub:
      __ sub_d(res, lreg, rreg);
      break;
    case lir_mul:
    case lir_mul_strictfp:
      __ mul_d(res, lreg, rreg);
      break;
    case lir_div:
    case lir_div_strictfp:
      __ div_d(res, lreg, rreg);
      break;
    default:
      ShouldNotReachHere();
    }
  } else if (left->is_single_stack() || left->is_address()) {
    ShouldNotReachHere();
    assert(left == dest, "left and dest must be equal");

    Address laddr;
    if (left->is_single_stack()) {
      laddr = frame_map()->address_for_slot(left->single_stack_ix());
    } else if (left->is_address()) {
      laddr = rebase_Address(left->as_address_ptr());
    } else {
      ShouldNotReachHere();
    }

    if (right->is_single_cpu()) {
      Register rreg = right->as_register();
      switch (code) {
      case lir_add:
#ifndef _LP64
        __ lw(AT, laddr);
        __ addu(AT, AT, rreg);
        __ sw(AT, laddr);
#else
        __ ld(AT, laddr);
        __ daddu(AT, AT, rreg);
        __ sd(AT, laddr);
#endif
        break;
      case lir_sub:
#ifndef _LP64
        __ lw(AT, laddr);
        __ subu(AT, AT, rreg);
        __ sw(AT, laddr);
#else
        __ ld(AT, laddr);
        __ dsubu(AT, AT, rreg);
        __ sd(AT, laddr);
#endif
        break;
      default:
        ShouldNotReachHere();
      }
    } else if (right->is_constant()) {
#ifndef _LP64
      jint c = right->as_constant_ptr()->as_jint();
#else
      jlong c = right->as_constant_ptr()->as_jlong_bits();
#endif
      switch (code) {
      case lir_add: {
        __ ld_ptr(AT, laddr);
#ifndef _LP64
        __ addiu(AT, AT, c);
#else
        __ li(T8, c);
        __ addu(AT, AT, T8);
#endif
        __ st_ptr(AT, laddr);
        break;
      }
      case lir_sub: {
        __ ld_ptr(AT, laddr);
#ifndef _LP64
        __ addiu(AT, AT, -c);
#else
        __ li(T8, -c);
        __ addu(AT, AT, T8);
#endif
        __ st_ptr(AT, laddr);
        break;
      }
      default:
        ShouldNotReachHere();
      }
    } else {
      ShouldNotReachHere();
    }
  } else {
    ShouldNotReachHere();
  }
}


void LIR_Assembler::intrinsic_op(LIR_Code code, LIR_Opr value, LIR_Opr unused, LIR_Opr dest, LIR_Op *op) {
  if (value->is_double_fpu()) {
    switch(code) {
      case lir_log   : //__ flog() ; break;
      case lir_log10 : //__ flog10() ;
               Unimplemented();
        break;
      case lir_abs   : __ abs_d(dest->as_double_reg(), value->as_double_reg()) ; break;
      case lir_sqrt  : __ sqrt_d(dest->as_double_reg(), value->as_double_reg()); break;
      case lir_sin   :
        __ trigfunc('s', 0);
        break;
      case lir_cos :
        __ trigfunc('c', 0);
        break;
      case lir_tan :
        __ trigfunc('t', 0);
        break;
      default      : ShouldNotReachHere();
    }
  } else {
    Unimplemented();
  }
}

//FIXME, if right is on the stack!
void LIR_Assembler::logic_op(LIR_Code code, LIR_Opr left, LIR_Opr right, LIR_Opr dst) {
  if (left->is_single_cpu()) {
    Register dstreg = dst->as_register();
    Register reg = left->as_register();
    if (right->is_constant()) {
      int val = right->as_constant_ptr()->as_jint();
      __ move(AT, val);
      switch (code) {
        case lir_logic_and:
          __ andr (dstreg, reg, AT);
          break;
        case lir_logic_or:
          __ orr(dstreg, reg, AT);
          break;
        case lir_logic_xor:
          __ xorr(dstreg, reg, AT);
          break;
        default: ShouldNotReachHere();
      }
    } else if (right->is_stack()) {
      // added support for stack operands
      Address raddr = frame_map()->address_for_slot(right->single_stack_ix());
      switch (code) {
        case lir_logic_and:
          //FIXME. lw or ld_ptr?
          __ lw(AT, raddr);
          __ andr(reg, reg,AT);
          break;
        case lir_logic_or:
          __ lw(AT, raddr);
          __ orr(reg, reg, AT);
          break;
        case lir_logic_xor:
          __ lw(AT, raddr);
          __ xorr(reg, reg, AT);
          break;
        default: ShouldNotReachHere();
      }
    } else {
      Register rright = right->as_register();
      switch (code) {
        case lir_logic_and: __ andr (dstreg, reg, rright); break;
        case lir_logic_or : __ orr  (dstreg, reg, rright); break;
        case lir_logic_xor: __ xorr (dstreg, reg, rright); break;
        default: ShouldNotReachHere();
      }
    }
  } else {
    Register l_lo = left->as_register_lo();
    Register dst_lo = dst->as_register_lo();
#ifndef _LP64
    Register l_hi = left->as_register_hi();
    Register dst_hi = dst->as_register_hi();
#endif

    if (right->is_constant()) {
#ifndef _LP64

      int r_lo = right->as_constant_ptr()->as_jint_lo();
      int r_hi = right->as_constant_ptr()->as_jint_hi();

      switch (code) {
        case lir_logic_and:
          __ move(AT, r_lo);
          __ andr(dst_lo, l_lo, AT);
          __ move(AT, r_hi);
          __ andr(dst_hi, l_hi, AT);
          break;

        case lir_logic_or:
          __ move(AT, r_lo);
          __ orr(dst_lo, l_lo, AT);
          __ move(AT, r_hi);
          __ orr(dst_hi, l_hi, AT);
          break;

        case lir_logic_xor:
          __ move(AT, r_lo);
          __ xorr(dst_lo, l_lo, AT);
          __ move(AT, r_hi);
          __ xorr(dst_hi, l_hi, AT);
          break;

        default: ShouldNotReachHere();
      }
#else
      __ li(AT, right->as_constant_ptr()->as_jlong());

      switch (code) {
        case lir_logic_and:
          __ andr(dst_lo, l_lo, AT);
          break;

        case lir_logic_or:
          __ orr(dst_lo, l_lo, AT);
          break;

        case lir_logic_xor:
          __ xorr(dst_lo, l_lo, AT);
          break;

        default: ShouldNotReachHere();
      }
#endif

    } else {
      Register r_lo = right->as_register_lo();
      Register r_hi = right->as_register_hi();

      switch (code) {
        case lir_logic_and:
          __ andr(dst_lo, l_lo, r_lo);
          NOT_LP64(__ andr(dst_hi, l_hi, r_hi);)
            break;
        case lir_logic_or:
          __ orr(dst_lo, l_lo, r_lo);
          NOT_LP64(__ orr(dst_hi, l_hi, r_hi);)
            break;
        case lir_logic_xor:
          __ xorr(dst_lo, l_lo, r_lo);
          NOT_LP64(__ xorr(dst_hi, l_hi, r_hi);)
            break;
        default: ShouldNotReachHere();
      }
    }
  }
}

void LIR_Assembler::arithmetic_idiv(LIR_Code code, LIR_Opr left, LIR_Opr right, LIR_Opr temp, LIR_Opr result, CodeEmitInfo* info) {

  assert(left->is_single_cpu(),   "left must be register");
  assert(right->is_single_cpu() || right->is_constant(),  "right must be register or constant");
  assert(result->is_single_cpu(), "result must be register");

  Register lreg = left->as_register();
  Register rreg = AT;
  Register dreg = result->as_register();

  if (right->is_constant()) {
    int divisor = right->as_constant_ptr()->as_jint();
#ifndef _LP64
    __ move(rreg, divisor);
#else
    __ li(rreg, divisor);
#endif
    int idivl_offset = code_offset();
    __ teq(R0, rreg, 0x7);
    add_debug_info_for_div0(idivl_offset, info);

  } else {
    rreg = right->as_register();
    int idivl_offset = code_offset();
    __ teq(R0, rreg, 0x7);
    add_debug_info_for_div0(idivl_offset, info);
  }

  // get the result
  if (code == lir_irem) {
    __ div(lreg, rreg);
    __ mfhi(dreg);
  } else if (code == lir_idiv) {
    if (UseLEXT1) {
      __ gsdiv(dreg, lreg, rreg);
    } else {
      __ div(lreg, rreg);
      __ mflo(dreg);
    }
  } else {
    ShouldNotReachHere();
  }
}

void LIR_Assembler::arithmetic_frem(LIR_Code code, LIR_Opr left, LIR_Opr right, LIR_Opr temp, LIR_Opr result, CodeEmitInfo* info) {
  if (left->is_single_fpu()) {
    assert(right->is_single_fpu(),"right must be float");
    assert(result->is_single_fpu(), "dest must be float");
    assert(temp->is_single_fpu(), "dest must be float");

    FloatRegister lreg = left->as_float_reg();
    FloatRegister rreg = right->as_float_reg();
    FloatRegister res = result->as_float_reg();
    FloatRegister tmp = temp->as_float_reg();

    switch (code) {
      case lir_frem:
  __ rem_s(res, lreg, rreg, tmp);
  break;
      default     : ShouldNotReachHere();
    }
  } else if (left->is_double_fpu()) {
    assert(right->is_double_fpu(),"right must be double");
    assert(result->is_double_fpu(), "dest must be double");
    assert(temp->is_double_fpu(), "dest must be double");

    FloatRegister lreg = left->as_double_reg();
    FloatRegister rreg = right->as_double_reg();
    FloatRegister res = result->as_double_reg();
    FloatRegister tmp = temp->as_double_reg();

    switch (code) {
      case lir_frem:
  __ rem_d(res, lreg, rreg, tmp);
  break;
      default     : ShouldNotReachHere();
    }
  }
}

void LIR_Assembler::comp_fl2i(LIR_Code code, LIR_Opr left, LIR_Opr right, LIR_Opr dst,LIR_Op2 * op) {
  Register dstreg = dst->as_register();
  if (code == lir_cmp_fd2i) {
    if (left->is_single_fpu()) {
      FloatRegister leftreg = left->as_float_reg();
      FloatRegister rightreg = right->as_float_reg();

      Label done;
      // equal?
      __ c_eq_s(leftreg, rightreg);
      __ bc1t(done);
      __ delayed();
      __ move(dstreg, R0);
      // less?
      __ c_olt_s(leftreg, rightreg);
      __ bc1t(done);
      __ delayed();
      __ move(dstreg, -1);
      // great
      __ move(dstreg, 1);

      __ bind(done);
    } else {
      assert(left->is_double_fpu(), "Must double");
      FloatRegister leftreg = left->as_double_reg();
      FloatRegister rightreg = right->as_double_reg();

      Label done;
      // equal?
      __ c_eq_d(leftreg, rightreg);
      __ bc1t(done);
      __ delayed();
      __ move(dstreg, R0);
      // less?
      __ c_olt_d(leftreg, rightreg);
      __ bc1t(done);
      __ delayed();
      __ move(dstreg, -1);
      // great
      __ move(dstreg, 1);

      __ bind(done);
    }
  } else if (code == lir_ucmp_fd2i) {
    if (left->is_single_fpu()) {
      FloatRegister leftreg = left->as_float_reg();
      FloatRegister rightreg = right->as_float_reg();

      Label done;
      // equal?
      __ c_eq_s(leftreg, rightreg);
      __ bc1t(done);
      __ delayed();
      __ move(dstreg, R0);
      // less?
      __ c_ult_s(leftreg, rightreg);
      __ bc1t(done);
      __ delayed();
      __ move(dstreg, -1);
      // great
      __ move(dstreg, 1);

      __ bind(done);
    } else {
      assert(left->is_double_fpu(), "Must double");
      FloatRegister leftreg = left->as_double_reg();
      FloatRegister rightreg = right->as_double_reg();

      Label done;
      // equal?
      __ c_eq_d(leftreg, rightreg);
      __ bc1t(done);
      __ delayed();
      __ move(dstreg, R0);
      // less?
      __ c_ult_d(leftreg, rightreg);
      __ bc1t(done);
      __ delayed();
      __ move(dstreg, -1);
      // great
      __ move(dstreg, 1);

      __ bind(done);
    }
  } else {
    assert(code == lir_cmp_l2i, "check");
    Register l_lo, l_hi, r_lo, r_hi, d_lo, d_hi;
    l_lo = left->as_register_lo();
    l_hi = left->as_register_hi();
    r_lo = right->as_register_lo();
    r_hi = right->as_register_hi();

    Label done;
#ifndef _LP64
    // less?
    __ slt(AT, l_hi, r_hi);
    __ bne(AT, R0, done);
    __ delayed();
    __ move(dstreg, -1);
    // great?
    __ slt(AT, r_hi, l_hi);
    __ bne(AT, R0, done);
    __ delayed();
    __ move(dstreg, 1);
#endif

    // now compare low 32 bits
    // below?
#ifndef _LP64
    __ sltu(AT, l_lo, r_lo);
#else
    __ slt(AT, l_lo, r_lo);
#endif
    __ bne(AT, R0, done);
    __ delayed();
    __ move(dstreg, -1);
    // above?
#ifndef _LP64
    __ sltu(AT, r_lo, l_lo);
#else
    __ slt(AT, r_lo, l_lo);
#endif
    __ bne(AT, R0, done);
    __ delayed();
    __ move(dstreg, 1);
    // equal
    __ move(dstreg, R0);

    __ bind(done);
  }
}


void LIR_Assembler::align_call(LIR_Code code) {
  if (os::is_MP()) {
    // make sure that the displacement word of the call ends up word aligned
    int offset = __ offset();
    switch (code) {
      case lir_static_call:
      case lir_optvirtual_call:
      case lir_dynamic_call:
        offset += NativeCall::displacement_offset;
        break;
      case lir_icvirtual_call:
        offset += NativeCall::displacement_offset + NativeMovConstReg::instruction_size;
      break;
      case lir_virtual_call:  // currently, sparc-specific for niagara
      default: ShouldNotReachHere();
    }
    while (offset % BytesPerWord != 0) {
      __ nop();
      offset += 4;
    }
  }
}


void LIR_Assembler::call(LIR_OpJavaCall* op, relocInfo::relocType rtype) {
  assert(!os::is_MP() || (__ offset() + NativeCall::displacement_offset) % BytesPerWord == 0, "must be aligned");
  __ call(op->addr(), rtype);
  __ delayed()->nop();
  add_call_info(code_offset(), op->info());
}


void LIR_Assembler::ic_call(LIR_OpJavaCall* op) {
  __ ic_call(op->addr());
  add_call_info(code_offset(), op->info());
}


// Currently, vtable-dispatch is only enabled for sparc platforms
void LIR_Assembler::vtable_call(LIR_OpJavaCall* op) {
    ShouldNotReachHere();
}

void LIR_Assembler::emit_static_call_stub() {
  address call_pc = __ pc();
  address stub = __ start_a_stub(call_stub_size);
  if (stub == NULL) {
    bailout("static call stub overflow");
    return;
  }
  int start = __ offset();
  __ relocate(static_stub_Relocation::spec(call_pc));

  Metadata *o = NULL;
  int index = __ oop_recorder()->allocate_metadata_index(o);
  RelocationHolder rspec = metadata_Relocation::spec(index);
  __ relocate(rspec);
//see set_to_interpreted
  __ patchable_set48(Rmethod, (long)o);

  __ patchable_set48(AT, (long)-1);
  __ jr(AT);
  __ delayed()->nop();
  assert(__ offset() - start <= call_stub_size, "stub too big");
  __ end_a_stub();
}


void LIR_Assembler::throw_op(LIR_Opr exceptionPC, LIR_Opr exceptionOop, CodeEmitInfo* info) {
  assert(exceptionOop->as_register()== V0, "must match");
  assert(exceptionPC->as_register()== V1, "must match");

  // exception object is not added to oop map by LinearScan
  // (LinearScan assumes that no oops are in fixed registers)

  info->add_register_oop(exceptionOop);
  long pc_for_athrow  = (long)__ pc();
  int pc_for_athrow_offset = __ offset();
  Register epc = exceptionPC->as_register();
  __ relocate(relocInfo::internal_pc_type);
  __ li48(epc, pc_for_athrow);
  add_call_info(pc_for_athrow_offset, info); // for exception handler
  __ verify_not_null_oop(V0);
  if (compilation()->has_fpu_code()) {
    __ call(Runtime1::entry_for(Runtime1::handle_exception_id),
      relocInfo::runtime_call_type);
  } else {
    __ call(Runtime1::entry_for(Runtime1::handle_exception_nofpu_id),
      relocInfo::runtime_call_type);
  }
  __ delayed()->nop();
}

void LIR_Assembler::unwind_op(LIR_Opr exceptionOop) {
  assert(exceptionOop->as_register()== FSR, "must match");
  __ b_far(_unwind_handler_entry);
  __ delayed()->nop();
}

void LIR_Assembler::shift_op(LIR_Code code, LIR_Opr left, LIR_Opr count, LIR_Opr dest, LIR_Opr tmp) {
  // optimized version for linear scan:
  // * tmp must be unused
  assert(tmp->is_illegal(), "wasting a register if tmp is allocated");

#ifdef _LP64
  Register count_reg = count->as_register();
  Register value_reg;
  Register dest_reg;
  if (left->is_single_cpu()) {
    value_reg = left->as_register();
    dest_reg = dest->as_register();
    switch (code) {
      case lir_shl:  __ sllv(dest_reg, value_reg, count_reg); break;
      case lir_shr:  __ srav(dest_reg, value_reg, count_reg); break;
      case lir_ushr: __ srlv(dest_reg, value_reg, count_reg); break;
      default: ShouldNotReachHere();
    }

  } else if (left->is_double_cpu()) {
    value_reg = left->as_register_lo();
    dest_reg = dest->as_register_lo();
    switch (code) {
      case lir_shl:  __ dsllv(dest_reg, value_reg, count_reg); break;
      case lir_shr:  __ dsrav(dest_reg, value_reg, count_reg); break;
      case lir_ushr: __ dsrlv(dest_reg, value_reg, count_reg); break;
      default: ShouldNotReachHere();
    }

  } else {
    ShouldNotReachHere();
  }

#else
  if (left->is_single_cpu()) {
    Register value_reg = left->as_register();
    Register count_reg = count->as_register();
    Register dest_reg = dest->as_register();
    assert_different_registers(count_reg, value_reg);

    switch (code) {
      case lir_shl:  __ sllv(dest_reg, value_reg, count_reg); break;
      case lir_shr:  __ srav(dest_reg, value_reg, count_reg); break;
      case lir_ushr: __ srlv(dest_reg, value_reg, count_reg); break;
      default: ShouldNotReachHere();
    }

  } else if (left->is_double_cpu()) {
    Register creg = count->as_register();
    Register lo = left->as_register_lo();
    Register hi = left->as_register_hi();
    Register dlo = dest->as_register_lo();
    Register dhi = dest->as_register_hi();

    __ andi(creg, creg, 0x3f);
    switch (code) {
      case lir_shl:
  {
    Label normal, done, notzero;

    //count=0
    __ bne(creg, R0, notzero);
    __ delayed()->nop();
    __ move(dlo, lo);
    __ b(done);
    __ delayed();
    __ move(dhi, hi);

    //count>=32
    __ bind(notzero);
    __ sltiu(AT, creg, BitsPerWord);
    __ bne(AT, R0, normal);
    __ delayed();
    __ addiu(AT, creg, (-1) * BitsPerWord);
    __ sllv(dhi, lo, AT);
    __ b(done);
    __ delayed();
    __ move(dlo, R0);

    //count<32
    __ bind(normal);
    __ sllv(dhi, hi, creg);
    __ move(AT, BitsPerWord);
    __ subu(AT, AT, creg);
    __ srlv(AT, lo, AT);
    __ orr(dhi, dhi, AT);
    __ sllv(dlo, lo, creg);
    __ bind(done);
  }
  break;
      case lir_shr:
  {
    Label normal, done, notzero;

    //count=0
    __ bne(creg, R0, notzero);
    __ delayed()->nop();
    __ move(dhi, hi);
    __ b(done);
    __ delayed();
    __ move(dlo, lo);

    //count>=32
    __ bind(notzero);
    __ sltiu(AT, creg, BitsPerWord);
    __ bne(AT, R0, normal);
    __ delayed();
    __ addiu(AT, creg, (-1) * BitsPerWord);
    __ srav(dlo, hi, AT);
    __ b(done);
    __ delayed();
    __ sra(dhi, hi, BitsPerWord - 1);

    //count<32
    __ bind(normal);
    __ srlv(dlo, lo, creg);
    __ move(AT, BitsPerWord);
    __ subu(AT, AT, creg);
    __ sllv(AT, hi, AT);
    __ orr(dlo, dlo, AT);
    __ srav(dhi, hi, creg);
    __ bind(done);
  }
  break;
      case lir_ushr:
  {
    Label normal, done, notzero;

    //count=zero
    __ bne(creg, R0, notzero);
    __ delayed()->nop();
    __ move(dhi, hi);
    __ b(done);
    __ delayed();
    __ move(dlo, lo);

    //count>=32
    __ bind(notzero);
    __ sltiu(AT, creg, BitsPerWord);
    __ bne(AT, R0, normal);
    __ delayed();
    __ addiu(AT, creg, (-1) * BitsPerWord);
    __ srlv(dlo, hi, AT);
    __ b(done);
    __ delayed();
    __ move(dhi, R0);

    //count<32
    __ bind(normal);
    __ srlv(dlo, lo, creg);
    __ move(AT, BitsPerWord);
    __ subu(AT, AT, creg);
    __ sllv(AT, hi, AT);
    __ orr(dlo, dlo, AT);
    __ srlv(dhi, hi, creg);
    __ bind(done);
  }
  break;
      default: ShouldNotReachHere();
    }
  } else {
    ShouldNotReachHere();
  }
#endif

}

void LIR_Assembler::shift_op(LIR_Code code, LIR_Opr left, jint  count, LIR_Opr dest) {
  if (dest->is_single_cpu()) {
    Register value_reg = left->is_single_cpu() ? left->as_register() : left->as_register_lo();
    Register dest_reg = dest->as_register();
    count = count & 0x1F; // Java spec

    switch (code) {
      case lir_shl:  __ sll(dest_reg, value_reg, count); break;
      case lir_shr:  __ sra(dest_reg, value_reg, count); break;
      case lir_ushr: __ srl(dest_reg, value_reg, count); break;
      default: ShouldNotReachHere();
    }

  } else if (dest->is_double_cpu()) {
    Register valuelo = left->is_single_cpu() ? left->as_register() : left->as_register_lo();
    Register destlo = dest->as_register_lo();
    count = count & 0x3f;
#ifdef _LP64
    switch (code) {
      case lir_shl:  __ dsll(destlo, valuelo, count); break;
      case lir_shr:  __ dsra(destlo, valuelo, count); break;
      case lir_ushr: __ dsrl(destlo, valuelo, count); break;
      default: ShouldNotReachHere();
    }
#else
    Register desthi = dest->as_register_hi();
    Register valuehi = left->as_register_hi();
    assert_different_registers(destlo, valuehi, desthi);
    switch (code) {
      case lir_shl:
  if (count==0) {
    __ move(destlo, valuelo);
    __ move(desthi, valuehi);
  } else if (count>=32) {
    __ sll(desthi, valuelo, count-32);
    __ move(destlo, R0);
  } else {
    __ srl(AT, valuelo, 32 - count);
    __ sll(destlo, valuelo, count);
    __ sll(desthi, valuehi, count);
    __ orr(desthi, desthi, AT);
  }
  break;

      case lir_shr:
  if (count==0) {
    __ move(destlo, valuelo);
    __ move(desthi, valuehi);
  } else if (count>=32) {
    __ sra(destlo, valuehi, count-32);
    __ sra(desthi, valuehi, 31);
  } else {
    __ sll(AT, valuehi, 32 - count);
    __ sra(desthi, valuehi, count);
    __ srl(destlo, valuelo, count);
    __ orr(destlo, destlo, AT);
  }
  break;

      case lir_ushr:
  if (count==0) {
    __ move(destlo, valuelo);
    __ move(desthi, valuehi);
  } else if (count>=32) {
    __ sra(destlo, valuehi, count-32);
    __ move(desthi, R0);
  } else {
    __ sll(AT, valuehi, 32 - count);
    __ srl(desthi, valuehi, count);
    __ srl(destlo, valuelo, count);
    __ orr(destlo, destlo, AT);
  }
  break;

      default: ShouldNotReachHere();
    }
#endif
  } else {
    ShouldNotReachHere();
  }
}

void LIR_Assembler::store_parameter(Register r, int offset_from_esp_in_words) {
  assert(offset_from_esp_in_words >= 0, "invalid offset from esp");
  int offset_from_sp_in_bytes = offset_from_esp_in_words * BytesPerWord;
  assert(offset_from_esp_in_words < frame_map()->reserved_argument_area_size(), "invalid offset");
  __ st_ptr(r, SP, offset_from_sp_in_bytes);
}


void LIR_Assembler::store_parameter(jint c,     int offset_from_esp_in_words) {
  assert(offset_from_esp_in_words >= 0, "invalid offset from esp");
  int offset_from_sp_in_bytes = offset_from_esp_in_words * BytesPerWord;
  assert(offset_from_esp_in_words < frame_map()->reserved_argument_area_size(), "invalid offset");
  __ move(AT, c);
  __ st_ptr(AT, SP, offset_from_sp_in_bytes);
}

void LIR_Assembler::store_parameter(jobject o,  int offset_from_esp_in_words) {
  assert(offset_from_esp_in_words >= 0, "invalid offset from esp");
  int offset_from_sp_in_bytes = offset_from_esp_in_words * BytesPerWord;
  assert(offset_from_sp_in_bytes < frame_map()->reserved_argument_area_size(), "invalid offset");
  int oop_index = __ oop_recorder()->find_index(o);
  RelocationHolder rspec = oop_Relocation::spec(oop_index);
  __ relocate(rspec);
#ifndef _LP64
  __ lui(AT, Assembler::split_high((int)o));
  __ addiu(AT, AT, Assembler::split_low((int)o));
#else
  __ li48(AT, (long)o);
#endif

  __ st_ptr(AT, SP, offset_from_sp_in_bytes);

}


// This code replaces a call to arraycopy; no exception may
// be thrown in this code, they must be thrown in the System.arraycopy
// activation frame; we could save some checks if this would not be the case
void LIR_Assembler::emit_arraycopy(LIR_OpArrayCopy* op) {
  Unimplemented();

  ciArrayKlass* default_type = op->expected_type();
  Register src = op->src()->as_register();
  Register dst = op->dst()->as_register();
  Register src_pos = op->src_pos()->as_register();
  Register dst_pos = op->dst_pos()->as_register();
  Register length  = op->length()->as_register();
  Register tmp = T8;
#ifndef OPT_THREAD
  Register java_thread = T8;
#else
  Register java_thread = TREG;
#endif
  CodeStub* stub = op->stub();

  int flags = op->flags();
  BasicType basic_type = default_type != NULL ? default_type->element_type()->basic_type() : T_ILLEGAL;
  if (basic_type == T_ARRAY) basic_type = T_OBJECT;

  // if we don't know anything or it's an object array, just go through the generic arraycopy
  if (default_type == NULL) {
    Label done;
    // save outgoing arguments on stack in case call to System.arraycopy is needed
    // HACK ALERT. This code used to push the parameters in a hardwired fashion
    // for interpreter calling conventions. Now we have to do it in new style conventions.
    // For the moment until C1 gets the new register allocator I just force all the
    // args to the right place (except the register args) and then on the back side
    // reload the register args properly if we go slow path. Yuck

    // this is saved in the caller's reserved argument area
    //FIXME, maybe It will change something in the stack;
    // These are proper for the calling convention
    //store_parameter(length, 2);
    //store_parameter(dst_pos, 1);
    //store_parameter(dst, 0);

    // these are just temporary placements until we need to reload
    //store_parameter(src_pos, 3);
    //store_parameter(src, 4);
    assert(src == T0 && src_pos == A0, "mismatch in calling convention");
    // pass arguments: may push as this is not a safepoint; SP must be fix at each safepoint

    __ push(src);
    __ push(dst);
    __ push(src_pos);
    __ push(dst_pos);
    __ push(length);


    // save SP and align
#ifndef OPT_THREAD
    __ get_thread(java_thread);
#endif
    __ st_ptr(SP, java_thread, in_bytes(JavaThread::last_Java_sp_offset()));
#ifndef _LP64
    __ addiu(SP, SP, (-5) * wordSize);
    __ move(AT, -(StackAlignmentInBytes));
    __ andr(SP, SP, AT);
    // push argument
    __ sw(length, SP, 4 * wordSize);
#else
    __ move(A4, length);
#endif
    __ move(A3, dst_pos);
    __ move(A2, dst);
    __ move(A1, src_pos);
    __ move(A0, src);
    // make call
    address entry = CAST_FROM_FN_PTR(address, Runtime1::arraycopy);
    __ call(entry, relocInfo::runtime_call_type);
    __ delayed()->nop();
    // restore SP
#ifndef OPT_THREAD
    __ get_thread(java_thread);
#endif
    __ ld_ptr(SP, java_thread, in_bytes(JavaThread::last_Java_sp_offset()));
    __ super_pop(length);
    __ super_pop(dst_pos);
    __ super_pop(src_pos);
    __ super_pop(dst);
    __ super_pop(src);

    __ beq_far(V0, R0, *stub->continuation());
    __ delayed()->nop();


    __ b_far(*stub->entry());
    __ delayed()->nop();
    __ bind(*stub->continuation());
    return;
  }
  assert(default_type != NULL
      && default_type->is_array_klass()
      && default_type->is_loaded(),
      "must be true at this point");

  int elem_size = type2aelembytes(basic_type);
  int shift_amount;
  switch (elem_size) {
    case 1 :shift_amount = 0; break;
    case 2 :shift_amount = 1; break;
    case 4 :shift_amount = 2; break;
    case 8 :shift_amount = 3; break;
    default:ShouldNotReachHere();
  }

  Address src_length_addr = Address(src, arrayOopDesc::length_offset_in_bytes());
  Address dst_length_addr = Address(dst, arrayOopDesc::length_offset_in_bytes());
  Address src_klass_addr = Address(src, oopDesc::klass_offset_in_bytes());
  Address dst_klass_addr = Address(dst, oopDesc::klass_offset_in_bytes());

  // test for NULL
  if (flags & LIR_OpArrayCopy::src_null_check) {
    __ beq_far(src, R0, *stub->entry());
    __ delayed()->nop();
  }
  if (flags & LIR_OpArrayCopy::dst_null_check) {
    __ beq_far(dst, R0, *stub->entry());
    __ delayed()->nop();
  }

  // check if negative
  if (flags & LIR_OpArrayCopy::src_pos_positive_check) {
    __ bltz(src_pos, *stub->entry());
    __ delayed()->nop();
  }
  if (flags & LIR_OpArrayCopy::dst_pos_positive_check) {
    __ bltz(dst_pos, *stub->entry());
    __ delayed()->nop();
  }
  if (flags & LIR_OpArrayCopy::length_positive_check) {
    __ bltz(length, *stub->entry());
    __ delayed()->nop();
  }

  if (flags & LIR_OpArrayCopy::src_range_check) {
    __ addu(AT, src_pos, length);
    __ lw(tmp, src_length_addr);
    __ sltu(AT, tmp, AT);
    __ bne_far(AT, R0, *stub->entry());
    __ delayed()->nop();
  }
  if (flags & LIR_OpArrayCopy::dst_range_check) {
    __ addu(AT, dst_pos, length);
    __ lw(tmp, dst_length_addr);
    __ sltu(AT, tmp, AT);
    __ bne_far(AT, R0, *stub->entry());
    __ delayed()->nop();
  }

  if (flags & LIR_OpArrayCopy::type_check) {
    if (UseCompressedClassPointers) {
      __ lwu(AT, src_klass_addr);
      __ lwu(tmp, dst_klass_addr);
    } else {
      __ ld(AT, src_klass_addr);
      __ ld(tmp, dst_klass_addr);
    }
    __ bne_far(AT, tmp, *stub->entry());
    __ delayed()->nop();
  }

#ifdef ASSERT
  if (basic_type != T_OBJECT || !(flags & LIR_OpArrayCopy::type_check)) {
    // Sanity check the known type with the incoming class.  For the
    // primitive case the types must match exactly.  For the object array
    // case, if no type check is needed then the dst type must match the
    // expected type and the src type is so subtype which we can't check.  If
    // a type check i needed then at this point the classes are known to be
    // the same but again which don't know which type so we can't check them.
    Label known_ok, halt;
    __ mov_metadata(tmp, default_type->constant_encoding());
#ifdef _LP64
    if (UseCompressedClassPointers) {
      __ encode_klass_not_null(tmp);
    }
#endif
    if (basic_type != T_OBJECT) {
      if (UseCompressedClassPointers) {
        __ lwu(AT, dst_klass_addr);
      } else {
        __ ld(AT, dst_klass_addr);
      }
      __ bne(AT, tmp, halt);
      __ delayed()->nop();
      if (UseCompressedClassPointers) {
        __ lwu(AT, src_klass_addr);
      }  else {
        __ ld(AT, src_klass_addr);
      }
      __ beq(AT, tmp, known_ok);
      __ delayed()->nop();
    } else {
      if (UseCompressedClassPointers) {
        __ lwu(AT, dst_klass_addr);
      } else {
        __ ld(AT, dst_klass_addr);
      }
      __ beq(AT, tmp, known_ok);
      __ delayed()->nop();
      __ beq(src, dst, known_ok);
      __ delayed()->nop();
    }
    __ bind(halt);
    __ stop("incorrect type information in arraycopy");
    __ bind(known_ok);
  }
#endif
  __ push(src);
  __ push(dst);
  __ push(src_pos);
  __ push(dst_pos);
  __ push(length);


  assert(A0 != A1 &&
      A0 != length &&
      A1 != length, "register checks");
  __ move(AT, dst_pos);
  if (shift_amount > 0 && basic_type != T_OBJECT) {
#ifndef _LP64
    __ sll(A2, length, shift_amount);
#else
    __ dsll(A2, length, shift_amount);
#endif
  } else {
    if (length!=A2)
      __ move(A2, length);
  }
  __ move(A3, src_pos );
  assert(A0 != dst_pos &&
      A0 != dst &&
      dst_pos != dst, "register checks");

  assert_different_registers(A0, dst_pos, dst);
#ifndef _LP64
  __ sll(AT, AT, shift_amount);
#else
  __ dsll(AT, AT, shift_amount);
#endif
  __ addiu(AT, AT, arrayOopDesc::base_offset_in_bytes(basic_type));
  __ addu(A1, dst, AT);

#ifndef _LP64
  __ sll(AT, A3, shift_amount);
#else
  __ dsll(AT, A3, shift_amount);
#endif
  __ addiu(AT, AT, arrayOopDesc::base_offset_in_bytes(basic_type));
  __ addu(A0, src, AT);



  if (basic_type == T_OBJECT) {
    __ call_VM_leaf(CAST_FROM_FN_PTR(address, Runtime1::oop_arraycopy), 3);
  } else {
    __ call_VM_leaf(CAST_FROM_FN_PTR(address, Runtime1::primitive_arraycopy), 3);
  }
  __ super_pop(length);
  __ super_pop(dst_pos);
  __ super_pop(src_pos);
  __ super_pop(dst);
  __ super_pop(src);

  __ bind(*stub->continuation());
}

void LIR_Assembler::emit_updatecrc32(LIR_OpUpdateCRC32* op) {
  tty->print_cr("LIR_Assembler::emit_updatecrc32 unimplemented yet !");
  Unimplemented();
}

void LIR_Assembler::emit_lock(LIR_OpLock* op) {
  Register obj = op->obj_opr()->as_register();  // may not be an oop
  Register hdr = op->hdr_opr()->as_register();
  Register lock = op->lock_opr()->is_single_cpu() ? op->lock_opr()->as_register(): op->lock_opr()->as_register_lo();
  if (!UseFastLocking) {
    __ b_far(*op->stub()->entry());
    __ delayed()->nop();
  } else if (op->code() == lir_lock) {
    Register scratch = noreg;
    if (UseBiasedLocking) {
      scratch = op->scratch_opr()->as_register();
    }
    assert(BasicLock::displaced_header_offset_in_bytes() == 0,
      "lock_reg must point to the displaced header");
    // add debug info for NullPointerException only if one is possible
    int null_check_offset = __ lock_object(hdr, obj, lock, scratch, *op->stub()->entry());
    if (op->info() != NULL) {
      //add_debug_info_for_null_check_here(op->info());
      add_debug_info_for_null_check(null_check_offset,op->info());
    }
    // done
  } else if (op->code() == lir_unlock) {
    assert(BasicLock::displaced_header_offset_in_bytes() == 0,
      "lock_reg must point to the displaced header");
    __ unlock_object(hdr, obj, lock, *op->stub()->entry());
  } else {
    Unimplemented();
  }
  __ bind(*op->stub()->continuation());
}



void LIR_Assembler::emit_profile_call(LIR_OpProfileCall* op) {
  ciMethod* method = op->profiled_method();
  int bci          = op->profiled_bci();
  ciMethod* callee = op->profiled_callee();

  Register tmp = T9;

  // Update counter for all call types
  ciMethodData* md = method->method_data_or_null();
  assert(md != NULL, "Sanity");
  ciProfileData* data = md->bci_to_data(bci);
  assert(data->is_CounterData(), "need CounterData for calls");
  assert(op->mdo()->is_single_cpu(),  "mdo must be allocated");
  Register mdo  = op->mdo()->as_register();
  __ mov_metadata(mdo, md->constant_encoding());
  Address counter_addr(mdo, md->byte_offset_of_slot(data, CounterData::count_offset()));
  Bytecodes::Code bc = method->java_code_at_bci(bci);
  const bool callee_is_static = callee->is_loaded() && callee->is_static();

  // Perform additional virtual call profiling for invokevirtual and
  // invokeinterface bytecodes
  if ((bc == Bytecodes::_invokevirtual || bc == Bytecodes::_invokeinterface) &&
      !callee_is_static && //required for optimized MH invokes
      C1ProfileVirtualCalls) {
      assert(op->recv()->is_single_cpu(), "recv must be allocated");
      Register recv = op->recv()->as_register();
      assert_different_registers(mdo, recv);
      assert(data->is_VirtualCallData(), "need VirtualCallData for virtual calls");
      ciKlass* known_klass = op->known_holder();
    if (C1OptimizeVirtualCallProfiling && known_klass != NULL) {
      // We know the type that will be seen at this call site; we can
      // statically update the MethodData* rather than needing to do
      // dynamic tests on the receiver type

      // NOTE: we should probably put a lock around this search to
      // avoid collisions by concurrent compilations
      ciVirtualCallData* vc_data = (ciVirtualCallData*) data;
      uint i;
      for (i = 0; i < VirtualCallData::row_limit(); i++) {
        ciKlass* receiver = vc_data->receiver(i);
        if (known_klass->equals(receiver)) {
          Address data_addr(mdo, md->byte_offset_of_slot(data, VirtualCallData::receiver_count_offset(i)));
          __ ld_ptr(tmp, data_addr);
          __ addiu(tmp, tmp, DataLayout::counter_increment);
          __ st_ptr(tmp, data_addr);
          return;
        }
      }

      // Receiver type not found in profile data; select an empty slot

      // Note that this is less efficient than it should be because it
      // always does a write to the receiver part of the
      // VirtualCallData rather than just the first time
      for (i = 0; i < VirtualCallData::row_limit(); i++) {
        ciKlass* receiver = vc_data->receiver(i);
        if (receiver == NULL) {
          Address recv_addr(mdo, md->byte_offset_of_slot(data, VirtualCallData::receiver_offset(i)));
          __ mov_metadata(tmp, known_klass->constant_encoding());
          __ st_ptr(tmp,recv_addr);
          Address data_addr(mdo, md->byte_offset_of_slot(data, VirtualCallData::receiver_count_offset(i)));
          __ ld_ptr(tmp, data_addr);
          __ addiu(tmp, tmp, DataLayout::counter_increment);
          __ st_ptr(tmp, data_addr);
          return;
        }
      }
    } else {
      __ load_klass(recv, recv);
      Label update_done;
      type_profile_helper(mdo, md, data, recv, &update_done);

      // Receiver did not match any saved receiver and there is no empty row for it.
      // Increment total counter to indicate polymorphic case.
      __ ld_ptr(tmp, counter_addr);
      __ addiu(tmp, tmp, DataLayout::counter_increment);
      __ st_ptr(tmp, counter_addr);

      __ bind(update_done);
    }
  } else {
    __ ld_ptr(tmp, counter_addr);
    __ addiu(tmp, tmp, DataLayout::counter_increment);
    __ st_ptr(tmp, counter_addr);
  }
}

void LIR_Assembler::emit_profile_type(LIR_OpProfileType* op) {
  Register obj = op->obj()->as_register();
  Register tmp = op->tmp()->as_pointer_register();
  Register tmp1 = T9;
  Address mdo_addr = rebase_Address(op->mdp()->as_address_ptr());
  ciKlass* exact_klass = op->exact_klass();
  intptr_t current_klass = op->current_klass();
  bool not_null = op->not_null();
  bool no_conflict = op->no_conflict();

  Label update, next, none;

  bool do_null = !not_null;
  bool exact_klass_set = exact_klass != NULL && ciTypeEntries::valid_ciklass(current_klass) == exact_klass;
  bool do_update = !TypeEntries::is_type_unknown(current_klass) && !exact_klass_set;

  assert(do_null || do_update, "why are we here?");
  assert(!TypeEntries::was_null_seen(current_klass) || do_update, "why are we here?");
  guarantee(mdo_addr.base() != tmp1, "The base register will be corrupted");

  __ verify_oop(obj);

  if (tmp != obj) {
    __ move(tmp, obj);
  }
  if (do_null) {
    __ bne(tmp, R0, update);
    __ delayed()->nop();
    if (!TypeEntries::was_null_seen(current_klass)) {
      __ push(tmp1);
      if (mdo_addr.index() == noreg) {
        __ ld(tmp1, mdo_addr);
      } else {
        guarantee(tmp1 != mdo_addr.index(), "The index register will be corrupted !");

        __ dsll(AT, mdo_addr.index(), mdo_addr.scale());
        __ daddu(AT, AT, mdo_addr.base());
        __ ld(tmp1, AT, mdo_addr.disp());
      }
      __ li(AT, TypeEntries::null_seen);
      __ orr(AT, tmp1, AT);
      __ sd(AT, mdo_addr);
      __ pop(tmp1);

    }
    if (do_update) {
#ifndef ASSERT
      __ b(next);
      __ delayed()->nop();
    }
#else
      __ b(next);
      __ delayed()->nop();
    }
  } else {
    __ bne(tmp, R0, update);
    __ delayed()->nop();
    __ stop("unexpect null obj");
#endif
  }

  __ bind(update);

  if (do_update) {
#ifdef ASSERT
    if (exact_klass != NULL) {
      Label ok;
      __ load_klass(tmp, tmp);
      __ push(tmp);
      __ mov_metadata(tmp, exact_klass->constant_encoding());
      __ ld(AT, Address(SP, 0));
      __ beq(tmp, AT, ok);
      __ delayed()->nop();
      __ stop("exact klass and actual klass differ");
      __ bind(ok);
      __ pop(tmp);
    }
#endif
    if (!no_conflict) {
      if (exact_klass == NULL || TypeEntries::is_type_none(current_klass)) {
        if (exact_klass != NULL) {
          __ mov_metadata(tmp, exact_klass->constant_encoding());
        } else {
          __ load_klass(tmp, tmp);
        }

        if (mdo_addr.index() == noreg) {
          __ ld(AT, mdo_addr);
        } else {
          __ dsll(AT, mdo_addr.index(), mdo_addr.scale());
          __ daddu(AT, AT, mdo_addr.base());
          __ ld(AT, AT, mdo_addr.disp());
        }
        __ xorr(tmp, tmp, AT);
        __ li(AT, TypeEntries::type_klass_mask);
        __ andr(AT, tmp, AT);
        // klass seen before, nothing to do. The unknown bit may have been
        // set already but no need to check.
        __ beq(AT, R0, next);
        __ delayed()->nop();

        __ li(AT, TypeEntries::type_unknown);
        __ andr(AT, tmp, AT);
        __ bne(AT, R0, next); // already unknown. Nothing to do anymore.
        __ delayed()->nop();

        if (TypeEntries::is_type_none(current_klass)) {
          if (mdo_addr.index() == noreg) {
            __ ld(AT, mdo_addr);
          } else {
            __ dsll(AT, mdo_addr.index(), mdo_addr.scale());
            __ daddu(AT, AT, mdo_addr.base());
            __ ld(AT, AT, mdo_addr.disp());
          }
          __ beq(AT, R0, none);
          __ delayed()->nop();

          __ push(tmp1);
          if (mdo_addr.index() == noreg) {
            __ ld(tmp1, mdo_addr);
          } else {
            guarantee(tmp1 != mdo_addr.index(), "The index register will be corrupted !");

            __ dsll(AT, mdo_addr.index(), mdo_addr.scale());
            __ daddu(AT, AT, mdo_addr.base());
            __ ld(tmp1, AT, mdo_addr.disp());
          }
          __ li(AT, TypeEntries::null_seen);
          __ subu(AT, AT, tmp1);
          __ pop(tmp1);
          __ beq(AT, R0, none);
          __ delayed()->nop();
          // There is a chance that the checks above (re-reading profiling
          // data from memory) fail if another thread has just set the
          // profiling to this obj's klass

          if (mdo_addr.index() == noreg) {
            __ ld(AT, mdo_addr);
          } else {
            __ dsll(AT, mdo_addr.index(), mdo_addr.scale());
            __ daddu(AT, AT, mdo_addr.base());
            __ ld(AT, AT, mdo_addr.disp());
          }
          __ xorr(tmp, tmp, AT);
          __ li(AT, TypeEntries::type_klass_mask);
          __ andr(AT, tmp, AT);
          __ beq(AT, R0, next);
          __ delayed()->nop();
        }
      } else {
        assert(ciTypeEntries::valid_ciklass(current_klass) != NULL &&
               ciTypeEntries::valid_ciklass(current_klass) != exact_klass, "conflict only");
        if (mdo_addr.index() == noreg) {
          __ ld(tmp, mdo_addr);
        } else {
          __ dsll(AT, mdo_addr.index(), mdo_addr.scale());
          __ daddu(AT, AT, mdo_addr.base());
          __ ld(tmp, AT, mdo_addr.disp());
        }
        __ li(AT, TypeEntries::type_unknown);
        __ andr(AT, tmp, AT);
        __ bne(AT, R0, next); // already unknown. Nothing to do anymore.
        __ delayed()->nop();
      }

      // different than before. Cannot keep accurate profile.
      __ push(tmp1);
      if (mdo_addr.index() == noreg) {
        __ ld(tmp1, mdo_addr);
      } else {
        guarantee(tmp1 != mdo_addr.index(), "The index register will be corrupted !");

        __ dsll(AT, mdo_addr.index(), mdo_addr.scale());
        __ daddu(AT, AT, mdo_addr.base());
        __ ld(tmp1, AT, mdo_addr.disp());
      }
      __ li(AT, TypeEntries::type_unknown);
      __ orr(AT, tmp1, AT);
      __ sd(AT, mdo_addr);
      __ pop(tmp1);

      if (TypeEntries::is_type_none(current_klass)) {
        __ b(next);
        __ delayed()->nop();

        __ bind(none);
        // first time here. Set profile type.
        __ sd(tmp, mdo_addr);
      }
    } else {
      // There's a single possible klass at this profile point
      assert(exact_klass != NULL, "should be");
      if (TypeEntries::is_type_none(current_klass)) {
        __ mov_metadata(tmp, exact_klass->constant_encoding());
        if (mdo_addr.index() == noreg) {
          __ ld(AT, mdo_addr);
        } else {
          __ dsll(AT, mdo_addr.index(), mdo_addr.scale());
          __ daddu(AT, AT, mdo_addr.base());
          __ ld(AT, AT, mdo_addr.disp());
        }
        __ xorr(tmp, tmp, AT);
        __ li(AT, TypeEntries::type_klass_mask);
        __ andr(AT, tmp, AT);
#ifdef ASSERT
        __ beq(AT, R0, next);
        __ delayed()->nop();

        {
          Label ok;
          __ push(tmp);
          if (mdo_addr.index() == noreg) {
            __ ld(AT, mdo_addr);
          } else {
            __ dsll(AT, mdo_addr.index(), mdo_addr.scale());
            __ daddu(AT, AT, mdo_addr.base());
            __ ld(AT, AT, mdo_addr.disp());
          }
          __ beq(AT, R0, ok);
          __ delayed()->nop();

          __ push(tmp1);
          if (mdo_addr.index() == noreg) {
            __ ld(tmp1, mdo_addr);
          } else {
            guarantee(tmp1 != mdo_addr.index(), "The index register will be corrupted !");

            __ dsll(AT, mdo_addr.index(), mdo_addr.scale());
            __ daddu(AT, AT, mdo_addr.base());
            __ ld(tmp1, AT, mdo_addr.disp());
          }
          __ li(AT, TypeEntries::null_seen);
          __ subu(AT, AT, tmp1);
          __ pop(tmp1);
          __ beq(AT, R0, ok);
          __ delayed()->nop();
          // may have been set by another thread
          __ mov_metadata(tmp, exact_klass->constant_encoding());
          if (mdo_addr.index() == noreg) {
            __ ld(AT, mdo_addr);
          } else {
            __ dsll(AT, mdo_addr.index(), mdo_addr.scale());
            __ daddu(AT, AT, mdo_addr.base());
            __ ld(AT, AT, mdo_addr.disp());
          }
          __ xorr(tmp, tmp, AT);
          __ li(AT, TypeEntries::type_mask);
          __ andr(AT, tmp, AT);
          __ beq(AT, R0, ok);
          __ delayed()->nop();

          __ stop("unexpected profiling mismatch");
          __ bind(ok);
          __ pop(tmp);
        }
#else
        __ beq(AT, R0, next);
        __ delayed()->nop();
#endif
        // first time here. Set profile type.
        __ sd(tmp, mdo_addr);
      } else {
        assert(ciTypeEntries::valid_ciklass(current_klass) != NULL &&
               ciTypeEntries::valid_ciklass(current_klass) != exact_klass, "inconsistent");

        if (mdo_addr.index() == noreg) {
          __ ld(tmp, mdo_addr);
        } else {
          __ dsll(AT, mdo_addr.index(), mdo_addr.scale());
          __ daddu(AT, AT, mdo_addr.base());
          __ ld(tmp, AT, mdo_addr.disp());
        }
        __ li(AT, TypeEntries::type_unknown);
        __ andr(AT, tmp, AT);
        __ bne(AT, R0, next); // already unknown. Nothing to do anymore.
        __ delayed()->nop();

        __ push(tmp1);
        if (mdo_addr.index() == noreg) {
          __ ld(tmp1, mdo_addr);
        } else {
          guarantee(tmp1 != mdo_addr.index(), "The index register will be corrupted !");

          __ dsll(AT, mdo_addr.index(), mdo_addr.scale());
          __ daddu(AT, AT, mdo_addr.base());
          __ ld(tmp1, AT, mdo_addr.disp());
        }
        __ li(AT, TypeEntries::type_unknown);
        __ orr(AT, tmp1, AT);
        __ sd(AT, mdo_addr);
        __ pop(tmp1);
      }
    }

    __ bind(next);
  }
}

void LIR_Assembler::emit_delay(LIR_OpDelay*) {
  Unimplemented();
}


void LIR_Assembler::monitor_address(int monitor_no, LIR_Opr dst) {
  if (dst->is_single_cpu())
    __ lea(dst->as_register(), frame_map()->address_for_monitor_lock(monitor_no));
  else if (dst->is_double_cpu())
    __ lea(dst->as_register_lo(), frame_map()->address_for_monitor_lock(monitor_no));
}

void LIR_Assembler::align_backward_branch_target() {
  __ align(BytesPerWord);
}


void LIR_Assembler::negate(LIR_Opr left, LIR_Opr dest) {
  if (left->is_single_cpu()) {
    __ subu32(dest->as_register(), R0, left->as_register());
  } else if (left->is_double_cpu()) {
#ifndef _LP64
    Register lo = left->as_register_lo();
    Register hi = left->as_register_hi();
    Register dlo = dest->as_register_lo();
    Register dhi = dest->as_register_hi();
    assert(dlo != hi, "register checks");
    __ nor(dlo, R0, lo);
    __ addiu(dlo, dlo, 1);
    __ sltiu(AT, dlo, 1);
    __ nor(dhi, R0, hi);
    __ addu(dhi, dhi, AT);
#else
    __ subu(dest->as_register_lo(), R0, left->as_register_lo());
#endif
  } else if (left->is_single_fpu()) {
    //for mips , does it required ?
    __ neg_s(dest->as_float_reg(), left->as_float_reg());
  } else if (left->is_double_fpu()) {
    //for mips , does it required ?
    __ neg_d(dest->as_double_reg(), left->as_double_reg());
  } else {
    ShouldNotReachHere();
  }
}

void LIR_Assembler::leal(LIR_Opr addr, LIR_Opr dest) {
  assert(addr->is_address() && dest->is_register(), "check");
  Register reg;
  reg = dest->as_pointer_register();
  __ lea(reg, rebase_Address(addr->as_address_ptr()));
}


void LIR_Assembler::jobject2reg(jobject o, Register reg) {
  if (o == NULL) {
    // This seems wrong as we do not emit relocInfo
    // for classes that are not loaded yet, i.e., they will be
    // never GC'd
#ifndef _LP64
    __ lui(reg, Assembler::split_high((int)o));
    __ addiu(reg, reg, Assembler::split_low((int)o));
#else
    __ li48(reg, (long)o);
    //__ patchable_set48(reg, (long)o);
#endif
  } else {
    int oop_index = __ oop_recorder()->find_index(o);
    RelocationHolder rspec = oop_Relocation::spec(oop_index);
    __ relocate(rspec);
#ifndef _LP64
    __ lui(reg, Assembler::split_high((int)o));
    __ addiu(reg, reg, Assembler::split_low((int)o));
#else
    __ li48(reg, (long)o);
    //__ patchable_set48(reg, (long)o);
#endif
  }
}

void LIR_Assembler::rt_call(LIR_Opr result, address dest, const LIR_OprList* args, LIR_Opr tmp, CodeEmitInfo* info) {
   assert(!tmp->is_valid(), "don't need temporary");
  __ call(dest, relocInfo::runtime_call_type);
  __ delayed()->nop();
  if (info != NULL) {
    add_call_info_here(info);
  }
}

void LIR_Assembler::volatile_move_op(LIR_Opr src, LIR_Opr dest, BasicType type, CodeEmitInfo* info) {
  assert(type == T_LONG, "only for volatile long fields");
  if (info != NULL) {
    add_debug_info_for_null_check_here(info);
  }

  if(src->is_register() && dest->is_address()) {
    if(src->is_double_cpu()) {
#ifdef _LP64
      __ sd(src->as_register_lo(), rebase_Address(dest->as_address_ptr()));
#else
      Unimplemented();
      //__ sw(src->as_register_lo(), as_Address(dest->as_address_ptr()));
      //__ sw(src->as_register_hi(), as_Address(dest->as_address_ptr()).base(),
      //as_Address(dest->as_address_ptr()).disp() +4);
#endif
    } else if (src->is_double_fpu()) {
#ifdef _LP64
      __ sdc1(src->as_double_reg(), rebase_Address(dest->as_address_ptr()));
#else
      Unimplemented();
      //__ swc1(src->as_fpu_lo(), as_Address(dest->as_address_ptr()));
      //__ swc1(src->as_fpu_hi(), as_Address(dest->as_address_ptr()).base(),
      //as_Address(dest->as_address_ptr()).disp() +4);
#endif
    } else {
      ShouldNotReachHere();
    }
  } else if (src->is_address() && dest->is_register()){
    if(dest->is_double_cpu()) {
#ifdef _LP64
      __ ld(dest->as_register_lo(), rebase_Address(src->as_address_ptr()));
#else
      Unimplemented();
     // __ lw(dest->as_register_lo(), as_Address(src->as_address_ptr()));
     // __ lw(dest->as_register_hi(), as_Address(src->as_address_ptr()).base(),
     // as_Address(src->as_address_ptr()).disp() +4);
#endif
    } else if (dest->is_double_fpu()) {
#ifdef _LP64
      __ ldc1(dest->as_double_reg(), rebase_Address(src->as_address_ptr()));
#else
      Unimplemented();
     // __ lwc1(dest->as_fpu_lo(), as_Address(src->as_address_ptr()));
     // __ lwc1(dest->as_fpu_hi(), as_Address(src->as_address_ptr()).base(),
     // as_Address(src->as_address_ptr()).disp() +4);
#endif
    } else {
      ShouldNotReachHere();
    }
  } else {
    ShouldNotReachHere();
  }
}

#ifdef ASSERT
// emit run-time assertion
void LIR_Assembler::emit_assert(LIR_OpAssert* op) {
  tty->print_cr("LIR_Assembler::emit_assert unimplemented yet!");
  Unimplemented();
}
#endif

void LIR_Assembler::membar() {
  __ sync();
}

void LIR_Assembler::membar_acquire() {
  __ sync();
}

void LIR_Assembler::membar_release() {
  __ sync();
}

void LIR_Assembler::membar_loadload() {
  __ sync();
}

void LIR_Assembler::membar_storestore() {
  __ sync();
}

void LIR_Assembler::membar_loadstore() {
  __ sync();
}

void LIR_Assembler::membar_storeload() {
  __ sync();
}


void LIR_Assembler::get_thread(LIR_Opr result_reg) {
  assert(result_reg->is_register(), "check");
#ifndef OPT_THREAD
  __ get_thread(NOT_LP64(result_reg->as_register()) LP64_ONLY(result_reg->as_register_lo()));
#else
  __ move(NOT_LP64(result_reg->as_register()) LP64_ONLY(result_reg->as_register_lo()), TREG);
#endif
}

void LIR_Assembler::peephole(LIR_List*) {
  // do nothing for now
}

void LIR_Assembler::atomic_op(LIR_Code code, LIR_Opr src, LIR_Opr data, LIR_Opr dest, LIR_Opr tmp) {
/*  assert(data == dest, "xchg/xadd uses only 2 operands");

  if (data->type() == T_INT) {
    if (code == lir_xadd) {
      if (os::is_MP()) {
        __ lock();
      }
      __ xaddl(as_Address(src->as_address_ptr()), data->as_register());
    } else {
      __ xchgl(data->as_register(), as_Address(src->as_address_ptr()));
    }
  } else if (data->is_oop()) {
    assert (code == lir_xchg, "xadd for oops");
    Register obj = data->as_register();
#ifdef _LP64
    if (UseCompressedOops) {
      __ encode_heap_oop(obj);
      __ xchgl(obj, as_Address(src->as_address_ptr()));
      __ decode_heap_oop(obj);
    } else {
      __ xchgptr(obj, as_Address(src->as_address_ptr()));
    }
#else
    __ xchgl(obj, as_Address(src->as_address_ptr()));
#endif
  } else if (data->type() == T_LONG) {
#ifdef _LP64
    assert(data->as_register_lo() == data->as_register_hi(), "should be a single register");
    if (code == lir_xadd) {
      if (os::is_MP()) {
        __ lock();
      }
      __ xaddq(as_Address(src->as_address_ptr()), data->as_register_lo());
    } else {
      __ xchgq(data->as_register_lo(), as_Address(src->as_address_ptr()));
    }
#else
    ShouldNotReachHere();
#endif
  } else {
    ShouldNotReachHere();
  }*/
  ShouldNotReachHere();
}

#undef __
