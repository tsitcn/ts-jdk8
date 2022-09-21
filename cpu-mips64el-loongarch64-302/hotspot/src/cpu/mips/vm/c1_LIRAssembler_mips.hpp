/*
 * Copyright (c) 2000, 2010, Oracle and/or its affiliates. All rights reserved.
 * Copyright (c) 2015, 2018, Loongson Technology. All rights reserved.
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

#ifndef CPU_MIPS_VM_C1_LIRASSEMBLER_MIPS_HPP
#define CPU_MIPS_VM_C1_LIRASSEMBLER_MIPS_HPP

 private:

  Address::ScaleFactor array_element_size(BasicType type) const;

  void arith_fpu_implementation(LIR_Code code, int left_index, int right_index, int dest_index, bool pop_fpu_stack);

  // helper functions which checks for overflow and sets bailout if it
  // occurs.  Always returns a valid embeddable pointer but in the
  // bailout case the pointer won't be to unique storage.
  address float_constant(float f);
  address double_constant(double d);

  bool is_literal_address(LIR_Address* addr);

  // Reset base register
  Address rebase_Address(LIR_Address* addr, Register tmp = NOREG);

  // Record the type of the receiver in ReceiverTypeData
  void type_profile_helper(Register mdo,
                           ciMethodData *md, ciProfileData *data,
                           Register recv, Label* update_done);

public:

  void store_parameter(Register r, int offset_from_esp_in_words);
  void store_parameter(jint c,     int offset_from_esp_in_words);
  void store_parameter(jobject c,  int offset_from_esp_in_words);

  enum { call_stub_size = NOT_LP64(24) LP64_ONLY(40),
         exception_handler_size = DEBUG_ONLY(1*K) NOT_DEBUG(175),
         deopt_handler_size = NOT_LP64(16) LP64_ONLY(32)
       };

#endif // CPU_MIPS_VM_C1_LIRASSEMBLER_MIPS_HPP
