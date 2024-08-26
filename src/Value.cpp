#include <codegen/Value.h>
#include <codegen/FunctionBuilder.h>
#include <bind/Registry.hpp>
#include <bind/util/Array.hpp>
#include <bind/Function.h>

namespace codegen {
    using namespace bind;
    
    Value::Value() : m_owner(nullptr), m_type(nullptr), m_isImm(false), m_isLabel(false), m_regId(NullRegister) {}
    Value::Value(const Value& v) : m_owner(v.m_owner), m_type(v.m_type), m_isImm(v.m_isImm), m_isLabel(v.m_isLabel), m_regId(v.m_regId), m_imm(v.m_imm) {}
    Value::Value(FunctionBuilder* func, bool imm) : m_owner(func), m_type(Registry::GetType<bool>()), m_isImm(true), m_isLabel(false), m_regId(NullRegister), m_imm(imm) {}
    Value::Value(FunctionBuilder* func, u8   imm) : m_owner(func), m_type(Registry::GetType<u8  >()), m_isImm(true), m_isLabel(false), m_regId(NullRegister), m_imm(imm) {}
    Value::Value(FunctionBuilder* func, u16  imm) : m_owner(func), m_type(Registry::GetType<u16 >()), m_isImm(true), m_isLabel(false), m_regId(NullRegister), m_imm(imm) {}
    Value::Value(FunctionBuilder* func, u32  imm) : m_owner(func), m_type(Registry::GetType<u32 >()), m_isImm(true), m_isLabel(false), m_regId(NullRegister), m_imm(imm) {}
    Value::Value(FunctionBuilder* func, u64  imm) : m_owner(func), m_type(Registry::GetType<u64 >()), m_isImm(true), m_isLabel(false), m_regId(NullRegister), m_imm(imm) {}
    Value::Value(FunctionBuilder* func, i8   imm) : m_owner(func), m_type(Registry::GetType<i8  >()), m_isImm(true), m_isLabel(false), m_regId(NullRegister), m_imm(imm) {}
    Value::Value(FunctionBuilder* func, i16  imm) : m_owner(func), m_type(Registry::GetType<i16 >()), m_isImm(true), m_isLabel(false), m_regId(NullRegister), m_imm(imm) {}
    Value::Value(FunctionBuilder* func, i32  imm) : m_owner(func), m_type(Registry::GetType<i32 >()), m_isImm(true), m_isLabel(false), m_regId(NullRegister), m_imm(imm) {}
    Value::Value(FunctionBuilder* func, i64  imm) : m_owner(func), m_type(Registry::GetType<i64 >()), m_isImm(true), m_isLabel(false), m_regId(NullRegister), m_imm(imm) {}
    Value::Value(FunctionBuilder* func, f32  imm) : m_owner(func), m_type(Registry::GetType<f32 >()), m_isImm(true), m_isLabel(false), m_regId(NullRegister), m_imm(imm) {}
    Value::Value(FunctionBuilder* func, f64  imm) : m_owner(func), m_type(Registry::GetType<f64 >()), m_isImm(true), m_isLabel(false), m_regId(NullRegister), m_imm(imm) {}
    Value::Value(FunctionBuilder* func, ptr  imm) : m_owner(func), m_type(Registry::GetType<ptr >()), m_isImm(true), m_isLabel(false), m_regId(NullRegister), m_imm(imm) {}
    Value::Value(
        vreg_id regId,
        FunctionBuilder* func,
        bind::DataType* type
    ) : m_owner(func), m_type(type), m_isImm(false), m_isLabel(false), m_regId(regId) {}
    
    void Value::reset(const Value& v) {
        m_owner = v.m_owner;
        m_type = v.m_type;
        m_isImm = v.m_isImm;
        m_regId = v.m_regId;
        m_imm = v.m_imm;
    }

    FunctionBuilder* Value::getOwner() const {
        return m_owner;
    }

    vreg_id Value::getRegisterId() const {
        return m_regId;
    }

    const Immediate& Value::getImm() const {
        return m_imm;
    }

    Immediate& Value::getImm() {
        return m_imm;
    }
    
    bool Value::isImm() const {
        return m_isImm;
    }
    
    bool Value::isReg() const {
        return !m_isImm;
    }
    
    bool Value::isLabel() const {
        return m_isLabel;
    }

    bool Value::isEmpty() const {
        return m_owner == nullptr;
    }

    Value Value::convertedTo(DataType* to) const {
        if (m_isLabel) {
            m_owner->logError("Invalid use of label as a value");
            return Value();
        }

        return Value();
    }

    Value Value::operator +  (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::iadd, OpCode::uadd, OpCode::fadd, OpCode::dadd, "+");
    }

    Value Value::operator += (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::iadd, OpCode::uadd, OpCode::fadd, OpCode::dadd, "+=", true);
    }

    Value Value::operator -  (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::isub, OpCode::usub, OpCode::fsub, OpCode::dsub, "-");
    }

    Value Value::operator -= (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::isub, OpCode::usub, OpCode::fsub, OpCode::dsub, "-=", true);
    }

    Value Value::operator *  (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::imul, OpCode::umul, OpCode::fmul, OpCode::dmul, "*");
    }

    Value Value::operator *= (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::imul, OpCode::umul, OpCode::fmul, OpCode::dmul, "*=", true);
    }

    Value Value::operator /  (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::idiv, OpCode::udiv, OpCode::fdiv, OpCode::ddiv, "/");
    }

    Value Value::operator /= (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::idiv, OpCode::udiv, OpCode::fdiv, OpCode::ddiv, "/=", true);
    }

    Value Value::operator %  (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::imod, OpCode::umod, OpCode::fmod, OpCode::dmod, "%");
    }

    Value Value::operator %= (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::imod, OpCode::umod, OpCode::fmod, OpCode::dmod, "%=", true);
    }

    Value Value::operator ^  (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::_xor, OpCode::_xor, OpCode::noop, OpCode::noop, "^");
    }

    Value Value::operator ^= (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::_xor, OpCode::_xor, OpCode::noop, OpCode::noop, "^=", true);
    }

    Value Value::operator &  (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::band, OpCode::band, OpCode::noop, OpCode::noop, "&");
    }

    Value Value::operator &= (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::band, OpCode::band, OpCode::noop, OpCode::noop, "&=", true);
    }

    Value Value::operator |  (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::bor, OpCode::bor, OpCode::noop, OpCode::noop, "|");
    }

    Value Value::operator |= (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::bor, OpCode::bor, OpCode::noop, OpCode::noop, "|=", true);
    }

    Value Value::operator << (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::shl, OpCode::shl, OpCode::noop, OpCode::noop, "<<");
    }

    Value Value::operator <<=(const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::shl, OpCode::shl, OpCode::noop, OpCode::noop, "<<=", true);
    }

    Value Value::operator >> (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::shr, OpCode::shr, OpCode::noop, OpCode::noop, ">>");
    }

    Value Value::operator >>=(const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::shr, OpCode::shr, OpCode::noop, OpCode::noop, ">>=", true);
    }

    Value Value::operator != (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::ineq, OpCode::uneq, OpCode::fneq, OpCode::dneq, "!=");
    }

    Value Value::operator && (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::land, OpCode::land, OpCode::land, OpCode::land, "&&");
    }

    Value Value::operator || (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::lor, OpCode::lor, OpCode::lor, OpCode::lor, "||");
    }

    Value Value::operator =  (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::assign, OpCode::assign, OpCode::assign, OpCode::assign, "=", true);
    }

    Value Value::operator == (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::ieq, OpCode::ueq, OpCode::feq, OpCode::deq, "==");
    }

    Value Value::operator <  (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::ilt, OpCode::ult, OpCode::flt, OpCode::dlt, "<");
    }

    Value Value::operator <= (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::ilte, OpCode::ulte, OpCode::flte, OpCode::dlte, "<=");
    }

    Value Value::operator >  (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::igt, OpCode::ugt, OpCode::fgt, OpCode::dgt, ">");
    }

    Value Value::operator >= (const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::igte, OpCode::ugte, OpCode::fgte, OpCode::dgte, ">=");
    }

    Value Value::operator [] (const Value& rhs) const {
        if (isEmpty() || rhs.isEmpty()) return Value();
        if (m_isLabel) {
            m_owner->logError("Invalid use of label as a value");
            return Value();
        }
        
        Function* strictMatch = nullptr;
        auto opMethods = m_type->findMethods(FuncMatch("[]").argTps({ rhs.m_type }, false), &strictMatch);

        if (strictMatch) {
            return m_owner->generateCall(strictMatch, { rhs }, *this);
        }

        if (opMethods.size() > 1) {
            m_owner->logError(
                "Reference to operator '[]' of type '%s' with arguments (%s) is ambiguous",
                m_type->getName().c_str(),
                rhs.m_type->getName().c_str()
            );

            for (u32 i = 0;i < opMethods.size();i++) {
                m_owner->logInfo("^ Could be: '%s'", opMethods[i]->getSignature()->getName().c_str());
            }

            return Value();
        } else if (opMethods.size() == 0) {
            m_owner->logError(
                "Type '%s' has no operator '[]' with arguments matching (%s)",
                m_type->getName().c_str(),
                rhs.m_type->getName().c_str()
            );

            return Value();
        }
        
        return m_owner->generateCall(strictMatch, { rhs }, *this);
    }

    Value Value::operator () (const bind::Array<Value>& args, Value* self) const {
        if (isEmpty() || (self && self->isEmpty())) return Value();
        if (m_isLabel) {
            m_owner->logError("Invalid use of label as a value");
            return Value();
        }

        Array<DataType*> argTps;
        argTps.reserve(args.size());
        for (u32 i = 0;i < args.size();i++) {
            if (args[i].isEmpty()) return Value();
            if (args[i].m_isLabel) {
                m_owner->logError("Invalid use of label as a value");
                return Value();
            }

            argTps.push(args[i].m_type);
        }

        if (m_type->getInfo().is_function) {
            if (m_isImm) {
                return m_owner->generateCall((Function*)m_imm.p, args, self ? *self : Value());
            }

            return m_owner->generateCall(*this, args, self ? *self : Value());
        }
        
        Function* strictMatch = nullptr;
        auto opMethods = m_type->findMethods(FuncMatch("()").argTps(argTps, false), &strictMatch);

        if (strictMatch) {
            return m_owner->generateCall(strictMatch, args, *this);
        }

        if (opMethods.size() > 1) {
            String argStr;
            for (u32 i = 0;i < argTps.size();i++) {
                if (i > 0) argStr += ", ";
                argStr += argTps[i]->getName().c_str();
            }

            m_owner->logError(
                "Reference to operator '()' of type '%s' with arguments (%s) is ambiguous",
                m_type->getName().c_str(),
                argStr.c_str()
            );

            for (u32 i = 0;i < opMethods.size();i++) {
                m_owner->logInfo("^ Could be: '%s'", opMethods[i]->getSignature()->getName().c_str());
            }

            return Value();
        } else if (opMethods.size() == 0) {
            String argStr;
            for (u32 i = 0;i < argTps.size();i++) {
                if (i > 0) argStr += ", ";
                argStr += argTps[i]->getName().c_str();
            }

            m_owner->logError(
                "Type '%s' has no operator '()' with arguments matching (%s)",
                m_type->getName().c_str(),
                argStr.c_str()
            );

            return Value();
        }
        
        return m_owner->generateCall(strictMatch, args, *this);
    }

    Value Value::operator -  () const {
        return genUnaryOp(OpCode::ineg, OpCode::noop, OpCode::fneg, OpCode::dneg, "-");
    }

    Value Value::operator -- () const {
        return genUnaryOp(OpCode::idec, OpCode::udec, OpCode::fdec, OpCode::ddec, "--", false, true, true);
    }

    Value Value::operator -- (int) const {
        return genUnaryOp(OpCode::idec, OpCode::udec, OpCode::fdec, OpCode::ddec, "--", true, true, true);
    }

    Value Value::operator ++ () const {
        return genUnaryOp(OpCode::iinc, OpCode::uinc, OpCode::finc, OpCode::dinc, "++", false, true, true);
    }

    Value Value::operator ++ (int) const {
        return genUnaryOp(OpCode::iinc, OpCode::uinc, OpCode::finc, OpCode::dinc, "++", true, true, true);
    }

    Value Value::operator !  () const {
        return genUnaryOp(OpCode::_not, OpCode::_not, OpCode::_not, OpCode::_not, "!");
    }

    Value Value::operator ~  () const {
        return genUnaryOp(OpCode::inv, OpCode::inv, OpCode::noop, OpCode::noop, "~");
    }

    Value Value::operator *  () const {
        if (isEmpty()) Value();
        if (m_isLabel) {
            m_owner->logError("Invalid use of label as a value");
            return Value();
        }

        const type_meta& i = m_type->getInfo();

        if (i.is_pointer) {
            DataType* destTp = ((PointerType*)m_type)->getDestinationType();

            if (destTp->getInfo().is_primitive || destTp->getInfo().is_pointer) {
                Value result = m_owner->val(destTp);
                m_owner->load(result, *this);
                return result;
            }

            // Can't actually dereference pointer to non-primitive
            // Due to complex circumstances, objects should always
            // be treated as pointers 
            Value result = *this;
            result.m_type = destTp;
            return result;
        }

        Function* strictMatch = nullptr;
        auto opMethods = m_type->findMethods(FuncMatch("*").noArgs(), &strictMatch);

        if (strictMatch) {
            return m_owner->generateCall(strictMatch, {}, *this);
        }

        if (opMethods.size() > 1) {
            m_owner->logError(
                "Reference to operator '*' of type '%s' with arguments () is ambiguous",
                m_type->getName().c_str()
            );

            for (u32 i = 0;i < opMethods.size();i++) {
                m_owner->logInfo("^ Could be: '%s'", opMethods[i]->getSignature()->getName().c_str());
            }

            return Value();
        } else if (opMethods.size() == 0) {
            m_owner->logError(
                "Type '%s' has no operator '()' with arguments matching ()",
                m_type->getName().c_str()
            );

            return Value();
        }
        
        return m_owner->generateCall(strictMatch, {}, *this);
    }

    Value Value::operator_logicalAndAssign(const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::land, OpCode::land, OpCode::land, OpCode::land, "&&=", true);
    }

    Value Value::operator_logicalOrAssign(const Value& rhs) const {
        return genBinaryOp(rhs, OpCode::lor, OpCode::lor, OpCode::lor, OpCode::lor, "||=", true);
    }

    bool Value::isEquivalentTo(const Value& v) const {
        if (m_type != v.m_type) return false;
        if (m_owner != v.m_owner) return false;
        if (m_isLabel != v.m_isLabel) return false;

        if (m_isImm) {
            if (!v.m_isImm) return false;
            if (m_type->getInfo().is_floating_point) {
                if (m_type->getInfo().size == sizeof(f32)) return m_imm.f == v.m_imm.f;
                return m_imm.d == v.m_imm.d;
            }
            return m_imm.u == v.m_imm.u;
        }

        if (v.m_isImm) return false;

        return m_regId == v.m_regId;
    }

    String Value::toString() const {
        if (isEmpty()) return "<Invalid Value>";
        if (m_isLabel) return String::Format("LABEL_%d", m_imm.u);

        const type_meta& i = m_type->getInfo();

        if (m_isImm) {
            if (i.is_floating_point) {
                if (i.size == sizeof(f32)) return String::Format("%f", m_imm.f);
                return String::Format("%lf", m_imm.d);
            }

            if (i.is_function) {
                return String::Format("<Function %s>", ((Function*)m_imm.p)->getSymbolName().c_str());
            }

            if (i.is_unsigned) return String::Format("%llu", m_imm.u);
            return String::Format("%lld", m_imm.i);
        }

        if (i.is_floating_point) return String::Format("FP%d", m_regId);
        return String::Format("GP%d", m_regId);
    }

    Value Value::genBinaryOp(
        const Value& rhs,
        OpCode _i,
        OpCode _u,
        OpCode _f,
        OpCode _d,
        const char* overrideName,
        bool assignmentOp
    ) const {
        if (isEmpty() || rhs.isEmpty()) return Value();
        if (m_isLabel || rhs.m_isLabel) {
            m_owner->logError("Invalid use of label as a value");
            return Value();
        }

        const type_meta& ai = m_type->getInfo();
        const type_meta& bi = rhs.m_type->getInfo();

        if (ai.is_primitive) {
            Instruction i(OpCode::noop);

            if (ai.is_integral) {
                if (ai.is_unsigned) i.op = _u;
                else i.op = _i;
            } else if (ai.is_floating_point) {
                if (ai.size == sizeof(f32)) i.op = _f;
                else if (ai.size == sizeof(f64)) i.op = _d;
                else {
                    m_owner->logError("Arithmetic involving >64-bit wide floating point values is unsupported");
                    return Value();
                }
            }

            Value result;

            if (i.op == OpCode::assign) {
                i.operands[0].reset(*this);
                i.operands[1].reset(rhs.convertedTo(m_type));
                result.reset(*this);
            } else if (assignmentOp) {
                i.operands[0].reset(*this);
                i.operands[1].reset(*this);
                i.operands[2].reset(rhs.convertedTo(m_type));
                result.reset(*this);
            } else {
                result.reset(m_owner->val(m_type));
                i.operands[0].reset(result);
                i.operands[1].reset(*this);
                i.operands[2].reset(rhs.convertedTo(m_type));
            }

            m_owner->add(i);
            return result;
        }

        Function* strictMatch = nullptr;
        auto opMethods = m_type->findMethods(FuncMatch(overrideName).argTps({ rhs.m_type }, false), &strictMatch);

        if (strictMatch) {
            return m_owner->generateCall(strictMatch, { rhs }, *this);
        }

        if (opMethods.size() > 1) {
            m_owner->logError(
                "Reference to operator '%s' of type '%s' with arguments (%s) is ambiguous",
                overrideName,
                m_type->getName().c_str(),
                rhs.m_type->getName().c_str()
            );

            for (u32 i = 0;i < opMethods.size();i++) {
                m_owner->logInfo("^ Could be: '%s'", opMethods[i]->getSignature()->getName().c_str());
            }

            return Value();
        } else if (opMethods.size() == 0) {
            m_owner->logError(
                "Type '%s' has no operator '%s' with arguments matching (%s)",
                m_type->getName().c_str(),
                overrideName,
                rhs.m_type->getName().c_str()
            );

            return Value();
        }
        
        return m_owner->generateCall(strictMatch, { rhs }, *this);
    }

    Value Value::genUnaryOp(
        OpCode _i,
        OpCode _u,
        OpCode _f,
        OpCode _d,
        const char* overrideName,
        bool resultIsPreOp,
        bool assignmentOp,
        bool noResultReg
    ) const {
        if (isEmpty()) return Value();
        if (m_isLabel) {
            m_owner->logError("Invalid use of label as a value");
            return Value();
        }

        const type_meta& ai = m_type->getInfo();

        if (ai.is_primitive) {
            Instruction i(OpCode::noop);

            if (ai.is_integral) {
                if (ai.is_unsigned) i.op = _u;
                else i.op = _i;
            } else if (ai.is_floating_point) {
                if (ai.size == sizeof(f32)) i.op = _f;
                else if (ai.size == sizeof(f64)) i.op = _d;
                else {
                    m_owner->logError("Arithmetic involving >64-bit wide floating point values is unsupported");
                    return Value();
                }
            }

            Value result = m_owner->val(m_type);

            if (resultIsPreOp) m_owner->assign(result, *this);

            if (noResultReg) {
                i.operands[0].reset(*this);
            } else {
                i.operands[0].reset(result);
                i.operands[1].reset(*this);
            }

            m_owner->add(i);

            if (noResultReg && !resultIsPreOp) {
                m_owner->assign(result, *this);
            }

            return result;
        }

        Function* strictMatch = nullptr;
        auto opMethods = m_type->findMethods(FuncMatch(overrideName).noArgs(), &strictMatch);

        if (strictMatch) {
            return m_owner->generateCall(strictMatch, {}, *this);
        }

        if (opMethods.size() > 1) {
            m_owner->logError(
                "Reference to operator '%s' of type '%s' with arguments () is ambiguous",
                overrideName,
                m_type->getName().c_str()
            );

            for (u32 i = 0;i < opMethods.size();i++) {
                m_owner->logInfo("^ Could be: '%s'", opMethods[i]->getSignature()->getName().c_str());
            }

            return Value();
        } else if (opMethods.size() == 0) {
            m_owner->logError(
                "Type '%s' has no operator '%s' with arguments matching ()",
                m_type->getName().c_str(),
                overrideName
            );

            return Value();
        }
        
        return m_owner->generateCall(strictMatch, {}, *this);
    }
};