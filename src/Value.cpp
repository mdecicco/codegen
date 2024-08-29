#include <codegen/Value.h>
#include <codegen/FunctionBuilder.h>
#include <bind/Registry.hpp>
#include <utils/Array.hpp>
#include <bind/Function.h>

namespace codegen {
    using namespace bind;
    
    Value::Value() {
        m_owner = nullptr;
        m_type = Registry::GetType<void>();
        m_isImm = false;
        m_isLabel = false;
        m_regId = NullRegister;
        m_stackRef = NullStack;
        m_nameStringId = -1;
    }

    Value::Value(const Value& v) {
        m_owner = v.m_owner;
        m_type = v.m_type;
        m_isImm = v.m_isImm;
        m_isLabel = v.m_isLabel;
        m_regId = v.m_regId;
        m_stackRef = v.m_stackRef;
        m_imm = v.m_imm;
        m_nameStringId = v.m_nameStringId;
    }

    Value::Value(vreg_id regId, FunctionBuilder* func, DataType* type) {
        m_owner = func;
        m_type = type;
        m_isImm = false;
        m_isLabel = false;
        m_regId = regId;
        m_stackRef = NullStack;
        m_nameStringId = -1;
    }

    Value::Value(FunctionBuilder* func, bool imm) {
        m_owner = func;
        m_type = Registry::GetType<bool>();
        m_isImm = true;
        m_isLabel = false;
        m_regId = NullRegister;
        m_stackRef = NullStack;
        m_imm = imm;
        m_nameStringId = -1;
    }
    
    Value::Value(FunctionBuilder* func, u8   imm) {
        m_owner = func;
        m_type = Registry::GetType<u8  >();
        m_isImm = true;
        m_isLabel = false;
        m_regId = NullRegister;
        m_stackRef = NullStack;
        m_imm = imm;
        m_nameStringId = -1;
    }
    
    Value::Value(FunctionBuilder* func, u16  imm) {
        m_owner = func;
        m_type = Registry::GetType<u16 >();
        m_isImm = true;
        m_isLabel = false;
        m_regId = NullRegister;
        m_stackRef = NullStack;
        m_imm = imm;
        m_nameStringId = -1;
    }
    
    Value::Value(FunctionBuilder* func, u32  imm) {
        m_owner = func;
        m_type = Registry::GetType<u32 >();
        m_isImm = true;
        m_isLabel = false;
        m_regId = NullRegister;
        m_stackRef = NullStack;
        m_imm = imm;
        m_nameStringId = -1;
    }
    
    Value::Value(FunctionBuilder* func, u64  imm) {
        m_owner = func;
        m_type = Registry::GetType<u64 >();
        m_isImm = true;
        m_isLabel = false;
        m_regId = NullRegister;
        m_stackRef = NullStack;
        m_imm = imm;
        m_nameStringId = -1;
    }
    
    Value::Value(FunctionBuilder* func, i8   imm) {
        m_owner = func;
        m_type = Registry::GetType<i8  >();
        m_isImm = true;
        m_isLabel = false;
        m_regId = NullRegister;
        m_stackRef = NullStack;
        m_imm = imm;
        m_nameStringId = -1;
    }
    
    Value::Value(FunctionBuilder* func, i16  imm) {
        m_owner = func;
        m_type = Registry::GetType<i16 >();
        m_isImm = true;
        m_isLabel = false;
        m_regId = NullRegister;
        m_stackRef = NullStack;
        m_imm = imm;
        m_nameStringId = -1;
    }
    
    Value::Value(FunctionBuilder* func, i32  imm) {
        m_owner = func;
        m_type = Registry::GetType<i32 >();
        m_isImm = true;
        m_isLabel = false;
        m_regId = NullRegister;
        m_stackRef = NullStack;
        m_imm = imm;
        m_nameStringId = -1;
    }
    
    Value::Value(FunctionBuilder* func, i64  imm) {
        m_owner = func;
        m_type = Registry::GetType<i64 >();
        m_isImm = true;
        m_isLabel = false;
        m_regId = NullRegister;
        m_stackRef = NullStack;
        m_imm = imm;
        m_nameStringId = -1;
    }
    
    Value::Value(FunctionBuilder* func, f32  imm) {
        m_owner = func;
        m_type = Registry::GetType<f32 >();
        m_isImm = true;
        m_isLabel = false;
        m_regId = NullRegister;
        m_stackRef = NullStack;
        m_imm = imm;
        m_nameStringId = -1;
    }
    
    Value::Value(FunctionBuilder* func, f64  imm) {
        m_owner = func;
        m_type = Registry::GetType<f64 >();
        m_isImm = true;
        m_isLabel = false;
        m_regId = NullRegister;
        m_stackRef = NullStack;
        m_imm = imm;
        m_nameStringId = -1;
    }
    
    Value::Value(FunctionBuilder* func, ptr  imm) {
        m_owner = func;
        m_type = Registry::GetType<ptr >();
        m_isImm = true;
        m_isLabel = false;
        m_regId = NullRegister;
        m_stackRef = NullStack;
        m_imm = imm;
        m_nameStringId = -1;
    }
    
    void Value::reset(const Value& v) {
        m_owner = v.m_owner;
        m_type = v.m_type;
        m_isImm = v.m_isImm;
        m_isLabel = v.m_isLabel;
        m_regId = v.m_regId;
        m_stackRef = v.m_stackRef;
        m_imm = v.m_imm;
        m_nameStringId = v.m_nameStringId;
    }

    FunctionBuilder* Value::getOwner() const {
        return m_owner;
    }

    DataType* Value::getType() const {
        return m_type;
    }
    
    void Value::setType(DataType* type) {
        m_type = type ? type : Registry::GetType<void>();
    }
    
    vreg_id Value::getRegisterId() const {
        return m_regId;
    }

    stack_id Value::getStackRef() const {
        return m_stackRef;
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

    void Value::setName(const String& name) {
        m_owner->setName(*this, name);
    }

    const String& Value::getName() const {
        return m_owner->getString(m_nameStringId);
    }

    Value Value::convertedTo(DataType* tp) const {
        if (m_isLabel) {
            m_owner->logError("Invalid use of label as a value");
            return Value();
        }

        if (tp->isEquivalentTo(m_type)) {
            return *this;
        }

        if (m_type->getInfo().is_pointer && tp->getInfo().is_pointer) {
            Value ret = Value(*this);
            ret.m_type = tp;
            return ret;
        }

        if (m_type->getInfo().is_primitive && tp->getInfo().is_primitive) {
            if (m_isImm) {
                const auto& ai = m_type->getInfo();
                const auto& bi = tp->getInfo();

                if (ai.is_floating_point) {
                    if (bi.is_floating_point) {
                        if (bi.size == ai.size) return *this;
                        Value ret = m_owner->val(tp);
                        m_owner->cvt(ret, *this);
                        return ret;
                    }
                    
                    if (bi.is_unsigned) {
                        Value ret;

                        switch (bi.size) {
                            case sizeof(u64): ret.reset(Value(m_owner, u64(m_imm.f)));
                            case sizeof(u32): ret.reset(Value(m_owner, u64(u32(m_imm.f))));
                            case sizeof(u16): ret.reset(Value(m_owner, u64(u16(m_imm.f))));
                            case sizeof(u8) : ret.reset(Value(m_owner, u64(u8(m_imm.f))));
                            default: ret.reset(Value(m_owner, u64(m_imm.f)));
                        }

                        ret.setType(tp);
                        return ret;
                    }

                    Value ret;

                    switch (bi.size) {
                        case sizeof(i64): ret.reset(Value(m_owner, i64(m_imm.f)));
                        case sizeof(i32): ret.reset(Value(m_owner, i64(i32(m_imm.f))));
                        case sizeof(i16): ret.reset(Value(m_owner, i64(i16(m_imm.f))));
                        case sizeof(i8) : ret.reset(Value(m_owner, i64(i8(m_imm.f))));
                        default: ret.reset(Value(m_owner, i64(m_imm.f)));
                    }

                    ret.setType(tp);
                    return ret;
                } else {
                    if (bi.is_floating_point) {
                        if (ai.is_unsigned) {
                            Value ret;

                            switch (bi.size) {
                                case sizeof(f64): ret.reset(Value(m_owner, f64(m_imm.u)));
                                case sizeof(f32): ret.reset(Value(m_owner, f64(f32(m_imm.u))));
                                default: ret.reset(Value(m_owner, f64(m_imm.u)));
                            }

                            ret.setType(tp);
                            return ret;
                        }

                        Value ret;

                        switch (bi.size) {
                            case sizeof(f64): ret.reset(Value(m_owner, f64(m_imm.i)));
                            case sizeof(f32): ret.reset(Value(m_owner, f64(f32(m_imm.i))));
                            default: ret.reset(Value(m_owner, f64(m_imm.i)));
                        }

                        ret.setType(tp);
                        return ret;
                    } else {
                        if (ai.is_unsigned) {
                            if (bi.is_unsigned) {
                                Value ret;

                                switch (bi.size) {
                                    case sizeof(u64): ret.reset(Value(m_owner, u64(m_imm.u)));
                                    case sizeof(u32): ret.reset(Value(m_owner, u64(u32(m_imm.u))));
                                    case sizeof(u16): ret.reset(Value(m_owner, u64(u16(m_imm.u))));
                                    case sizeof(u8) : ret.reset(Value(m_owner, u64(u8(m_imm.u))));
                                    default: ret.reset(Value(m_owner, u64(m_imm.u)));
                                }

                                ret.setType(tp);
                                return ret;
                            }

                            Value ret;

                            switch (bi.size) {
                                case sizeof(i64): ret.reset(Value(m_owner, i64(m_imm.u)));
                                case sizeof(i32): ret.reset(Value(m_owner, i64(i32(m_imm.u))));
                                case sizeof(i16): ret.reset(Value(m_owner, i64(i16(m_imm.u))));
                                case sizeof(i8) : ret.reset(Value(m_owner, i64(i8(m_imm.u))));
                                default: ret.reset(Value(m_owner, i64(m_imm.u)));
                            }

                            ret.setType(tp);
                            return ret;
                        }

                        if (bi.is_unsigned) {
                            Value ret;

                            switch (bi.size) {
                                case sizeof(u64): ret.reset(Value(m_owner, u64(m_imm.i)));
                                case sizeof(u32): ret.reset(Value(m_owner, u64(u32(m_imm.i))));
                                case sizeof(u16): ret.reset(Value(m_owner, u64(u16(m_imm.i))));
                                case sizeof(u8) : ret.reset(Value(m_owner, u64(u8(m_imm.i))));
                                default: ret.reset(Value(m_owner, u64(m_imm.i)));
                            }

                            ret.setType(tp);
                            return ret;
                        }

                        Value ret;

                        switch (bi.size) {
                            case sizeof(i64): ret.reset(Value(m_owner, i64(m_imm.i)));
                            case sizeof(i32): ret.reset(Value(m_owner, i64(i32(m_imm.i))));
                            case sizeof(i16): ret.reset(Value(m_owner, i64(i16(m_imm.i))));
                            case sizeof(i8) : ret.reset(Value(m_owner, i64(i8(m_imm.i))));
                            default: ret.reset(Value(m_owner, i64(m_imm.i)));
                        }

                        ret.setType(tp);
                        return ret;
                    }
                }
            }

            Value ret = m_owner->val(tp);
            m_owner->cvt(ret, *this);
            return ret;
        } else {
            // search for cast operators
            {
                // todo: access rights
                Function* castOp = m_type->findConversionOperator(tp, FullAccessRights);
                if (castOp) return m_owner->generateCall(castOp, {}, *this);
            }
        
            // search for copy constructor
            {
                // todo: access rights
                auto ctors = tp->findConstructors({ m_type }, true, FullAccessRights);

                if (ctors.size() == 1) {
                    Value result = m_owner->val(tp);
                    m_owner->generateCall(ctors[0], { *this }, result);
                    return result;
                } else if (ctors.size() > 1) {
                    m_owner->logError(
                        "Construction of type '%s' with arguments (%s) is ambiguous",
                        tp->getFullName().c_str(),
                        m_type->getFullName().c_str()
                    );

                    for (u32 i = 0;i < ctors.size();i++) {
                        m_owner->logInfo(
                            "^ Could be '%s'",
                            ctors[i]->getFullName().c_str()
                        );
                    }

                    return Value();
                }
            }
        }

        m_owner->logError(
            "No conversion from type '%s' to '%s' is available",
            m_type->getFullName().c_str(),
            tp->getFullName().c_str()
        );

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
                m_type->getFullName().c_str(),
                rhs.m_type->getFullName().c_str()
            );

            for (u32 i = 0;i < opMethods.size();i++) {
                m_owner->logInfo("^ Could be: '%s'", opMethods[i]->getSignature()->getFullName().c_str());
            }

            return Value();
        } else if (opMethods.size() == 0) {
            m_owner->logError(
                "Type '%s' has no operator '[]' with arguments matching (%s)",
                m_type->getFullName().c_str(),
                rhs.m_type->getFullName().c_str()
            );

            return Value();
        }
        
        return m_owner->generateCall(strictMatch, { rhs }, *this);
    }

    Value Value::operator () (const Array<Value>& args, Value* self) const {
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
                argStr += argTps[i]->getFullName().c_str();
            }

            m_owner->logError(
                "Reference to operator '()' of type '%s' with arguments (%s) is ambiguous",
                m_type->getFullName().c_str(),
                argStr.c_str()
            );

            for (u32 i = 0;i < opMethods.size();i++) {
                m_owner->logInfo("^ Could be: '%s'", opMethods[i]->getSignature()->getFullName().c_str());
            }

            return Value();
        } else if (opMethods.size() == 0) {
            String argStr;
            for (u32 i = 0;i < argTps.size();i++) {
                if (i > 0) argStr += ", ";
                argStr += argTps[i]->getFullName().c_str();
            }

            m_owner->logError(
                "Type '%s' has no operator '()' with arguments matching (%s)",
                m_type->getFullName().c_str(),
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
                m_type->getFullName().c_str()
            );

            for (u32 i = 0;i < opMethods.size();i++) {
                m_owner->logInfo("^ Could be: '%s'", opMethods[i]->getSignature()->getFullName().c_str());
            }

            return Value();
        } else if (opMethods.size() == 0) {
            m_owner->logError(
                "Type '%s' has no operator '()' with arguments matching ()",
                m_type->getFullName().c_str()
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
        if (m_isLabel) {
            const String& name = m_owner->getLabelName(label_id(m_imm.u));
            if (name.size() > 0) return String::Format("%s_%d", name.c_str(), m_imm.u);
            return String::Format("LABEL_%d", m_imm.u);
        }

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

        const String& name = getName();
        if (name.size() > 0) return name;

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
                m_type->getFullName().c_str(),
                rhs.m_type->getFullName().c_str()
            );

            for (u32 i = 0;i < opMethods.size();i++) {
                m_owner->logInfo("^ Could be: '%s'", opMethods[i]->getSignature()->getFullName().c_str());
            }

            return Value();
        } else if (opMethods.size() == 0) {
            m_owner->logError(
                "Type '%s' has no operator '%s' with arguments matching (%s)",
                m_type->getFullName().c_str(),
                overrideName,
                rhs.m_type->getFullName().c_str()
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
                m_type->getFullName().c_str()
            );

            for (u32 i = 0;i < opMethods.size();i++) {
                m_owner->logInfo("^ Could be: '%s'", opMethods[i]->getSignature()->getFullName().c_str());
            }

            return Value();
        } else if (opMethods.size() == 0) {
            m_owner->logError(
                "Type '%s' has no operator '%s' with arguments matching ()",
                m_type->getFullName().c_str(),
                overrideName
            );

            return Value();
        }
        
        return m_owner->generateCall(strictMatch, {}, *this);
    }
};