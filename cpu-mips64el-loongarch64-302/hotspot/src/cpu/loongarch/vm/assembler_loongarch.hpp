/*
 * Copyright (c) 1997, 2013, Oracle and/or its affiliates. All rights reserved.
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

#ifndef CPU_LOONGARCH_VM_ASSEMBLER_LOONGARCH_HPP
#define CPU_LOONGARCH_VM_ASSEMBLER_LOONGARCH_HPP

#include "asm/register.hpp"

class BiasedLockingCounters;


// Note: A register location is represented via a Register, not
//       via an address for efficiency & simplicity reasons.

class ArrayAddress;

class Address VALUE_OBJ_CLASS_SPEC {
 public:
  enum ScaleFactor {
    no_scale = -1,
    times_1  =  0,
    times_2  =  1,
    times_4  =  2,
    times_8  =  3,
    times_ptr = LP64_ONLY(times_8) NOT_LP64(times_4)
  };
  static ScaleFactor times(int size) {
    assert(size >= 1 && size <= 8 && is_power_of_2(size), "bad scale size");
    if (size == 8)  return times_8;
    if (size == 4)  return times_4;
    if (size == 2)  return times_2;
    return times_1;
  }

 private:
  Register         _base;
  Register         _index;
  ScaleFactor      _scale;
  int              _disp;
  RelocationHolder _rspec;

  // Easily misused constructors make them private
  Address(address loc, RelocationHolder spec);
  Address(int disp, address loc, relocInfo::relocType rtype);
  Address(int disp, address loc, RelocationHolder spec);

 public:

  // creation
  Address()
    : _base(noreg),
      _index(noreg),
      _scale(no_scale),
      _disp(0) {
  }

  // No default displacement otherwise Register can be implicitly
  // converted to 0(Register) which is quite a different animal.

  Address(Register base, int disp = 0)
    : _base(base),
      _index(noreg),
      _scale(no_scale),
      _disp(disp) {
    assert_different_registers(_base, AT);
  }

  Address(Register base, Register index, ScaleFactor scale, int disp = 0)
    : _base (base),
      _index(index),
      _scale(scale),
      _disp (disp) {
    assert(!index->is_valid() == (scale == Address::no_scale), "inconsistent address");
    assert_different_registers(_base, _index, AT);
  }

  // The following two overloads are used in connection with the
  // ByteSize type (see sizes.hpp).  They simplify the use of
  // ByteSize'd arguments in assembly code. Note that their equivalent
  // for the optimized build are the member functions with int disp
  // argument since ByteSize is mapped to an int type in that case.
  //
  // Note: DO NOT introduce similar overloaded functions for WordSize
  // arguments as in the optimized mode, both ByteSize and WordSize
  // are mapped to the same type and thus the compiler cannot make a
  // distinction anymore (=> compiler errors).

#ifdef ASSERT
  Address(Register base, ByteSize disp)
    : _base(base),
      _index(noreg),
      _scale(no_scale),
      _disp(in_bytes(disp)) {
    assert_different_registers(_base, AT);
  }

  Address(Register base, Register index, ScaleFactor scale, ByteSize disp)
    : _base(base),
      _index(index),
      _scale(scale),
      _disp(in_bytes(disp)) {
    assert(!index->is_valid() == (scale == Address::no_scale), "inconsistent address");
    assert_different_registers(_base, _index, AT);
  }
#endif // ASSERT

  // accessors
  bool        uses(Register reg) const { return _base == reg || _index == reg; }
  Register    base()             const { return _base;  }
  Register    index()            const { return _index; }
  ScaleFactor scale()            const { return _scale; }
  int         disp()             const { return _disp;  }

  static Address make_array(ArrayAddress);

  friend class Assembler;
  friend class MacroAssembler;
  friend class LIR_Assembler; // base/index/scale/disp
};

// Calling convention
class Argument VALUE_OBJ_CLASS_SPEC {
 public:
  enum {
    n_register_parameters = 8,   // 8 integer registers used to pass parameters
    n_float_register_parameters = 8   // 8 float registers used to pass parameters
  };
};

//
// AddressLiteral has been split out from Address because operands of this type
// need to be treated specially on 32bit vs. 64bit platforms. By splitting it out
// the few instructions that need to deal with address literals are unique and the
// MacroAssembler does not have to implement every instruction in the Assembler
// in order to search for address literals that may need special handling depending
// on the instruction and the platform. As small step on the way to merging i486/amd64
// directories.
//
class AddressLiteral VALUE_OBJ_CLASS_SPEC {
  friend class ArrayAddress;
  RelocationHolder _rspec;
  // Typically we use AddressLiterals we want to use their rval
  // However in some situations we want the lval (effect address) of the item.
  // We provide a special factory for making those lvals.
  bool _is_lval;

  // If the target is far we'll need to load the ea of this to
  // a register to reach it. Otherwise if near we can do rip
  // relative addressing.

  address          _target;

 protected:
  // creation
  AddressLiteral()
    : _is_lval(false),
      _target(NULL)
  {}

  public:


  AddressLiteral(address target, relocInfo::relocType rtype);

  AddressLiteral(address target, RelocationHolder const& rspec)
    : _rspec(rspec),
      _is_lval(false),
      _target(target)
  {}
#ifdef _LP64
   // 32-bit complains about a multiple declaration for int*.
   AddressLiteral(intptr_t* addr, relocInfo::relocType rtype = relocInfo::none)
     : _target((address) addr),
       _rspec(rspec_from_rtype(rtype, (address) addr)) {}
#endif

  AddressLiteral addr() {
    AddressLiteral ret = *this;
    ret._is_lval = true;
    return ret;
  }


 private:

  address target() { return _target; }
  bool is_lval() { return _is_lval; }

  relocInfo::relocType reloc() const { return _rspec.type(); }
  const RelocationHolder& rspec() const { return _rspec; }

  friend class Assembler;
  friend class MacroAssembler;
  friend class Address;
  friend class LIR_Assembler;
  RelocationHolder rspec_from_rtype(relocInfo::relocType rtype, address addr) {
    switch (rtype) {
      case relocInfo::external_word_type:
        return external_word_Relocation::spec(addr);
      case relocInfo::internal_word_type:
        return internal_word_Relocation::spec(addr);
      case relocInfo::opt_virtual_call_type:
        return opt_virtual_call_Relocation::spec();
      case relocInfo::static_call_type:
        return static_call_Relocation::spec();
      case relocInfo::runtime_call_type:
        return runtime_call_Relocation::spec();
      case relocInfo::poll_type:
      case relocInfo::poll_return_type:
        return Relocation::spec_simple(rtype);
      case relocInfo::none:
      case relocInfo::oop_type:
        // Oops are a special case. Normally they would be their own section
        // but in cases like icBuffer they are literals in the code stream that
        // we don't have a section for. We use none so that we get a literal address
        // which is always patchable.
        return RelocationHolder();
      default:
        ShouldNotReachHere();
        return RelocationHolder();
    }
  }

};

// Convience classes
class RuntimeAddress: public AddressLiteral {

 public:

  RuntimeAddress(address target) : AddressLiteral(target, relocInfo::runtime_call_type) {}

};

class OopAddress: public AddressLiteral {

 public:

  OopAddress(address target) : AddressLiteral(target, relocInfo::oop_type){}

};

class ExternalAddress: public AddressLiteral {

 public:

  ExternalAddress(address target) : AddressLiteral(target, relocInfo::external_word_type){}

};

class InternalAddress: public AddressLiteral {

 public:

  InternalAddress(address target) : AddressLiteral(target, relocInfo::internal_word_type) {}

};

// x86 can do array addressing as a single operation since disp can be an absolute
// address amd64 can't. We create a class that expresses the concept but does extra
// magic on amd64 to get the final result

class ArrayAddress VALUE_OBJ_CLASS_SPEC {
  private:

  AddressLiteral _base;
  Address        _index;

  public:

  ArrayAddress() {};
  ArrayAddress(AddressLiteral base, Address index): _base(base), _index(index) {};
  AddressLiteral base() { return _base; }
  Address index() { return _index; }

};

// The LoongArch Assembler: Pure assembler doing NO optimizations on the instruction
// level ; i.e., what you write is what you get. The Assembler is generating code into
// a CodeBuffer.

class Assembler : public AbstractAssembler  {
  friend class AbstractAssembler; // for the non-virtual hack
  friend class LIR_Assembler; // as_Address()
  friend class StubGenerator;

 public:
  // 22-bit opcode, highest 22 bits: bits[31...10]
  enum ops22 {
    clo_w_op         = 0b0000000000000000000100,
    clz_w_op         = 0b0000000000000000000101,
    cto_w_op         = 0b0000000000000000000110,
    ctz_w_op         = 0b0000000000000000000111,
    clo_d_op         = 0b0000000000000000001000,
    clz_d_op         = 0b0000000000000000001001,
    cto_d_op         = 0b0000000000000000001010,
    ctz_d_op         = 0b0000000000000000001011,
    revb_2h_op       = 0b0000000000000000001100,
    revb_4h_op       = 0b0000000000000000001101,
    revb_2w_op       = 0b0000000000000000001110,
    revb_d_op        = 0b0000000000000000001111,
    revh_2w_op       = 0b0000000000000000010000,
    revh_d_op        = 0b0000000000000000010001,
    bitrev_4b_op     = 0b0000000000000000010010,
    bitrev_8b_op     = 0b0000000000000000010011,
    bitrev_w_op      = 0b0000000000000000010100,
    bitrev_d_op      = 0b0000000000000000010101,
    ext_w_h_op       = 0b0000000000000000010110,
    ext_w_b_op       = 0b0000000000000000010111,
    rdtimel_w_op     = 0b0000000000000000011000,
    rdtimeh_w_op     = 0b0000000000000000011001,
    rdtime_d_op      = 0b0000000000000000011010,
    cpucfg_op        = 0b0000000000000000011011,
    fabs_s_op        = 0b0000000100010100000001,
    fabs_d_op        = 0b0000000100010100000010,
    fneg_s_op        = 0b0000000100010100000101,
    fneg_d_op        = 0b0000000100010100000110,
    flogb_s_op       = 0b0000000100010100001001,
    flogb_d_op       = 0b0000000100010100001010,
    fclass_s_op      = 0b0000000100010100001101,
    fclass_d_op      = 0b0000000100010100001110,
    fsqrt_s_op       = 0b0000000100010100010001,
    fsqrt_d_op       = 0b0000000100010100010010,
    frecip_s_op      = 0b0000000100010100010101,
    frecip_d_op      = 0b0000000100010100010110,
    frsqrt_s_op      = 0b0000000100010100011001,
    frsqrt_d_op      = 0b0000000100010100011010,
    fmov_s_op        = 0b0000000100010100100101,
    fmov_d_op        = 0b0000000100010100100110,
    movgr2fr_w_op    = 0b0000000100010100101001,
    movgr2fr_d_op    = 0b0000000100010100101010,
    movgr2frh_w_op   = 0b0000000100010100101011,
    movfr2gr_s_op    = 0b0000000100010100101101,
    movfr2gr_d_op    = 0b0000000100010100101110,
    movfrh2gr_s_op   = 0b0000000100010100101111,
    movgr2fcsr_op    = 0b0000000100010100110000,
    movfcsr2gr_op    = 0b0000000100010100110010,
    movfr2cf_op      = 0b0000000100010100110100,
    movcf2fr_op      = 0b0000000100010100110101,
    movgr2cf_op      = 0b0000000100010100110110,
    movcf2gr_op      = 0b0000000100010100110111,
    fcvt_s_d_op      = 0b0000000100011001000110,
    fcvt_d_s_op      = 0b0000000100011001001001,
    ftintrm_w_s_op   = 0b0000000100011010000001,
    ftintrm_w_d_op   = 0b0000000100011010000010,
    ftintrm_l_s_op   = 0b0000000100011010001001,
    ftintrm_l_d_op   = 0b0000000100011010001010,
    ftintrp_w_s_op   = 0b0000000100011010010001,
    ftintrp_w_d_op   = 0b0000000100011010010010,
    ftintrp_l_s_op   = 0b0000000100011010011001,
    ftintrp_l_d_op   = 0b0000000100011010011010,
    ftintrz_w_s_op   = 0b0000000100011010100001,
    ftintrz_w_d_op   = 0b0000000100011010100010,
    ftintrz_l_s_op   = 0b0000000100011010101001,
    ftintrz_l_d_op   = 0b0000000100011010101010,
    ftintrne_w_s_op  = 0b0000000100011010110001,
    ftintrne_w_d_op  = 0b0000000100011010110010,
    ftintrne_l_s_op  = 0b0000000100011010111001,
    ftintrne_l_d_op  = 0b0000000100011010111010,
    ftint_w_s_op     = 0b0000000100011011000001,
    ftint_w_d_op     = 0b0000000100011011000010,
    ftint_l_s_op     = 0b0000000100011011001001,
    ftint_l_d_op     = 0b0000000100011011001010,
    ffint_s_w_op     = 0b0000000100011101000100,
    ffint_s_l_op     = 0b0000000100011101000110,
    ffint_d_w_op     = 0b0000000100011101001000,
    ffint_d_l_op     = 0b0000000100011101001010,
    frint_s_op       = 0b0000000100011110010001,
    frint_d_op       = 0b0000000100011110010010,
    iocsrrd_b_op     = 0b0000011001001000000000,
    iocsrrd_h_op     = 0b0000011001001000000001,
    iocsrrd_w_op     = 0b0000011001001000000010,
    iocsrrd_d_op     = 0b0000011001001000000011,
    iocsrwr_b_op     = 0b0000011001001000000100,
    iocsrwr_h_op     = 0b0000011001001000000101,
    iocsrwr_w_op     = 0b0000011001001000000110,
    iocsrwr_d_op     = 0b0000011001001000000111,

    unknow_ops22     = 0b1111111111111111111111
  };

  // 17-bit opcode, highest 17 bits: bits[31...15]
  enum ops17 {
    asrtle_d_op      = 0b00000000000000010,
    asrtgt_d_op      = 0b00000000000000011,
    add_w_op         = 0b00000000000100000,
    add_d_op         = 0b00000000000100001,
    sub_w_op         = 0b00000000000100010,
    sub_d_op         = 0b00000000000100011,
    slt_op           = 0b00000000000100100,
    sltu_op          = 0b00000000000100101,
    maskeqz_op       = 0b00000000000100110,
    masknez_op       = 0b00000000000100111,
    nor_op           = 0b00000000000101000,
    and_op           = 0b00000000000101001,
    or_op            = 0b00000000000101010,
    xor_op           = 0b00000000000101011,
    orn_op           = 0b00000000000101100,
    andn_op          = 0b00000000000101101,
    sll_w_op         = 0b00000000000101110,
    srl_w_op         = 0b00000000000101111,
    sra_w_op         = 0b00000000000110000,
    sll_d_op         = 0b00000000000110001,
    srl_d_op         = 0b00000000000110010,
    sra_d_op         = 0b00000000000110011,
    rotr_w_op        = 0b00000000000110110,
    rotr_d_op        = 0b00000000000110111,
    mul_w_op         = 0b00000000000111000,
    mulh_w_op        = 0b00000000000111001,
    mulh_wu_op       = 0b00000000000111010,
    mul_d_op         = 0b00000000000111011,
    mulh_d_op        = 0b00000000000111100,
    mulh_du_op       = 0b00000000000111101,
    mulw_d_w_op      = 0b00000000000111110,
    mulw_d_wu_op     = 0b00000000000111111,
    div_w_op         = 0b00000000001000000,
    mod_w_op         = 0b00000000001000001,
    div_wu_op        = 0b00000000001000010,
    mod_wu_op        = 0b00000000001000011,
    div_d_op         = 0b00000000001000100,
    mod_d_op         = 0b00000000001000101,
    div_du_op        = 0b00000000001000110,
    mod_du_op        = 0b00000000001000111,
    crc_w_b_w_op     = 0b00000000001001000,
    crc_w_h_w_op     = 0b00000000001001001,
    crc_w_w_w_op     = 0b00000000001001010,
    crc_w_d_w_op     = 0b00000000001001011,
    crcc_w_b_w_op    = 0b00000000001001100,
    crcc_w_h_w_op    = 0b00000000001001101,
    crcc_w_w_w_op    = 0b00000000001001110,
    crcc_w_d_w_op    = 0b00000000001001111,
    break_op         = 0b00000000001010100,
    fadd_s_op        = 0b00000001000000001,
    fadd_d_op        = 0b00000001000000010,
    fsub_s_op        = 0b00000001000000101,
    fsub_d_op        = 0b00000001000000110,
    fmul_s_op        = 0b00000001000001001,
    fmul_d_op        = 0b00000001000001010,
    fdiv_s_op        = 0b00000001000001101,
    fdiv_d_op        = 0b00000001000001110,
    fmax_s_op        = 0b00000001000010001,
    fmax_d_op        = 0b00000001000010010,
    fmin_s_op        = 0b00000001000010101,
    fmin_d_op        = 0b00000001000010110,
    fmaxa_s_op       = 0b00000001000011001,
    fmaxa_d_op       = 0b00000001000011010,
    fmina_s_op       = 0b00000001000011101,
    fmina_d_op       = 0b00000001000011110,
    fscaleb_s_op     = 0b00000001000100001,
    fscaleb_d_op     = 0b00000001000100010,
    fcopysign_s_op   = 0b00000001000100101,
    fcopysign_d_op   = 0b00000001000100110,
    ldx_b_op         = 0b00111000000000000,
    ldx_h_op         = 0b00111000000001000,
    ldx_w_op         = 0b00111000000010000,
    ldx_d_op         = 0b00111000000011000,
    stx_b_op         = 0b00111000000100000,
    stx_h_op         = 0b00111000000101000,
    stx_w_op         = 0b00111000000110000,
    stx_d_op         = 0b00111000000111000,
    ldx_bu_op        = 0b00111000001000000,
    ldx_hu_op        = 0b00111000001001000,
    ldx_wu_op        = 0b00111000001010000,
    fldx_s_op        = 0b00111000001100000,
    fldx_d_op        = 0b00111000001101000,
    fstx_s_op        = 0b00111000001110000,
    fstx_d_op        = 0b00111000001111000,
    amswap_w_op      = 0b00111000011000000,
    amswap_d_op      = 0b00111000011000001,
    amadd_w_op       = 0b00111000011000010,
    amadd_d_op       = 0b00111000011000011,
    amand_w_op       = 0b00111000011000100,
    amand_d_op       = 0b00111000011000101,
    amor_w_op        = 0b00111000011000110,
    amor_d_op        = 0b00111000011000111,
    amxor_w_op       = 0b00111000011001000,
    amxor_d_op       = 0b00111000011001001,
    ammax_w_op       = 0b00111000011001010,
    ammax_d_op       = 0b00111000011001011,
    ammin_w_op       = 0b00111000011001100,
    ammin_d_op       = 0b00111000011001101,
    ammax_wu_op      = 0b00111000011001110,
    ammax_du_op      = 0b00111000011001111,
    ammin_wu_op      = 0b00111000011010000,
    ammin_du_op      = 0b00111000011010001,
    amswap_db_w_op   = 0b00111000011010010,
    amswap_db_d_op   = 0b00111000011010011,
    amadd_db_w_op    = 0b00111000011010100,
    amadd_db_d_op    = 0b00111000011010101,
    amand_db_w_op    = 0b00111000011010110,
    amand_db_d_op    = 0b00111000011010111,
    amor_db_w_op     = 0b00111000011011000,
    amor_db_d_op     = 0b00111000011011001,
    amxor_db_w_op    = 0b00111000011011010,
    amxor_db_d_op    = 0b00111000011011011,
    ammax_db_w_op    = 0b00111000011011100,
    ammax_db_d_op    = 0b00111000011011101,
    ammin_db_w_op    = 0b00111000011011110,
    ammin_db_d_op    = 0b00111000011011111,
    ammax_db_wu_op   = 0b00111000011100000,
    ammax_db_du_op   = 0b00111000011100001,
    ammin_db_wu_op   = 0b00111000011100010,
    ammin_db_du_op   = 0b00111000011100011,
    dbar_op          = 0b00111000011100100,
    ibar_op          = 0b00111000011100101,
    fldgt_s_op       = 0b00111000011101000,
    fldgt_d_op       = 0b00111000011101001,
    fldle_s_op       = 0b00111000011101010,
    fldle_d_op       = 0b00111000011101011,
    fstgt_s_op       = 0b00111000011101100,
    fstgt_d_op       = 0b00111000011101101,
    fstle_s_op       = 0b00111000011101110,
    fstle_d_op       = 0b00111000011101111,
    ldgt_b_op        = 0b00111000011110000,
    ldgt_h_op        = 0b00111000011110001,
    ldgt_w_op        = 0b00111000011110010,
    ldgt_d_op        = 0b00111000011110011,
    ldle_b_op        = 0b00111000011110100,
    ldle_h_op        = 0b00111000011110101,
    ldle_w_op        = 0b00111000011110110,
    ldle_d_op        = 0b00111000011110111,
    stgt_b_op        = 0b00111000011111000,
    stgt_h_op        = 0b00111000011111001,
    stgt_w_op        = 0b00111000011111010,
    stgt_d_op        = 0b00111000011111011,
    stle_b_op        = 0b00111000011111100,
    stle_h_op        = 0b00111000011111101,
    stle_w_op        = 0b00111000011111110,
    stle_d_op        = 0b00111000011111111,

    unknow_ops17     = 0b11111111111111111
  };

  // 14-bit opcode, highest 14 bits: bits[31...18]
  enum ops14 {
    alsl_w_op       = 0b00000000000001,
    bytepick_w_op   = 0b00000000000010,
    bytepick_d_op   = 0b00000000000011,
    alsl_d_op       = 0b00000000001011,
    slli_op         = 0b00000000010000,
    srli_op         = 0b00000000010001,
    srai_op         = 0b00000000010010,
    rotri_op        = 0b00000000010011,
    lddir_op        = 0b00000110010000,
    ldpte_op        = 0b00000110010001,

    unknow_ops14    = 0b11111111111111
  };

  // 12-bit opcode, highest 12 bits: bits[31...20]
  enum ops12 {
    fmadd_s_op        = 0b000010000001,
    fmadd_d_op        = 0b000010000010,
    fmsub_s_op        = 0b000010000101,
    fmsub_d_op        = 0b000010000110,
    fnmadd_s_op       = 0b000010001001,
    fnmadd_d_op       = 0b000010001010,
    fnmsub_s_op       = 0b000010001101,
    fnmsub_d_op       = 0b000010001110,
    fcmp_cond_s_op    = 0b000011000001,
    fcmp_cond_d_op    = 0b000011000010,
    fsel_op           = 0b000011010000,

    unknow_ops12      = 0b111111111111
  };

  // 10-bit opcode, highest 10 bits: bits[31...22]
  enum ops10 {
    bstr_w_op       = 0b0000000001,
    bstrins_d_op    = 0b0000000010,
    bstrpick_d_op   = 0b0000000011,
    slti_op         = 0b0000001000,
    sltui_op        = 0b0000001001,
    addi_w_op       = 0b0000001010,
    addi_d_op       = 0b0000001011,
    lu52i_d_op      = 0b0000001100,
    andi_op         = 0b0000001101,
    ori_op          = 0b0000001110,
    xori_op         = 0b0000001111,
    ld_b_op         = 0b0010100000,
    ld_h_op         = 0b0010100001,
    ld_w_op         = 0b0010100010,
    ld_d_op         = 0b0010100011,
    st_b_op         = 0b0010100100,
    st_h_op         = 0b0010100101,
    st_w_op         = 0b0010100110,
    st_d_op         = 0b0010100111,
    ld_bu_op        = 0b0010101000,
    ld_hu_op        = 0b0010101001,
    ld_wu_op        = 0b0010101010,
    preld_op        = 0b0010101011,
    fld_s_op        = 0b0010101100,
    fst_s_op        = 0b0010101101,
    fld_d_op        = 0b0010101110,
    fst_d_op        = 0b0010101111,

    unknow_ops10    = 0b1111111111
  };

  // 8-bit opcode, highest 8 bits: bits[31...22]
  enum ops8 {
    ll_w_op        = 0b00100000,
    sc_w_op        = 0b00100001,
    ll_d_op        = 0b00100010,
    sc_d_op        = 0b00100011,
    ldptr_w_op     = 0b00100100,
    stptr_w_op     = 0b00100101,
    ldptr_d_op     = 0b00100110,
    stptr_d_op     = 0b00100111,

    unknow_ops8    = 0b11111111
  };

  // 7-bit opcode, highest 7 bits: bits[31...25]
  enum ops7 {
    lu12i_w_op     = 0b0001010,
    lu32i_d_op     = 0b0001011,
    pcaddi_op      = 0b0001100,
    pcalau12i_op   = 0b0001101,
    pcaddu12i_op   = 0b0001110,
    pcaddu18i_op   = 0b0001111,

    unknow_ops7    = 0b1111111
  };

  // 6-bit opcode, highest 6 bits: bits[31...25]
  enum ops6 {
    addu16i_d_op   = 0b000100,
    beqz_op        = 0b010000,
    bnez_op        = 0b010001,
    bccondz_op     = 0b010010,
    jirl_op        = 0b010011,
    b_op           = 0b010100,
    bl_op          = 0b010101,
    beq_op         = 0b010110,
    bne_op         = 0b010111,
    blt_op         = 0b011000,
    bge_op         = 0b011001,
    bltu_op        = 0b011010,
    bgeu_op        = 0b011011,

    unknow_ops6    = 0b111111
  };

  enum fcmp_cond {
    fcmp_caf     = 0x00,
    fcmp_cun     = 0x08,
    fcmp_ceq     = 0x04,
    fcmp_cueq    = 0x0c,
    fcmp_clt     = 0x02,
    fcmp_cult    = 0x0a,
    fcmp_cle     = 0x06,
    fcmp_cule    = 0x0e,
    fcmp_cne     = 0x10,
    fcmp_cor     = 0x14,
    fcmp_cune    = 0x18,
    fcmp_saf     = 0x01,
    fcmp_sun     = 0x09,
    fcmp_seq     = 0x05,
    fcmp_sueq    = 0x0d,
    fcmp_slt     = 0x03,
    fcmp_sult    = 0x0b,
    fcmp_sle     = 0x07,
    fcmp_sule    = 0x0f,
    fcmp_sne     = 0x11,
    fcmp_sor     = 0x15,
    fcmp_sune    = 0x19
  };

  enum Condition {
    zero         ,
    notZero      ,
    equal        ,
    notEqual     ,
    less         ,
    lessEqual    ,
    greater      ,
    greaterEqual ,
    below        ,
    belowEqual   ,
    above        ,
    aboveEqual
  };

  static const int LogInstructionSize = 2;
  static const int InstructionSize    = 1 << LogInstructionSize;

  enum WhichOperand {
    // input to locate_operand, and format code for relocations
    imm_operand  = 0,            // embedded 32-bit|64-bit immediate operand
    disp32_operand = 1,          // embedded 32-bit displacement or address
    call32_operand = 2,          // embedded 32-bit self-relative displacement
#ifndef _LP64
    _WhichOperand_limit = 3
#else
     narrow_oop_operand = 3,     // embedded 32-bit immediate narrow oop
    _WhichOperand_limit = 4
#endif
  };

  static int low  (int x, int l) { return bitfield(x, 0, l); }
  static int low16(int x)        { return low(x, 16); }
  static int low26(int x)        { return low(x, 26); }

  static int high  (int x, int l) { return bitfield(x, 32-l, l); }
  static int high16(int x)        { return high(x, 16); }
  static int high6 (int x)        { return high(x, 6); }


 protected:
  // help methods for instruction ejection

  // 2R-type
  //  31                          10 9      5 4     0
  // |   opcode                     |   rj   |  rd   |
  static inline int insn_RR   (int op, int rj, int rd) { return (op<<10) | (rj<<5) | rd; }

  // 3R-type
  //  31                    15 14 10 9      5 4     0
  // |   opcode               |  rk |   rj   |  rd   |
  static inline int insn_RRR  (int op, int rk, int rj, int rd)  { return (op<<15) | (rk<<10) | (rj<<5) | rd; }

  // 4R-type
  //  31             20 19  15 14  10 9     5 4     0
  // |   opcode        |  ra  |  rk |    rj  |  rd   |
  static inline int insn_RRRR (int op, int ra,  int rk, int rj, int rd)  { return (op<<20) | (ra << 15) | (rk<<10) | (rj<<5) | rd; }

  // 2RI8-type
  //  31                18 17     10 9      5 4     0
  // |   opcode           |    I8   |    rj  |  rd   |
  static inline int insn_I8RR (int op, int imm8, int rj, int rd)  { /*assert(is_simm(imm8, 8), "not a signed 8-bit int");*/ return (op<<18) | (low(imm8, 8)<<10) | (rj<<5) | rd; }

  // 2RI12-type
  //  31           22 21          10 9      5 4     0
  // |   opcode      |     I12      |    rj  |  rd   |
  static inline int insn_I12RR(int op, int imm12, int rj, int rd) { /* assert(is_simm(imm12, 12), "not a signed 12-bit int");*/  return (op<<22) | (low(imm12, 12)<<10) | (rj<<5) | rd; }


  // 2RI14-type
  //  31         24 23            10 9      5 4     0
  // |   opcode    |      I14       |    rj  |  rd   |
  static inline int insn_I14RR(int op, int imm14, int rj, int rd) { assert(is_simm(imm14, 14), "not a signed 14-bit int"); return (op<<24) | (low(imm14, 14)<<10) | (rj<<5) | rd; }

  // 2RI16-type
  //  31       26 25              10 9      5 4     0
  // |   opcode  |       I16        |    rj  |  rd   |
  static inline int insn_I16RR(int op, int imm16, int rj, int rd) { assert(is_simm16(imm16), "not a signed 16-bit int"); return (op<<26) | (low16(imm16)<<10) | (rj<<5) | rd; }

  // 1RI20-type (?)
  //  31        25 24                      5 4     0
  // |   opcode   |               I20       |  rd   |
  static inline int insn_I20R (int op, int imm20, int rd) { assert(is_simm(imm20, 20), "not a signed 20-bit int"); return (op<<25) | (low(imm20, 20)<<5) | rd; }

  // 1RI21-type
  //  31       26 25              10 9     5 4        0
  // |   opcode  |     I21[15:0]    |   rj  |I21[20:16]|
  static inline int insn_IRI(int op, int imm21, int rj) { assert(is_simm(imm21, 21), "not a signed 21-bit int"); return (op << 26) | (low16(imm21) << 10) | (rj << 5) | low(imm21 >> 16, 5); }

  // I26-type
  //  31       26 25              10 9               0
  // |   opcode  |     I26[15:0]    |    I26[25:16]   |
  static inline int insn_I26(int op, int imm26) { assert(is_simm(imm26, 26), "not a signed 26-bit int"); return (op << 26) | (low16(imm26) << 10) | low(imm26 >> 16, 10); }

  // imm15
  //  31                    15 14                    0
  // |         opcode         |          I15          |
  static inline int insn_I15  (int op, int imm15) { assert(is_uimm(imm15, 15), "not a unsigned 15-bit int"); return (op<<15) | low(imm15, 15); }


  // get the offset field of beq, bne, blt[u], bge[u] instruction
  int offset16(address entry) {
    assert(is_simm16((entry - pc()) / 4), "change this code");
    if (!is_simm16((entry - pc()) / 4)) {
      tty->print_cr("!!! is_simm16: %lx", (entry - pc()) / 4);
    }
    return (entry - pc()) / 4;
  }

  // get the offset field of beqz, bnez instruction
  int offset21(address entry) {
    assert(is_simm((int)(entry - pc()) / 4, 21), "change this code");
    if (!is_simm((int)(entry - pc()) / 4, 21)) {
      tty->print_cr("!!! is_simm21: %lx", (entry - pc()) / 4);
    }
    return (entry - pc()) / 4;
  }

  // get the offset field of b instruction
  int offset26(address entry) {
    assert(is_simm((int)(entry - pc()) / 4, 26), "change this code");
    if (!is_simm((int)(entry - pc()) / 4, 26)) {
      tty->print_cr("!!! is_simm26: %lx", (entry - pc()) / 4);
    }
    return (entry - pc()) / 4;
  }

public:
  using AbstractAssembler::offset;

  //sign expand with the sign bit is h
  static int expand(int x, int h) { return -(x & (1<<h)) | x;  }

  // If x is a mask, return the number of one-bit in x.
  // else return -1.
  static int is_int_mask(int x);

  // If x is a mask, return the number of one-bit in x.
  // else return -1.
  static int is_jlong_mask(jlong x);

  // LOONGARCH lui is sign extended, so if you wan't to use imm, you have to use the follow
  static int split_low16(int x) {
    return (x & 0xffff);
  }

  // Convert 16-bit x to a sign-extended 16-bit integer
  static int simm16(int x) {
    assert(x == (x & 0xFFFF), "must be 16-bit only");
    return (x << 16) >> 16;
  }

  static int split_high16(int x) {
    return ( (x >> 16) + ((x & 0x8000) != 0) ) & 0xffff;
  }

  static int split_low20(int x) {
    return (x & 0xfffff);
  }

  // Convert 20-bit x to a sign-extended 20-bit integer
  static int simm20(int x) {
    assert(x == (x & 0xFFFFF), "must be 20-bit only");
    return (x << 12) >> 12;
  }

  static int split_low12(int x) {
    return (x & 0xfff);
  }

  static inline void split_simm38(jlong si38, jint& si18, jint& si20) {
    si18 = ((jint)(si38 & 0x3ffff) << 14) >> 14;
    si38 += (si38 & 0x20000) << 1;
    si20 = si38 >> 18;
  }

  // Convert 12-bit x to a sign-extended 12-bit integer
  static int simm12(int x) {
    assert(x == (x & 0xFFF), "must be 12-bit only");
    return (x << 20) >> 20;
  }

  // Convert 26-bit x to a sign-extended 26-bit integer
  static int simm26(int x) {
    assert(x == (x & 0x3FFFFFF), "must be 26-bit only");
    return (x << 6) >> 6;
  }

  static intptr_t merge(intptr_t x0, intptr_t x12) {
    //lu12i, ori
    return (((x12 << 12) | x0) << 32) >> 32;
  }

  static intptr_t merge(intptr_t x0, intptr_t x12, intptr_t x32) {
    //lu32i, lu12i, ori
    return (((x32 << 32) | (x12 << 12) | x0) << 12) >> 12;
  }

  static intptr_t merge(intptr_t x0, intptr_t x12, intptr_t x32, intptr_t x52) {
    //lu52i, lu32i, lu12i, ori
    return (x52 << 52) | (x32 << 32) | (x12 << 12) | x0;
  }

  // Test if x is within signed immediate range for nbits.
  static bool is_simm  (int x, unsigned int nbits) {
    assert(0 < nbits && nbits < 32, "out of bounds");
    const int   min      = -( ((int)1) << nbits-1 );
    const int   maxplus1 =  ( ((int)1) << nbits-1 );
    return min <= x && x < maxplus1;
  }

  static bool is_simm(jlong x, unsigned int nbits) {
    assert(0 < nbits && nbits < 64, "out of bounds");
    const jlong min      = -( ((jlong)1) << nbits-1 );
    const jlong maxplus1 =  ( ((jlong)1) << nbits-1 );
    return min <= x && x < maxplus1;
  }

  static bool is_simm16(int x)            { return is_simm(x, 16); }
  static bool is_simm16(long x)           { return is_simm((jlong)x, (unsigned int)16); }

  // Test if x is within unsigned immediate range for nbits
  static bool is_uimm(int x, unsigned int nbits) {
    assert(0 < nbits && nbits < 32, "out of bounds");
    const int   maxplus1 = ( ((int)1) << nbits );
    return 0 <= x && x < maxplus1;
  }

  static bool is_uimm(jlong x, unsigned int nbits) {
    assert(0 < nbits && nbits < 64, "out of bounds");
    const jlong maxplus1 =  ( ((jlong)1) << nbits );
    return 0 <= x && x < maxplus1;
  }

public:

  void flush() {
    AbstractAssembler::flush();
  }

  inline void emit_int32(int);
  inline void emit_data(int x) { emit_int32(x); }
  inline void emit_data(int, RelocationHolder const&);
  inline void emit_data(int, relocInfo::relocType rtype);


  // Generic instructions
  // Does 32bit or 64bit as needed for the platform. In some sense these
  // belong in macro assembler but there is no need for both varieties to exist

  void clo_w  (Register rd, Register rj) { emit_int32(insn_RR(clo_w_op, (int)rj->encoding(), (int)rd->encoding())); }
  void clz_w  (Register rd, Register rj) { emit_int32(insn_RR(clz_w_op, (int)rj->encoding(), (int)rd->encoding())); }
  void cto_w  (Register rd, Register rj) { emit_int32(insn_RR(cto_w_op, (int)rj->encoding(), (int)rd->encoding())); }
  void ctz_w  (Register rd, Register rj) { emit_int32(insn_RR(ctz_w_op, (int)rj->encoding(), (int)rd->encoding())); }
  void clo_d  (Register rd, Register rj) { emit_int32(insn_RR(clo_d_op, (int)rj->encoding(), (int)rd->encoding())); }
  void clz_d  (Register rd, Register rj) { emit_int32(insn_RR(clz_d_op, (int)rj->encoding(), (int)rd->encoding())); }
  void cto_d  (Register rd, Register rj) { emit_int32(insn_RR(cto_d_op, (int)rj->encoding(), (int)rd->encoding())); }
  void ctz_d  (Register rd, Register rj) { emit_int32(insn_RR(ctz_d_op, (int)rj->encoding(), (int)rd->encoding())); }

  void revb_2h(Register rd, Register rj) { emit_int32(insn_RR(revb_2h_op, (int)rj->encoding(), (int)rd->encoding())); }
  void revb_4h(Register rd, Register rj) { emit_int32(insn_RR(revb_4h_op, (int)rj->encoding(), (int)rd->encoding())); }
  void revb_2w(Register rd, Register rj) { emit_int32(insn_RR(revb_2w_op, (int)rj->encoding(), (int)rd->encoding())); }
  void revb_d (Register rd, Register rj) { emit_int32(insn_RR( revb_d_op, (int)rj->encoding(), (int)rd->encoding())); }
  void revh_2w(Register rd, Register rj) { emit_int32(insn_RR(revh_2w_op, (int)rj->encoding(), (int)rd->encoding())); }
  void revh_d (Register rd, Register rj) { emit_int32(insn_RR( revh_d_op, (int)rj->encoding(), (int)rd->encoding())); }

  void bitrev_4b(Register rd, Register rj) { emit_int32(insn_RR(bitrev_4b_op, (int)rj->encoding(), (int)rd->encoding())); }
  void bitrev_8b(Register rd, Register rj) { emit_int32(insn_RR(bitrev_8b_op, (int)rj->encoding(), (int)rd->encoding())); }
  void bitrev_w (Register rd, Register rj) { emit_int32(insn_RR(bitrev_w_op,  (int)rj->encoding(), (int)rd->encoding())); }
  void bitrev_d (Register rd, Register rj) { emit_int32(insn_RR(bitrev_d_op,  (int)rj->encoding(), (int)rd->encoding())); }

  void ext_w_h(Register rd, Register rj) { emit_int32(insn_RR(ext_w_h_op, (int)rj->encoding(), (int)rd->encoding())); }
  void ext_w_b(Register rd, Register rj) { emit_int32(insn_RR(ext_w_b_op, (int)rj->encoding(), (int)rd->encoding())); }

  void rdtimel_w(Register rd, Register rj) { emit_int32(insn_RR(rdtimel_w_op, (int)rj->encoding(), (int)rd->encoding())); }
  void rdtimeh_w(Register rd, Register rj) { emit_int32(insn_RR(rdtimeh_w_op, (int)rj->encoding(), (int)rd->encoding())); }
  void rdtime_d(Register rd, Register rj)  { emit_int32(insn_RR(rdtime_d_op, (int)rj->encoding(), (int)rd->encoding())); }

  void cpucfg(Register rd, Register rj) { emit_int32(insn_RR(cpucfg_op, (int)rj->encoding(), (int)rd->encoding())); }

  void asrtle_d (Register rj, Register rk) { emit_int32(insn_RRR(asrtle_d_op , (int)rk->encoding(), (int)rj->encoding(), 0)); }
  void asrtgt_d (Register rj, Register rk) { emit_int32(insn_RRR(asrtgt_d_op , (int)rk->encoding(), (int)rj->encoding(), 0)); }

  void alsl_w(Register rd, Register rj, Register rk, int sa2)  { assert(is_uimm(sa2, 2), "not a unsigned 2-bit int");  emit_int32(insn_I8RR(alsl_w_op, ( (0 << 7) | (sa2 << 5) | (int)rk->encoding() ), (int)rj->encoding(), (int)rd->encoding())); }
  void alsl_wu(Register rd, Register rj, Register rk, int sa2) { assert(is_uimm(sa2, 2), "not a unsigned 2-bit int"); emit_int32(insn_I8RR(alsl_w_op, ( (1 << 7) | (sa2 << 5) | (int)rk->encoding() ), (int)rj->encoding(), (int)rd->encoding())); }
  void bytepick_w(Register rd, Register rj, Register rk, int sa2) { assert(is_uimm(sa2, 2), "not a unsigned 2-bit int"); emit_int32(insn_I8RR(bytepick_w_op, ( (0 << 7) | (sa2 << 5) | (int)rk->encoding() ), (int)rj->encoding(), (int)rd->encoding())); }
  void bytepick_d(Register rd, Register rj, Register rk, int sa3) { assert(is_uimm(sa3, 3), "not a unsigned 3-bit int"); emit_int32(insn_I8RR(bytepick_d_op, ( (sa3 << 5) | (int)rk->encoding() ), (int)rj->encoding(), (int)rd->encoding())); }

  void add_w (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(add_w_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void add_d (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(add_d_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void sub_w (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(sub_w_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void sub_d (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(sub_d_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void slt  (Register rd, Register rj, Register rk)  { emit_int32(insn_RRR(slt_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void sltu (Register rd, Register rj, Register rk)  { emit_int32(insn_RRR(sltu_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }

  void maskeqz (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(maskeqz_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void masknez (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(masknez_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }

  void nor (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(nor_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void AND (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(and_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void OR  (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(or_op,   (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void XOR (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(xor_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void orn (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(orn_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void andn(Register rd, Register rj, Register rk) { emit_int32(insn_RRR(andn_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }

  void sll_w (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(sll_w_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void srl_w (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(srl_w_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void sra_w (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(sra_w_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void sll_d (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(sll_d_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void srl_d (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(srl_d_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void sra_d (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(sra_d_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }

  void rotr_w (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(rotr_w_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void rotr_d (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(rotr_d_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }

  void mul_w     (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(mul_w_op,     (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void mulh_w    (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(mulh_w_op,    (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void mulh_wu   (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(mulh_wu_op,   (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void mul_d     (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(mul_d_op,     (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void mulh_d    (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(mulh_d_op,    (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void mulh_du   (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(mulh_du_op,   (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void mulw_d_w  (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(mulw_d_w_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void mulw_d_wu (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(mulw_d_wu_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }

  void div_w (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(div_w_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void mod_w (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(mod_w_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void div_wu(Register rd, Register rj, Register rk) { emit_int32(insn_RRR(div_wu_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void mod_wu(Register rd, Register rj, Register rk) { emit_int32(insn_RRR(mod_wu_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void div_d (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(div_d_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void mod_d (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(mod_d_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void div_du(Register rd, Register rj, Register rk) { emit_int32(insn_RRR(div_du_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void mod_du(Register rd, Register rj, Register rk) { emit_int32(insn_RRR(mod_du_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }

  void crc_w_b_w  (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(crc_w_b_w_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void crc_w_h_w  (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(crc_w_h_w_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void crc_w_w_w  (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(crc_w_w_w_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void crc_w_d_w  (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(crc_w_d_w_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void crcc_w_b_w (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(crcc_w_b_w_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void crcc_w_h_w (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(crcc_w_h_w_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void crcc_w_w_w (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(crcc_w_w_w_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void crcc_w_d_w (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(crcc_w_d_w_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }

  void brk(int code)      { assert(is_uimm(code, 15), "not a unsigned 15-bit int"); emit_int32(insn_I15(break_op, code)); }

  void alsl_d(Register rd, Register rj, Register rk, int sa2)  { assert(is_uimm(sa2, 2), "not a unsigned 2-bit int");  emit_int32(insn_I8RR(alsl_d_op, ( (sa2 << 5) | (int)rk->encoding() ), (int)rj->encoding(), (int)rd->encoding())); }

  void slli_w(Register rd, Register rj, int ui5)  { assert(is_uimm(ui5, 5), "not a unsigned 5-bit int"); emit_int32(insn_I8RR(slli_op, ( (0b001 << 5) | ui5 ), (int)rj->encoding(), (int)rd->encoding())); }
  void slli_d(Register rd, Register rj, int ui6)  { assert(is_uimm(ui6, 6), "not a unsigned 6-bit int"); emit_int32(insn_I8RR(slli_op, ( (0b01  << 6) | ui6 ), (int)rj->encoding(), (int)rd->encoding())); }
  void srli_w(Register rd, Register rj, int ui5)  { assert(is_uimm(ui5, 5), "not a unsigned 5-bit int"); emit_int32(insn_I8RR(srli_op, ( (0b001 << 5) | ui5 ), (int)rj->encoding(), (int)rd->encoding())); }
  void srli_d(Register rd, Register rj, int ui6)  { assert(is_uimm(ui6, 6), "not a unsigned 6-bit int"); emit_int32(insn_I8RR(srli_op, ( (0b01  << 6) | ui6 ), (int)rj->encoding(), (int)rd->encoding())); }
  void srai_w(Register rd, Register rj, int ui5)  { assert(is_uimm(ui5, 5), "not a unsigned 5-bit int"); emit_int32(insn_I8RR(srai_op, ( (0b001 << 5) | ui5 ), (int)rj->encoding(), (int)rd->encoding())); }
  void srai_d(Register rd, Register rj, int ui6)  { assert(is_uimm(ui6, 6), "not a unsigned 6-bit int"); emit_int32(insn_I8RR(srai_op, ( (0b01  << 6) | ui6 ), (int)rj->encoding(), (int)rd->encoding())); }
  void rotri_w(Register rd, Register rj, int ui5) { assert(is_uimm(ui5, 5), "not a unsigned 5-bit int"); emit_int32(insn_I8RR(rotri_op, ( (0b001 << 5) | ui5 ), (int)rj->encoding(), (int)rd->encoding())); }
  void rotri_d(Register rd, Register rj, int ui6) { assert(is_uimm(ui6, 6), "not a unsigned 6-bit int"); emit_int32(insn_I8RR(rotri_op, ( (0b01  << 6) | ui6 ), (int)rj->encoding(), (int)rd->encoding())); }

  void bstrins_w  (Register rd, Register rj, int msbw, int lsbw)  { assert(is_uimm(msbw, 5) && is_uimm(lsbw, 5), "not a unsigned 5-bit int"); emit_int32(insn_I12RR(bstr_w_op, ( (1<<11) | (low(msbw, 5)<<6) | (0<<5) | low(lsbw, 5) ), (int)rj->encoding(), (int)rd->encoding())); }
  void bstrpick_w  (Register rd, Register rj, int msbw, int lsbw) { assert(is_uimm(msbw, 5) && is_uimm(lsbw, 5), "not a unsigned 5-bit int"); emit_int32(insn_I12RR(bstr_w_op, ( (1<<11) | (low(msbw, 5)<<6) | (1<<5) | low(lsbw, 5) ), (int)rj->encoding(), (int)rd->encoding())); }
  void bstrins_d  (Register rd, Register rj, int msbd, int lsbd)  { assert(is_uimm(msbd, 6) && is_uimm(lsbd, 6), "not a unsigned 6-bit int"); emit_int32(insn_I12RR(bstrins_d_op, ( (low(msbd, 6)<<6) | low(lsbd, 6) ), (int)rj->encoding(), (int)rd->encoding())); }
  void bstrpick_d  (Register rd, Register rj, int msbd, int lsbd) { assert(is_uimm(msbd, 6) && is_uimm(lsbd, 6), "not a unsigned 6-bit int"); emit_int32(insn_I12RR(bstrpick_d_op, ( (low(msbd, 6)<<6) | low(lsbd, 6) ), (int)rj->encoding(), (int)rd->encoding())); }

  void fadd_s  (FloatRegister fd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRR(fadd_s_op,  (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fadd_d  (FloatRegister fd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRR(fadd_d_op,  (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fsub_s  (FloatRegister fd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRR(fsub_s_op,  (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fsub_d  (FloatRegister fd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRR(fsub_d_op,  (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fmul_s  (FloatRegister fd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRR(fmul_s_op,  (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fmul_d  (FloatRegister fd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRR(fmul_d_op,  (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fdiv_s  (FloatRegister fd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRR(fdiv_s_op,  (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fdiv_d  (FloatRegister fd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRR(fdiv_d_op,  (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fmax_s  (FloatRegister fd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRR(fmax_s_op,  (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fmax_d  (FloatRegister fd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRR(fmax_d_op,  (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fmin_s  (FloatRegister fd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRR(fmin_s_op,  (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fmin_d  (FloatRegister fd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRR(fmin_d_op,  (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fmaxa_s (FloatRegister fd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRR(fmaxa_s_op, (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fmaxa_d (FloatRegister fd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRR(fmaxa_d_op, (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fmina_s (FloatRegister fd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRR(fmina_s_op, (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fmina_d (FloatRegister fd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRR(fmina_d_op, (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }

  void fscaleb_s (FloatRegister fd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRR(fscaleb_s_op, (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fscaleb_d (FloatRegister fd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRR(fscaleb_d_op, (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fcopysign_s (FloatRegister fd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRR(fcopysign_s_op, (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fcopysign_d (FloatRegister fd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRR(fcopysign_d_op, (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }

  void fabs_s(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(fabs_s_op, (int)fj->encoding(), (int)fd->encoding())); }
  void fabs_d(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(fabs_d_op, (int)fj->encoding(), (int)fd->encoding())); }
  void fneg_s(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(fneg_s_op, (int)fj->encoding(), (int)fd->encoding())); }
  void fneg_d(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(fneg_d_op, (int)fj->encoding(), (int)fd->encoding())); }
  void flogb_s(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(flogb_s_op, (int)fj->encoding(), (int)fd->encoding())); }
  void flogb_d(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(flogb_d_op, (int)fj->encoding(), (int)fd->encoding())); }
  void fclass_s(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(fclass_s_op, (int)fj->encoding(), (int)fd->encoding())); }
  void fclass_d(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(fclass_d_op, (int)fj->encoding(), (int)fd->encoding())); }
  void fsqrt_s(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(fsqrt_s_op, (int)fj->encoding(), (int)fd->encoding())); }
  void fsqrt_d(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(fsqrt_d_op, (int)fj->encoding(), (int)fd->encoding())); }
  void frecip_s(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(frecip_s_op, (int)fj->encoding(), (int)fd->encoding())); }
  void frecip_d(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(frecip_d_op, (int)fj->encoding(), (int)fd->encoding())); }
  void frsqrt_s(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(frsqrt_s_op, (int)fj->encoding(), (int)fd->encoding())); }
  void frsqrt_d(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(frsqrt_d_op, (int)fj->encoding(), (int)fd->encoding())); }
  void fmov_s(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(fmov_s_op, (int)fj->encoding(), (int)fd->encoding())); }
  void fmov_d(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(fmov_d_op, (int)fj->encoding(), (int)fd->encoding())); }

  void movgr2fr_w (FloatRegister fd, Register rj)  { emit_int32(insn_RR(movgr2fr_w_op,  (int)rj->encoding(), (int)fd->encoding())); }
  void movgr2fr_d (FloatRegister fd, Register rj)  { emit_int32(insn_RR(movgr2fr_d_op,  (int)rj->encoding(), (int)fd->encoding())); }
  void movgr2frh_w(FloatRegister fd, Register rj)  { emit_int32(insn_RR(movgr2frh_w_op, (int)rj->encoding(), (int)fd->encoding())); }
  void movfr2gr_s (Register rd, FloatRegister fj)  { emit_int32(insn_RR(movfr2gr_s_op,  (int)fj->encoding(), (int)rd->encoding())); }
  void movfr2gr_d (Register rd, FloatRegister fj)  { emit_int32(insn_RR(movfr2gr_d_op,  (int)fj->encoding(), (int)rd->encoding())); }
  void movfrh2gr_s(Register rd, FloatRegister fj)  { emit_int32(insn_RR(movfrh2gr_s_op, (int)fj->encoding(), (int)rd->encoding())); }
  void movgr2fcsr (int fcsr, Register rj)  { assert(is_uimm(fcsr, 2), "not a unsigned 2-bit init: fcsr0-fcsr3"); emit_int32(insn_RR(movgr2fcsr_op,  (int)rj->encoding(), fcsr)); }
  void movfcsr2gr (Register rd, int fcsr)  { assert(is_uimm(fcsr, 2), "not a unsigned 2-bit init: fcsr0-fcsr3"); emit_int32(insn_RR(movfcsr2gr_op,  fcsr, (int)rd->encoding())); }
  void movfr2cf   (ConditionalFlagRegister cd, FloatRegister fj)  { emit_int32(insn_RR(movfr2cf_op,    (int)fj->encoding(), (int)cd->encoding())); }
  void movcf2fr   (FloatRegister fd, ConditionalFlagRegister cj)  { emit_int32(insn_RR(movcf2fr_op,    (int)cj->encoding(), (int)fd->encoding())); }
  void movgr2cf   (ConditionalFlagRegister cd, Register rj)  { emit_int32(insn_RR(movgr2cf_op,    (int)rj->encoding(), (int)cd->encoding())); }
  void movcf2gr   (Register rd, ConditionalFlagRegister cj)  { emit_int32(insn_RR(movcf2gr_op,    (int)cj->encoding(), (int)rd->encoding())); }

  void fcvt_s_d(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(fcvt_s_d_op, (int)fj->encoding(), (int)fd->encoding())); }
  void fcvt_d_s(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(fcvt_d_s_op, (int)fj->encoding(), (int)fd->encoding())); }

  void ftintrm_w_s(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ftintrm_w_s_op, (int)fj->encoding(), (int)fd->encoding())); }
  void ftintrm_w_d(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ftintrm_w_d_op, (int)fj->encoding(), (int)fd->encoding())); }
  void ftintrm_l_s(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ftintrm_l_s_op, (int)fj->encoding(), (int)fd->encoding())); }
  void ftintrm_l_d(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ftintrm_l_d_op, (int)fj->encoding(), (int)fd->encoding())); }
  void ftintrp_w_s(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ftintrp_w_s_op, (int)fj->encoding(), (int)fd->encoding())); }
  void ftintrp_w_d(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ftintrp_w_d_op, (int)fj->encoding(), (int)fd->encoding())); }
  void ftintrp_l_s(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ftintrp_l_s_op, (int)fj->encoding(), (int)fd->encoding())); }
  void ftintrp_l_d(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ftintrp_l_d_op, (int)fj->encoding(), (int)fd->encoding())); }
  void ftintrz_w_s(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ftintrz_w_s_op, (int)fj->encoding(), (int)fd->encoding())); }
  void ftintrz_w_d(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ftintrz_w_d_op, (int)fj->encoding(), (int)fd->encoding())); }
  void ftintrz_l_s(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ftintrz_l_s_op, (int)fj->encoding(), (int)fd->encoding())); }
  void ftintrz_l_d(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ftintrz_l_d_op, (int)fj->encoding(), (int)fd->encoding())); }
  void ftintrne_w_s(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ftintrne_w_s_op, (int)fj->encoding(), (int)fd->encoding())); }
  void ftintrne_w_d(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ftintrne_w_d_op, (int)fj->encoding(), (int)fd->encoding())); }
  void ftintrne_l_s(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ftintrne_l_s_op, (int)fj->encoding(), (int)fd->encoding())); }
  void ftintrne_l_d(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ftintrne_l_d_op, (int)fj->encoding(), (int)fd->encoding())); }
  void ftint_w_s(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ftint_w_s_op, (int)fj->encoding(), (int)fd->encoding())); }
  void ftint_w_d(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ftint_w_d_op, (int)fj->encoding(), (int)fd->encoding())); }
  void ftint_l_s(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ftint_l_s_op, (int)fj->encoding(), (int)fd->encoding())); }
  void ftint_l_d(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ftint_l_d_op, (int)fj->encoding(), (int)fd->encoding())); }
  void ffint_s_w(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ffint_s_w_op, (int)fj->encoding(), (int)fd->encoding())); }
  void ffint_s_l(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ffint_s_l_op, (int)fj->encoding(), (int)fd->encoding())); }
  void ffint_d_w(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ffint_d_w_op, (int)fj->encoding(), (int)fd->encoding())); }
  void ffint_d_l(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(ffint_d_l_op, (int)fj->encoding(), (int)fd->encoding())); }
  void frint_s(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(frint_s_op, (int)fj->encoding(), (int)fd->encoding())); }
  void frint_d(FloatRegister fd, FloatRegister fj)  { emit_int32(insn_RR(frint_d_op, (int)fj->encoding(), (int)fd->encoding())); }

  void slti  (Register rd, Register rj, int si12)  { assert(is_simm(si12, 12), "not a signed 12-bit int"); emit_int32(insn_I12RR(slti_op,   si12, (int)rj->encoding(), (int)rd->encoding())); }
  void sltui (Register rd, Register rj, int si12)  { assert(is_simm(si12, 12), "not a signed 12-bit int"); emit_int32(insn_I12RR(sltui_op,  si12, (int)rj->encoding(), (int)rd->encoding())); }
  void addi_w(Register rd, Register rj, int si12)  { assert(is_simm(si12, 12), "not a signed 12-bit int"); emit_int32(insn_I12RR(addi_w_op, si12, (int)rj->encoding(), (int)rd->encoding())); }
  void addi_d(Register rd, Register rj, int si12)  { assert(is_simm(si12, 12), "not a signed 12-bit int"); emit_int32(insn_I12RR(addi_d_op, si12, (int)rj->encoding(), (int)rd->encoding())); }
  void lu52i_d(Register rd, Register rj, int si12) { /*assert(is_simm(si12, 12), "not a signed 12-bit int");*/ emit_int32(insn_I12RR(lu52i_d_op,  simm12(si12), (int)rj->encoding(), (int)rd->encoding())); }
  void andi  (Register rd, Register rj, int ui12)  { assert(is_uimm(ui12, 12), "not a unsigned 12-bit int"); emit_int32(insn_I12RR(andi_op,   ui12, (int)rj->encoding(), (int)rd->encoding())); }
  void ori   (Register rd, Register rj, int ui12)  { assert(is_uimm(ui12, 12), "not a unsigned 12-bit int"); emit_int32(insn_I12RR(ori_op,    ui12, (int)rj->encoding(), (int)rd->encoding())); }
  void xori  (Register rd, Register rj, int ui12)  { assert(is_uimm(ui12, 12), "not a unsigned 12-bit int"); emit_int32(insn_I12RR(xori_op,   ui12, (int)rj->encoding(), (int)rd->encoding())); }

  void fmadd_s (FloatRegister fd, FloatRegister fj, FloatRegister fk, FloatRegister fa) { emit_int32(insn_RRRR(fmadd_s_op , (int)fa->encoding(), (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fmadd_d (FloatRegister fd, FloatRegister fj, FloatRegister fk, FloatRegister fa) { emit_int32(insn_RRRR(fmadd_d_op , (int)fa->encoding(), (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fmsub_s (FloatRegister fd, FloatRegister fj, FloatRegister fk, FloatRegister fa) { emit_int32(insn_RRRR(fmsub_s_op , (int)fa->encoding(), (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fmsub_d (FloatRegister fd, FloatRegister fj, FloatRegister fk, FloatRegister fa) { emit_int32(insn_RRRR(fmsub_d_op , (int)fa->encoding(), (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fnmadd_s (FloatRegister fd, FloatRegister fj, FloatRegister fk, FloatRegister fa) { emit_int32(insn_RRRR(fnmadd_s_op , (int)fa->encoding(), (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fnmadd_d (FloatRegister fd, FloatRegister fj, FloatRegister fk, FloatRegister fa) { emit_int32(insn_RRRR(fnmadd_d_op , (int)fa->encoding(), (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fnmsub_s (FloatRegister fd, FloatRegister fj, FloatRegister fk, FloatRegister fa)  { emit_int32(insn_RRRR(fnmsub_s_op , (int)fa->encoding(), (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }
  void fnmsub_d (FloatRegister fd, FloatRegister fj, FloatRegister fk, FloatRegister fa)  { emit_int32(insn_RRRR(fnmsub_d_op , (int)fa->encoding(), (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }

  void fcmp_caf_s  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_s_op, fcmp_caf, (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_cun_s  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_s_op, fcmp_cun , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_ceq_s  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_s_op, fcmp_ceq , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_cueq_s (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_s_op, fcmp_cueq, (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_clt_s  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_s_op, fcmp_clt , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_cult_s (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_s_op, fcmp_cult, (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_cle_s  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_s_op, fcmp_cle , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_cule_s (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_s_op, fcmp_cule, (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_cne_s  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_s_op, fcmp_cne , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_cor_s  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_s_op, fcmp_cor , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_cune_s (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_s_op, fcmp_cune, (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_saf_s  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_s_op, fcmp_saf , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_sun_s  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_s_op, fcmp_sun , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_seq_s  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_s_op, fcmp_seq , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_sueq_s (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_s_op, fcmp_sueq, (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_slt_s  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_s_op, fcmp_slt , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_sult_s (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_s_op, fcmp_sult, (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_sle_s  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_s_op, fcmp_sle , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_sule_s (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_s_op, fcmp_sule, (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_sne_s  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_s_op, fcmp_sne , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_sor_s  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_s_op, fcmp_sor , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_sune_s (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_s_op, fcmp_sune, (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }

  void fcmp_caf_d  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_d_op, fcmp_caf, (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_cun_d  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_d_op, fcmp_cun , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_ceq_d  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_d_op, fcmp_ceq , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_cueq_d (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_d_op, fcmp_cueq, (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_clt_d  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_d_op, fcmp_clt , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_cult_d (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_d_op, fcmp_cult, (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_cle_d  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_d_op, fcmp_cle , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_cule_d (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_d_op, fcmp_cule, (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_cne_d  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_d_op, fcmp_cne , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_cor_d  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_d_op, fcmp_cor , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_cune_d (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_d_op, fcmp_cune, (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_saf_d  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_d_op, fcmp_saf , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_sun_d  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_d_op, fcmp_sun , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_seq_d  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_d_op, fcmp_seq , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_sueq_d (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_d_op, fcmp_sueq, (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_slt_d  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_d_op, fcmp_slt , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_sult_d (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_d_op, fcmp_sult, (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_sle_d  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_d_op, fcmp_sle , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_sule_d (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_d_op, fcmp_sule, (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_sne_d  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_d_op, fcmp_sne , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_sor_d  (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_d_op, fcmp_sor , (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }
  void fcmp_sune_d (ConditionalFlagRegister cd, FloatRegister fj, FloatRegister fk) { emit_int32(insn_RRRR(fcmp_cond_d_op, fcmp_sune, (int)fk->encoding(), (int)fj->encoding(), (int)cd->encoding())); }

  void fsel (FloatRegister fd, FloatRegister fj, FloatRegister fk, ConditionalFlagRegister ca) { emit_int32(insn_RRRR(fsel_op, (int)ca->encoding(), (int)fk->encoding(), (int)fj->encoding(), (int)fd->encoding())); }

  void addu16i_d(Register rj, Register rd, int si16)      { assert(is_simm(si16, 16), "not a signed 16-bit int"); emit_int32(insn_I16RR(addu16i_d_op, si16, (int)rj->encoding(), (int)rd->encoding())); }

  void lu12i_w(Register rj, int si20)      { /*assert(is_simm(si20, 20), "not a signed 20-bit int");*/ emit_int32(insn_I20R(lu12i_w_op, simm20(si20), (int)rj->encoding())); }
  void lu32i_d(Register rj, int si20)      { /*assert(is_simm(si20, 20), "not a signed 20-bit int");*/ emit_int32(insn_I20R(lu32i_d_op, simm20(si20), (int)rj->encoding())); }
  void pcaddi(Register rj, int si20)      { assert(is_simm(si20, 20), "not a signed 20-bit int"); emit_int32(insn_I20R(pcaddi_op, si20, (int)rj->encoding())); }
  void pcalau12i(Register rj, int si20)      { assert(is_simm(si20, 20), "not a signed 20-bit int"); emit_int32(insn_I20R(pcalau12i_op, si20, (int)rj->encoding())); }
  void pcaddu12i(Register rj, int si20)      { assert(is_simm(si20, 20), "not a signed 20-bit int"); emit_int32(insn_I20R(pcaddu12i_op, si20, (int)rj->encoding())); }
  void pcaddu18i(Register rj, int si20)      { assert(is_simm(si20, 20), "not a signed 20-bit int"); emit_int32(insn_I20R(pcaddu18i_op, si20, (int)rj->encoding())); }

  void ll_w  (Register rd, Register rj, int si16)   { assert(is_simm(si16, 16) && ((si16 & 0x3) == 0), "not a signed 16-bit int"); emit_int32(insn_I14RR(ll_w_op, si16>>2, (int)rj->encoding(), (int)rd->encoding())); }
  void sc_w  (Register rd, Register rj, int si16)   { assert(is_simm(si16, 16) && ((si16 & 0x3) == 0), "not a signed 16-bit int"); emit_int32(insn_I14RR(sc_w_op, si16>>2, (int)rj->encoding(), (int)rd->encoding())); }
  void ll_d  (Register rd, Register rj, int si16)   { assert(is_simm(si16, 16) && ((si16 & 0x3) == 0), "not a signed 16-bit int"); emit_int32(insn_I14RR(ll_d_op, si16>>2, (int)rj->encoding(), (int)rd->encoding())); }
  void sc_d  (Register rd, Register rj, int si16)   { assert(is_simm(si16, 16) && ((si16 & 0x3) == 0), "not a signed 16-bit int"); emit_int32(insn_I14RR(sc_d_op, si16>>2, (int)rj->encoding(), (int)rd->encoding())); }
  void ldptr_w  (Register rd, Register rj, int si16)  { assert(is_simm(si16, 16) && ((si16 & 0x3) == 0), "not a signed 16-bit int"); emit_int32(insn_I14RR(ldptr_w_op, si16>>2, (int)rj->encoding(), (int)rd->encoding())); }
  void stptr_w  (Register rd, Register rj, int si16)  { assert(is_simm(si16, 16) && ((si16 & 0x3) == 0), "not a signed 16-bit int"); emit_int32(insn_I14RR(stptr_w_op, si16>>2, (int)rj->encoding(), (int)rd->encoding())); }
  void ldptr_d  (Register rd, Register rj, int si16)  { assert(is_simm(si16, 16) && ((si16 & 0x3) == 0), "not a signed 16-bit int"); emit_int32(insn_I14RR(ldptr_d_op, si16>>2, (int)rj->encoding(), (int)rd->encoding())); }
  void stptr_d  (Register rd, Register rj, int si16)  { assert(is_simm(si16, 16) && ((si16 & 0x3) == 0), "not a signed 16-bit int"); emit_int32(insn_I14RR(stptr_d_op, si16>>2, (int)rj->encoding(), (int)rd->encoding())); }

  void ld_b  (Register rd, Register rj, int si12)  { assert(is_simm(si12, 12), "not a signed 12-bit int"); emit_int32(insn_I12RR(ld_b_op,  si12, (int)rj->encoding(), (int)rd->encoding())); }
  void ld_h  (Register rd, Register rj, int si12)  { assert(is_simm(si12, 12), "not a signed 12-bit int"); emit_int32(insn_I12RR(ld_h_op,  si12, (int)rj->encoding(), (int)rd->encoding())); }
  void ld_w  (Register rd, Register rj, int si12)  { assert(is_simm(si12, 12), "not a signed 12-bit int"); emit_int32(insn_I12RR(ld_w_op,  si12, (int)rj->encoding(), (int)rd->encoding())); }
  void ld_d  (Register rd, Register rj, int si12)  { assert(is_simm(si12, 12), "not a signed 12-bit int"); emit_int32(insn_I12RR(ld_d_op,  si12, (int)rj->encoding(), (int)rd->encoding())); }
  void st_b  (Register rd, Register rj, int si12)  { assert(is_simm(si12, 12), "not a signed 12-bit int"); emit_int32(insn_I12RR(st_b_op,  si12, (int)rj->encoding(), (int)rd->encoding())); }
  void st_h  (Register rd, Register rj, int si12)  { assert(is_simm(si12, 12), "not a signed 12-bit int"); emit_int32(insn_I12RR(st_h_op,  si12, (int)rj->encoding(), (int)rd->encoding())); }
  void st_w  (Register rd, Register rj, int si12)  { assert(is_simm(si12, 12), "not a signed 12-bit int"); emit_int32(insn_I12RR(st_w_op,  si12, (int)rj->encoding(), (int)rd->encoding())); }
  void st_d  (Register rd, Register rj, int si12)  { assert(is_simm(si12, 12), "not a signed 12-bit int"); emit_int32(insn_I12RR(st_d_op,  si12, (int)rj->encoding(), (int)rd->encoding())); }
  void ld_bu (Register rd, Register rj, int si12)  { assert(is_simm(si12, 12), "not a signed 12-bit int"); emit_int32(insn_I12RR(ld_bu_op, si12, (int)rj->encoding(), (int)rd->encoding())); }
  void ld_hu (Register rd, Register rj, int si12)  { assert(is_simm(si12, 12), "not a signed 12-bit int"); emit_int32(insn_I12RR(ld_hu_op, si12, (int)rj->encoding(), (int)rd->encoding())); }
  void ld_wu (Register rd, Register rj, int si12)  { assert(is_simm(si12, 12), "not a signed 12-bit int"); emit_int32(insn_I12RR(ld_wu_op, si12, (int)rj->encoding(), (int)rd->encoding())); }
  void preld (int hint, Register rj, int si12)  { assert(is_uimm(hint, 5), "not a unsigned 5-bit int"); assert(is_simm(si12, 12), "not a signed 12-bit int"); emit_int32(insn_I12RR(preld_op, si12, (int)rj->encoding(), hint)); }
  void fld_s (FloatRegister fd, Register rj, int si12)  { assert(is_simm(si12, 12), "not a signed 12-bit int"); emit_int32(insn_I12RR(fld_s_op, si12, (int)rj->encoding(), (int)fd->encoding())); }
  void fst_s (FloatRegister fd, Register rj, int si12)  { assert(is_simm(si12, 12), "not a signed 12-bit int"); emit_int32(insn_I12RR(fst_s_op, si12, (int)rj->encoding(), (int)fd->encoding())); }
  void fld_d (FloatRegister fd, Register rj, int si12)  { assert(is_simm(si12, 12), "not a signed 12-bit int"); emit_int32(insn_I12RR(fld_d_op, si12, (int)rj->encoding(), (int)fd->encoding())); }
  void fst_d (FloatRegister fd, Register rj, int si12)  { assert(is_simm(si12, 12), "not a signed 12-bit int"); emit_int32(insn_I12RR(fst_d_op, si12, (int)rj->encoding(), (int)fd->encoding())); }

  void ldx_b  (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(ldx_b_op,     (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ldx_h  (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(ldx_h_op,     (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ldx_w  (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(ldx_w_op,     (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ldx_d  (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(ldx_d_op,     (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void stx_b  (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(stx_b_op,     (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void stx_h  (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(stx_h_op,     (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void stx_w  (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(stx_w_op,     (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void stx_d  (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(stx_d_op,     (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ldx_bu (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(ldx_bu_op,    (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ldx_hu (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(ldx_hu_op,    (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ldx_wu (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(ldx_wu_op,    (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void fldx_s (FloatRegister fd, Register rj, Register rk) { emit_int32(insn_RRR(fldx_s_op,    (int)rk->encoding(), (int)rj->encoding(), (int)fd->encoding())); }
  void fldx_d (FloatRegister fd, Register rj, Register rk) { emit_int32(insn_RRR(fldx_d_op,    (int)rk->encoding(), (int)rj->encoding(), (int)fd->encoding())); }
  void fstx_s (FloatRegister fd, Register rj, Register rk) { emit_int32(insn_RRR(fstx_s_op,    (int)rk->encoding(), (int)rj->encoding(), (int)fd->encoding())); }
  void fstx_d (FloatRegister fd, Register rj, Register rk) { emit_int32(insn_RRR(fstx_d_op,    (int)rk->encoding(), (int)rj->encoding(), (int)fd->encoding())); }

  void ld_b  (Register rd, Address src);
  void ld_bu (Register rd, Address src);
  void ld_d  (Register rd, Address src);
  void ld_h  (Register rd, Address src);
  void ld_hu (Register rd, Address src);
  void ll_w  (Register rd, Address src);
  void ll_d  (Register rd, Address src);
  void ld_wu (Register rd, Address src);
  void ld_w  (Register rd, Address src);
  void st_b  (Register rd, Address dst);
  void st_d  (Register rd, Address dst);
  void st_w  (Register rd, Address dst);
  void sc_w  (Register rd, Address dst);
  void sc_d  (Register rd, Address dst);
  void st_h  (Register rd, Address dst);
  void fld_s (FloatRegister fd, Address src);
  void fld_d (FloatRegister fd, Address src);
  void fst_s (FloatRegister fd, Address dst);
  void fst_d (FloatRegister fd, Address dst);

  void amswap_w   (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(amswap_w_op,    (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void amswap_d   (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(amswap_d_op,    (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void amadd_w    (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(amadd_w_op,     (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void amadd_d    (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(amadd_d_op,     (int)rk->encoding(), (int)rj->encoding(), (int)rj->encoding())); }
  void amand_w    (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(amand_w_op,     (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void amand_d    (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(amand_d_op,     (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void amor_w     (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(amor_w_op,      (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void amor_d     (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(amor_d_op,      (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void amxor_w    (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(amxor_w_op,     (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void amxor_d    (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(amxor_d_op,     (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ammax_w    (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(ammax_w_op,     (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ammax_d    (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(ammax_d_op,     (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ammin_w    (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(ammin_w_op,     (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ammin_d    (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(ammin_d_op,     (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ammax_wu   (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(ammax_wu_op,    (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ammax_du   (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(ammax_du_op,    (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ammin_wu   (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(ammin_wu_op,    (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ammin_du   (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(ammin_du_op,    (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void amswap_db_w(Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(amswap_db_w_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void amswap_db_d(Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(amswap_db_d_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void amadd_db_w (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(amadd_db_w_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void amadd_db_d (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(amadd_db_d_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void amand_db_w (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(amand_db_w_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void amand_db_d (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(amand_db_d_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void amor_db_w  (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(amor_db_w_op,   (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void amor_db_d  (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(amor_db_d_op,   (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void amxor_db_w (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(amxor_db_w_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void amxor_db_d (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(amxor_db_d_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ammax_db_w (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(ammax_db_w_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ammax_db_d (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(ammax_db_d_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ammin_db_w (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(ammin_db_w_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ammin_db_d (Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(ammin_db_d_op,  (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ammax_db_wu(Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(ammax_db_wu_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ammax_db_du(Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(ammax_db_du_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ammin_db_wu(Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(ammin_db_wu_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ammin_db_du(Register rd, Register rk, Register rj) { assert_different_registers(rd, rj); assert_different_registers(rd, rk); emit_int32(insn_RRR(ammin_db_du_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }

  void dbar(int hint)      { assert(is_uimm(hint, 15), "not a unsigned 15-bit int"); emit_int32(insn_I15(dbar_op, hint)); }
  void ibar(int hint)      { assert(is_uimm(hint, 15), "not a unsigned 15-bit int"); emit_int32(insn_I15(ibar_op, hint)); }

  void fldgt_s (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(fldgt_s_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void fldgt_d (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(fldgt_d_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void fldle_s (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(fldle_s_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void fldle_d (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(fldle_d_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void fstgt_s (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(fstgt_s_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void fstgt_d (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(fstgt_d_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void fstle_s (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(fstle_s_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void fstle_d (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(fstle_d_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }

  void ldgt_b (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(ldgt_b_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ldgt_h (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(ldgt_h_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ldgt_w (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(ldgt_w_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ldgt_d (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(ldgt_d_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ldle_b (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(ldle_b_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ldle_h (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(ldle_h_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ldle_w (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(ldle_w_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void ldle_d (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(ldle_d_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void stgt_b (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(stgt_b_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void stgt_h (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(stgt_h_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void stgt_w (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(stgt_w_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void stgt_d (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(stgt_d_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void stle_b (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(stle_b_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void stle_h (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(stle_h_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void stle_w (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(stle_w_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }
  void stle_d (Register rd, Register rj, Register rk) { emit_int32(insn_RRR(stle_d_op, (int)rk->encoding(), (int)rj->encoding(), (int)rd->encoding())); }

  void beqz(Register rj, int offs)      { assert(is_simm(offs, 21), "not a signed 21-bit int"); emit_int32(insn_IRI(beqz_op, offs, (int)rj->encoding())); }
  void bnez(Register rj, int offs)      { assert(is_simm(offs, 21), "not a signed 21-bit int"); emit_int32(insn_IRI(bnez_op, offs, (int)rj->encoding())); }
  void bceqz(ConditionalFlagRegister cj, int offs)     { assert(is_simm(offs, 21), "not a signed 21-bit int"); emit_int32(insn_IRI(bccondz_op, offs, ( (0b00<<3) | (int)cj->encoding()))); }
  void bcnez(ConditionalFlagRegister cj, int offs)     { assert(is_simm(offs, 21), "not a signed 21-bit int"); emit_int32(insn_IRI(bccondz_op, offs, ( (0b01<<3) | (int)cj->encoding()))); }

  void jirl(Register rd, Register rj, int offs)      { assert(is_simm(offs, 18) && ((offs & 3) == 0), "not a signed 18-bit int"); emit_int32(insn_I16RR(jirl_op, offs >> 2, (int)rj->encoding(), (int)rd->encoding())); }

  void b(int offs)      { assert(is_simm(offs, 26), "not a signed 26-bit int"); emit_int32(insn_I26(b_op, offs)); }
  void bl(int offs)     { assert(is_simm(offs, 26), "not a signed 26-bit int"); emit_int32(insn_I26(bl_op, offs)); }


  void beq(Register rj, Register rd, int offs)      { assert(is_simm(offs, 16), "not a signed 16-bit int"); emit_int32(insn_I16RR(beq_op, offs, (int)rj->encoding(), (int)rd->encoding())); }
  void bne(Register rj, Register rd, int offs)      { assert(is_simm(offs, 16), "not a signed 16-bit int"); emit_int32(insn_I16RR(bne_op, offs, (int)rj->encoding(), (int)rd->encoding())); }
  void blt(Register rj, Register rd, int offs)      { assert(is_simm(offs, 16), "not a signed 16-bit int"); emit_int32(insn_I16RR(blt_op, offs, (int)rj->encoding(), (int)rd->encoding())); }
  void bge(Register rj, Register rd, int offs)      { assert(is_simm(offs, 16), "not a signed 16-bit int"); emit_int32(insn_I16RR(bge_op, offs, (int)rj->encoding(), (int)rd->encoding())); }
  void bltu(Register rj, Register rd, int offs)      { assert(is_simm(offs, 16), "not a signed 16-bit int"); emit_int32(insn_I16RR(bltu_op, offs, (int)rj->encoding(), (int)rd->encoding())); }
  void bgeu(Register rj, Register rd, int offs)      { assert(is_simm(offs, 16), "not a signed 16-bit int"); emit_int32(insn_I16RR(bgeu_op, offs, (int)rj->encoding(), (int)rd->encoding())); }

  void beq   (Register rj, Register rd, address entry) { beq   (rj, rd, offset16(entry)); }
  void bne   (Register rj, Register rd, address entry) { bne   (rj, rd, offset16(entry)); }
  void blt   (Register rj, Register rd, address entry) { blt   (rj, rd, offset16(entry)); }
  void bge   (Register rj, Register rd, address entry) { bge   (rj, rd, offset16(entry)); }
  void bltu  (Register rj, Register rd, address entry) { bltu  (rj, rd, offset16(entry)); }
  void bgeu  (Register rj, Register rd, address entry) { bgeu  (rj, rd, offset16(entry)); }
  void beqz  (Register rj, address entry) { beqz  (rj, offset21(entry)); }
  void bnez  (Register rj, address entry) { bnez  (rj, offset21(entry)); }
  void b(address entry) { b(offset26(entry)); }
  void bl(address entry) { bl(offset26(entry)); }
  void bceqz(ConditionalFlagRegister cj, address entry)     { bceqz(cj, offset21(entry)); }
  void bcnez(ConditionalFlagRegister cj, address entry)     { bcnez(cj, offset21(entry)); }

  void beq   (Register rj, Register rd, Label& L) { beq   (rj, rd, target(L)); }
  void bne   (Register rj, Register rd, Label& L) { bne   (rj, rd, target(L)); }
  void blt   (Register rj, Register rd, Label& L) { blt   (rj, rd, target(L)); }
  void bge   (Register rj, Register rd, Label& L) { bge   (rj, rd, target(L)); }
  void bltu  (Register rj, Register rd, Label& L) { bltu  (rj, rd, target(L)); }
  void bgeu  (Register rj, Register rd, Label& L) { bgeu  (rj, rd, target(L)); }
  void beqz  (Register rj, Label& L) { beqz  (rj, target(L)); }
  void bnez  (Register rj, Label& L) { bnez  (rj, target(L)); }
  void b(Label& L)      { b(target(L)); }
  void bl(Label& L)     { bl(target(L)); }
  void bceqz(ConditionalFlagRegister cj, Label& L)     { bceqz(cj, target(L)); }
  void bcnez(ConditionalFlagRegister cj, Label& L)     { bcnez(cj, target(L)); }


public:
  // Creation
  Assembler(CodeBuffer* code) : AbstractAssembler(code) {}

  // Decoding
  static address locate_operand(address inst, WhichOperand which);
  static address locate_next_instruction(address inst);
};

#endif // CPU_LOONGARCH_VM_ASSEMBLER_LOONGARCH_HPP
