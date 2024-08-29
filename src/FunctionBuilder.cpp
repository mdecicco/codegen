#include <codegen/FunctionBuilder.h>
#include <bind/DataType.h>
#include <bind/FunctionType.h>
#include <bind/PointerType.h>
#include <bind/Function.h>
#include <bind/ValuePointer.h>
#include <utils/Exception.h>
#include <utils/Array.hpp>

namespace codegen {
    FunctionBuilder::FunctionBuilder(Function* func)
        : m_function(func), m_parent(nullptr), m_nextLabel(1), m_nextReg(1),
        m_nextAlloc(1), m_currentSrcLoc({ 0, 0, 0, 0, 0, 0, 0 }),
        m_validationEnabled(false), m_ownScope(this), m_currentScope(&m_ownScope)
    {
        m_strings.reserve(128);
        addString("");
        emitPrologue();
    }

    FunctionBuilder::FunctionBuilder(Function* func, FunctionBuilder* parent)
        : m_function(func), m_parent(parent), m_nextLabel(1), m_nextReg(1),
        m_nextAlloc(1), m_currentSrcLoc({ 0, 0, 0, 0, 0, 0, 0 }),
        m_validationEnabled(false), m_ownScope(this), m_currentScope(&m_ownScope)
    {
        m_strings.reserve(128);
        addString("");
        emitPrologue();
    }

    FunctionBuilder::~FunctionBuilder() {
    }

    InstructionRef FunctionBuilder::add(const Instruction& i) {
        m_code.push(i);
        m_srcMap.add(m_code.size() - 1, m_currentSrcLoc);
        return InstructionRef(this, m_code.size() - 1);
    }

    Value FunctionBuilder::ptrOffset(const Value& ptr, const Value& offset, DataType* destType) {
        auto oi = offset.m_type->getInfo();
        if (m_validationEnabled) {
            auto pi = ptr.m_type->getInfo();
            if (!pi.is_pointer) throw Exception("FunctionBuilder::ptrOffset - ptr should have a pointer type");
            if (destType && !destType->getInfo().is_pointer) {
                throw Exception("FunctionBuilder::ptrOffset - destType, if set, should be a pointer type");
            }

            if (!oi.is_integral) throw Exception("FunctionBuilder::ptrOffset - offset should have an integral type");
        }

        Value result = val(destType ? destType : ptr.m_type);

        if (offset.isImm()) {
            if (oi.is_unsigned) {
                Instruction i(OpCode::uadd);
                i.operands[0].reset(result);
                i.operands[1].reset(ptr);
                i.operands[2].reset(offset);
                add(i);
            } else {
                i64 ioff = offset.getImm();
                if (ioff > 0) {
                    Instruction i(OpCode::uadd);
                    i.operands[0].reset(result);
                    i.operands[1].reset(ptr);
                    i.operands[2].reset(offset);
                    add(i);
                } else {
                    Instruction i(OpCode::usub);
                    i.operands[0].reset(result);
                    i.operands[1].reset(ptr);
                    i.operands[2].reset(val(-ioff));
                    add(i);
                }
            }
        } else {
            if (oi.is_unsigned) {
                Instruction i(OpCode::uadd);
                i.operands[0].reset(result);
                i.operands[1].reset(ptr);
                i.operands[2].reset(offset);
                add(i);
            } else {
                generateIf(offset > val(0), [&]() {
                    Instruction i(OpCode::uadd);
                    i.operands[0].reset(result);
                    i.operands[1].reset(ptr);
                    i.operands[2].reset(offset);
                    add(i);
                }, [&]() {
                    Instruction i(OpCode::usub);
                    i.operands[0].reset(result);
                    i.operands[1].reset(ptr);
                    i.operands[2].reset(-offset);
                    add(i);
                });
            }
        }

        return result;
    }

    Value FunctionBuilder::ptrOffset(const Value& ptr, i64 offset, DataType* destType) {
        if (m_validationEnabled) {
            auto pi = ptr.m_type->getInfo();
            if (!pi.is_pointer) throw Exception("FunctionBuilder::ptrOffset - ptr should have a pointer type");
            if (destType && !destType->getInfo().is_pointer) {
                throw Exception("FunctionBuilder::ptrOffset - destType, if set, should be a pointer type");
            }
        }

        Value result = val(destType ? destType : ptr.m_type);

        if (offset > 0) {
            Instruction i(OpCode::uadd);
            i.operands[0].reset(result);
            i.operands[1].reset(ptr);
            i.operands[2].reset(val(offset));
            add(i);
        } else {
            Instruction i(OpCode::usub);
            i.operands[0].reset(result);
            i.operands[1].reset(ptr);
            i.operands[2].reset(val(-offset));
            add(i);
        }

        return result;
    }

    Value FunctionBuilder::generateCall(
        FunctionBuilder* func,
        const Array<Value>& args,
        const Value& selfPtr
    ) {
        return generateCall(func->getFunction(), args, selfPtr);
    }

    Value FunctionBuilder::generateCall(
        Function* func,
        const Array<Value>& args,
        const Value& selfPtr
    ) {
        FunctionType* sig = func->getSignature();
        auto argInfo = sig->getArgs();

        if (argInfo.size() != args.size()) {
            logError(
                "Incorrect number of arguments provided to function '%s'. Expected %u, got %u",
                func->getFullName().c_str(),
                argInfo.size(),
                args.size()
            );

            return Value();
        }

        for (u32 i = 0;i < args.size();i++) {
            Value arg = args[i].convertedTo(argInfo[i].type);
            param(arg);
        }

        Value result = val(sig->getReturnType());
        call(func, result, selfPtr);

        return result;
    }

    Value FunctionBuilder::generateCall(
        const Value& func,
        const Array<Value>& args,
        const Value& selfPtr
    ) {
        if (func.m_isImm) {
            return generateCall((Function*)func.m_imm.p, args, selfPtr);
        }

        FunctionType* sig = (FunctionType*)func.m_type;
        auto argInfo = sig->getArgs();

        if (argInfo.size() != args.size()) {
            logError(
                "Incorrect number of arguments provided to function. Expected %u, got %u",
                argInfo.size(),
                args.size()
            );

            return Value();
        }

        for (u32 i = 0;i < args.size();i++) {
            Value arg = args[i].convertedTo(argInfo[i].type);
            param(arg);
        }

        Value result = val(sig->getReturnType());
        call(func, result, selfPtr);

        return result;
    }

    void FunctionBuilder::generateConstruction(const Value& destPtr, const Array<Value>& args, AccessFlags accessMask) {
        if (!destPtr.m_type->getInfo().is_pointer) {
            throw Exception("FunctionBuilder::generateConstruction - destPtr should have a pointer type");
        }

        DataType* tp = ((PointerType*)destPtr.m_type)->getDestinationType();
        auto info = tp->getInfo();

        if (info.is_primitive || info.is_pointer) {
            if (args.size() == 0) return;
            if (args.size() == 1) {
                if (args[0].m_type->isConvertibleTo(tp)) {
                    store(args[0].convertedTo(tp), destPtr);
                    return;
                }

                Array<DataType*> argTps = args.map([](const Value& v) { return v.m_type; });
                Function* ctor = nullptr;
                Array<Function*> ctors = tp->findConstructors(argTps, false, accessMask, &ctor);

                if (ctors.size() == 0) {
                    logError(
                        "Value of type '%s' can not be initialized from a value of type '%s', or no constructor is accessible",
                        tp->getFullName().c_str(),
                        args[0].m_type->getFullName()
                    );
                } else if (ctors.size() == 1) {
                    generateCall(ctors[0], args, destPtr);
                } else if (ctor) {
                    generateCall(ctor, args, destPtr);
                } else {
                    String argStr;
                    for (u32 i = 0;i < args.size();i++) {
                        if (i > 0) argStr += ", ";
                        argStr += args[i].m_type->getFullName();
                    }

                    logError(
                        "Constructor for type '%s' with arguments (%s) is ambiguous",
                        tp->getFullName().c_str(),
                        argStr.c_str()
                    );

                    for (u32 i = 0;i < ctors.size();i++) {
                        logInfo("^ Could be '%s'", ctors[i]->getSignature()->getFullName().c_str());
                    }
                }

                return;
            }
        }

        Array<DataType*> argTps = args.map([](const Value& v) { return v.m_type; });
        Function* ctor = nullptr;
        Array<Function*> ctors = tp->findConstructors(argTps, false, accessMask, &ctor);

        if (ctors.size() == 0) {
            String argStr;
            for (u32 i = 0;i < args.size();i++) {
                if (i > 0) argStr += ", ";
                argStr += args[i].m_type->getFullName();
            }

            logError(
                "No constructor for type '%s' with arguments (%s) is accessible",
                tp->getFullName().c_str(),
                argStr.c_str()
            );
        } else if (ctors.size() == 1) {
            generateCall(ctors[0], args, destPtr);
        } else if (ctor) {
            generateCall(ctor, args, destPtr);
        } else {
            String argStr;
            for (u32 i = 0;i < args.size();i++) {
                if (i > 0) argStr += ", ";
                argStr += args[i].m_type->getFullName();
            }

            logError(
                "Constructor for type '%s' with arguments (%s) is ambiguous",
                tp->getFullName().c_str(),
                argStr.c_str()
            );

            for (u32 i = 0;i < ctors.size();i++) {
                logInfo("^ Could be '%s'", ctors[i]->getSignature()->getFullName().c_str());
            }
        }
    }

    void FunctionBuilder::generateReturn(const Value& val) {
        DataType* retTp = m_function->getSignature()->getReturnType();
        auto retInfo = retTp->getInfo();

        if (retInfo.size == 0 || retInfo.is_primitive) {
            m_currentScope->emitPreReturnInstructions();
            ret(val);
            return;
        }

        Value ptr = Value(m_nextReg++, this, retTp->getPointerType());
        retPtr(ptr);
        generateConstruction(ptr, { val });
        m_currentScope->emitPreReturnInstructions();
        ret();
    }

    Value FunctionBuilder::labelVal(label_id label) {
        Value ret(this, label);
        ret.m_isLabel = true;
        return ret;
    }

    Value FunctionBuilder::val(DataType* tp) { return Value(m_nextReg++, this, tp); }
    Value FunctionBuilder::val(ValuePointer* value) {
        Value ret = Value(m_nextReg++, this, (DataType*)value->getType()->getPointerType());
        valuePtr(ret, value);
        return ret;
    }
    Value FunctionBuilder::val(bool imm) { return Value(this, imm); }
    Value FunctionBuilder::val(u8   imm) { return Value(this, imm); }
    Value FunctionBuilder::val(u16  imm) { return Value(this, imm); }
    Value FunctionBuilder::val(u32  imm) { return Value(this, imm); }
    Value FunctionBuilder::val(u64  imm) { return Value(this, imm); }
    Value FunctionBuilder::val(i8   imm) { return Value(this, imm); }
    Value FunctionBuilder::val(i16  imm) { return Value(this, imm); }
    Value FunctionBuilder::val(i32  imm) { return Value(this, imm); }
    Value FunctionBuilder::val(i64  imm) { return Value(this, imm); }
    Value FunctionBuilder::val(f32  imm) { return Value(this, imm); }
    Value FunctionBuilder::val(f64  imm) { return Value(this, imm); }
    Value FunctionBuilder::val(ptr  imm) { return Value(this, imm); }

    Function* FunctionBuilder::getFunction() const {
        return m_function;
    }

    Array<Instruction>& FunctionBuilder::getCode() {
        return m_code;
    }

    const Array<Instruction>& FunctionBuilder::getCode() const {
        return m_code;
    }

    Value FunctionBuilder::getThis() const {
        return m_thisPtr;
    }

    Value FunctionBuilder::getArg(u32 index) const {
        if (index > m_args.size()) {
            if (m_validationEnabled) {
                throw Exception("FunctionBuilder::getArg - invalid argument index specified");
            }

            return Value();
        }

        return m_args[index];
    }

    Value FunctionBuilder::getRetPtr() {
        DataType* retTp = m_function->getSignature()->getReturnType();
        Value ptr = Value(m_nextReg++, this, retTp->getPointerType());
        retPtr(ptr);
        return ptr;
    }

    stack_id FunctionBuilder::getNextAllocId() const {
        return m_nextAlloc;
    }

    stack_id FunctionBuilder::reserveAllocId() {
        return m_nextAlloc++;
    }

    void FunctionBuilder::setCurrentSourceLocation(const SourceLocation& src) {
        m_currentSrcLoc = src;
    }

    const SourceMap* FunctionBuilder::getSourceMap() const {
        return &m_srcMap;
    }

    Scope* FunctionBuilder::getCurrentScope() const {
        return m_currentScope;
    }

    void FunctionBuilder::enableValidation() {
        m_validationEnabled = true;
    }
    
    void FunctionBuilder::setName(Value& v, const String& name) {
        v.m_nameStringId = addString(name);
        if (v.isReg()) {
            for (u32 i = 0;i < m_code.size();i++) {
                for (u32 o = 0;o < 3;o++) {
                    auto& op = m_code[i].operands[o];
                    if (op.isReg() && op.m_regId == v.m_regId) {
                        op.m_nameStringId = v.m_nameStringId;
                    }
                }
            }
        }
    }
    
    const String& FunctionBuilder::getString(i32 stringId) const {
        if (stringId < 0 || u32(stringId) > m_strings.size()) return m_strings[0];

        return m_strings[stringId];
    }
    
    const String& FunctionBuilder::getLabelName(label_id label) const {
        auto it = m_labelNameStringIds.find(label);
        if (it == m_labelNameStringIds.end()) return m_strings[0];
        return m_strings[it->second];
    }
    
    void FunctionBuilder::emitPrologue() {
        FunctionType* sig = m_function->getSignature();

        DataType* thisTp = sig->getThisType();
        if (thisTp) {
            m_thisPtr.reset(Value(m_nextReg++, this, thisTp));
            m_thisPtr.setName("this");
            thisPtr(m_thisPtr);
        }

        auto args = sig->getArgs();
        for (u32 i = 0;i < args.size();i++) {
            Value arg = Value(m_nextReg++, this, args[i].type);
            arg.setName(String::Format("param_%d", i));
            m_args.push(arg);
            argument(arg, i);
        }
    }
    
    void FunctionBuilder::enterScope(Scope* s) {
        m_currentScope = s;
    }

    void FunctionBuilder::exitScope(Scope* s) {
        m_currentScope = m_currentScope->m_parent;
    }

    i32 FunctionBuilder::addString(const String& str) {
        m_strings.push(str);
        return i32(m_strings.size() - 1);
    }
};