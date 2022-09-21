/*
 * Copyright (c) 1999, 2010, Oracle and/or its affiliates. All rights reserved.
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

#ifndef CPU_MIPS_VM_C1_MACROASSEMBLER_MIPS_HPP
#define CPU_MIPS_VM_C1_MACROASSEMBLER_MIPS_HPP

// C1_MacroAssembler contains high-level macros for C1

 private:
  int _sp_offset;    // track sp changes
  // initialization
  void pd_init() { _sp_offset = 0; }

 public:
  void try_allocate(
    Register obj,                      // result: pointer to object after successful allocation
    Register var_size_in_bytes,        // object size in bytes if unknown at compile time; invalid otherwise
    int      con_size_in_bytes,        // object size in bytes if   known at compile time
    Register t1,                       // temp register
    Register t2,                       // temp register
    Label&   slow_case                 // continuation point if fast allocation fails
  );

  void initialize_header(Register obj, Register klass, Register len, Register t1, Register t2);
  void initialize_body(Register obj, Register len_in_bytes, int hdr_size_in_bytes, Register t1);

  // locking
  // hdr     : contents destroyed
  // obj     : must point to the object to lock, contents preserved
  // disp_hdr: must point to the displaced header location, contents preserved
  // scratch : scratch register, contents destroyed
  // returns code offset at which to add null check debug information
  int lock_object  (Register swap, Register obj, Register disp_hdr, Register scratch, Label& slow_case);

  // unlocking
  // hdr     : contents destroyed
  // obj     : must point to the object to lock, contents preserved
  // disp_hdr: must point to the displaced header location, contents destroyed
  void unlock_object(Register swap, Register obj, Register lock, Label& slow_case);

  void initialize_object(
    Register obj,                      // result: pointer to object after successful allocation
    Register klass,                    // object klass
    Register var_size_in_bytes,        // object size in bytes if unknown at compile time; invalid otherwise
    int      con_size_in_bytes,        // object size in bytes if   known at compile time
    Register t1,                       // temp register
    Register t2                        // temp register
  );

  // allocation of fixed-size objects
  // (can also be used to allocate fixed-size arrays, by setting
  // hdr_size correctly and storing the array length afterwards)
  // obj        : will contain pointer to allocated object
  // t1, t2     : scratch registers - contents destroyed
  // header_size: size of object header in words
  // object_size: total size of object in words
  // slow_case  : exit to slow case implementation if fast allocation fails
  void allocate_object(Register obj, Register t1, Register t2, int header_size, int object_size, Register klass, Label& slow_case);

  enum {
    max_array_allocation_length = 0x00FFFFFF
  };

  // allocation of arrays
  // obj        : must be V0, will contain pointer to allocated object
  // len        : array length in number of elements
  // t          : scratch register - contents destroyed
  // header_size: size of object header in words
  // f          : element scale factor
  // slow_case  : exit to slow case implementation if fast allocation fails
  void allocate_array(Register obj, Register len, Register t1, Register t2, Register t3, int header_size, int scale, Register klass, Label& slow_case);

  int  sp_offset() const { return _sp_offset; }
  void set_sp_offset(int n) { _sp_offset = n; }

  // Note: NEVER push values directly, but only through following push_xxx functions;
  //       This helps us to track the sp changes compared to the entry sp (->_sp_offset)

  void push_jint (jint i)     { _sp_offset++; move(AT, (int)i); push(AT); }
#ifdef _LP64
  void push_oop  (jobject o)  { ShouldNotReachHere(); _sp_offset++; li(AT, (long)o); push(AT);}
#else
  void push_oop  (jobject o)  { ShouldNotReachHere(); _sp_offset++; move(AT, (int)o); push(AT);}
#endif
  // Seems to always be in wordSize
  void push_addr (Address a)  { _sp_offset++; addiu(AT, a.base(), a.disp()); push(AT);}
  void push_reg  (Register r) { _sp_offset++; push(r); }
  void pop_reg   (Register r) { _sp_offset--; pop(r); assert(_sp_offset >= 0, "stack offset underflow"); }
  void super_pop(Register r) {MacroAssembler::pop(r);}

  void dec_stack (int nof_words) {
    _sp_offset -= nof_words;
    assert(_sp_offset >= 0, "stack offset underflow");
    addiu(SP, SP, wordSize * nof_words);
  }

  void dec_stack_after_call (int nof_words) {
    _sp_offset -= nof_words;
    assert(_sp_offset >= 0, "stack offset underflow");
  }

  void invalidate_registers(bool inv_v0, bool inv_v1, bool inv_t3, bool inv_t7, bool inv_s0, bool inv_s7) PRODUCT_RETURN;

#endif // CPU_MIPS_VM_C1_MACROASSEMBLER_MIPS_HPP
