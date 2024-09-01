#include <codegen/Execute.h>
#include <codegen/CodeHolder.h>
#include <codegen/FunctionBuilder.h>
#include <bind/Function.h>
#include <bind/FunctionType.h>
#include <bind/DataType.h>
#include <bind/Registry.h>
#include <bind/ValuePointer.h>
#include <utils/Exception.h>
#include <utils/Array.hpp>

namespace codegen {
    TestExecuterCallHandler::TestExecuterCallHandler(CodeHolder* ch) : m_code(ch) {
    }

    void TestExecuterCallHandler::call(Function* target, void* retDest, void** args) {
        TestExecuter exe(m_code);

        exe.setReturnValuePointer(retDest);

        FunctionType* sig = target->getSignature();
        auto argInfo = sig->getArgs();

        u32 off = 0;
        if (sig->getThisType()) {
            exe.setThisPtr(args[0]);
            off++;
        }

        for (u32 i = 0;i < argInfo.size();i++) {
            const type_meta& ai = argInfo[i].type->getInfo();

            if (ai.is_primitive || ai.is_pointer) {
                if (ai.size == sizeof(u64)) exe.setArg(i, *(u64*)args[i + off]);
                else if (ai.size == sizeof(u32)) exe.setArg(i, *(u32*)args[i + off]);
                else if (ai.size == sizeof(u16)) exe.setArg(i, *(u16*)args[i + off]);
                else if (ai.size == sizeof(u8)) exe.setArg(i, *(u8*)args[i + off]);
            } else {
                exe.setArg(i, args[i + off]);
            }
        }

        exe.execute();
    }

    TestExecuter::TestExecuter(CodeHolder* ch)
        : m_code(ch), m_fb(ch->owner), m_func(m_fb->getFunction()), m_stack(nullptr), m_registers(nullptr),
          m_returnPtr(nullptr), m_stackOffset(0), m_instructionIdx(0)
    {
        u32 maxStackSize = 0;
        u32 maxRegister = 0;

        for (u32 c = 0;c < ch->code.size();c++) {
            Instruction& i = ch->code[c];
            if (i.op == OpCode::stack_alloc) {
                auto it = m_stackAddrs.find(i.operands[1].getImm());
                if (it != m_stackAddrs.end()) continue;

                m_stackAddrs.insert(std::pair<stack_id, u32>(i.operands[1].getImm(), maxStackSize));
                maxStackSize += (u32)i.operands[0].getImm();
            } else if (i.op == OpCode::label) {
                m_labelAddrs.insert(std::pair<label_id, i32>(i.operands[0].getImm(), i32(c + 1)));
            }

            auto& info = Instruction::Info(i.op);
            for (u32 o = 0;o < info.operandCount;o++) {
                if (i.operands[o].isReg()) {
                    if (i.operands[o].getRegisterId() > maxRegister) maxRegister = i.operands[o].getRegisterId();
                }
            }
        }

        m_stack = new u8[maxStackSize];
        memset(m_stack, 0, maxStackSize);

        m_registers = new u64[maxRegister + 1];
        memset(m_registers, 0, maxRegister * sizeof(u64));
    }

    TestExecuter::~TestExecuter() {
        if (m_stack) delete [] m_stack;
        m_stack = nullptr;

        if (m_registers) delete [] m_registers;
        m_registers = nullptr;
    }

    void TestExecuter::setArg(u32 index, bool  value) { setRegister(m_fb->getArg(index).getRegisterId(), value); }
    void TestExecuter::setArg(u32 index, u8    value) { setRegister(m_fb->getArg(index).getRegisterId(), value); }
    void TestExecuter::setArg(u32 index, u16   value) { setRegister(m_fb->getArg(index).getRegisterId(), value); }
    void TestExecuter::setArg(u32 index, u32   value) { setRegister(m_fb->getArg(index).getRegisterId(), value); }
    void TestExecuter::setArg(u32 index, u64   value) { setRegister(m_fb->getArg(index).getRegisterId(), value); }
    void TestExecuter::setArg(u32 index, i8    value) { setRegister(m_fb->getArg(index).getRegisterId(), value); }
    void TestExecuter::setArg(u32 index, i16   value) { setRegister(m_fb->getArg(index).getRegisterId(), value); }
    void TestExecuter::setArg(u32 index, i32   value) { setRegister(m_fb->getArg(index).getRegisterId(), value); }
    void TestExecuter::setArg(u32 index, i64   value) { setRegister(m_fb->getArg(index).getRegisterId(), value); }
    void TestExecuter::setArg(u32 index, f32   value) { setRegister(m_fb->getArg(index).getRegisterId(), value); }
    void TestExecuter::setArg(u32 index, f64   value) { setRegister(m_fb->getArg(index).getRegisterId(), value); }
    void TestExecuter::setArg(u32 index, void* value) { setRegister(m_fb->getArg(index).getRegisterId(), value); }
    void TestExecuter::setThisPtr(void* thisPtr) {
        if (!m_func->getSignature()->getThisType()) return;
        setRegister(m_fb->getThis().getRegisterId(), thisPtr);
    }
    void TestExecuter::setReturnValuePointer(void* retDest) { m_returnPtr = retDest; }

    void TestExecuter::execute() {
        for (m_instructionIdx = 0;m_instructionIdx < i32(m_code->code.size());m_instructionIdx++) {
            Instruction& i = m_code->code[m_instructionIdx];

            Value& op0 = i.operands[0];
            Value& op1 = i.operands[1];
            Value& op2 = i.operands[2];
            DataType* tp0 = op0.getType();
            DataType* tp1 = op1.getType();
            DataType* tp2 = op2.getType();
            const type_meta& ti0 = tp0->getInfo();
            const type_meta& ti1 = tp1->getInfo();
            const type_meta& ti2 = tp2->getInfo();
            #define reg0 m_registers[i.operands[0].getRegisterId()]
            #define reg1 m_registers[i.operands[1].getRegisterId()]
            #define reg2 m_registers[i.operands[2].getRegisterId()]
            #define imm0 i.operands[0].getImm()
            #define imm1 i.operands[1].getImm()
            #define imm2 i.operands[2].getImm()
            u64 v0 = op0.isImm() ? imm0.u : reg0;
            u64 v1 = op1.isImm() ? imm1.u : reg1;
            u64 v2 = op2.isImm() ? imm2.u : reg2;

            switch (i.op) {
                case OpCode::stack_ptr: {
                    reg0 = reinterpret_cast<u64>(m_stack + m_stackAddrs[stack_id(imm1.u)]);
                    break;
                }
                case OpCode::value_ptr: {
                    ValuePointer* vp = Registry::GetValue(imm1);
                    reg0 = reinterpret_cast<u64>(vp->getAddress());
                    break;
                }
                case OpCode::ret_ptr: {
                    reg0 = reinterpret_cast<u64>(m_returnPtr);
                    break;
                }
                case OpCode::resolve: {
                    reg0 = v1;
                    break;
                }
                case OpCode::load: {
                    void* ptr = (reinterpret_cast<u8*>(reg1) + imm2.u);
                    switch (ti0.size) {
                        case 1: { reg0 = *(u8*)ptr; break; }
                        case 2: { reg0 = *(u16*)ptr; break; }
                        case 4: { reg0 = *(u32*)ptr; break; }
                        case 8: { reg0 = *(u64*)ptr; break; }
                    }
                    break;
                }
                case OpCode::store: {
                    void* ptr = (reinterpret_cast<u8*>(reg1) + imm2.u);
                    switch (ti0.size) {
                        case 1: { *(u8*)ptr = u8(v0); break; }
                        case 2: { *(u16*)ptr = u16(v0); break; }
                        case 4: { *(u32*)ptr = u32(v0); break; }
                        case 8: { *(u64*)ptr = u64(v0); break; }
                    }
                    break;
                }
                case OpCode::jump: {
                    m_instructionIdx = m_labelAddrs[label_id(imm0.u)] - 1;
                    continue;
                }
                case OpCode::cvt: {
                    #pragma warning(push, 0)

                    auto& ai = ti1;
                    auto& bi = Registry::GetType(imm2)->getInfo();
                    void* a = &reg0;
                    u64 b = op1.isImm() ? imm1.u : reg1;

                    if (ai.is_floating_point) {
                        if (ai.size == sizeof(f32)) {
                            if (bi.is_floating_point) {
                                if (bi.size == sizeof(f32)) *((f32*)a) = *((f32*)&b);
                                else                        *((f32*)a) = *((f64*)&b);
                            } else if (bi.is_unsigned) {
                                switch (bi.size) {
                                    case sizeof(u8 ): { *((f32*)a) = *((u8*)&b); break; }
                                    case sizeof(u16): { *((f32*)a) = *((u16*)&b); break; }
                                    case sizeof(u32): { *((f32*)a) = *((u32*)&b); break; }
                                    case sizeof(u64): { *((f32*)a) = *((u64*)&b); break; }
                                }
                            } else {
                                switch (bi.size) {
                                    case sizeof(i8 ): { *((f32*)a) = *((i8*)&b); break; }
                                    case sizeof(i16): { *((f32*)a) = *((i16*)&b); break; }
                                    case sizeof(i32): { *((f32*)a) = *((i32*)&b); break; }
                                    case sizeof(i64): { *((f32*)a) = *((i64*)&b); break; }
                                }
                            }
                        } else {
                            if (bi.is_floating_point) {
                                if (bi.size == sizeof(f32)) *((f64*)a) = *((f32*)&b);
                                else *((f64*)a) = *((f64*)&b);
                            } else if (bi.is_unsigned) {
                                switch (bi.size) {
                                    case sizeof(u8 ): { *((f64*)a) = *((u8*)&b); break; }
                                    case sizeof(u16): { *((f64*)a) = *((u16*)&b); break; }
                                    case sizeof(u32): { *((f64*)a) = *((u32*)&b); break; }
                                    case sizeof(u64): { *((f64*)a) = *((u64*)&b); break; }
                                }
                            } else {
                                switch (bi.size) {
                                    case sizeof(i8 ): { *((f64*)a) = *((i8*)&b); break; }
                                    case sizeof(i16): { *((f64*)a) = *((i16*)&b); break; }
                                    case sizeof(i32): { *((f64*)a) = *((i32*)&b); break; }
                                    case sizeof(i64): { *((f64*)a) = *((i64*)&b); break; }
                                }
                            }
                        }
                    } else if (ai.is_unsigned) {
                        switch (ai.size) {
                            case sizeof(u8): {
                                if (bi.is_floating_point) {
                                    if (bi.size == sizeof(f32)) *((u8*)a) = *((f32*)&b);
                                    else                        *((u8*)a) = *((f64*)&b);
                                } else if (bi.is_unsigned) {
                                    switch (bi.size) {
                                        case sizeof(u8 ): { *((u8*)a) = *((u8*)&b); break; }
                                        case sizeof(u16): { *((u8*)a) = *((u16*)&b); break; }
                                        case sizeof(u32): { *((u8*)a) = *((u32*)&b); break; }
                                        case sizeof(u64): { *((u8*)a) = *((u64*)&b); break; }
                                    }
                                } else {
                                    switch (bi.size) {
                                        case sizeof(i8 ): { *((u8*)a) = *((i8*)&b); break; }
                                        case sizeof(i16): { *((u8*)a) = *((i16*)&b); break; }
                                        case sizeof(i32): { *((u8*)a) = *((i32*)&b); break; }
                                        case sizeof(i64): { *((u8*)a) = *((i64*)&b); break; }
                                    }
                                }
                                break;
                            }
                            case sizeof(u16): {
                                if (bi.is_floating_point) {
                                    if (bi.size == sizeof(f32)) *((u16*)a) = *((f32*)&b);
                                    else                        *((u16*)a) = *((f64*)&b);
                                } else if (bi.is_unsigned) {
                                    switch (bi.size) {
                                        case sizeof(u8 ): { *((u16*)a) = *((u8*)&b); break; }
                                        case sizeof(u16): { *((u16*)a) = *((u16*)&b); break; }
                                        case sizeof(u32): { *((u16*)a) = *((u32*)&b); break; }
                                        case sizeof(u64): { *((u16*)a) = *((u64*)&b); break; }
                                    }
                                } else {
                                    switch (bi.size) {
                                        case sizeof(i8 ): { *((u16*)a) = *((i8*)&b); break; }
                                        case sizeof(i16): { *((u16*)a) = *((i16*)&b); break; }
                                        case sizeof(i32): { *((u16*)a) = *((i32*)&b); break; }
                                        case sizeof(i64): { *((u16*)a) = *((i64*)&b); break; }
                                    }
                                }
                                break;
                            }
                            case sizeof(u32): {
                                if (bi.is_floating_point) {
                                    if (bi.size == sizeof(f32)) *((u32*)a) = *((f32*)&b);
                                    else                        *((u32*)a) = *((f64*)&b);
                                } else if (bi.is_unsigned) {
                                    switch (bi.size) {
                                        case sizeof(u8 ): { *((u32*)a) = *((u8*)&b); break; }
                                        case sizeof(u16): { *((u32*)a) = *((u16*)&b); break; }
                                        case sizeof(u32): { *((u32*)a) = *((u32*)&b); break; }
                                        case sizeof(u64): { *((u32*)a) = *((u64*)&b); break; }
                                    }
                                } else {
                                    switch (bi.size) {
                                        case sizeof(i8 ): { *((u32*)a) = *((i8*)&b); break; }
                                        case sizeof(i16): { *((u32*)a) = *((i16*)&b); break; }
                                        case sizeof(i32): { *((u32*)a) = *((i32*)&b); break; }
                                        case sizeof(i64): { *((u32*)a) = *((i64*)&b); break; }
                                    }
                                }
                                break;
                            }
                            case sizeof(u64): {
                                if (bi.is_floating_point) {
                                    if (bi.size == sizeof(f32)) *((u64*)a) = *((f32*)&b);
                                    else                        *((u64*)a) = *((f64*)&b);
                                } else if (bi.is_unsigned) {
                                    switch (bi.size) {
                                        case sizeof(u8 ): { *((u64*)a) = *((u8*)&b); break; }
                                        case sizeof(u16): { *((u64*)a) = *((u16*)&b); break; }
                                        case sizeof(u32): { *((u64*)a) = *((u32*)&b); break; }
                                        case sizeof(u64): { *((u64*)a) = *((u64*)&b); break; }
                                    }
                                } else {
                                    switch (bi.size) {
                                        case sizeof(i8 ): { *((u64*)a) = *((i8*)&b); break; }
                                        case sizeof(i16): { *((u64*)a) = *((i16*)&b); break; }
                                        case sizeof(i32): { *((u64*)a) = *((i32*)&b); break; }
                                        case sizeof(i64): { *((u64*)a) = *((i64*)&b); break; }
                                    }
                                }
                                break;
                            }
                        }
                    } else {
                        switch (ai.size) {
                            case sizeof(i8): {
                                if (bi.is_floating_point) {
                                    if (bi.size == sizeof(f32)) *((i8*)a) = *((f32*)&b);
                                    else                        *((i8*)a) = *((f64*)&b);
                                } else if (bi.is_unsigned) {
                                    switch (bi.size) {
                                        case sizeof(u8 ): { *((i8*)a) = *((u8*)&b); break; }
                                        case sizeof(u16): { *((i8*)a) = *((u16*)&b); break; }
                                        case sizeof(u32): { *((i8*)a) = *((u32*)&b); break; }
                                        case sizeof(u64): { *((i8*)a) = *((u64*)&b); break; }
                                    }
                                } else {
                                    switch (bi.size) {
                                        case sizeof(i8 ): { *((i8*)a) = *((i8*)&b); break; }
                                        case sizeof(i16): { *((i8*)a) = *((i16*)&b); break; }
                                        case sizeof(i32): { *((i8*)a) = *((i32*)&b); break; }
                                        case sizeof(i64): { *((i8*)a) = *((i64*)&b); break; }
                                    }
                                }
                                break;
                            }
                            case sizeof(i16): {
                                if (bi.is_floating_point) {
                                    if (bi.size == sizeof(f32)) *((i16*)a) = *((f32*)&b);
                                    else                        *((i16*)a) = *((f64*)&b);
                                } else if (bi.is_unsigned) {
                                    switch (bi.size) {
                                        case sizeof(u8 ): { *((i16*)a) = *((u8*)&b); break; }
                                        case sizeof(u16): { *((i16*)a) = *((u16*)&b); break; }
                                        case sizeof(u32): { *((i16*)a) = *((u32*)&b); break; }
                                        case sizeof(u64): { *((i16*)a) = *((u64*)&b); break; }
                                    }
                                } else {
                                    switch (bi.size) {
                                        case sizeof(i8 ): { *((i16*)a) = *((i8*)&b); break; }
                                        case sizeof(i16): { *((i16*)a) = *((i16*)&b); break; }
                                        case sizeof(i32): { *((i16*)a) = *((i32*)&b); break; }
                                        case sizeof(i64): { *((i16*)a) = *((i64*)&b); break; }
                                    }
                                }
                                break;
                            }
                            case sizeof(i32): {
                                if (bi.is_floating_point) {
                                    if (bi.size == sizeof(f32)) *((i32*)a) = *((f32*)&b);
                                    else                        *((i32*)a) = *((f64*)&b);
                                } else if (bi.is_unsigned) {
                                    switch (bi.size) {
                                        case sizeof(u8 ): { *((i32*)a) = *((u8*)&b); break; }
                                        case sizeof(u16): { *((i32*)a) = *((u16*)&b); break; }
                                        case sizeof(u32): { *((i32*)a) = *((u32*)&b); break; }
                                        case sizeof(u64): { *((i32*)a) = *((u64*)&b); break; }
                                    }
                                } else {
                                    switch (bi.size) {
                                        case sizeof(i8 ): { *((i32*)a) = *((i8*)&b); break; }
                                        case sizeof(i16): { *((i32*)a) = *((i16*)&b); break; }
                                        case sizeof(i32): { *((i32*)a) = *((i32*)&b); break; }
                                        case sizeof(i64): { *((i32*)a) = *((i64*)&b); break; }
                                    }
                                }
                                break;
                            }
                            case sizeof(i64): {
                                if (bi.is_floating_point) {
                                    if (bi.size == sizeof(f32)) *((i64*)a) = *((f32*)&b);
                                    else                        *((i64*)a) = *((f64*)&b);
                                } else if (bi.is_unsigned) {
                                    switch (bi.size) {
                                        case sizeof(u8 ): { *((i64*)a) = *((u8*)&b); break; }
                                        case sizeof(u16): { *((i64*)a) = *((u16*)&b); break; }
                                        case sizeof(u32): { *((i64*)a) = *((u32*)&b); break; }
                                        case sizeof(u64): { *((i64*)a) = *((u64*)&b); break; }
                                    }
                                } else {
                                    switch (bi.size) {
                                        case sizeof(i8 ): { *((i64*)a) = *((i8*)&b); break; }
                                        case sizeof(i16): { *((i64*)a) = *((i16*)&b); break; }
                                        case sizeof(i32): { *((i64*)a) = *((i32*)&b); break; }
                                        case sizeof(i64): { *((i64*)a) = *((i64*)&b); break; }
                                    }
                                }
                                break;
                            }
                        }
                    }
                    
                    #pragma warning(pop)
                    break;
                }
                case OpCode::param: {
                    m_nextCallParams.push(v0);
                    break;
                }
                case OpCode::call: {
                    if (op0.isImm()) {
                        Function* fn = (Function*)op0.getImm().p;
                        FunctionType* sig = fn->getSignature();
                        auto args = sig->getArgs();
                        void* outArgs[32];
                        void* retPtr = nullptr;

                        u32 argOffset = 0;
                        if (sig->getThisType()) {
                            outArgs[0] = &reg2;
                            argOffset = 1;
                        }

                        const type_meta& ri = sig->getReturnType()->getInfo();
                        if (ri.is_primitive || ri.is_pointer) retPtr = &reg1;
                        else retPtr = reinterpret_cast<void*>(reg1);

                        for (u32 i = 0;i < m_nextCallParams.size();i++) {
                            if (i >= args.size()) break;
                            outArgs[i + argOffset] = &m_nextCallParams[i];
                        }

                        fn->getCallHandler()->call(fn, retPtr, outArgs);
                    } else {
                        // todo: function values
                    }

                    m_nextCallParams.clear();
                    break;
                }
                case OpCode::ret: {
                    if (!op0.isEmpty()) {
                        switch (m_func->getSignature()->getReturnType()->getInfo().size) {
                            case 1: { *(u8*)m_returnPtr = u8(v0); break; }
                            case 2: { *(u16*)m_returnPtr = u16(v0); break; }
                            case 4: { *(u32*)m_returnPtr = u32(v0); break; }
                            case 8: { *(u64*)m_returnPtr = u64(v0); break; }
                        }
                    }

                    return;
                }
                case OpCode::branch: {
                    if (bool(reg0)) continue;
                    m_instructionIdx = m_labelAddrs[label_id(imm1.u)] - 1;
                    break;
                }
                case OpCode::_not: { reg0 = !v1; break; }
                case OpCode::inv: { reg0 = ~v1; break; }
                case OpCode::shl: { reg0 = v1 << v2; break; }
                case OpCode::shr: { reg0 = v1 >> v2; break; }
                case OpCode::land: { reg0 = v1 && v2; break; }
                case OpCode::band: { reg0 = v1 & v2; break; }
                case OpCode::lor: { reg0 = v1 || v2; break; }
                case OpCode::bor: { reg0 = v1 | v2; break; }
                case OpCode::_xor: { reg0 = v1 ^ v2; break; }
                case OpCode::assign: { reg0 = v1; break; }
                case OpCode::vset: {
                    break;
                }
                case OpCode::vadd: {
                    break;
                }
                case OpCode::vsub: {
                    break;
                }
                case OpCode::vmul: {
                    break;
                }
                case OpCode::vdiv: {
                    break;
                }
                case OpCode::vmod: {
                    break;
                }
                case OpCode::vneg: {
                    break;
                }
                case OpCode::vdot: {
                    break;
                }
                case OpCode::vmag: {
                    break;
                }
                case OpCode::vmagsq: {
                    break;
                }
                case OpCode::vnorm: {
                    break;
                }
                case OpCode::vcross: {
                    break;
                }
                case OpCode::iadd: { *((i64*)&reg0) = *((i64*)&v1) + *((i64*)&v2); break; }
                case OpCode::uadd: { *((u64*)&reg0) = *((u64*)&v1) + *((u64*)&v2); break; }
                case OpCode::fadd: { *((f32*)&reg0) = *((f32*)&v1) + *((f32*)&v2); break; }
                case OpCode::dadd: { *((f64*)&reg0) = *((f64*)&v1) + *((f64*)&v2); break; }
                case OpCode::isub: { *((i64*)&reg0) = *((i64*)&v1) - *((i64*)&v2); break; }
                case OpCode::usub: { *((u64*)&reg0) = *((u64*)&v1) - *((u64*)&v2); break; }
                case OpCode::fsub: { *((f32*)&reg0) = *((f32*)&v1) - *((f32*)&v2); break; }
                case OpCode::dsub: { *((f64*)&reg0) = *((f64*)&v1) - *((f64*)&v2); break; }
                case OpCode::imul: { *((i64*)&reg0) = *((i64*)&v1) * *((i64*)&v2); break; }
                case OpCode::umul: { *((u64*)&reg0) = *((u64*)&v1) * *((u64*)&v2); break; }
                case OpCode::fmul: { *((f32*)&reg0) = *((f32*)&v1) * *((f32*)&v2); break; }
                case OpCode::dmul: { *((f64*)&reg0) = *((f64*)&v1) * *((f64*)&v2); break; }
                case OpCode::idiv: { *((i64*)&reg0) = *((i64*)&v1) / *((i64*)&v2); break; }
                case OpCode::udiv: { *((u64*)&reg0) = *((u64*)&v1) / *((u64*)&v2); break; }
                case OpCode::fdiv: { *((f32*)&reg0) = *((f32*)&v1) / *((f32*)&v2); break; }
                case OpCode::ddiv: { *((f64*)&reg0) = *((f64*)&v1) / *((f64*)&v2); break; }
                case OpCode::imod: { *((i64*)&reg0) = *((i64*)&v1) % *((i64*)&v2); break; }
                case OpCode::umod: { *((u64*)&reg0) = *((u64*)&v1) % *((u64*)&v2); break; }
                case OpCode::fmod: { *((f32*)&reg0) = fmodf(*((f32*)&v1), *((f32*)&v2)); break; }
                case OpCode::dmod: { *((f64*)&reg0) = fmod (*((f64*)&v1), *((f64*)&v2)); break; }
                case OpCode::ineg: { *((i64*)&reg0) = -*((i64*)&reg0); break; }
                case OpCode::fneg: { *((f32*)&reg0) = -*((f32*)&reg0); break; }
                case OpCode::dneg: { *((f64*)&reg0) = -*((f64*)&reg0); break; }
                case OpCode::iinc: { (*((i64*)&reg0))++; break; }
                case OpCode::uinc: { (*((u64*)&reg0))++; break; }
                case OpCode::finc: { (*((f32*)&reg0))++; break; }
                case OpCode::dinc: { (*((f64*)&reg0))++; break; }
                case OpCode::idec: { (*((i64*)&reg0))--; break; }
                case OpCode::udec: { (*((u64*)&reg0))--; break; }
                case OpCode::fdec: { (*((f32*)&reg0))--; break; }
                case OpCode::ddec: { (*((f64*)&reg0))--; break; }
                case OpCode::ilt: { *((i64*)&reg0) = *((i64*)&v1) < *((i64*)&v2); break; }
                case OpCode::ult: { *((u64*)&reg0) = *((u64*)&v1) < *((u64*)&v2); break; }
                case OpCode::flt: { *((f32*)&reg0) = *((f32*)&v1) < *((f32*)&v2); break; }
                case OpCode::dlt: { *((f64*)&reg0) = *((f64*)&v1) < *((f64*)&v2); break; }
                case OpCode::ilte: { *((i64*)&reg0) = *((i64*)&v1) <= *((i64*)&v2); break; }
                case OpCode::ulte: { *((u64*)&reg0) = *((u64*)&v1) <= *((u64*)&v2); break; }
                case OpCode::flte: { *((f32*)&reg0) = *((f32*)&v1) <= *((f32*)&v2); break; }
                case OpCode::dlte: { *((f64*)&reg0) = *((f64*)&v1) <= *((f64*)&v2); break; }
                case OpCode::igt: { *((i64*)&reg0) = *((i64*)&v1) > *((i64*)&v2); break; }
                case OpCode::ugt: { *((u64*)&reg0) = *((u64*)&v1) > *((u64*)&v2); break; }
                case OpCode::fgt: { *((f32*)&reg0) = *((f32*)&v1) > *((f32*)&v2); break; }
                case OpCode::dgt: { *((f64*)&reg0) = *((f64*)&v1) > *((f64*)&v2); break; }
                case OpCode::igte: { *((i64*)&reg0) = *((i64*)&v1) >= *((i64*)&v2); break; }
                case OpCode::ugte: { *((u64*)&reg0) = *((u64*)&v1) >= *((u64*)&v2); break; }
                case OpCode::fgte: { *((f32*)&reg0) = *((f32*)&v1) >= *((f32*)&v2); break; }
                case OpCode::dgte: { *((f64*)&reg0) = *((f64*)&v1) >= *((f64*)&v2); break; }
                case OpCode::ieq: { *((i64*)&reg0) = *((i64*)&v1) == *((i64*)&v2); break; }
                case OpCode::ueq: { *((u64*)&reg0) = *((u64*)&v1) == *((u64*)&v2); break; }
                case OpCode::feq: { *((f32*)&reg0) = *((f32*)&v1) == *((f32*)&v2); break; }
                case OpCode::deq: { *((f64*)&reg0) = *((f64*)&v1) == *((f64*)&v2); break; }
                case OpCode::ineq: { *((i64*)&reg0) = *((i64*)&v1) != *((i64*)&v2); break; }
                case OpCode::uneq: { *((u64*)&reg0) = *((u64*)&v1) != *((u64*)&v2); break; }
                case OpCode::fneq: { *((f32*)&reg0) = *((f32*)&v1) != *((f32*)&v2); break; }
                case OpCode::dneq: { *((f64*)&reg0) = *((f64*)&v1) != *((f64*)&v2); break; }
                default: break;
            }
        }
    }
};