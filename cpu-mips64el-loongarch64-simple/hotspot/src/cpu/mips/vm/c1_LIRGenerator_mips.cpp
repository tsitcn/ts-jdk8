/*
 * Copyright (c) 2005, 2013, Oracle and/or its affiliates. All rights reserved.
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
#include "c1/c1_Compilation.hpp"
#include "c1/c1_FrameMap.hpp"
#include "c1/c1_Instruction.hpp"
#include "c1/c1_LIRAssembler.hpp"
#include "c1/c1_LIRGenerator.hpp"
#include "c1/c1_Runtime1.hpp"
#include "c1/c1_ValueStack.hpp"
#include "ci/ciArray.hpp"
#include "ci/ciObjArrayKlass.hpp"
#include "ci/ciTypeArrayKlass.hpp"
#include "runtime/sharedRuntime.hpp"
#include "runtime/stubRoutines.hpp"
#include "vmreg_mips.inline.hpp"

#ifdef ASSERT
#define __ gen()->lir(__FILE__, __LINE__)->
#else
#define __ gen()->lir()->
#endif

// Item will be loaded into a byte register
void LIRItem::load_byte_item() {
  load_item();
  LIR_Opr res = result();

  if (!res->is_virtual() || !_gen->is_vreg_flag_set(res, LIRGenerator::byte_reg)) {
    // make sure that it is a byte register
    assert(!value()->type()->is_float() && !value()->type()->is_double(),
           "can't load floats in byte register");
    LIR_Opr reg = _gen->rlock_byte(T_BYTE);
    __ move(res, reg);

    _result = reg;
  }
}


void LIRItem::load_nonconstant() {
  LIR_Opr r = value()->operand();
  if (r->is_constant()) {
    _result = r;
  } else {
    load_item();
  }
}

//--------------------------------------------------------------
//               LIRGenerator
//--------------------------------------------------------------
LIR_Opr LIRGenerator::exceptionOopOpr()              { return FrameMap::_v0_oop_opr;     }
LIR_Opr LIRGenerator::exceptionPcOpr()               { return FrameMap::_v1_opr;     }
LIR_Opr LIRGenerator::divInOpr()                     { return FrameMap::_a0_opr; }//FIXME
LIR_Opr LIRGenerator::divOutOpr()                    { return FrameMap::_f0_opr; } //FIXME
LIR_Opr LIRGenerator::remOutOpr()                    { return FrameMap::_f0_opr; } //FIXME
LIR_Opr LIRGenerator::shiftCountOpr()                { return FrameMap::_t3_opr; } //
LIR_Opr LIRGenerator::syncTempOpr()                  { return FrameMap::_t2_opr;     }
LIR_Opr LIRGenerator::getThreadTemp()                { return  LIR_OprFact::illegalOpr;  } //


LIR_Opr LIRGenerator::result_register_for(ValueType* type, bool callee) {
  LIR_Opr opr;
  switch (type->tag()) {
    case intTag: {
      opr = FrameMap::_v0_opr;
      break;
    }
    case objectTag: {
      opr = FrameMap::_v0_oop_opr;
      break;
    }
    case longTag: {
      opr = FrameMap::_v0_v1_long_opr;
      break;
    }
    case floatTag: {
      opr = FrameMap::_f0_float_opr;
      break;
    }
    case doubleTag:  {
       opr = FrameMap::_d0_double_opr;
       break;
     }
    case addressTag:
    default: ShouldNotReachHere(); return LIR_OprFact::illegalOpr;
  }

  assert(opr->type_field() == as_OprType(as_BasicType(type)), "type mismatch");
  return opr;
}

LIR_Opr LIRGenerator::rlock_callee_saved(BasicType type) {
  LIR_Opr reg = new_register(type);
  set_vreg_flag(reg, callee_saved);
  return reg;
}


LIR_Opr LIRGenerator::rlock_byte(BasicType type) {
  return new_register(T_INT);
}


//--------- loading items into registers --------------------------------


// i486 instructions can inline constants
bool LIRGenerator::can_store_as_constant(Value v, BasicType type) const {
  if (type == T_SHORT || type == T_CHAR) {
    // there is no immediate move of word values in asembler_i486.?pp
    return false;
  }
  Constant* c = v->as_Constant();
  if (c && c->state_before() == NULL) {
    // constants of any type can be stored directly, except for
    // unloaded object constants.
    return true;
  }
  return false;
}


bool LIRGenerator::can_inline_as_constant(Value v) const {
  if (v->type()->is_constant() && v->type()->as_IntConstant() != NULL) {
    return Assembler::is_simm16(v->type()->as_IntConstant()->value());
  } else {
    return false;
  }
}


bool LIRGenerator::can_inline_as_constant(LIR_Const* c) const {
  if (c->type() == T_INT && c->as_constant() != NULL) {
    return Assembler::is_simm16(c->as_jint());
  } else {
    return false;
  }
}


LIR_Opr LIRGenerator::safepoint_poll_register() {
  return new_register(T_INT);
}


LIR_Address* LIRGenerator::generate_address(LIR_Opr base, LIR_Opr index,
                                            int shift, int disp, BasicType type) {
  assert(base->is_register(), "must be");
  if (index->is_constant()) {
    disp += index->as_constant_ptr()->as_jint() << shift;
    if (Assembler::is_simm16(disp)) {
      return new LIR_Address(base,disp, type);
  } else {

    if(disp!=0){
      LIR_Opr tmp = new_pointer_register();
      __ move(LIR_OprFact::intptrConst(disp), tmp);
      __ addu(tmp, base, tmp);
      return new LIR_Address(tmp, 0, type);
    }
    else
      return new LIR_Address(base, 0, type);
    }
  } else if( index->is_register()) {

    LIR_Opr tmpa = new_pointer_register();
    __ move(index, tmpa);
    __ shift_left(tmpa, shift, tmpa);
    __ addu(tmpa,base, tmpa);
    if (Assembler::is_simm16(disp)) {
      return new LIR_Address(tmpa, disp, type);
    } else {
      if (disp!=0) {
        LIR_Opr tmp = new_pointer_register();
        __ move(LIR_OprFact::intptrConst(disp), tmp);
        __ addu(tmp, tmpa, tmp);
        return new LIR_Address(tmp, 0, type);
      } else
        return new LIR_Address(tmpa, 0, type);
    }
  } else {
    if (Assembler::is_simm16(disp)) {
      return new LIR_Address(base,disp, type);
    } else {
    if (disp!=0) {
      LIR_Opr tmp = new_pointer_register();
      __ move(LIR_OprFact::intptrConst(disp), tmp);
      __ addu(tmp, base, tmp);
      return new LIR_Address(tmp, 0, type);
    } else
      return new LIR_Address(base, 0, type);
    }
  }
}

LIR_Address* LIRGenerator::emit_array_address(LIR_Opr array_opr, LIR_Opr index_opr,
                                              BasicType type, bool needs_card_mark) {
  int elem_size = type2aelembytes(type);
  int shift = exact_log2(elem_size);

  LIR_Opr base_opr;
  int offset = arrayOopDesc::base_offset_in_bytes(type);

  if (index_opr->is_constant()) {
    int i = index_opr->as_constant_ptr()->as_jint();
    int array_offset = i * elem_size;
    if (Assembler::is_simm16(array_offset + offset)) {
      base_opr = array_opr;
      offset = array_offset + offset;
    } else {
      base_opr = new_pointer_register();
      if (Assembler::is_simm16(array_offset)) {
        __ addu(array_opr, LIR_OprFact::intptrConst(array_offset), base_opr);
      } else {
        __ move(LIR_OprFact::intptrConst(array_offset), base_opr);
        __ addu(base_opr, array_opr, base_opr);
      }
    }
  } else {
#ifdef _LP64
    if (index_opr->type() == T_INT) {
      LIR_Opr tmp = new_register(T_LONG);
      __ convert(Bytecodes::_i2l, index_opr, tmp);
      index_opr = tmp;
    }
#endif

    base_opr = new_pointer_register();
    assert (index_opr->is_register(), "Must be register");
    if (shift > 0) {
      __ shift_left(index_opr, shift, base_opr);
      __ addu(base_opr, array_opr, base_opr);
    } else {
      __ addu(index_opr, array_opr, base_opr);
    }
  }
  if (needs_card_mark) {
    // This store will need a precise card mark, so go ahead and
    // compute the full adddres instead of computing once for the
    // store and again for the card mark.
    LIR_Opr ptr = new_pointer_register();
    __ addu(base_opr, LIR_OprFact::intptrConst(offset), ptr);
    return new LIR_Address(ptr, type);
  } else {
    return new LIR_Address(base_opr, offset, type);
  }
}


LIR_Opr LIRGenerator::load_immediate(int x, BasicType type) {
  LIR_Opr r;
  if (type == T_LONG) {
    r = LIR_OprFact::longConst(x);
  } else if (type == T_INT) {
    r = LIR_OprFact::intConst(x);
  } else {
    ShouldNotReachHere();
  }
  return r;
}

void LIRGenerator::increment_counter(address counter, BasicType type, int step) {
  LIR_Opr pointer = new_pointer_register();
  __ move(LIR_OprFact::intptrConst(counter), pointer);
  LIR_Address* addr = new LIR_Address(pointer, type);
  increment_counter(addr, step);
}

//void LIRGenerator::increment_counter(address counter, BasicType type, int step) {
//  LIR_Opr pointer = new_register(T_LONG);
//  __ move(LIR_OprFact::longConst((long)counter), pointer);
//  LIR_Opr addr = (LIR_Opr)new LIR_Address(pointer, type);
//  LIR_Opr c = LIR_OprFact::intConst((int)step);
//  __ addu(addr, c, addr);
//}

void LIRGenerator::increment_counter(LIR_Address* addr, int step) {
  LIR_Opr temp = new_register(addr->type());
  __ move(addr, temp);
  __ addu(temp, load_immediate(step, addr->type()), temp);
  __ move(temp, addr);
}

template<typename T>
void LIRGenerator::cmp_mem_int_branch(LIR_Condition condition, LIR_Opr base, int disp, int c, T tgt, CodeEmitInfo* info) {
  Unimplemented();
  __ cmp_mem_int(condition, base, disp, c, info);
}

// Explicit instantiation for all supported types.
template void LIRGenerator::cmp_mem_int_branch(LIR_Condition, LIR_Opr, int, int, Label*, CodeEmitInfo*);
template void LIRGenerator::cmp_mem_int_branch(LIR_Condition, LIR_Opr, int, int, BlockBegin*, CodeEmitInfo*);
template void LIRGenerator::cmp_mem_int_branch(LIR_Condition, LIR_Opr, int, int, CodeStub*, CodeEmitInfo*);

template<typename T>
void LIRGenerator::cmp_reg_mem_branch(LIR_Condition condition, LIR_Opr reg, LIR_Opr base, int disp, BasicType type, T tgt, CodeEmitInfo* info) {
  Unimplemented();
  __ cmp_reg_mem(condition, reg, new LIR_Address(base, disp, type), info);
}
 
// Explicit instantiation for all supported types.
template void LIRGenerator::cmp_reg_mem_branch(LIR_Condition, LIR_Opr, LIR_Opr, int, BasicType, Label*, CodeEmitInfo*);
template void LIRGenerator::cmp_reg_mem_branch(LIR_Condition, LIR_Opr, LIR_Opr, int, BasicType, BlockBegin*, CodeEmitInfo*);
template void LIRGenerator::cmp_reg_mem_branch(LIR_Condition, LIR_Opr, LIR_Opr, int, BasicType, CodeStub*, CodeEmitInfo*);

bool LIRGenerator::strength_reduce_multiply(LIR_Opr left, int c, LIR_Opr result, LIR_Opr tmp) {
  if (tmp->is_valid()) {
    if (is_power_of_2(c + 1)) {
      __ move(left, result);
      __ shift_left(result, log2_intptr(c + 1), result);
      __ subu(result, left, result);
      return true;
    } else if (is_power_of_2(c - 1)) {
      __ move(left, result);
      __ shift_left(result, log2_intptr(c - 1), result);
      __ addu(result, left, result);
      return true;
    }
  }
  return false;
}


void LIRGenerator::store_stack_parameter (LIR_Opr item, ByteSize offset_from_sp) {
  BasicType type = item->type();
  __ store(item, new LIR_Address(FrameMap::_sp_opr, in_bytes(offset_from_sp), type));
}

//----------------------------------------------------------------------
//             visitor functions
//----------------------------------------------------------------------


void LIRGenerator::do_StoreIndexed(StoreIndexed* x) {
  assert(x->is_pinned(),"");
  bool needs_range_check = true;
  bool use_length = x->length() != NULL;
  bool obj_store = x->elt_type() == T_ARRAY || x->elt_type() == T_OBJECT;
  bool needs_store_check = obj_store && (x->value()->as_Constant() == NULL ||
                                         !get_jobject_constant(x->value())->is_null_object());

  LIRItem array(x->array(), this);
  LIRItem index(x->index(), this);
  LIRItem value(x->value(), this);
  LIRItem length(this);

  array.load_item();
  index.load_nonconstant();

  if (use_length) {
    needs_range_check = x->compute_needs_range_check();
    if (needs_range_check) {
      length.set_instruction(x->length());
      length.load_item();
    }
  }
  if (needs_store_check) {
    value.load_item();
  } else {
    value.load_for_store(x->elt_type());
  }

  set_no_result(x);

  // the CodeEmitInfo must be duplicated for each different
  // LIR-instruction because spilling can occur anywhere between two
  // instructions and so the debug information must be different
  CodeEmitInfo* range_check_info = state_for(x);
  CodeEmitInfo* null_check_info = NULL;
  if (x->needs_null_check()) {
    null_check_info = new CodeEmitInfo(range_check_info);
  }

  // emit array address setup early so it schedules better
  LIR_Address* array_addr = emit_array_address(array.result(), index.result(), x->elt_type(), obj_store);

  if (GenerateRangeChecks && needs_range_check) {
    if (use_length) {
      __ branch(lir_cond_belowEqual, length.result(),index.result(),T_INT,new RangeCheckStub(range_check_info, index.result()));
    } else {
      array_range_check(array.result(), index.result(), null_check_info, range_check_info);
      // range_check also does the null check
      null_check_info = NULL;
    }
  }

  if (GenerateArrayStoreCheck && needs_store_check) {
    LIR_Opr tmp1 = new_register(objectType);
    LIR_Opr tmp2 = new_register(objectType);
    LIR_Opr tmp3 = new_register(objectType);

    CodeEmitInfo* store_check_info = new CodeEmitInfo(range_check_info);
    __ store_check(value.result(), array.result(), tmp1, tmp2, tmp3, store_check_info, x->profiled_method(), x->profiled_bci());
  }

  if (obj_store) {
    // Needs GC write barriers.
    pre_barrier(LIR_OprFact::address(array_addr), LIR_OprFact::illegalOpr, true, false, NULL);
    __ move(value.result(), array_addr, null_check_info);
    // Seems to be a precise
    post_barrier(LIR_OprFact::address(array_addr), value.result());
  } else {
    __ move(value.result(), array_addr, null_check_info);
  }
}


void LIRGenerator::do_MonitorEnter(MonitorEnter* x) {
  assert(x->is_pinned(),"");
  LIRItem obj(x->obj(), this);
  obj.load_item();

  set_no_result(x);

  // "lock" stores the address of the monitor stack slot, so this is not an oop
  LIR_Opr lock = new_register(T_INT);
  // Need a scratch register for biased locking on mips
  LIR_Opr scratch = LIR_OprFact::illegalOpr;
  if (UseBiasedLocking) {
    scratch = new_register(T_INT);
  }

  CodeEmitInfo* info_for_exception = NULL;
  if (x->needs_null_check()) {
    info_for_exception = state_for(x);
  }
  // this CodeEmitInfo must not have the xhandlers because here the
  // object is already locked (xhandlers expect object to be unlocked)
  CodeEmitInfo* info = state_for(x, x->state(), true);
  monitor_enter(obj.result(), lock, syncTempOpr(), scratch,
                        x->monitor_no(), info_for_exception, info);
}


void LIRGenerator::do_MonitorExit(MonitorExit* x) {
  assert(x->is_pinned(),"");

  LIRItem obj(x->obj(), this);
  obj.dont_load_item();

  LIR_Opr lock = new_register(T_INT);
  LIR_Opr obj_temp = new_register(T_INT);
  set_no_result(x);
  monitor_exit(obj_temp, lock, syncTempOpr(), LIR_OprFact::illegalOpr, x->monitor_no());
}


// _ineg, _lneg, _fneg, _dneg
void LIRGenerator::do_NegateOp(NegateOp* x) {
  LIRItem value(x->x(), this);
  value.set_destroys_register();
  value.load_item();
  LIR_Opr reg = rlock(x);
  __ negate(value.result(), reg);

  set_result(x, round_item(reg));
}



// for  _fadd, _fmul, _fsub, _fdiv, _frem
//      _dadd, _dmul, _dsub, _ddiv, _drem
void LIRGenerator::do_ArithmeticOp_FPU(ArithmeticOp* x) {
  switch (x->op()) {
    case Bytecodes::_fadd:
    case Bytecodes::_fmul:
    case Bytecodes::_fsub:
    case Bytecodes::_fdiv:
    case Bytecodes::_dadd:
    case Bytecodes::_dmul:
    case Bytecodes::_dsub:
    case Bytecodes::_ddiv: {
      LIRItem left(x->x(), this);
      LIRItem right(x->y(), this);
      left.load_item();
      right.load_item();
      rlock_result(x);
      arithmetic_op_fpu(x->op(), x->operand(), left.result(), right.result(), x->is_strictfp());
      break;
    }
    case Bytecodes::_frem:
    case Bytecodes::_drem: {
      address entry;
      switch (x->op()) {
        case Bytecodes::_frem:
          entry = CAST_FROM_FN_PTR(address, SharedRuntime::frem);
          break;
        case Bytecodes::_drem:
          entry = CAST_FROM_FN_PTR(address, SharedRuntime::drem);
          break;
        default:
          ShouldNotReachHere();
      }
      LIR_Opr result = call_runtime(x->x(), x->y(), entry, x->type(), NULL);
      set_result(x, result);
      break;
    }
    default:
      ShouldNotReachHere();
  }
}




// for  _ladd, _lmul, _lsub, _ldiv, _lrem
void LIRGenerator::do_ArithmeticOp_Long(ArithmeticOp* x) {
  switch (x->op()) {
  case Bytecodes::_lrem:
  case Bytecodes::_lmul:
  case Bytecodes::_ldiv: {

    if (x->op() == Bytecodes::_ldiv || x->op() == Bytecodes::_lrem) {
      LIRItem right(x->y(), this);
      right.load_item();

      CodeEmitInfo* info = state_for(x);
      LIR_Opr item = right.result();
      assert(item->is_register(), "must be");
      __ branch(lir_cond_equal,item,LIR_OprFact::longConst(0), T_LONG, new DivByZeroStub(info));
    }

    address entry;
    switch (x->op()) {
    case Bytecodes::_lrem:
      entry = CAST_FROM_FN_PTR(address, SharedRuntime::lrem);
      break; // check if dividend is 0 is done elsewhere
    case Bytecodes::_ldiv:
      entry = CAST_FROM_FN_PTR(address, SharedRuntime::ldiv);
      break; // check if dividend is 0 is done elsewhere
    case Bytecodes::_lmul:
      entry = CAST_FROM_FN_PTR(address, SharedRuntime::lmul);
      break;
    default:
      ShouldNotReachHere();
    }

    // order of arguments to runtime call is reversed.
    LIR_Opr result = call_runtime(x->y(), x->x(), entry, x->type(), NULL);
    set_result(x, result);
    break;
  }
  case Bytecodes::_ladd:
  case Bytecodes::_lsub: {
    LIRItem left(x->x(), this);
    LIRItem right(x->y(), this);
    left.load_item();
    right.load_item();
    rlock_result(x);

    arithmetic_op_long(x->op(), x->operand(), left.result(), right.result(), NULL);
    break;
  }
   default:
      ShouldNotReachHere();
  }
}




// for: _iadd, _imul, _isub, _idiv, _irem
void LIRGenerator::do_ArithmeticOp_Int(ArithmeticOp* x) {
  bool is_div_rem = x->op() == Bytecodes::_idiv || x->op() == Bytecodes::_irem;
  LIRItem left(x->x(), this);
  LIRItem right(x->y(), this);
  // missing test if instr is commutative and if we should swap
  right.load_nonconstant();
  assert(right.is_constant() || right.is_register(), "wrong state of right");
  left.load_item();
  rlock_result(x);
  if (is_div_rem) {
    CodeEmitInfo* info = state_for(x);
    LIR_Opr tmp =new_register(T_INT);
    if (x->op() == Bytecodes::_irem) {
      __ irem(left.result(), right.result(), x->operand(), tmp, info);
    } else if (x->op() == Bytecodes::_idiv) {
      __ idiv(left.result(), right.result(), x->operand(), tmp, info);
    }
  } else {
    //arithmetic_op_int(x->op(), x->operand(), left.result(),
    //right.result(), FrameMap::G1_opr);

    LIR_Opr tmp =new_register(T_INT);
    arithmetic_op_int(x->op(), x->operand(), left.result(), right.result(),
        tmp);
  }
}


void LIRGenerator::do_ArithmeticOp(ArithmeticOp* x) {
  // when an operand with use count 1 is the left operand, then it is
  // likely that no move for 2-operand-LIR-form is necessary
  if (x->is_commutative() && x->y()->as_Constant() == NULL && x->x()->use_count() > x->y()->use_count()) {
    x->swap_operands();
  }

  ValueTag tag = x->type()->tag();
  assert(x->x()->type()->tag() == tag && x->y()->type()->tag() == tag, "wrong parameters");
  switch (tag) {
    case floatTag:
    case doubleTag:  do_ArithmeticOp_FPU(x);  return;
    case longTag:    do_ArithmeticOp_Long(x); return;
    case intTag:     do_ArithmeticOp_Int(x);  return;
  }
  ShouldNotReachHere();
}


// _ishl, _lshl, _ishr, _lshr, _iushr, _lushr
void LIRGenerator::do_ShiftOp(ShiftOp* x) {
  LIRItem value(x->x(), this);
  LIRItem count(x->y(), this);

  ValueTag elemType = x->type()->tag();
  bool must_load_count = !count.is_constant() || elemType == longTag;
  if (must_load_count) {
    // count for long must be in register
    count.load_item();
  } else {
    count.dont_load_item();
  }
  value.load_item();
  LIR_Opr reg = rlock_result(x);

  shift_op(x->op(), reg, value.result(), count.result(), LIR_OprFact::illegalOpr);
}


// _iand, _land, _ior, _lor, _ixor, _lxor
void LIRGenerator::do_LogicOp(LogicOp* x) {
  // when an operand with use count 1 is the left operand, then it is
  // likely that no move for 2-operand-LIR-form is necessary
  if (x->is_commutative() && x->y()->as_Constant() == NULL && x->x()->use_count() > x->y()->use_count()) {
    x->swap_operands();
  }

  LIRItem left(x->x(), this);
  LIRItem right(x->y(), this);

  left.load_item();
  right.load_nonconstant();
  LIR_Opr reg = rlock_result(x);

  logic_op(x->op(), reg, left.result(), right.result());
}



// _lcmp, _fcmpl, _fcmpg, _dcmpl, _dcmpg
void LIRGenerator::do_CompareOp(CompareOp* x) {
  LIRItem left(x->x(), this);
  LIRItem right(x->y(), this);
  ValueTag tag = x->x()->type()->tag();
  if (tag == longTag) {
    left.set_destroys_register();
  }
  left.load_item();
  right.load_item();
  LIR_Opr reg = rlock_result(x);

  if (x->x()->type()->is_float_kind()) {
    Bytecodes::Code code = x->op();
    __ fcmp2int(left.result(), right.result(), reg, (code == Bytecodes::_fcmpl || code == Bytecodes::_dcmpl));
  } else if (x->x()->type()->tag() == longTag) {
    __ lcmp2int(left.result(), right.result(), reg);
  } else {
    Unimplemented();
  }
}

void LIRGenerator::do_CompareAndSwap(Intrinsic* x, ValueType* type) {
  assert(x->number_of_arguments() == 4, "wrong type");
  LIRItem obj   (x->argument_at(0), this);  // object
  LIRItem offset(x->argument_at(1), this);  // offset of field
  LIRItem cmp   (x->argument_at(2), this);  // value to compare with field
  LIRItem val   (x->argument_at(3), this);  // replace field with val if matches cmp

  assert(obj.type()->tag() == objectTag, "invalid type");

  //In 64bit the type can be long, sparc doesn't have this assert
  //assert(offset.type()->tag() == intTag, "invalid type");

  assert(cmp.type()->tag() == type->tag(), "invalid type");
  assert(val.type()->tag() == type->tag(), "invalid type");

  // Use temps to avoid kills
  LIR_Opr tmp1 = new_register(type);
  LIR_Opr tmp2 = new_register(type);
  LIR_Opr addr = new_pointer_register();

  // get address of field
  obj.load_item();
  offset.load_item();
  cmp.load_item();
  val.load_item();

  __ addu(obj.result(), offset.result(), addr);

  if (type == objectType) {  // Write-barrier needed for Object fields.
    pre_barrier(addr, LIR_OprFact::illegalOpr /* pre_val */,
                true /* do_load */, false /* patch */, NULL);
  }

  if (type == objectType)
    __ cas_obj(addr, cmp.result(), val.result(), tmp1, tmp2, FrameMap::_at_opr);
  else if (type == intType)
    __ cas_int(addr, cmp.result(), val.result(), tmp1, tmp2, FrameMap::_at_opr);
  else if (type == longType)
    __ cas_long(addr, cmp.result(), val.result(), tmp1, tmp2, FrameMap::_at_opr);
  else {
    ShouldNotReachHere();
  }

  LIR_Opr result = rlock_result(x);
  __ move(FrameMap::_at_opr, result);

  if (type == objectType) {  // Write-barrier needed for Object fields.
    // Precise card mark since could either be object or array
    post_barrier(addr, val.result());
  }
}


void LIRGenerator::do_MathIntrinsic(Intrinsic* x) {
  switch (x->id()) {
    case vmIntrinsics::_dabs:
    case vmIntrinsics::_dsqrt: {
      assert(x->number_of_arguments() == 1, "wrong type");
      LIRItem value(x->argument_at(0), this);
      value.load_item();
      LIR_Opr dst = rlock_result(x);

      switch (x->id()) {
        case vmIntrinsics::_dsqrt: {
          __ sqrt(value.result(), dst, LIR_OprFact::illegalOpr);
          break;
        }
        case vmIntrinsics::_dabs: {
          __ abs(value.result(), dst, LIR_OprFact::illegalOpr);
          break;
        }
      }
      break;
     }
    case vmIntrinsics::_dlog10: // fall through
    case vmIntrinsics::_dlog: // fall through
    case vmIntrinsics::_dsin: // fall through
    case vmIntrinsics::_dtan: // fall through
    case vmIntrinsics::_dcos: {
      assert(x->number_of_arguments() == 1, "wrong type");

      address runtime_entry = NULL;
      switch (x->id()) {
        case vmIntrinsics::_dsin:
          runtime_entry = CAST_FROM_FN_PTR(address, SharedRuntime::dsin);
          break;
        case vmIntrinsics::_dcos:
          runtime_entry = CAST_FROM_FN_PTR(address, SharedRuntime::dcos);
          break;
        case vmIntrinsics::_dtan:
          runtime_entry = CAST_FROM_FN_PTR(address, SharedRuntime::dtan);
          break;
        case vmIntrinsics::_dlog:
          runtime_entry = CAST_FROM_FN_PTR(address, SharedRuntime::dlog);
          break;
        case vmIntrinsics::_dlog10:
          runtime_entry = CAST_FROM_FN_PTR(address, SharedRuntime::dlog10);
          break;
        default:
          ShouldNotReachHere();
      }
      LIR_Opr result = call_runtime(x->argument_at(0), runtime_entry, x->type(), NULL);
      set_result(x, result);
      break;
    }
    case vmIntrinsics::_dexp: {
      assert(x->number_of_arguments() == 1, "wrong type");
      address runtime_entry = CAST_FROM_FN_PTR(address, SharedRuntime::dexp);
      LIR_Opr result = call_runtime(x->argument_at(0), runtime_entry, x->type(), NULL);
      set_result(x, result);
      break;
    }
    case vmIntrinsics::_dpow: {
      assert(x->number_of_arguments() == 2, "wrong type");
      address runtime_entry = CAST_FROM_FN_PTR(address, SharedRuntime::dpow);
      LIR_Opr result = call_runtime(x->argument_at(0), x->argument_at(1), runtime_entry, x->type(), NULL);
      set_result(x, result);
      break;
    }
  }
}

void LIRGenerator::do_ArrayCopy(Intrinsic* x) {
  assert(x->number_of_arguments() == 5, "wrong type");
  // Note: spill caller save before setting the item
  LIRItem src     (x->argument_at(0), this);
  LIRItem src_pos (x->argument_at(1), this);
  LIRItem dst     (x->argument_at(2), this);
  LIRItem dst_pos (x->argument_at(3), this);
  LIRItem length  (x->argument_at(4), this);
  // load all values in callee_save_registers, as this makes the
  // parameter passing to the fast case simpler
  src.load_item_force     (FrameMap::_t0_oop_opr);
  src_pos.load_item_force (FrameMap::_a0_opr);
  dst.load_item_force     (FrameMap::_a1_oop_opr);
  dst_pos.load_item_force (FrameMap::_a2_opr);
  length.load_item_force  (FrameMap::_a3_opr);

  int flags;
  ciArrayKlass* expected_type;
  arraycopy_helper(x, &flags, &expected_type);

  CodeEmitInfo* info = state_for(x, x->state());
  __ arraycopy(src.result(), src_pos.result(), dst.result(), dst_pos.result(), length.result(), rlock_callee_saved(T_INT), expected_type, flags, info);
  set_no_result(x);
}

void LIRGenerator::do_update_CRC32(Intrinsic* x) {
  Unimplemented();
}

// _i2l, _i2f, _i2d, _l2i, _l2f, _l2d, _f2i, _f2l, _f2d, _d2i, _d2l, _d2f
// _i2b, _i2c, _i2s
LIR_Opr fixed_register_for(BasicType type) {
  switch (type) {
    case T_FLOAT:  return FrameMap::_f0_float_opr;
    case T_DOUBLE: return FrameMap::_d0_double_opr;
    case T_INT:    return FrameMap::_v0_opr;
    case T_LONG:   return FrameMap::_v0_v1_long_opr;
    default:       ShouldNotReachHere(); return LIR_OprFact::illegalOpr;
  }
}


void LIRGenerator::do_Convert(Convert* x) {
  // flags that vary for the different operations and different SSE-settings
  bool fixed_input, fixed_result, round_result, needs_stub;

  switch (x->op()) {
    case Bytecodes::_i2l: // fall through
    case Bytecodes::_l2i: // fall through
    case Bytecodes::_i2b: // fall through
    case Bytecodes::_i2c: // fall through
    case Bytecodes::_i2s:
      fixed_input  = false;
      fixed_result = false;
      round_result = false;
      needs_stub   = false; break;
    case Bytecodes::_f2d:
      fixed_input  = UseSSE == 1;
      fixed_result = false;
      round_result = false;
      needs_stub   = false; break;
    case Bytecodes::_d2f:
      fixed_input  = false;
      fixed_result = UseSSE == 1;
      round_result = UseSSE < 1;
      needs_stub   = false; break;
    case Bytecodes::_i2f:
      fixed_input  = false;
      fixed_result = false;
      round_result = UseSSE < 1;
      needs_stub   = false; break;
    case Bytecodes::_i2d:
      fixed_input  = false;
      fixed_result = false;
      round_result = false;
      needs_stub   = false; break;
    case Bytecodes::_f2i:
      fixed_input  = false;
      fixed_result = false;
      round_result = false;
      needs_stub   = true;  break;
    case Bytecodes::_d2i:
      fixed_input  = false;
      fixed_result = false;
      round_result = false;
      needs_stub   = true;  break;
    case Bytecodes::_l2f:
      fixed_input  = false;
      fixed_result = UseSSE >= 1;
      round_result = UseSSE < 1;
      needs_stub   = false; break;
    case Bytecodes::_l2d:
      fixed_input  = false;
      fixed_result = UseSSE >= 2;
      round_result = UseSSE < 2;
      needs_stub   = false; break;
    case Bytecodes::_f2l:
      fixed_input  = true;
      fixed_result = true;
      round_result = false;
      needs_stub   = false; break;
    case Bytecodes::_d2l:
      fixed_input  = true;
      fixed_result = true;
      round_result = false;
      needs_stub   = false; break;
    default:
      ShouldNotReachHere();
  }

  LIRItem value(x->value(), this);
  value.load_item();
  LIR_Opr input = value.result();
  LIR_Opr result = rlock(x);

  // arguments of lir_convert
  LIR_Opr conv_input = input;
  LIR_Opr conv_result = result;
  ConversionStub* stub = NULL;

  if (fixed_input) {
    conv_input = fixed_register_for(input->type());
    __ move(input, conv_input);
  }

  assert(fixed_result == false || round_result == false, "cannot set both");
  if (fixed_result) {
    conv_result = fixed_register_for(result->type());
  } else if (round_result) {
    result = new_register(result->type());
    set_vreg_flag(result, must_start_in_memory);
  }

  if (needs_stub) {
    stub = new ConversionStub(x->op(), conv_input, conv_result);
  }

  __ convert(x->op(), conv_input, conv_result, stub);

  if (result != conv_result) {
    __ move(conv_result, result);
  }

  assert(result->is_virtual(), "result must be virtual register");
  set_result(x, result);
}

void LIRGenerator::do_NewInstance(NewInstance* x) {
  const LIR_Opr reg = result_register_for(x->type());
#ifndef PRODUCT
  if (PrintNotLoaded && !x->klass()->is_loaded()) {
    tty->print_cr("   ###class not loaded at new bci %d", x->printable_bci());
  }
#endif
  CodeEmitInfo* info = state_for(x, x->state());
//  LIR_Opr tmp1 = new_register(T_INT);
//  LIR_Opr tmp2 = new_register(T_INT);
//  LIR_Opr tmp3 = new_register(T_INT);
//  LIR_Opr tmp4 = new_register(T_INT);
#ifndef _LP64
  LIR_Opr klass_reg = FrameMap::_t4_metadata_opr;
#else
  LIR_Opr klass_reg = FrameMap::_a4_metadata_opr;
#endif
  new_instance(reg,
               x->klass(),
               x->is_unresolved(),
               FrameMap::_t0_oop_opr,
               FrameMap::_t1_oop_opr,
               FrameMap::_t2_oop_opr,
               FrameMap::_t3_oop_opr,
#ifndef _LP64
               FrameMap::_t5_oop_opr,
               FrameMap::_t6_oop_opr,
#else
               FrameMap::_a5_oop_opr,
               FrameMap::_a6_oop_opr,
#endif
               klass_reg,
               info);
  LIR_Opr result = rlock_result(x);
  __ move(reg, result);
}


void LIRGenerator::do_NewTypeArray(NewTypeArray* x) {
  CodeEmitInfo* info = state_for(x, x->state());

  LIRItem length(x->length(), this);
  length.load_item_force(FrameMap::_t2_opr);

  LIR_Opr reg = result_register_for(x->type());
  LIR_Opr tmp1 = FrameMap::_t0_oop_opr;
  LIR_Opr tmp2 = FrameMap::_t1_oop_opr;
  LIR_Opr tmp3 = FrameMap::_t3_oop_opr;
#ifndef _LP64
  LIR_Opr tmp4 = FrameMap::_t5_oop_opr;
  LIR_Opr tmp5 = FrameMap::_t6_oop_opr;
  LIR_Opr klass_reg = FrameMap::_t4_oop_opr;
#else
  LIR_Opr tmp4 = FrameMap::_a5_oop_opr;
  LIR_Opr tmp5 = FrameMap::_a6_oop_opr;
  LIR_Opr klass_reg = FrameMap::_a4_metadata_opr;
#endif
  LIR_Opr len = length.result();
  BasicType elem_type = x->elt_type();

  __ metadata2reg(ciTypeArrayKlass::make(elem_type)->constant_encoding(), klass_reg);

  CodeStub* slow_path = new NewTypeArrayStub(klass_reg, len, reg, info);
  __ allocate_array(reg, len, tmp1, tmp2, tmp3, tmp4,tmp5, elem_type, klass_reg, slow_path);

  LIR_Opr result = rlock_result(x);
  __ move(reg, result);
}



void LIRGenerator::do_NewObjectArray(NewObjectArray* x) {
  LIRItem length(x->length(), this);
  // in case of patching (i.e., object class is not yet loaded), we
  // need to reexecute the instruction
  // and therefore provide the state before the parameters have been consumed
  CodeEmitInfo* patching_info = NULL;
  if (!x->klass()->is_loaded() || PatchALot) {
    patching_info = state_for(x, x->state_before());
  }

  CodeEmitInfo* info = state_for(x, x->state());

  const LIR_Opr reg = result_register_for(x->type());
  LIR_Opr tmp1 = FrameMap::_t0_oop_opr;
  LIR_Opr tmp2 = FrameMap::_t1_oop_opr;
  LIR_Opr tmp3 = FrameMap::_t3_oop_opr;
#ifndef _LP64
  LIR_Opr tmp4 = FrameMap::_t5_oop_opr;
  LIR_Opr tmp5 = FrameMap::_t6_oop_opr;
  LIR_Opr klass_reg = FrameMap::_t4_oop_opr;
#else
  LIR_Opr tmp4 = FrameMap::_a5_oop_opr;
  LIR_Opr tmp5 = FrameMap::_a6_oop_opr;
  LIR_Opr klass_reg = FrameMap::_a4_metadata_opr;
#endif

  length.load_item_force(FrameMap::_t2_opr);
  LIR_Opr len = length.result();

  CodeStub* slow_path = new NewObjectArrayStub(klass_reg, len, reg, info);
  ciKlass* obj = (ciKlass*) ciObjArrayKlass::make(x->klass());
  if (obj == ciEnv::unloaded_ciobjarrayklass()) {
    BAILOUT("encountered unloaded_ciobjarrayklass due to out of memory error");
  }
  klass2reg_with_patching(klass_reg, obj, patching_info);
  __ allocate_array(reg, len, tmp1, tmp2, tmp3, tmp4, tmp5, T_OBJECT, klass_reg, slow_path);

  LIR_Opr result = rlock_result(x);
  __ move(reg, result);
}


void LIRGenerator::do_NewMultiArray(NewMultiArray* x) {
  Values* dims = x->dims();
  int i = dims->length();
  LIRItemList* items = new LIRItemList(dims->length(), NULL);
  while (i-- > 0) {
    LIRItem* size = new LIRItem(dims->at(i), this);
    items->at_put(i, size);
  }

  // need to get the info before, as the items may become invalid through item_free
  CodeEmitInfo* patching_info = NULL;
  if (!x->klass()->is_loaded() || PatchALot) {
    patching_info = state_for(x, x->state_before());
    // cannot re-use same xhandlers for multiple CodeEmitInfos, so
    // clone all handlers.
    x->set_exception_handlers(new XHandlers(x->exception_handlers()));
  }

  CodeEmitInfo* info = state_for(x, x->state());

  i = dims->length();
  while (i-- > 0) {
    LIRItem* size = items->at(i);
    size->load_nonconstant();
    store_stack_parameter(size->result(), in_ByteSize(i*4));
  }

  LIR_Opr klass_reg = FrameMap::_v0_metadata_opr;
  klass2reg_with_patching(klass_reg, x->klass(), patching_info);

  //  LIR_Opr rank = FrameMap::ebx_opr;
  LIR_Opr rank = FrameMap::_t2_opr;
  __ move(LIR_OprFact::intConst(x->rank()), rank);
  //  LIR_Opr varargs = FrameMap::ecx_opr;
  LIR_Opr varargs = FrameMap::_t0_opr;
  __ move(FrameMap::_sp_opr, varargs);
  LIR_OprList* args = new LIR_OprList(3);
  args->append(klass_reg);
  args->append(rank);
  args->append(varargs);
  LIR_Opr reg = result_register_for(x->type());
  __ call_runtime(Runtime1::entry_for(Runtime1::new_multi_array_id),
      LIR_OprFact::illegalOpr,
      reg, args, info);

  LIR_Opr result = rlock_result(x);
  __ move(reg, result);
}

void LIRGenerator::do_BlockBegin(BlockBegin* x) {
  // nothing to do for now
}


void LIRGenerator::do_CheckCast(CheckCast* x) {
  LIRItem obj(x->obj(), this);

  CodeEmitInfo* patching_info = NULL;
  if (!x->klass()->is_loaded() || (PatchALot && !x->is_incompatible_class_change_check())) {
    // must do this before locking the destination register as an oop register,
    // and before the obj is loaded (the latter is for deoptimization)
    patching_info = state_for(x, x->state_before());
  }
  obj.load_item();

  // info for exceptions
  CodeEmitInfo* info_for_exception = state_for(x);

  CodeStub* stub;
  if (x->is_incompatible_class_change_check()) {
    assert(patching_info == NULL, "can't patch this");
    stub = new SimpleExceptionStub(Runtime1::throw_incompatible_class_change_error_id, LIR_OprFact::illegalOpr, info_for_exception);
  } else {
    stub = new SimpleExceptionStub(Runtime1::throw_class_cast_exception_id, obj.result(), info_for_exception);
  }
  LIR_Opr reg = rlock_result(x);
  LIR_Opr tmp3 = LIR_OprFact::illegalOpr;
  if (!x->klass()->is_loaded() || UseCompressedClassPointers) {
    tmp3 = new_register(objectType);
  }
  __ checkcast(reg, obj.result(), x->klass(),
               new_register(objectType), new_register(objectType), tmp3,
               x->direct_compare(), info_for_exception, patching_info, stub,
               x->profiled_method(), x->profiled_bci());
}


void LIRGenerator::do_InstanceOf(InstanceOf* x) {
  LIRItem obj(x->obj(), this);

  // result and test object may not be in same register
  LIR_Opr reg = rlock_result(x);
  CodeEmitInfo* patching_info = NULL;
  if ((!x->klass()->is_loaded() || PatchALot)) {
    // must do this before locking the destination register as an oop register
    patching_info = state_for(x, x->state_before());
  }
  obj.load_item();
  LIR_Opr tmp = new_register(objectType);
  LIR_Opr tmp3 = LIR_OprFact::illegalOpr;
  if (!x->klass()->is_loaded() || UseCompressedClassPointers) {
      tmp3 = new_register(objectType);
  }

  __ instanceof(reg, obj.result(), x->klass(),
                tmp, new_register(objectType), tmp3,
                x->direct_compare(), patching_info, x->profiled_method(), x->profiled_bci());
}


void LIRGenerator::do_If(If* x) {
  assert(x->number_of_sux() == 2, "inconsistency");
  ValueTag tag = x->x()->type()->tag();
  bool is_safepoint = x->is_safepoint();

  If::Condition cond = x->cond();

  LIRItem xitem(x->x(), this);
  LIRItem yitem(x->y(), this);
  LIRItem* xin = &xitem;
  LIRItem* yin = &yitem;

  if (tag == longTag) {
    // for longs, only conditions "eql", "neq", "lss", "geq" are valid;
    // mirror for other conditions
    if (cond == If::gtr || cond == If::leq) {
      cond = Instruction::mirror(cond);
      xin = &yitem;
      yin = &xitem;
    }
    xin->set_destroys_register();
  }
  xin->load_item();
  if (tag == longTag && yin->is_constant() && yin->get_jlong_constant() == 0 && (cond == If::eql || cond == If::neq)) {
    // inline long zero
    yin->dont_load_item();
  } else if (tag == longTag || tag == floatTag || tag == doubleTag) {
    // longs cannot handle constants at right side
    yin->load_item();
  } else {
    yin->dont_load_item();
  }

  // add safepoint before generating condition code so it can be recomputed
  if (x->is_safepoint()) {
    // increment backedge counter if needed
    increment_backedge_counter(state_for(x, x->state_before()), x->profiled_bci());
    __ safepoint(safepoint_poll_register(), state_for(x, x->state_before()));
  }
  set_no_result(x);

  LIR_Opr left = xin->result();
  LIR_Opr right = yin->result();
  profile_branch(x, cond, left, right);
  move_to_phi(x->state());
  if (x->x()->type()->is_float_kind()) {
    __ branch(lir_cond(cond), left, right, right->type(), x->tsux(), x->usux());
  } else {
    __ branch(lir_cond(cond), left, right, right->type(), x->tsux());
  }
  assert(x->default_sux() == x->fsux(), "wrong destination above");
  __ jump(x->default_sux());
}


LIR_Opr LIRGenerator::getThreadPointer() {
  //FIXME, does as_pointer need to be implemented? or 64bit can use one register.
  //return FrameMap::as_pointer_opr(r15_thread);
  LIR_Opr result = new_pointer_register();
  __ get_thread(result);
  return result;
}

void LIRGenerator::trace_block_entry(BlockBegin* block) {
  store_stack_parameter(LIR_OprFact::intConst(block->block_id()), in_ByteSize(0));
  LIR_OprList* args = new LIR_OprList();
  address func = CAST_FROM_FN_PTR(address, Runtime1::trace_block_entry);
  __ call_runtime_leaf(func, LIR_OprFact::illegalOpr, LIR_OprFact::illegalOpr, args);
}


void LIRGenerator::volatile_field_store(LIR_Opr value, LIR_Address* address,
                                        CodeEmitInfo* info) {
  if (address->type() == T_LONG) {
    __ volatile_store_mem_reg(value, address, info);
  } else {
    __ store(value, address, info);
  }
}

void LIRGenerator::volatile_field_load(LIR_Address* address, LIR_Opr result,
      CodeEmitInfo* info) {

  if (address->type() == T_LONG) {
    __ volatile_load_mem_reg(address, result, info);
  } else {
    __ load(address, result, info);
  }
}

void LIRGenerator::get_Object_unsafe(LIR_Opr dst, LIR_Opr src, LIR_Opr offset,
                                     BasicType type, bool is_volatile) {
  __ addu(src, offset, FrameMap::_t9_opr);
  if (is_volatile && type == T_LONG) {
    LIR_Address* addr = new LIR_Address(FrameMap::_t9_opr, 0, T_DOUBLE);
    LIR_Opr tmp = new_register(T_DOUBLE);
    __ load(addr, tmp);
    LIR_Opr spill = new_register(T_LONG);
    set_vreg_flag(spill, must_start_in_memory);
    __ move(tmp, spill);
    __ move(spill, dst);
  } else {
    LIR_Address* addr = new LIR_Address(FrameMap::_t9_opr, 0, type);
    __ load(addr, dst);
  }
}


void LIRGenerator::put_Object_unsafe(LIR_Opr src, LIR_Opr offset, LIR_Opr data,
                                     BasicType type, bool is_volatile) {
  __ addu(src, offset, FrameMap::_t9_opr);
  if (is_volatile && type == T_LONG) {
    LIR_Address* addr = new LIR_Address(FrameMap::_t9_opr, 0, T_DOUBLE);
    LIR_Opr tmp = new_register(T_DOUBLE);
    LIR_Opr spill = new_register(T_DOUBLE);
    set_vreg_flag(spill, must_start_in_memory);
    __ move(data, spill);
    __ move(spill, tmp);
    __ move(tmp, addr);
  } else {
    LIR_Address* addr = new LIR_Address(FrameMap::_t9_opr, 0, type);
    bool is_obj = (type == T_ARRAY || type == T_OBJECT);
    if (is_obj) {
      // Do the pre-write barrier, if any.
      pre_barrier(LIR_OprFact::address(addr), LIR_OprFact::illegalOpr/* pre_val */,
                                    true/* do_load */,false /*patch*/, NULL);
      __ move(data, addr);
      assert(src->is_register(), "must be register");
      // Seems to be a precise address
      post_barrier(LIR_OprFact::address(addr), data);
    } else {
      __ move(data, addr);
    }
  }
}

void LIRGenerator::do_UnsafeGetAndSetObject(UnsafeGetAndSetObject* x) {
  BasicType type = x->basic_type();
  LIRItem src(x->object(), this);
  LIRItem off(x->offset(), this);
  LIRItem value(x->value(), this);

  src.load_item();
  value.load_item();
  off.load_nonconstant();

  LIR_Opr dst = rlock_result(x, type);
  LIR_Opr data = value.result();
  bool is_obj = (type == T_ARRAY || type == T_OBJECT);
  LIR_Opr offset = off.result();

  assert (type == T_INT || (!x->is_add() && is_obj) LP64_ONLY( || type == T_LONG ), "unexpected type");
  LIR_Address* addr;
  if (offset->is_constant()) {
#ifdef _LP64
    jlong c = offset->as_jlong();
    if ((jlong)((jint)c) == c) {
      addr = new LIR_Address(src.result(), (jint)c, type);
    } else {
      LIR_Opr tmp = new_register(T_LONG);
      __ move(offset, tmp);
      addr = new LIR_Address(src.result(), tmp, type);
    }
#else
    addr = new LIR_Address(src.result(), offset->as_jint(), type);
#endif
  } else {
    addr = new LIR_Address(src.result(), offset, type);
  }

  if (data != dst) {
    __ move(data, dst);
    data = dst;
  }
  if (x->is_add()) {
    __ xadd(LIR_OprFact::address(addr), data, dst, LIR_OprFact::illegalOpr);
  } else {
    if (is_obj) {
      // Do the pre-write barrier, if any.
      pre_barrier(LIR_OprFact::address(addr), LIR_OprFact::illegalOpr /* pre_val */,
                  true /* do_load */, false /* patch */, NULL);
    }
    __ xchg(LIR_OprFact::address(addr), data, dst, LIR_OprFact::illegalOpr);
    if (is_obj) {
      // Seems to be a precise address
      post_barrier(LIR_OprFact::address(addr), data);
    }
  }
}
