#include <codegen/FunctionBuilder.h>
#include <bind/DataType.h>
#include <bind/FunctionType.h>
#include <bind/PointerType.h>
#include <bind/Function.h>
#include <bind/ValuePointer.h>
#include <bind/Registry.h>
#include <utils/Exception.h>

namespace codegen {
    label_id FunctionBuilder::label(bool doAddToCode) {
        label_id id = m_nextLabel++;
        
        if (doAddToCode) {
            Instruction i(OpCode::label);
            i.operands[0].reset(labelVal(id));
            add(i);
        }

        return id;
    }

    InstructionRef FunctionBuilder::label(label_id label) {
        if (m_validationEnabled) {
            for (u32 i = 0;i < m_code.size();i++) {
                if (m_code[i].op == OpCode::label && m_code[i].operands[0].m_imm.u == label) {
                    throw Exception("FunctionBuilder::label - specified label id should only be added to the code one time");
                }
            }
        }

        Instruction i(OpCode::label);
        i.operands[0].reset(labelVal(label));
        return add(i);
    }

    stack_id FunctionBuilder::stackAlloc(u32 size) {
        stack_id id = m_nextAlloc++;
        stackAlloc(size, id);
        return id;
    }

    InstructionRef FunctionBuilder::stackAlloc(u32 size, stack_id alloc) {
        Instruction i(OpCode::stack_alloc);
        i.operands[0].reset(val(size));
        i.operands[1].reset(val(alloc));
        return add(i);
    }

    InstructionRef FunctionBuilder::stackPtr(const Value& ptrDest, stack_id alloc) {
        if (m_validationEnabled) {
            if (!ptrDest.isReg()) throw Exception("FunctionBuilder::stackPtr - destination value should refer to a register");
            if (!ptrDest.m_type->getInfo().is_pointer) {
                throw Exception("FunctionBuilder::stackPtr - destination value should be a pointer type");
            }
        }

        Instruction i(OpCode::stack_ptr);
        i.operands[0].reset(ptrDest);
        i.operands[1].reset(val(alloc));
        return add(i);
    }

    InstructionRef FunctionBuilder::stackFree(stack_id alloc) {
        Instruction i(OpCode::stack_free);
        i.operands[0].reset(val(alloc));
        return add(i);
    }

    InstructionRef FunctionBuilder::valuePtr(const Value& reg, symbol_id id) {
        Value ptr = val(id);

        if (m_validationEnabled) {
            if (!reg.isReg()) throw Exception("FunctionBuilder::valuePtr - destination value should refer to a register");
            if (!ptr.m_type->isEqualTo((DataType*)reg.m_type->getPointerType())) {
                throw Exception("FunctionBuilder::valuePtr - type of destination value is not a pointer to the symbol's type");
            }
        }

        Instruction i(OpCode::value_ptr);
        i.operands[0].reset(reg);
        i.operands[1].reset(ptr);
        return add(i);
    }

    InstructionRef FunctionBuilder::valuePtr(const Value& reg, ValuePointer* sym) {
        if (m_validationEnabled) {
            if (!reg.isReg()) throw Exception("FunctionBuilder::valuePtr - destination value should refer to a register");
            if (!sym->getType()->isEqualTo((DataType*)reg.m_type->getPointerType())) {
                throw Exception("FunctionBuilder::valuePtr - type of destination value is not a pointer to the symbol's type");
            }
        }

        Instruction i(OpCode::value_ptr);
        i.operands[0].reset(reg);
        i.operands[1].reset(val(sym->getSymbolId()));
        return add(i);
    }

    InstructionRef FunctionBuilder::thisPtr(const Value& reg) {
        if (m_validationEnabled) {
            if (m_code.size() > 0) {
                throw Exception("FunctionBuilder::thisPtr - this_ptr instruction should be the first emitted instruction");
            }

            if (!reg.isReg()) {
                throw Exception("FunctionBuilder::thisPtr - destination value should refer to a register");
            }

            
            FunctionType* sig = m_function->getSignature();
            DataType* thisTp = sig->getThisType();

            if (!thisTp) {
                throw Exception("FunctionBuilder::thisPtr - function does not have a 'this' pointer");
            }

            if (!thisTp->isEqualTo(reg.m_type)) {
                throw Exception("FunctionBuilder::thisPtr - destination value should have the same type as the function's 'this' pointer");
            }
        }

        Instruction i(OpCode::this_ptr);
        i.operands[0].reset(reg);
        return add(i);
    }

    InstructionRef FunctionBuilder::retPtr(const Value& reg) {
        if (m_validationEnabled) {
            DataType* retTp = m_function->getSignature()->getReturnType();
            auto info = retTp->getInfo();
            if (info.size == 0 || info.is_primitive) {
                throw Exception("FunctionBuilder::retPtr - function does not return on the stack, there is no return pointer");
            }

            if (!reg.m_type->isEqualTo(retTp->getPointerType())) {
                throw Exception("FunctionBuilder::retPtr - destination value should have a type that points to the function's return type");
            }
        }
        
        Instruction i(OpCode::ret_ptr);
        i.operands[0].reset(reg);
        return add(i);
    }

    InstructionRef FunctionBuilder::argument(const Value& reg, u32 argIndex) {
        if (m_validationEnabled) {
            for (u32 i = 0;i < m_code.size();i++) {
                if (m_code[i].op == OpCode::this_ptr) continue;
                if (m_code[i].op != OpCode::argument) {
                    throw Exception("FunctionBuilder::argument - argument instruction should be emitted before any other type of instruction (excluding this_ptr)");
                }

                if (u32(m_code[i].operands[1].getImm()) == argIndex) {
                    throw Exception("FunctionBuilder::argument - specified argument index has already been used with the argument instruction");
                }
            }

            FunctionType* sig = m_function->getSignature();
            auto args = sig->getArgs();

            if (argIndex >= args.size()) {
                throw Exception("FunctionBuilder::argument - specified argument index is greater than this function's argument count");
            }

            if (!args[argIndex].type->isEqualTo(reg.m_type)) {
                throw Exception("FunctionBuilder::argument - destination value should have the same type as the specified argument");
            }
        }

        Instruction i(OpCode::argument);
        i.operands[0].reset(reg);
        i.operands[1].reset(val(argIndex));
        return add(i);
    }

    InstructionRef FunctionBuilder::reserve(const Value& reg) {
        if (m_validationEnabled) {
            if (!reg.isReg()) throw Exception("FunctionBuilder::reserve - destination value should refer to a register");
        }

        Instruction i(OpCode::reserve);
        i.operands[0].reset(reg);
        return add(i);
    }

    InstructionRef FunctionBuilder::resolve(const Value& reg, const Value& assignTo) {
        if (m_validationEnabled) {
            if (!reg.isReg()) throw Exception("FunctionBuilder::resolve - destination value should refer to a register");
            if (!reg.m_type->isEqualTo(assignTo.m_type)) {
                throw Exception("FunctionBuilder::resolve - destination value has different type than the value being assigned to it");
            }
        }

        Instruction i(OpCode::resolve);
        i.operands[0].reset(reg);
        i.operands[1].reset(assignTo);
        return add(i);
    }

    InstructionRef FunctionBuilder::load(const Value& dest, const Value& src, u32 offset) {
        if (m_validationEnabled && !dest.m_type->isEqualTo(((PointerType*)src.m_type)->getDestinationType())) {
            throw Exception("FunctionBuilder::load - destination value has different type from the value pointed to by the source");
        }

        Instruction i(OpCode::load);
        i.operands[0].reset(dest);
        i.operands[1].reset(src);
        i.operands[2].reset(val(offset));
        return add(i);
    }

    InstructionRef FunctionBuilder::store(const Value& src, const Value& dest, u32 offset) {
        if (m_validationEnabled && !dest.m_type->isEqualTo(((PointerType*)src.m_type)->getDestinationType())) {
            throw Exception("FunctionBuilder::load - destination value has different type from the value pointed to by the source");
        }

        Instruction i(OpCode::store);
        i.operands[0].reset(src);
        i.operands[1].reset(dest);
        i.operands[2].reset(val(offset));
        return add(i);
    }

    InstructionRef FunctionBuilder::jump(label_id _label) {
        if (m_validationEnabled && _label >= m_nextLabel) throw Exception("FunctionBuilder::jump - Invalid label ID");

        Instruction i(OpCode::jump);
        i.operands[0].reset(labelVal(_label));
        return add(i);
    }

    InstructionRef FunctionBuilder::cvt(const Value& dest, const Value& src) {
        if (m_validationEnabled) {
            if (!dest.isReg()) throw Exception("FunctionBuilder::cvt - destination value should refer to a register");
            if (!dest.m_type->getInfo().is_primitive || !src.m_type->getInfo().is_primitive) {
                throw Exception("FunctionBuilder::cvt - Both dest and src values should be primitive types");
            }
        }

        Instruction i(OpCode::cvt);
        i.operands[0].reset(dest);
        i.operands[1].reset(src);
        i.operands[2].reset(val(dest.m_type->getSymbolId()));
        return add(i);
    }

    InstructionRef FunctionBuilder::param(const Value& val) {
        Instruction i(OpCode::param);
        i.operands[0].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::call(FunctionBuilder* func, const Value& retDest, const Value& selfPtr) {
        return call(func->getFunction(), retDest, selfPtr);
    }

    InstructionRef FunctionBuilder::call(Function* func, const Value& retDest, const Value& selfPtr) {
        if (m_validationEnabled) {
            FunctionType* sig = func->getSignature();
            if (sig->getReturnType()->getInfo().size == 0 && !retDest.isEmpty()) {
                throw Exception("FunctionBuilder::call - Provided function returns void but return destination was specified");
            } else if (sig->getReturnType()->getInfo().size > 0 && retDest.isEmpty()) {
                throw Exception("FunctionBuilder::call - Provided function returns non-void but return destination was not specified");
            }

            auto args = sig->getArgs();
            i32 paramIdx = args.size() - 1;
            u32 foundCount = 0;
            for (i32 i = m_code.size() - 1;i >= 0 && paramIdx >= 0;i--) {
                if (m_code[i].op == OpCode::call) break;
                if (m_code[i].op == OpCode::param) {
                    DataType* expect = args[paramIdx].type;
                    DataType* provided = m_code[i].operands[0].m_type;

                    if (!provided->isEqualTo(expect)) {
                        throw Exception(String::Format(
                            "FunctionBuilder::call - Type of argument %d is not equal to what the function expects for that parameter",
                            paramIdx
                        ));
                    }

                    paramIdx--;
                    foundCount++;
                }
            }

            if (foundCount != args.size()) {
                throw Exception(String::Format(
                    "FunctionBuilder::call - Function expected %lu arguments but %lu were provided",
                    args.size(),
                    foundCount
                ));
            }
        }

        Instruction i(OpCode::call);
        i.operands[0].reset(val(func));
        i.operands[1].reset(retDest);
        i.operands[2].reset(selfPtr);
        return add(i);
    }

    InstructionRef FunctionBuilder::call(const Value& func, const Value& retDest, const Value& selfPtr) {
        if (func.m_isImm) return call((Function*)func.m_imm.p, retDest, selfPtr);

        if (m_validationEnabled) {
            FunctionType* sig = (FunctionType*)func.m_type;
            if (sig->getReturnType()->getInfo().size == 0 && !retDest.isEmpty()) {
                throw Exception("FunctionBuilder::call - Provided function returns void but return destination was specified");
            } else if (sig->getReturnType()->getInfo().size > 0 && retDest.isEmpty()) {
                throw Exception("FunctionBuilder::call - Provided function returns non-void but return destination was not specified");
            }

            auto args = sig->getArgs();
            i32 paramIdx = args.size() - 1;
            u32 foundCount = 0;
            for (i32 i = m_code.size() - 1;i >= 0 && paramIdx >= 0;i--) {
                if (m_code[i].op == OpCode::call) break;
                if (m_code[i].op == OpCode::param) {
                    DataType* expect = args[paramIdx].type;
                    DataType* provided = m_code[i].operands[0].m_type;

                    if (!provided->isEqualTo(expect)) {
                        throw Exception(String::Format(
                            "FunctionBuilder::call - Type of argument %d is not equal to what the function expects for that parameter",
                            paramIdx
                        ));
                    }

                    paramIdx--;
                    foundCount++;
                }
            }

            if (foundCount != args.size()) {
                throw Exception(String::Format(
                    "FunctionBuilder::call - Function expected %lu arguments but %lu were provided",
                    args.size(),
                    foundCount
                ));
            }
        }

        Instruction i(OpCode::call);
        i.operands[0].reset(func);
        i.operands[1].reset(retDest);
        i.operands[2].reset(selfPtr);
        return add(i);
    }

    InstructionRef FunctionBuilder::ret(const Value& val) {
        if (m_validationEnabled) {
            DataType* retTp = m_function->getSignature()->getReturnType();
            auto retInfo = retTp->getInfo();

            if (retInfo.size == 0) {
                if (!val.isEmpty()) {
                    throw Exception("FunctionBuilder::ret - function does not return a value, but a return value was provided");
                }
            } else {
                if (!retInfo.is_primitive) {
                    if (!val.isEmpty()) {
                        throw Exception("FunctionBuilder::ret - function returns on the stack, but a return value was provided");
                    }
                } else {
                    if (val.isEmpty()) {
                        throw Exception("FunctionBuilder::ret - function returns a primitive value, but a return value was not provided");
                    } else if (!retTp->isEqualTo(val.m_type)) {
                        throw Exception("FunctionBuilder::ret - provided value should have a type that is equal to the function's return type");
                    }
                }
            }
        }

        Instruction i(OpCode::ret);
        i.operands[0].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::branch(const Value& cond, label_id destOnFalse) {
        if (m_validationEnabled && destOnFalse >= m_nextLabel) throw Exception("FunctionBuilder::branch - Invalid label ID");

        Instruction i(OpCode::branch);
        i.operands[0].reset(cond);
        i.operands[1].reset(labelVal(destOnFalse));
        return add(i);
    }

    InstructionRef FunctionBuilder::_not(const Value& result, const Value& val) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::_not - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::_not - result value should have boolean type");
            }
        }

        Instruction i(OpCode::_not);
        i.operands[0].reset(result);
        i.operands[1].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::inv(const Value& result, const Value& val) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::inv - result value should refer to a register");
            if (!result.m_type->getInfo().is_primitive) {
                throw Exception("FunctionBuilder::inv - result value should have a primitive type");
            }

            if (!val.m_type->getInfo().is_primitive) {
                throw Exception("FunctionBuilder::inv - inverted value should have a primitive type");
            }
        }

        Instruction i(OpCode::inv);
        i.operands[0].reset(result);
        i.operands[1].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::shl(const Value& result, const Value& val, const Value& bits) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::shl - result value should refer to a register");
            if (!result.m_type->getInfo().is_integral) {
                throw Exception("FunctionBuilder::shl - result value should have an integral type");
            }

            if (!val.m_type->getInfo().is_integral) {
                throw Exception("FunctionBuilder::shl - value to shift should have an integral type");
            }

            if (!bits.m_type->getInfo().is_integral) {
                throw Exception("FunctionBuilder::shl - shift amount should have an integral type");
            }
        }

        Instruction i(OpCode::shl);
        i.operands[0].reset(result);
        i.operands[1].reset(val);
        i.operands[2].reset(bits);
        return add(i);
    }

    InstructionRef FunctionBuilder::shr(const Value& result, const Value& val, const Value& bits) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::shr - result value should refer to a register");
            if (!result.m_type->getInfo().is_integral) {
                throw Exception("FunctionBuilder::shr - result value should have an integral type");
            }

            if (!val.m_type->getInfo().is_integral) {
                throw Exception("FunctionBuilder::shr - value to shift should have an integral type");
            }

            if (!bits.m_type->getInfo().is_integral) {
                throw Exception("FunctionBuilder::shr - shift amount should have an integral type");
            }
        }

        Instruction i(OpCode::shr);
        i.operands[0].reset(result);
        i.operands[1].reset(val);
        i.operands[2].reset(bits);
        return add(i);
    }

    InstructionRef FunctionBuilder::land(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::land - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::land - result value should have a boolean type");
            }
        }

        Instruction i(OpCode::land);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::band(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::band - result value should refer to a register");
            if (!result.m_type->getInfo().is_integral) {
                throw Exception("FunctionBuilder::band - result value should have an integral type");
            }

            if (!a.m_type->getInfo().is_integral) {
                throw Exception("FunctionBuilder::band - left-hand value should have an integral type");
            }

            if (!b.m_type->getInfo().is_integral) {
                throw Exception("FunctionBuilder::band - right-hand value should have an integral type");
            }
        }

        Instruction i(OpCode::band);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::lor(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::lor - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::land - result value should have a boolean type");
            }
        }

        Instruction i(OpCode::lor);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::bor(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::bor - result value should refer to a register");
            if (!result.m_type->getInfo().is_integral) {
                throw Exception("FunctionBuilder::bor - result value should have an integral type");
            }

            if (!a.m_type->getInfo().is_integral) {
                throw Exception("FunctionBuilder::bor - left-hand value should have an integral type");
            }

            if (!b.m_type->getInfo().is_integral) {
                throw Exception("FunctionBuilder::bor - right-hand value should have an integral type");
            }
        }

        Instruction i(OpCode::bor);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::_xor(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::_xor - result value should refer to a register");
            if (!result.m_type->getInfo().is_integral) {
                throw Exception("FunctionBuilder::xor - result value should have an integral type");
            }

            if (!a.m_type->getInfo().is_integral) {
                throw Exception("FunctionBuilder::xor - left-hand value should have an integral type");
            }

            if (!b.m_type->getInfo().is_integral) {
                throw Exception("FunctionBuilder::xor - right-hand value should have an integral type");
            }
        }

        Instruction i(OpCode::_xor);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::assign(const Value& dest, const Value& src) {
        if (m_validationEnabled) {
            if (!dest.isReg()) throw Exception("FunctionBuilder::assign - destination value should refer to a register");
            if (!dest.m_type->isEqualTo(src.m_type)) {
                throw Exception("FunctionBuilder::assign - destination value has different type than the value being assigned to it");
            }
        }

        Instruction i(OpCode::assign);
        i.operands[0].reset(dest);
        i.operands[1].reset(src);
        return add(i);
    }

    InstructionRef FunctionBuilder::vset(const Value& dest, const Value& src) {
        if (m_validationEnabled) {
            if (!dest.m_type->getInfo().is_pointer) {
                throw Exception("FunctionBuilder::vset - destination value should have a pointer type");
            }
            
            if (src.m_type->getInfo().is_pointer && !dest.m_type->isEqualTo(src.m_type)) {
                throw Exception(
                    "FunctionBuilder::vset - type of source value should either be the type "
                    "pointed to by destination type or the same pointer type as the destination"
                );
            } else if (!src.m_type->getInfo().is_pointer && !((PointerType*)dest.m_type)->getDestinationType()->isEqualTo(src.m_type)) {
                throw Exception(
                    "FunctionBuilder::vset - type of source value should either be the type "
                    "pointed to by destination type or the same pointer type as the destination"
                );
            }
        }

        Instruction i(OpCode::vset);
        i.operands[0].reset(dest);
        i.operands[1].reset(src);
        return add(i);
    }

    InstructionRef FunctionBuilder::vadd(const Value& dest, const Value& val) {
        if (m_validationEnabled) {
            if (!dest.m_type->getInfo().is_pointer) {
                throw Exception("FunctionBuilder::vadd - destination value should have a pointer type");
            }
            
            if (val.m_type->getInfo().is_pointer && !dest.m_type->isEqualTo(val.m_type)) {
                throw Exception(
                    "FunctionBuilder::vadd - type of right-hand value should either be the type "
                    "pointed to by destination type or the same pointer type as the destination"
                );
            } else if (!val.m_type->getInfo().is_pointer && !((PointerType*)dest.m_type)->getDestinationType()->isEqualTo(val.m_type)) {
                throw Exception(
                    "FunctionBuilder::vadd - type of right-hand value should either be the type "
                    "pointed to by destination type or the same pointer type as the destination"
                );
            }
        }

        Instruction i(OpCode::vadd);
        i.operands[0].reset(dest);
        i.operands[1].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::vsub(const Value& dest, const Value& val) {
        if (m_validationEnabled) {
            if (!dest.m_type->getInfo().is_pointer) {
                throw Exception("FunctionBuilder::vsub - destination value should have a pointer type");
            }
            
            if (val.m_type->getInfo().is_pointer && !dest.m_type->isEqualTo(val.m_type)) {
                throw Exception(
                    "FunctionBuilder::vsub - type of right-hand value should either be the type "
                    "pointed to by destination type or the same pointer type as the destination"
                );
            } else if (!val.m_type->getInfo().is_pointer && !((PointerType*)dest.m_type)->getDestinationType()->isEqualTo(val.m_type)) {
                throw Exception(
                    "FunctionBuilder::vsub - type of right-hand value should either be the type "
                    "pointed to by destination type or the same pointer type as the destination"
                );
            }
        }

        Instruction i(OpCode::vsub);
        i.operands[0].reset(dest);
        i.operands[1].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::vmul(const Value& dest, const Value& val) {
        if (m_validationEnabled) {
            if (!dest.m_type->getInfo().is_pointer) {
                throw Exception("FunctionBuilder::vmul - destination value should have a pointer type");
            }
            
            if (val.m_type->getInfo().is_pointer && !dest.m_type->isEqualTo(val.m_type)) {
                throw Exception(
                    "FunctionBuilder::vmul - type of right-hand value should either be the type "
                    "pointed to by destination type or the same pointer type as the destination"
                );
            } else if (!val.m_type->getInfo().is_pointer && !((PointerType*)dest.m_type)->getDestinationType()->isEqualTo(val.m_type)) {
                throw Exception(
                    "FunctionBuilder::vmul - type of right-hand value should either be the type "
                    "pointed to by destination type or the same pointer type as the destination"
                );
            }
        }

        Instruction i(OpCode::vmul);
        i.operands[0].reset(dest);
        i.operands[1].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::vdiv(const Value& dest, const Value& val) {
        if (m_validationEnabled) {
            if (!dest.m_type->getInfo().is_pointer) {
                throw Exception("FunctionBuilder::vdiv - destination value should have a pointer type");
            }
            
            if (val.m_type->getInfo().is_pointer && !dest.m_type->isEqualTo(val.m_type)) {
                throw Exception(
                    "FunctionBuilder::vdiv - type of right-hand value should either be the type "
                    "pointed to by destination type or the same pointer type as the destination"
                );
            } else if (!val.m_type->getInfo().is_pointer && !((PointerType*)dest.m_type)->getDestinationType()->isEqualTo(val.m_type)) {
                throw Exception(
                    "FunctionBuilder::vdiv - type of right-hand value should either be the type "
                    "pointed to by destination type or the same pointer type as the destination"
                );
            }
        }

        Instruction i(OpCode::vdiv);
        i.operands[0].reset(dest);
        i.operands[1].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::vmod(const Value& dest, const Value& val) {
        if (m_validationEnabled) {
            if (!dest.m_type->getInfo().is_pointer) {
                throw Exception("FunctionBuilder::vmod - destination value should have a pointer type");
            }
            
            if (val.m_type->getInfo().is_pointer && !dest.m_type->isEqualTo(val.m_type)) {
                throw Exception(
                    "FunctionBuilder::vmod - type of right-hand value should either be the type "
                    "pointed to by destination type or the same pointer type as the destination"
                );
            } else if (!val.m_type->getInfo().is_pointer && !((PointerType*)dest.m_type)->getDestinationType()->isEqualTo(val.m_type)) {
                throw Exception(
                    "FunctionBuilder::vmod - type of right-hand value should either be the type "
                    "pointed to by destination type or the same pointer type as the destination"
                );
            }
        }

        Instruction i(OpCode::vmod);
        i.operands[0].reset(dest);
        i.operands[1].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::vneg(const Value& val) {
        if (m_validationEnabled) {
            if (!val.m_type->getInfo().is_pointer) {
                throw Exception("FunctionBuilder::vneg - value to negate should have a pointer type");
            }
        }

        Instruction i(OpCode::vneg);
        i.operands[0].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::vdot(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::vdot - result value should refer to a register");
            if (!result.m_type->getInfo().is_pointer) {
                throw Exception("FunctionBuilder::vdot - destination value, left-hand value, right-hand value should all be the same pointer type");
            }
            
            if (!result.m_type->isEqualTo(a.m_type) || !result.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::vdot - destination value, left-hand value, right-hand value should all be the same pointer type");
            }
        }

        Instruction i(OpCode::vdot);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::vmag(const Value& result, const Value& val) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::vmag - result value should refer to a register");
            if (!val.m_type->getInfo().is_pointer) {
                throw Exception("FunctionBuilder::vmag - value to get the magnitude of should be a pointer");
            }
            
            if (!result.m_type->isEqualTo(((PointerType*)val.m_type)->getDestinationType())) {
                throw Exception("FunctionBuilder::vmag - result value should be the same type as the type pointed to by the value to get the magnitude of");
            }
        }

        Instruction i(OpCode::vmag);
        i.operands[0].reset(result);
        i.operands[1].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::vmagsq(const Value& result, const Value& val) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::vmagsq - result value should refer to a register");
            if (!val.m_type->getInfo().is_pointer) {
                throw Exception("FunctionBuilder::vmagsq - value to get the squared magnitude of should be a pointer");
            }
            
            if (!result.m_type->isEqualTo(((PointerType*)val.m_type)->getDestinationType())) {
                throw Exception("FunctionBuilder::vmagsq - result value should be the same type as the type pointed to by the value to get the squared magnitude of");
            }
        }

        Instruction i(OpCode::vmagsq);
        i.operands[0].reset(result);
        i.operands[1].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::vnorm(const Value& val) {
        if (m_validationEnabled && !val.m_type->getInfo().is_pointer) {
            if (!val.isReg()) throw Exception("FunctionBuilder::vnorm - value to normalize should refer to a register");
            throw Exception("FunctionBuilder::vnorm - value to normalize should have a pointer type");
        }

        Instruction i(OpCode::vnorm);
        i.operands[0].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::vcross(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.m_type->getInfo().is_pointer) {
                throw Exception("FunctionBuilder::vcross - destination value, left-hand value, right-hand value should all be the same pointer type");
            }
            
            if (!result.m_type->isEqualTo(a.m_type) || !result.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::vcross - destination value, left-hand value, right-hand value should all be the same pointer type");
            }
        }

        Instruction i(OpCode::vcross);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::iadd(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::iadd - result value should refer to a register");
            if (!result.m_type->isEqualTo(a.m_type) || !result.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::iadd - result value, left-hand value, right-hand value should all have the same signed integral type");
            }

            if (!result.m_type->getInfo().is_integral || result.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::iadd - result value, left-hand value, right-hand value should all have the same signed integral type");
            }
        }

        Instruction i(OpCode::iadd);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::uadd(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::uadd - result value should refer to a register");
            if (!result.m_type->isEqualTo(a.m_type) || !result.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::uadd - result value, left-hand value, right-hand value should all have the same unsigned integral type");
            }
            
            if (!result.m_type->getInfo().is_integral || !result.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::uadd - result value, left-hand value, right-hand value should all have the same unsigned integral type");
            }
        }

        Instruction i(OpCode::uadd);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::fadd(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::fadd - result value should refer to a register");
            if (!result.m_type->isEqualTo(a.m_type) || !result.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::fadd - result value, left-hand value, right-hand value should all have the same 32-bit floating point type");
            }
            
            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f32)) {
                throw Exception("FunctionBuilder::fadd - result value, left-hand value, right-hand value should all have the same 32-bit floating point type");
            }
        }

        Instruction i(OpCode::fadd);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::dadd(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::dadd - result value should refer to a register");
            if (!result.m_type->isEqualTo(a.m_type) || !result.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::dadd - result value, left-hand value, right-hand value should all have the same 64-bit floating point type");
            }
            
            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f64)) {
                throw Exception("FunctionBuilder::dadd - result value, left-hand value, right-hand value should all have the same 64-bit floating point type");
            }
        }

        Instruction i(OpCode::dadd);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::isub(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::isub - result value should refer to a register");
            if (!result.m_type->isEqualTo(a.m_type) || !result.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::isub - result value, left-hand value, right-hand value should all have the same signed integral type");
            }

            if (!result.m_type->getInfo().is_integral || result.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::isub - result value, left-hand value, right-hand value should all have the same signed integral type");
            }
        }

        Instruction i(OpCode::isub);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::usub(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::usub - result value should refer to a register");
            if (!result.m_type->isEqualTo(a.m_type) || !result.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::usub - result value, left-hand value, right-hand value should all have the same unsigned integral type");
            }
            
            if (!result.m_type->getInfo().is_integral || !result.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::usub - result value, left-hand value, right-hand value should all have the same unsigned integral type");
            }
        }

        Instruction i(OpCode::usub);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::fsub(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::fsub - result value should refer to a register");
            if (!result.m_type->isEqualTo(a.m_type) || !result.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::fsub - result value, left-hand value, right-hand value should all have the same 32-bit floating point type");
            }
            
            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f32)) {
                throw Exception("FunctionBuilder::fsub - result value, left-hand value, right-hand value should all have the same 32-bit floating point type");
            }
        }

        Instruction i(OpCode::fsub);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::dsub(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::dsub - result value should refer to a register");
            if (!result.m_type->isEqualTo(a.m_type) || !result.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::dsub - result value, left-hand value, right-hand value should all have the same 64-bit floating point type");
            }
            
            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f64)) {
                throw Exception("FunctionBuilder::dsub - result value, left-hand value, right-hand value should all have the same 64-bit floating point type");
            }
        }

        Instruction i(OpCode::dsub);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::imul(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::imul - result value should refer to a register");
            if (!result.m_type->isEqualTo(a.m_type) || !result.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::imul - result value, left-hand value, right-hand value should all have the same signed integral type");
            }

            if (!result.m_type->getInfo().is_integral || result.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::imul - result value, left-hand value, right-hand value should all have the same signed integral type");
            }
        }

        Instruction i(OpCode::imul);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::umul(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::umul - result value should refer to a register");
            if (!result.m_type->isEqualTo(a.m_type) || !result.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::umul - result value, left-hand value, right-hand value should all have the same unsigned integral type");
            }
            
            if (!result.m_type->getInfo().is_integral || !result.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::umul - result value, left-hand value, right-hand value should all have the same unsigned integral type");
            }
        }

        Instruction i(OpCode::umul);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::fmul(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::fmul - result value should refer to a register");
            if (!result.m_type->isEqualTo(a.m_type) || !result.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::fmul - result value, left-hand value, right-hand value should all have the same 32-bit floating point type");
            }
            
            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f32)) {
                throw Exception("FunctionBuilder::fmul - result value, left-hand value, right-hand value should all have the same 32-bit floating point type");
            }
        }

        Instruction i(OpCode::fmul);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::dmul(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::dmul - result value should refer to a register");
            if (!result.m_type->isEqualTo(a.m_type) || !result.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::dmul - result value, left-hand value, right-hand value should all have the same 64-bit floating point type");
            }
            
            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f64)) {
                throw Exception("FunctionBuilder::dmul - result value, left-hand value, right-hand value should all have the same 64-bit floating point type");
            }
        }

        Instruction i(OpCode::dmul);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::idiv(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::idiv - result value should refer to a register");
            if (!result.m_type->isEqualTo(a.m_type) || !result.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::idiv - result value, left-hand value, right-hand value should all have the same signed integral type");
            }

            if (!result.m_type->getInfo().is_integral || result.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::idiv - result value, left-hand value, right-hand value should all have the same signed integral type");
            }
        }

        Instruction i(OpCode::idiv);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::udiv(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::udiv - result value should refer to a register");
            if (!result.m_type->isEqualTo(a.m_type) || !result.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::udiv - result value, left-hand value, right-hand value should all have the same unsigned integral type");
            }
            
            if (!result.m_type->getInfo().is_integral || !result.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::udiv - result value, left-hand value, right-hand value should all have the same unsigned integral type");
            }
        }

        Instruction i(OpCode::udiv);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::fdiv(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::fdiv - result value should refer to a register");
            if (!result.m_type->isEqualTo(a.m_type) || !result.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::fdiv - result value, left-hand value, right-hand value should all have the same 32-bit floating point type");
            }
            
            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f32)) {
                throw Exception("FunctionBuilder::fdiv - result value, left-hand value, right-hand value should all have the same 32-bit floating point type");
            }
        }

        Instruction i(OpCode::fdiv);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::ddiv(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::ddiv - result value should refer to a register");
            if (!result.m_type->isEqualTo(a.m_type) || !result.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::ddiv - result value, left-hand value, right-hand value should all have the same 64-bit floating point type");
            }
            
            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f64)) {
                throw Exception("FunctionBuilder::ddiv - result value, left-hand value, right-hand value should all have the same 64-bit floating point type");
            }
        }

        Instruction i(OpCode::ddiv);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::imod(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::imod - result value should refer to a register");
            if (!result.m_type->isEqualTo(a.m_type) || !result.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::imod - result value, left-hand value, right-hand value should all have the same signed integral type");
            }

            if (!result.m_type->getInfo().is_integral || result.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::imod - result value, left-hand value, right-hand value should all have the same signed integral type");
            }
        }

        Instruction i(OpCode::imod);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::umod(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::umod - result value should refer to a register");
            if (!result.m_type->isEqualTo(a.m_type) || !result.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::umod - result value, left-hand value, right-hand value should all have the same unsigned integral type");
            }
            
            if (!result.m_type->getInfo().is_integral || !result.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::umod - result value, left-hand value, right-hand value should all have the same unsigned integral type");
            }
        }

        Instruction i(OpCode::umod);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::fmod(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::fmod - result value should refer to a register");
            if (!result.m_type->isEqualTo(a.m_type) || !result.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::fmod - result value, left-hand value, right-hand value should all have the same 32-bit floating point type");
            }
            
            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f32)) {
                throw Exception("FunctionBuilder::fmod - result value, left-hand value, right-hand value should all have the same 32-bit floating point type");
            }
        }

        Instruction i(OpCode::fmod);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::dmod(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::dmod - result value should refer to a register");
            if (!result.m_type->isEqualTo(a.m_type) || !result.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::dmod - result value, left-hand value, right-hand value should all have the same 64-bit floating point type");
            }
            
            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f64)) {
                throw Exception("FunctionBuilder::dmod - result value, left-hand value, right-hand value should all have the same 64-bit floating point type");
            }
        }

        Instruction i(OpCode::dmod);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::ineg(const Value& result, const Value& val) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::ineg - result value should refer to a register");
            if (!result.m_type->isEqualTo(val.m_type)) {
                throw Exception("FunctionBuilder::ineg - result value and value to negate should have the same signed integral type");
            }

            if (!result.m_type->getInfo().is_integral || result.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::ineg - result value and value to negate should have the same signed integral type");
            }
        }

        Instruction i(OpCode::ineg);
        i.operands[0].reset(result);
        i.operands[1].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::fneg(const Value& result, const Value& val) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::fneg - result value should refer to a register");
            if (!result.m_type->isEqualTo(val.m_type)) {
                throw Exception("FunctionBuilder::fneg - result value and value to negate should have the same 32-bit floating point type");
            }

            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f32)) {
                throw Exception("FunctionBuilder::fneg - result value and value to negate should have the same 32-bit floating point type");
            }
        }

        Instruction i(OpCode::fneg);
        i.operands[0].reset(result);
        i.operands[1].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::dneg(const Value& result, const Value& val) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::dneg - result value should refer to a register");
            if (!result.m_type->isEqualTo(val.m_type)) {
                throw Exception("FunctionBuilder::dneg - result value and value to negate should have the same 64-bit floating point type");
            }

            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f64)) {
                throw Exception("FunctionBuilder::dneg - result value and value to negate should have the same 64-bit floating point type");
            }
        }

        Instruction i(OpCode::dneg);
        i.operands[0].reset(result);
        i.operands[1].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::iinc(const Value& val) {
        if (m_validationEnabled) {
            if (!val.isReg()) throw Exception("FunctionBuilder::iinc - value to increment should refer to a register");
            
            if (!val.m_type->getInfo().is_integral || val.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::iinc - value to increment should have a signed integral type");
            }
        }

        Instruction i(OpCode::iinc);
        i.operands[0].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::uinc(const Value& val) {
        if (m_validationEnabled) {
            if (!val.isReg()) throw Exception("FunctionBuilder::uinc - value to increment should refer to a register");
            
            if (!val.m_type->getInfo().is_integral || val.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::uinc - value to increment should have an unsigned integral type");
            }
        }

        Instruction i(OpCode::uinc);
        i.operands[0].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::finc(const Value& val) {
        if (m_validationEnabled) {
            if (!val.isReg()) throw Exception("FunctionBuilder::finc - value to increment should refer to a register");
            
            if (!val.m_type->getInfo().is_floating_point || val.m_type->getInfo().size == sizeof(f32)) {
                throw Exception("FunctionBuilder::finc - value to increment should have a 32-bit floating point type");
            }
        }

        Instruction i(OpCode::finc);
        i.operands[0].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::dinc(const Value& val) {
        if (m_validationEnabled) {
            if (!val.isReg()) throw Exception("FunctionBuilder::dinc - value to increment should refer to a register");
            
            if (!val.m_type->getInfo().is_floating_point || val.m_type->getInfo().size == sizeof(f64)) {
                throw Exception("FunctionBuilder::dinc - value to increment should have a 64-bit floating point type");
            }
        }

        Instruction i(OpCode::dinc);
        i.operands[0].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::idec(const Value& val) {
        if (m_validationEnabled) {
            if (!val.isReg()) throw Exception("FunctionBuilder::idec - value to decrement should refer to a register");
            
            if (!val.m_type->getInfo().is_integral || val.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::idec - value to decrement should have a signed integral type");
            }
        }

        Instruction i(OpCode::idec);
        i.operands[0].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::udec(const Value& val) {
        if (m_validationEnabled) {
            if (!val.isReg()) throw Exception("FunctionBuilder::udec - value to decrement should refer to a register");
            
            if (!val.m_type->getInfo().is_integral || val.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::udec - value to decrement should have an unsigned integral type");
            }
        }

        Instruction i(OpCode::udec);
        i.operands[0].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::fdec(const Value& val) {
        if (m_validationEnabled) {
            if (!val.isReg()) throw Exception("FunctionBuilder::fdec - value to decrement should refer to a register");
            
            if (!val.m_type->getInfo().is_floating_point || val.m_type->getInfo().size == sizeof(f32)) {
                throw Exception("FunctionBuilder::fdec - value to decrement should have a 32-bit floating point type");
            }
        }

        Instruction i(OpCode::fdec);
        i.operands[0].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::ddec(const Value& val) {
        if (m_validationEnabled) {
            if (!val.isReg()) throw Exception("FunctionBuilder::ddec - value to decrement should refer to a register");
            
            if (!val.m_type->getInfo().is_floating_point || val.m_type->getInfo().size == sizeof(f64)) {
                throw Exception("FunctionBuilder::ddec - value to decrement should have a 64-bit floating point type");
            }
        }

        Instruction i(OpCode::ddec);
        i.operands[0].reset(val);
        return add(i);
    }

    InstructionRef FunctionBuilder::ilt(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::ilt - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::ilt - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::ilt - left-hand value and right-hand value should both have the same signed integral type");
            }

            if (!result.m_type->getInfo().is_integral || result.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::ilt - left-hand value and right-hand value should both have the same signed integral type");
            }
        }

        Instruction i(OpCode::ilt);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::ult(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::ult - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::ult - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::ult - left-hand value and right-hand value should both have the same unsigned integral type");
            }

            if (!result.m_type->getInfo().is_integral || !result.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::ult - left-hand value and right-hand value should both have the same unsigned integral type");
            }
        }
        
        Instruction i(OpCode::ult);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::flt(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::flt - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::flt - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::flt - left-hand value and right-hand value should both have the same 32-bit floating point type");
            }

            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f32)) {
                throw Exception("FunctionBuilder::flt - left-hand value and right-hand value should both have the same 32-bit floating point type");
            }
        }

        Instruction i(OpCode::flt);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::dlt(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::dlt - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::dlt - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::dlt - left-hand value and right-hand value should both have the same 64-bit floating point type");
            }

            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f64)) {
                throw Exception("FunctionBuilder::dlt - left-hand value and right-hand value should both have the same 64-bit floating point type");
            }
        }

        Instruction i(OpCode::dlt);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::ilte(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::ilte - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::ilte - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::ilte - left-hand value and right-hand value should both have the same signed integral type");
            }

            if (!result.m_type->getInfo().is_integral || result.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::ilte - left-hand value and right-hand value should both have the same signed integral type");
            }
        }

        Instruction i(OpCode::ilte);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::ulte(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::ulte - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::ulte - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::ulte - left-hand value and right-hand value should both have the same unsigned integral type");
            }

            if (!result.m_type->getInfo().is_integral || !result.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::ulte - left-hand value and right-hand value should both have the same unsigned integral type");
            }
        }
        
        Instruction i(OpCode::ulte);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::flte(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::flte - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::flte - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::flte - left-hand value and right-hand value should both have the same 32-bit floating point type");
            }

            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f32)) {
                throw Exception("FunctionBuilder::flte - left-hand value and right-hand value should both have the same 32-bit floating point type");
            }
        }

        Instruction i(OpCode::flte);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::dlte(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::dlte - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::dlte - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::dlte - left-hand value and right-hand value should both have the same 64-bit floating point type");
            }

            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f64)) {
                throw Exception("FunctionBuilder::dlte - left-hand value and right-hand value should both have the same 64-bit floating point type");
            }
        }

        Instruction i(OpCode::dlte);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::igt(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::igt - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::igt - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::igt - left-hand value and right-hand value should both have the same signed integral type");
            }

            if (!result.m_type->getInfo().is_integral || result.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::igt - left-hand value and right-hand value should both have the same signed integral type");
            }
        }

        Instruction i(OpCode::igt);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::ugt(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::ugt - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::ugt - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::ugt - left-hand value and right-hand value should both have the same unsigned integral type");
            }

            if (!result.m_type->getInfo().is_integral || !result.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::ugt - left-hand value and right-hand value should both have the same unsigned integral type");
            }
        }
        
        Instruction i(OpCode::ugt);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::fgt(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::fgt - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::fgt - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::fgt - left-hand value and right-hand value should both have the same 32-bit floating point type");
            }

            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f32)) {
                throw Exception("FunctionBuilder::fgt - left-hand value and right-hand value should both have the same 32-bit floating point type");
            }
        }

        Instruction i(OpCode::fgt);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::dgt(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::dgt - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::dgt - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::dgt - left-hand value and right-hand value should both have the same 64-bit floating point type");
            }

            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f64)) {
                throw Exception("FunctionBuilder::dgt - left-hand value and right-hand value should both have the same 64-bit floating point type");
            }
        }

        Instruction i(OpCode::dgt);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::igte(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::igte - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::igte - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::igte - left-hand value and right-hand value should both have the same signed integral type");
            }

            if (!result.m_type->getInfo().is_integral || result.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::igte - left-hand value and right-hand value should both have the same signed integral type");
            }
        }

        Instruction i(OpCode::igte);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::ugte(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::ugte - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::ugte - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::ugte - left-hand value and right-hand value should both have the same unsigned integral type");
            }

            if (!result.m_type->getInfo().is_integral || !result.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::ugte - left-hand value and right-hand value should both have the same unsigned integral type");
            }
        }
        
        Instruction i(OpCode::ugte);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::fgte(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::fgte - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::fgte - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::fgte - left-hand value and right-hand value should both have the same 32-bit floating point type");
            }

            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f32)) {
                throw Exception("FunctionBuilder::fgte - left-hand value and right-hand value should both have the same 32-bit floating point type");
            }
        }

        Instruction i(OpCode::fgte);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::dgte(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::dgte - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::dgte - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::dgte - left-hand value and right-hand value should both have the same 64-bit floating point type");
            }

            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f64)) {
                throw Exception("FunctionBuilder::dgte - left-hand value and right-hand value should both have the same 64-bit floating point type");
            }
        }

        Instruction i(OpCode::dgte);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::ieq(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::ieq - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::ieq - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::ieq - left-hand value and right-hand value should both have the same signed integral type");
            }

            if (!result.m_type->getInfo().is_integral || result.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::ieq - left-hand value and right-hand value should both have the same signed integral type");
            }
        }

        Instruction i(OpCode::ieq);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::ueq(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::ueq - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::ueq - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::ueq - left-hand value and right-hand value should both have the same unsigned integral type");
            }

            if (!result.m_type->getInfo().is_integral || !result.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::ueq - left-hand value and right-hand value should both have the same unsigned integral type");
            }
        }
        
        Instruction i(OpCode::ueq);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::feq(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::feq - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::feq - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::feq - left-hand value and right-hand value should both have the same 32-bit floating point type");
            }

            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f32)) {
                throw Exception("FunctionBuilder::feq - left-hand value and right-hand value should both have the same 32-bit floating point type");
            }
        }

        Instruction i(OpCode::feq);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::deq(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::deq - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::deq - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::deq - left-hand value and right-hand value should both have the same 64-bit floating point type");
            }

            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f64)) {
                throw Exception("FunctionBuilder::deq - left-hand value and right-hand value should both have the same 64-bit floating point type");
            }
        }

        Instruction i(OpCode::deq);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::ineq(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::ineq - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::ineq - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::ineq - left-hand value and right-hand value should both have the same signed integral type");
            }

            if (!result.m_type->getInfo().is_integral || result.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::ineq - left-hand value and right-hand value should both have the same signed integral type");
            }
        }

        Instruction i(OpCode::ineq);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::uneq(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::uneq - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::uneq - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::uneq - left-hand value and right-hand value should both have the same unsigned integral type");
            }

            if (!result.m_type->getInfo().is_integral || !result.m_type->getInfo().is_unsigned) {
                throw Exception("FunctionBuilder::uneq - left-hand value and right-hand value should both have the same unsigned integral type");
            }
        }
        
        Instruction i(OpCode::uneq);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::fneq(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::fneq - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::fneq - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::fneq - left-hand value and right-hand value should both have the same 32-bit floating point type");
            }

            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f32)) {
                throw Exception("FunctionBuilder::fneq - left-hand value and right-hand value should both have the same 32-bit floating point type");
            }
        }

        Instruction i(OpCode::fneq);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }

    InstructionRef FunctionBuilder::dneq(const Value& result, const Value& a, const Value& b) {
        if (m_validationEnabled) {
            if (!result.isReg()) throw Exception("FunctionBuilder::dneq - result value should refer to a register");
            if (!result.m_type->isEqualTo(Registry::GetType<bool>())) {
                throw Exception("FunctionBuilder::dneq - result value should have a boolean type");
            }

            if (!a.m_type->isEqualTo(b.m_type)) {
                throw Exception("FunctionBuilder::dneq - left-hand value and right-hand value should both have the same 64-bit floating point type");
            }

            if (!result.m_type->getInfo().is_floating_point || result.m_type->getInfo().size != sizeof(f64)) {
                throw Exception("FunctionBuilder::dneq - left-hand value and right-hand value should both have the same 64-bit floating point type");
            }
        }

        Instruction i(OpCode::dneq);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        return add(i);
    }
};