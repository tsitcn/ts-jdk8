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
#include "c1/c1_MacroAssembler.hpp"
#include "c1/c1_Runtime1.hpp"
#include "classfile/systemDictionary.hpp"
#include "gc_interface/collectedHeap.hpp"
#include "interpreter/interpreter.hpp"
#include "oops/arrayOop.hpp"
#include "oops/markOop.hpp"
#include "runtime/basicLock.hpp"
#include "runtime/biasedLocking.hpp"
#include "runtime/os.hpp"
#include "runtime/stubRoutines.hpp"

int C1_MacroAssembler::lock_object(Register hdr, Register obj, Register disp_hdr,Register scratch, Label& slow_case) {
  const int aligned_mask = BytesPerWord -1;
  const int hdr_offset = oopDesc::mark_offset_in_bytes();

  // hdr is just a temperary register, it cannot be AT, however
  if ( hdr == NOREG ) {
    hdr = T8;
  }

  assert_different_registers(hdr, obj, disp_hdr);
  Label done;
  // The following move must be the first instruction of emitted since debug
  // information may be generated for it.
  // Load object header
  int null_check_offset = -1;
  verify_oop(obj);

  // save object being locked into the BasicObjectLock
  st_ptr(obj, disp_hdr, BasicObjectLock::obj_offset_in_bytes());
  if (UseBiasedLocking) {
    assert(scratch != noreg, "should have scratch register at this point");
    null_check_offset = biased_locking_enter(disp_hdr, obj, hdr, scratch, false,
  done, &slow_case);
  } else {
    null_check_offset = offset();
  }

  // Load object header
  ld_ptr(hdr, obj, hdr_offset);
  // and mark it as unlocked
  ori(hdr, hdr, markOopDesc::unlocked_value);
  // save unlocked object header into the displaced header location on the stack
  sd(hdr, disp_hdr, 0);

  // test if object header is still the same (i.e. unlocked), and if so, store the
  // displaced header address in the object header - if it is not the same, get the
  // object header instead
  //if (os::is_MP()) MacroAssembler::lock(); // must be immediately before cmpxchg!
  cmpxchg(disp_hdr, Address(obj, hdr_offset), hdr);
  // if the object header was the same, we're done
  if (PrintBiasedLockingStatistics) {
    Label L;
    beq(AT, R0, L);
    delayed()->nop();
    push(T0);
    push(T1);
    li(T0, (address) BiasedLocking::fast_path_entry_count_addr());
    lw(T1, T0, 0);
    addiu(T1, T1, 1);
    sw(T1, T0, 0);
    pop(T1);
    pop(T0);
    bind(L);
  }


  bne(AT, R0, done);
  delayed()->nop();
  // if the object header was not the same, it is now in the hdr register
  // => test if it is a stack pointer into the same stack (recursive locking), i.e.:
  //
  // 1) (hdr & aligned_mask) == 0
  // 2) SP <= hdr
  // 3) hdr <= SP + page_size
  //
  // these 3 tests can be done by evaluating the following expression:
  //
  // (hdr - SP) & (aligned_mask - page_size)
  //
  // assuming both the stack pointer and page_size have their least
  // significant 2 bits cleared and page_size is a power of 2
  subu(hdr, hdr, SP);
  move(AT, aligned_mask - os::vm_page_size());
  andr(hdr, hdr, AT);
  // for recursive locking, the result is zero => save it in the displaced header
  // location (NULL in the displaced hdr location indicates recursive locking)
  st_ptr(hdr, disp_hdr, 0);
  // otherwise we don't care about the result and handle locking via runtime call
  bne_far(hdr, R0, slow_case);
  delayed()->nop();
  // done
  bind(done);
  return null_check_offset;
}


void C1_MacroAssembler::unlock_object(Register hdr, Register obj, Register disp_hdr, Label& slow_case) {
  const int aligned_mask = BytesPerWord -1;
  const int hdr_offset = oopDesc::mark_offset_in_bytes();

  // hdr is just a temparay register, however, it cannot be AT
  if ( hdr == NOREG ) {
    hdr = T8;
  }

  assert_different_registers(hdr, obj, disp_hdr);
  assert(BytesPerWord == 8, "adjust aligned_mask and code");
  Label done;
  if (UseBiasedLocking) {
    // load object
    ld_ptr(obj, Address(disp_hdr, BasicObjectLock::obj_offset_in_bytes()));
    biased_locking_exit(obj, hdr, done);
  }

  // load displaced header
  ld_ptr(hdr, disp_hdr, 0);
  // if the loaded hdr is NULL we had recursive locking
  // if we had recursive locking, we are done
  beq(hdr, R0, done);
  delayed()->nop();
  // load object
  if(!UseBiasedLocking){
    ld_ptr(obj, disp_hdr, BasicObjectLock::obj_offset_in_bytes());
  }

  verify_oop(obj);
  // test if object header is pointing to the displaced header, and if so, restore
  // the displaced header in the object - if the object header is not pointing to
  // the displaced header, get the object header instead
  //if (os::is_MP()) MacroAssembler::lock(); // must be immediately before cmpxchg!
  cmpxchg(hdr, Address(obj, hdr_offset), disp_hdr);
  // if the object header was not pointing to the displaced header,
  // we do unlocking via runtime call
  beq_far(AT, R0, slow_case);
  delayed()->nop();
  // done
  bind(done);
}


// Defines obj, preserves var_size_in_bytes
void C1_MacroAssembler::try_allocate(Register obj, Register var_size_in_bytes, int con_size_in_bytes, Register t1, Register t2, Label& slow_case) {
  if (UseTLAB) {
    tlab_allocate(obj, var_size_in_bytes, con_size_in_bytes, t1, t2, slow_case);
  } else {
    eden_allocate(obj, var_size_in_bytes, con_size_in_bytes, t1, t2, slow_case);
    incr_allocated_bytes(noreg, var_size_in_bytes, con_size_in_bytes, t1);
  }
}

void C1_MacroAssembler::initialize_header(Register obj, Register klass, Register len, Register t1 , Register t2) {
  assert_different_registers(obj, klass, len, T9);

  if (UseBiasedLocking && !len->is_valid()) {
    assert_different_registers(obj, klass, len, t1, t2);
    ld_ptr(t1, klass, in_bytes(Klass::prototype_header_offset()));
    st_ptr(t1, obj, oopDesc::mark_offset_in_bytes());
  } else {
    li(T9, (intptr_t)markOopDesc::prototype());
    st_ptr(T9, obj, oopDesc::mark_offset_in_bytes());
  }
  //st_ptr(klass, obj, oopDesc::klass_offset_in_bytes());
#ifdef _LP64
  if (UseCompressedClassPointers) {
    move(T9, klass);
    store_klass(obj, T9);
  } else
#endif
  {
    st_ptr(klass, obj, oopDesc::klass_offset_in_bytes());
  }

  if (len->is_valid()) {
    sw(len, obj, arrayOopDesc::length_offset_in_bytes());
  }
#ifdef _LP64
  else if (UseCompressedClassPointers) {
    store_klass_gap(obj, R0);
  }
#endif
}


// preserves obj, destroys len_in_bytes
void C1_MacroAssembler::initialize_body(Register obj, Register len_in_bytes, int hdr_size_in_bytes, Register t1) {
  Label done;
  Register ptr = t1;
  assert_different_registers(obj, ptr, len_in_bytes);
  assert((hdr_size_in_bytes & (BytesPerWord - 1)) == 0,
      "header size is not a multiple of BytesPerWord");
  Register index = len_in_bytes;

  assert(is_simm16(hdr_size_in_bytes), "change this code");
  addiu(index, index, - hdr_size_in_bytes);
  beq(index, R0, done);
  delayed();

  // initialize topmost word, divide index by 2, check if odd and test if zero
  // note: for the remaining code to work, index must be a multiple of BytesPerWord
#ifdef ASSERT
  {
    Label L;
    andi(AT, index, BytesPerWord - 1);
    beq(AT, R0, L);
    delayed()->nop();
    stop("index is not a multiple of BytesPerWord");
    bind(L);
  }
#endif
  // index could have been not a multiple of 8 (i.e., bit 2 was set)
  {
    Label even;
    // note: if index was a multiple of 8, than it cannot
    //       be 0 now otherwise it must have been 0 before
    //       => if it is even, we don't need to check for 0 again
#ifdef _LP64
    andi(AT, index, 8);
    shr(index, 4);
    shl(index, 4);
#else
    andi(AT, index, 4);
    shr(index, 3);
    shl(index, 3);
#endif
    beq(AT, R0, even);
    delayed()->addu(ptr, obj, index);
    // clear topmost word (no jump needed if conditional assignment would work here)
    st_ptr(R0, ptr, hdr_size_in_bytes);
    // index could be 0 now, need to check again
    beq(index, R0, done);
    delayed()->nop();
    bind(even);
  }
  {
    Label loop;
    bind(loop);
    st_ptr(R0, ptr, hdr_size_in_bytes - 1*BytesPerWord);
    st_ptr(R0, ptr, hdr_size_in_bytes - 2*BytesPerWord);

    addiu(index, index, - 2 * wordSize);
    bne(index, R0, loop);
    delayed()->addiu(ptr, ptr, - 2 * wordSize);
  }

  // done
  bind(done);
}

void C1_MacroAssembler::allocate_object(Register obj, Register t1, Register t2, int header_size, int object_size, Register klass, Label& slow_case) {
  assert(obj != t1 && obj != t2 && t1 != t2, "registers must be different"); // XXX really?
  assert(header_size >= 0 && object_size >= header_size, "illegal sizes");

  try_allocate(obj, noreg, object_size * BytesPerWord, t1, t2, slow_case);

  initialize_object(obj, klass, noreg, object_size * HeapWordSize, t1, t2);
}

void C1_MacroAssembler::initialize_object(Register obj, Register klass, Register var_size_in_bytes, int con_size_in_bytes, Register t1, Register t2) {
  assert((con_size_in_bytes & MinObjAlignmentInBytesMask) == 0,
      "con_size_in_bytes is not multiple of alignment");
  //Merged from b25
  const int hdr_size_in_bytes = instanceOopDesc::header_size() * HeapWordSize;

  initialize_header(obj, klass, NOREG, t1, t2);

  // clear rest of allocated space
  const Register index = t2;
  //FIXME, x86 changed the value in jdk6
  // const int threshold = hdr_size_in_bytes + 36;
  // approximate break even point for code size (see comments below)
  const int threshold = 6 * BytesPerWord;
  // approximate break even point for code size (see comments below)
  if (var_size_in_bytes != NOREG) {
    move(index, var_size_in_bytes);
    initialize_body(obj, index, hdr_size_in_bytes, t1);
  } else if (con_size_in_bytes <= threshold) {
    // use explicit null stores
    // code size = 4*n bytes (n = number of fields to clear)

    for (int i = hdr_size_in_bytes; i < con_size_in_bytes; i += BytesPerWord) {
      st_ptr(R0, obj, i);
    }



  } else if(con_size_in_bytes > hdr_size_in_bytes) {
    // use loop to null out the fields
    // code size = 32 bytes for even n (n = number of fields to clear)
    // initialize last object field first if odd number of fields
    assert( ((con_size_in_bytes - hdr_size_in_bytes) >> 3)!=0, "change code here");

#ifdef _LP64
    move(index, (con_size_in_bytes - hdr_size_in_bytes) >> 4);
    sll(t1, index, 4);
#else
    move(index, (con_size_in_bytes - hdr_size_in_bytes) >> 3);
    sll(t1, index, 3);
#endif
    addu(t1, obj, t1);

    // initialize last object field if constant size is odd
#ifdef _LP64
    if (! UseCompressedOops)
    {
        if (((con_size_in_bytes - hdr_size_in_bytes) & 8) != 0) {
      sd(R0, t1, hdr_size_in_bytes);
        }
    } else if (UseCompressedOops) {
        int extra = (con_size_in_bytes - hdr_size_in_bytes) % 16;
        while (extra != 0) {
      sw(R0, t1, hdr_size_in_bytes + extra - 4);
      extra -= 4;
        }
    }
#else
    if (((con_size_in_bytes - hdr_size_in_bytes) & 4) != 0) {
      sw(R0, t1, hdr_size_in_bytes);
    }
#endif
    {
      Label loop;
      bind(loop);
      st_ptr(R0, t1, hdr_size_in_bytes - (1*BytesPerWord));
      st_ptr(R0, t1, hdr_size_in_bytes - (2*BytesPerWord));
      addiu(index, index, -1);
      bne(index, R0, loop);
      delayed()->addiu(t1, t1, - 2 * wordSize);
    }
  }

  if (DTraceAllocProbes) {
    call(CAST_FROM_FN_PTR(address,
               Runtime1::entry_for(Runtime1::dtrace_object_alloc_id)), relocInfo::runtime_call_type);
    delayed()->nop();
  }
  verify_oop(obj);
}

void C1_MacroAssembler::allocate_array(Register obj, Register len, Register t1, Register t2, Register t3,int header_size,
          int scale, Register klass, Label& slow_case) {
  assert(obj == V0, "obj must be in V0 for cmpxchg");
  assert_different_registers(obj, len, t1, t2, t3,klass, AT);

  // determine alignment mask
  assert(BytesPerWord == 8, "must be a multiple of 2 for masking code to work");

  move(AT, max_array_allocation_length);
  sltu(AT, AT, len);
  bne_far(AT, R0, slow_case);
  delayed()->nop();

  const Register arr_size = t3;
  // align object end
  move(arr_size, header_size * BytesPerWord + MinObjAlignmentInBytesMask);
  sll(AT, len, scale);
  addu(arr_size, arr_size, AT);
  move(AT, ~MinObjAlignmentInBytesMask);
  andr(arr_size, arr_size, AT);

  try_allocate(obj, arr_size, 0, t1, t2, slow_case);

  initialize_header(obj, klass, len,t1,t2);

         // clear rest of allocated space
  const Register len_zero = len;
  initialize_body(obj, arr_size, header_size * BytesPerWord, len_zero);
  if (DTraceAllocProbes) {
    call(CAST_FROM_FN_PTR(address,
    Runtime1::entry_for(Runtime1::dtrace_object_alloc_id)),
          relocInfo::runtime_call_type);
    delayed()->nop();
  }

  verify_oop(obj);
}


void C1_MacroAssembler::inline_cache_check(Register receiver, Register iCache) {
  verify_oop(receiver);
  // explicit NULL check not needed since load from [klass_offset] causes a trap
  // check against inline cache
  assert(!MacroAssembler::needs_explicit_null_check(oopDesc::klass_offset_in_bytes()), "must add explicit null check");
  // if icache check fails, then jump to runtime routine
  // Note: RECEIVER must still contain the receiver!
  Label L;
#ifdef _LP64
  //ld_ptr(AT, receiver, oopDesc::klass_offset_in_bytes());
  //add for compressedoops
  load_klass(T9, receiver);
#else
  lw(T9, receiver, oopDesc::klass_offset_in_bytes());
#endif
  beq(T9, iCache, L);
  delayed()->nop();
  jmp(SharedRuntime::get_ic_miss_stub(), relocInfo::runtime_call_type);
  delayed()->nop();
  bind(L);
  // assert(UseCompressedOops, "check alignment in emit_method_entry");
}


void C1_MacroAssembler::build_frame(int frame_size_in_bytes, int bang_size_in_bytes) {
  assert(bang_size_in_bytes >= frame_size_in_bytes, "stack bang size incorrect");
  // Make sure there is enough stack space for this method's activation.
  // Note that we do this before doing an enter(). This matches the
  // ordering of C2's stack overflow check / sp decrement and allows
  // the SharedRuntime stack overflow handling to be consistent
  // between the two compilers.
  generate_stack_overflow_check(bang_size_in_bytes);

  push(RA);
  push(FP);
  move(FP, SP);

  decrement(SP, frame_size_in_bytes); // does not emit code for frame_size == 0
}

void C1_MacroAssembler::remove_frame(int frame_size_in_bytes) {
  increment(SP, frame_size_in_bytes);  // Does not emit code for frame_size == 0
  pop(FP);
  pop(RA);
}

void C1_MacroAssembler::unverified_entry(Register receiver, Register ic_klass) {
  inline_cache_check(receiver, ic_klass);
}


void C1_MacroAssembler::verified_entry() {
  // build frame
  verify_FPU(0, "method_entry");
}


#ifndef PRODUCT
void C1_MacroAssembler::verify_stack_oop(int stack_offset) {
  if (!VerifyOops) return;
  verify_oop_addr(Address(SP, stack_offset));
}

void C1_MacroAssembler::verify_not_null_oop(Register r) {
  if (!VerifyOops) return;
  Label not_null;
  bne(r, R0, not_null);
  delayed()->nop();
  stop("non-null oop required");
  bind(not_null);
  verify_oop(r);
}

void C1_MacroAssembler::invalidate_registers(bool inv_v0, bool inv_v1, bool inv_t3, bool inv_t7, bool inv_s0, bool inv_s7) {
#ifdef ASSERT
  //if (inv_v0) move(V0, 0xDEAD);
  //if (inv_v1) move(V1, 0xDEAD);
  //if (inv_t3) move(T3, 0xDEAD);
  //if (inv_t7) move(T7, 0xDEAD);
  //if (inv_s0) move(S0, 0xDEAD);
  //if (inv_s7) move(S7, 0xDEAD);
#endif
}
#endif // ifndef PRODUCT
