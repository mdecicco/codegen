#include <codegen/Execute.h>
#include <codegen/CodeHolder.h>
#include <codegen/FunctionBuilder.h>
#include <bind/Function.h>
#include <bind/FunctionType.h>
#include <bind/PointerType.h>
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

    template <typename T>
    inline void vcross(void* result, void* a, void* b) {
        constexpr u32 X = 0;
        constexpr u32 Y = 1;
        constexpr u32 Z = 2;
        T x = ((T*)a)[Y] * ((T*)b)[Z] - ((T*)a)[Z] * ((T*)b)[Y];
        T y = ((T*)a)[Z] * ((T*)b)[X] - ((T*)a)[X] * ((T*)b)[Z];
        T z = ((T*)a)[X] * ((T*)b)[Y] - ((T*)a)[Y] * ((T*)b)[X];
        ((T*)result)[X] = x;
        ((T*)result)[Y] = y;
        ((T*)result)[Z] = z;
    }

    template <typename T>
    inline void vnorm(void* v, u8 compCnt) {
        T result = 0;
        for (u8 c = 0;c < compCnt;c++) result += ((T*)v)[c] * ((T*)v)[c];

        if constexpr (std::is_floating_point_v<T>) {
            T f = T(1.0) / std::sqrt(result);
            for (u8 c = 0;c < compCnt;c++) ((T*)v)[c] *= f;
        } else {
            f32 f = 1.0f / sqrtf(f32(result));
            for (u8 c = 0;c < compCnt;c++) ((T*)v)[c] = T(f32(((T*)v)[c]) * f);
        }
    }

    template <typename T> inline void vadd(void* a, u64   b, u8 compCnt) { for (u8 i = 0;i < compCnt;i++) ((T*)a)[i] += *((T*)&b); }
    template <typename T> inline void vadd(void* a, void* b, u8 compCnt) { for (u8 i = 0;i < compCnt;i++) ((T*)a)[i] += ((T*)b)[i]; }
    template <typename T> inline void vsub(void* a, u64   b, u8 compCnt) { for (u8 i = 0;i < compCnt;i++) ((T*)a)[i] -= *((T*)&b); }
    template <typename T> inline void vsub(void* a, void* b, u8 compCnt) { for (u8 i = 0;i < compCnt;i++) ((T*)a)[i] -= ((T*)b)[i]; }
    template <typename T> inline void vmul(void* a, u64   b, u8 compCnt) { for (u8 i = 0;i < compCnt;i++) ((T*)a)[i] *= *((T*)&b); }
    template <typename T> inline void vmul(void* a, void* b, u8 compCnt) { for (u8 i = 0;i < compCnt;i++) ((T*)a)[i] *= ((T*)b)[i]; }
    template <typename T> inline void vdiv(void* a, u64   b, u8 compCnt) { for (u8 i = 0;i < compCnt;i++) ((T*)a)[i] /= *((T*)&b); }
    template <typename T> inline void vdiv(void* a, void* b, u8 compCnt) { for (u8 i = 0;i < compCnt;i++) ((T*)a)[i] /= ((T*)b)[i]; }
    template <typename T> inline void vmod(void* a, u64   b, u8 compCnt) {
        if constexpr (std::is_floating_point_v<T>) {
            for (u8 i = 0;i < compCnt;i++) ((T*)a)[i] = std::fmod(((T*)a)[i], *((T*)&b));
        } else {
            for (u8 i = 0;i < compCnt;i++) ((T*)a)[i] %= *((T*)&b);
        }
    }
    template <typename T> inline void vmod(void* a, void* b, u8 compCnt) {
        if constexpr (std::is_floating_point_v<T>) {
            for (u8 i = 0;i < compCnt;i++) ((T*)a)[i] = std::fmod(((T*)a)[i], ((T*)b)[i]);
        } else {
            for (u8 i = 0;i < compCnt;i++) ((T*)a)[i] %= ((T*)b)[i];
        }
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
                    void* dest = reinterpret_cast<void*>(v0);
                    DataType* vtp = ((PointerType*)tp0)->getDestinationType();
                    const type_meta& vi = vtp->getInfo();
                    u8 compCnt = i.options.vset.componentCount;

                    if (ti1.is_pointer) {
                        void* src = reinterpret_cast<void*>(v1);
                        switch (vi.size) {
                            case 1: { for (u8 c = 0;c < compCnt;c++) ((u8* )dest)[c] = ((u8* )src)[c]; break; }
                            case 2: { for (u8 c = 0;c < compCnt;c++) ((u16*)dest)[c] = ((u16*)src)[c]; break; }
                            case 4: { for (u8 c = 0;c < compCnt;c++) ((u32*)dest)[c] = ((u32*)src)[c]; break; }
                            case 8: { for (u8 c = 0;c < compCnt;c++) ((u64*)dest)[c] = ((u64*)src)[c]; break; }
                        }
                    } else {
                        switch (vi.size) {
                            case 1: { for (u8 c = 0;c < compCnt;c++) ((u8* )dest)[c] = *((u8 *)&v1); break; }
                            case 2: { for (u8 c = 0;c < compCnt;c++) ((u16*)dest)[c] = *((u16*)&v1); break; }
                            case 4: { for (u8 c = 0;c < compCnt;c++) ((u32*)dest)[c] = *((u32*)&v1); break; }
                            case 8: { for (u8 c = 0;c < compCnt;c++) ((u64*)dest)[c] = *((u64*)&v1); break; }
                        }
                    }

                    break;
                }
                case OpCode::vadd: {
                    void* dest = reinterpret_cast<void*>(v0);
                    DataType* vtp = ((PointerType*)tp0)->getDestinationType();
                    const type_meta& vi = vtp->getInfo();
                    u8 compCnt = i.options.vadd.componentCount;

                    if (ti1.is_pointer) {
                        void* src = reinterpret_cast<void*>(v1);
                        if (vi.is_integral) {
                            if (vi.is_unsigned) {
                                switch (vi.size) {
                                    case 1: { vadd<u8 >(dest, src, compCnt); break; }
                                    case 2: { vadd<u16>(dest, src, compCnt); break; }
                                    case 4: { vadd<u32>(dest, src, compCnt); break; }
                                    case 8: { vadd<u64>(dest, src, compCnt); break; }
                                }
                            } else {
                                switch (vi.size) {
                                    case 1: { vadd<i8 >(dest, src, compCnt); break; }
                                    case 2: { vadd<i16>(dest, src, compCnt); break; }
                                    case 4: { vadd<i32>(dest, src, compCnt); break; }
                                    case 8: { vadd<i64>(dest, src, compCnt); break; }
                                }
                            }
                        } else {
                            switch (vi.size) {
                                case 4: { vadd<f32>(dest, src, compCnt); break; }
                                case 8: { vadd<f64>(dest, src, compCnt); break; }
                            }
                        }
                    } else {
                        if (vi.is_integral) {
                            if (vi.is_unsigned) {
                                switch (vi.size) {
                                    case 1: { vadd<u8 >(dest, v1, compCnt); break; }
                                    case 2: { vadd<u16>(dest, v1, compCnt); break; }
                                    case 4: { vadd<u32>(dest, v1, compCnt); break; }
                                    case 8: { vadd<u64>(dest, v1, compCnt); break; }
                                }
                            } else {
                                switch (vi.size) {
                                    case 1: { vadd<i8 >(dest, v1, compCnt); break; }
                                    case 2: { vadd<i16>(dest, v1, compCnt); break; }
                                    case 4: { vadd<i32>(dest, v1, compCnt); break; }
                                    case 8: { vadd<i64>(dest, v1, compCnt); break; }
                                }
                            }
                        } else {
                            switch (vi.size) {
                                case 4: { vadd<f32>(dest, v1, compCnt); break; }
                                case 8: { vadd<f64>(dest, v1, compCnt); break; }
                            }
                        }
                    }

                    break;
                }
                case OpCode::vsub: {
                    void* dest = reinterpret_cast<void*>(v0);
                    DataType* vtp = ((PointerType*)tp0)->getDestinationType();
                    const type_meta& vi = vtp->getInfo();
                    u8 compCnt = i.options.vsub.componentCount;

                    if (ti1.is_pointer) {
                        void* src = reinterpret_cast<void*>(v1);
                        if (vi.is_integral) {
                            if (vi.is_unsigned) {
                                switch (vi.size) {
                                    case 1: { vsub<u8 >(dest, src, compCnt); break; }
                                    case 2: { vsub<u16>(dest, src, compCnt); break; }
                                    case 4: { vsub<u32>(dest, src, compCnt); break; }
                                    case 8: { vsub<u64>(dest, src, compCnt); break; }
                                }
                            } else {
                                switch (vi.size) {
                                    case 1: { vsub<i8 >(dest, src, compCnt); break; }
                                    case 2: { vsub<i16>(dest, src, compCnt); break; }
                                    case 4: { vsub<i32>(dest, src, compCnt); break; }
                                    case 8: { vsub<i64>(dest, src, compCnt); break; }
                                }
                            }
                        } else {
                            switch (vi.size) {
                                case 4: { vsub<f32>(dest, src, compCnt); break; }
                                case 8: { vsub<f64>(dest, src, compCnt); break; }
                            }
                        }
                    } else {
                        if (vi.is_integral) {
                            if (vi.is_unsigned) {
                                switch (vi.size) {
                                    case 1: { vsub<u8 >(dest, v1, compCnt); break; }
                                    case 2: { vsub<u16>(dest, v1, compCnt); break; }
                                    case 4: { vsub<u32>(dest, v1, compCnt); break; }
                                    case 8: { vsub<u64>(dest, v1, compCnt); break; }
                                }
                            } else {
                                switch (vi.size) {
                                    case 1: { vsub<i8 >(dest, v1, compCnt); break; }
                                    case 2: { vsub<i16>(dest, v1, compCnt); break; }
                                    case 4: { vsub<i32>(dest, v1, compCnt); break; }
                                    case 8: { vsub<i64>(dest, v1, compCnt); break; }
                                }
                            }
                        } else {
                            switch (vi.size) {
                                case 4: { vsub<f32>(dest, v1, compCnt); break; }
                                case 8: { vsub<f64>(dest, v1, compCnt); break; }
                            }
                        }
                    }

                    break;
                }
                case OpCode::vmul: {
                    void* dest = reinterpret_cast<void*>(v0);
                    DataType* vtp = ((PointerType*)tp0)->getDestinationType();
                    const type_meta& vi = vtp->getInfo();
                    u8 compCnt = i.options.vmul.componentCount;

                    if (ti1.is_pointer) {
                        void* src = reinterpret_cast<void*>(v1);
                        if (vi.is_integral) {
                            if (vi.is_unsigned) {
                                switch (vi.size) {
                                    case 1: { vmul<u8 >(dest, src, compCnt); break; }
                                    case 2: { vmul<u16>(dest, src, compCnt); break; }
                                    case 4: { vmul<u32>(dest, src, compCnt); break; }
                                    case 8: { vmul<u64>(dest, src, compCnt); break; }
                                }
                            } else {
                                switch (vi.size) {
                                    case 1: { vmul<i8 >(dest, src, compCnt); break; }
                                    case 2: { vmul<i16>(dest, src, compCnt); break; }
                                    case 4: { vmul<i32>(dest, src, compCnt); break; }
                                    case 8: { vmul<i64>(dest, src, compCnt); break; }
                                }
                            }
                        } else {
                            switch (vi.size) {
                                case 4: { vmul<f32>(dest, src, compCnt); break; }
                                case 8: { vmul<f64>(dest, src, compCnt); break; }
                            }
                        }
                    } else {
                        if (vi.is_integral) {
                            if (vi.is_unsigned) {
                                switch (vi.size) {
                                    case 1: { vmul<u8 >(dest, v1, compCnt); break; }
                                    case 2: { vmul<u16>(dest, v1, compCnt); break; }
                                    case 4: { vmul<u32>(dest, v1, compCnt); break; }
                                    case 8: { vmul<u64>(dest, v1, compCnt); break; }
                                }
                            } else {
                                switch (vi.size) {
                                    case 1: { vmul<i8 >(dest, v1, compCnt); break; }
                                    case 2: { vmul<i16>(dest, v1, compCnt); break; }
                                    case 4: { vmul<i32>(dest, v1, compCnt); break; }
                                    case 8: { vmul<i64>(dest, v1, compCnt); break; }
                                }
                            }
                        } else {
                            switch (vi.size) {
                                case 4: { vmul<f32>(dest, v1, compCnt); break; }
                                case 8: { vmul<f64>(dest, v1, compCnt); break; }
                            }
                        }
                    }

                    break;
                }
                case OpCode::vdiv: {
                    void* dest = reinterpret_cast<void*>(v0);
                    DataType* vtp = ((PointerType*)tp0)->getDestinationType();
                    const type_meta& vi = vtp->getInfo();
                    u8 compCnt = i.options.vdiv.componentCount;

                    if (ti1.is_pointer) {
                        void* src = reinterpret_cast<void*>(v1);
                        if (vi.is_integral) {
                            if (vi.is_unsigned) {
                                switch (vi.size) {
                                    case 1: { vdiv<u8 >(dest, src, compCnt); break; }
                                    case 2: { vdiv<u16>(dest, src, compCnt); break; }
                                    case 4: { vdiv<u32>(dest, src, compCnt); break; }
                                    case 8: { vdiv<u64>(dest, src, compCnt); break; }
                                }
                            } else {
                                switch (vi.size) {
                                    case 1: { vdiv<i8 >(dest, src, compCnt); break; }
                                    case 2: { vdiv<i16>(dest, src, compCnt); break; }
                                    case 4: { vdiv<i32>(dest, src, compCnt); break; }
                                    case 8: { vdiv<i64>(dest, src, compCnt); break; }
                                }
                            }
                        } else {
                            switch (vi.size) {
                                case 4: { vdiv<f32>(dest, src, compCnt); break; }
                                case 8: { vdiv<f64>(dest, src, compCnt); break; }
                            }
                        }
                    } else {
                        if (vi.is_integral) {
                            if (vi.is_unsigned) {
                                switch (vi.size) {
                                    case 1: { vdiv<u8 >(dest, v1, compCnt); break; }
                                    case 2: { vdiv<u16>(dest, v1, compCnt); break; }
                                    case 4: { vdiv<u32>(dest, v1, compCnt); break; }
                                    case 8: { vdiv<u64>(dest, v1, compCnt); break; }
                                }
                            } else {
                                switch (vi.size) {
                                    case 1: { vdiv<i8 >(dest, v1, compCnt); break; }
                                    case 2: { vdiv<i16>(dest, v1, compCnt); break; }
                                    case 4: { vdiv<i32>(dest, v1, compCnt); break; }
                                    case 8: { vdiv<i64>(dest, v1, compCnt); break; }
                                }
                            }
                        } else {
                            switch (vi.size) {
                                case 4: { vdiv<f32>(dest, v1, compCnt); break; }
                                case 8: { vdiv<f64>(dest, v1, compCnt); break; }
                            }
                        }
                    }

                    break;
                }
                case OpCode::vmod: {
                    void* dest = reinterpret_cast<void*>(v0);
                    DataType* vtp = ((PointerType*)tp0)->getDestinationType();
                    const type_meta& vi = vtp->getInfo();
                    u8 compCnt = i.options.vmod.componentCount;

                    if (ti1.is_pointer) {
                        void* src = reinterpret_cast<void*>(v1);
                        if (vi.is_integral) {
                            if (vi.is_unsigned) {
                                switch (vi.size) {
                                    case 1: { vmod<u8>(dest, src, compCnt); break; }
                                    case 2: { vmod<u16>(dest, src, compCnt); break; }
                                    case 4: { vmod<u32>(dest, src, compCnt); break; }
                                    case 8: { vmod<u64>(dest, src, compCnt); break; }
                                }
                            } else {
                                switch (vi.size) {
                                    case 1: { vmod<i8>(dest, src, compCnt); break; }
                                    case 2: { vmod<i16>(dest, src, compCnt); break; }
                                    case 4: { vmod<i32>(dest, src, compCnt); break; }
                                    case 8: { vmod<i64>(dest, src, compCnt); break; }
                                }
                            }
                        } else {
                            switch (vi.size) {
                                case 4: { vmod<f32>(dest, src, compCnt); break; }
                                case 8: { vmod<f64>(dest, src, compCnt); break; }
                            }
                        }
                    } else {
                        if (vi.is_integral) {
                            if (vi.is_unsigned) {
                                switch (vi.size) {
                                    case 1: { vmod<u8>(dest, v1, compCnt); break; }
                                    case 2: { vmod<u16>(dest, v1, compCnt); break; }
                                    case 4: { vmod<u32>(dest, v1, compCnt); break; }
                                    case 8: { vmod<u64>(dest, v1, compCnt); break; }
                                }
                            } else {
                                switch (vi.size) {
                                    case 1: { vmod<i8>(dest, v1, compCnt); break; }
                                    case 2: { vmod<i16>(dest, v1, compCnt); break; }
                                    case 4: { vmod<i32>(dest, v1, compCnt); break; }
                                    case 8: { vmod<i64>(dest, v1, compCnt); break; }
                                }
                            }
                        } else {
                            switch (vi.size) {
                                case 4: { vmod<f32>(dest, v1, compCnt); break; }
                                case 8: { vmod<f64>(dest, v1, compCnt); break; }
                            }
                        }
                    }

                    break;
                }
                case OpCode::vneg: {
                    void* dest = reinterpret_cast<void*>(v0);
                    DataType* vtp = ((PointerType*)tp0)->getDestinationType();
                    const type_meta& vi = vtp->getInfo();
                    u8 compCnt = i.options.vneg.componentCount;

                    if (vi.is_integral) {
                        switch (vi.size) {
                            case 1: { for (u8 c = 0;c < compCnt;c++) ((i8 *)dest)[c] = -((i8 *)dest)[c]; break; }
                            case 2: { for (u8 c = 0;c < compCnt;c++) ((i16*)dest)[c] = -((i16*)dest)[c]; break; }
                            case 4: { for (u8 c = 0;c < compCnt;c++) ((i32*)dest)[c] = -((i32*)dest)[c]; break; }
                            case 8: { for (u8 c = 0;c < compCnt;c++) ((i64*)dest)[c] = -((i64*)dest)[c]; break; }
                        }
                    } else {
                        switch (vi.size) {
                            case 4: { for (u8 c = 0;c < compCnt;c++) ((f32*)dest)[c] = -((f32*)dest)[c]; break; }
                            case 8: { for (u8 c = 0;c < compCnt;c++) ((f64*)dest)[c] = -((f64*)dest)[c]; break; }
                        }
                    }
                    break;
                }
                case OpCode::vdot: {
                    void* a = reinterpret_cast<void*>(v1);
                    void* b = reinterpret_cast<void*>(v2);
                    DataType* vtp = ((PointerType*)tp0)->getDestinationType();
                    const type_meta& vi = vtp->getInfo();
                    u8 compCnt = i.options.vdot.componentCount;

                    if (vi.is_integral) {
                        if (vi.is_unsigned) {
                            switch (vi.size) {
                                case 1: {
                                    u8 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((u8*)a)[c] * ((u8*)b)[c];
                                    reg0 = result;
                                    break;
                                }
                                case 2: {
                                    u16 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((u16*)a)[c] * ((u16*)b)[c];
                                    reg0 = result;
                                    break;
                                }
                                case 4: {
                                    u32 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((u32*)a)[c] * ((u32*)b)[c];
                                    reg0 = result;
                                    break;
                                }
                                case 8: {
                                    u64 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((u64*)a)[c] * ((u64*)b)[c];
                                    reg0 = result;
                                    break;
                                }
                            }
                        } else {
                            switch (vi.size) {
                                case 1: {
                                    i8 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((i8*)a)[c] * ((i8*)b)[c];
                                    *((i8*)&reg0) = result;
                                    break;
                                }
                                case 2: {
                                    i16 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((i16*)a)[c] * ((i16*)b)[c];
                                    *((i16*)&reg0) = result;
                                    break;
                                }
                                case 4: {
                                    i32 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((i32*)a)[c] * ((i32*)b)[c];
                                    *((i32*)&reg0) = result;
                                    break;
                                }
                                case 8: {
                                    i64 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((i64*)a)[c] * ((i64*)b)[c];
                                    *((i64*)&reg0) = result;
                                    break;
                                }
                            }
                        }
                    } else {
                        switch (vi.size) {
                            case 4: {
                                f32 result = 0.0f;
                                for (u8 c = 0;c < compCnt;c++) result += ((f32*)a)[c] * ((f32*)b)[c];
                                *((f32*)&reg0) = result;
                                break;
                            }
                            case 8: {
                                f64 result = 0.0;
                                for (u8 c = 0;c < compCnt;c++) result += ((f64*)a)[c] * ((f64*)b)[c];
                                *((f64*)&reg0) = result;
                                break;
                            }
                        }
                    }

                    break;
                }
                case OpCode::vmag: {
                    void* a = reinterpret_cast<void*>(v1);
                    DataType* vtp = ((PointerType*)tp0)->getDestinationType();
                    const type_meta& vi = vtp->getInfo();
                    u8 compCnt = i.options.vmag.componentCount;

                    if (vi.is_integral) {
                        if (vi.is_unsigned) {
                            switch (vi.size) {
                                case 1: {
                                    u8 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((u8*)a)[c] * ((u8*)a)[c];
                                    reg0 = (u8)sqrtf((f32)result);
                                    break;
                                }
                                case 2: {
                                    u16 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((u16*)a)[c] * ((u16*)a)[c];
                                    reg0 = (u16)sqrtf((f32)result);
                                    break;
                                }
                                case 4: {
                                    u32 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((u32*)a)[c] * ((u32*)a)[c];
                                    reg0 = (u32)sqrtf((f32)result);
                                    break;
                                }
                                case 8: {
                                    u64 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((u64*)a)[c] * ((u64*)a)[c];
                                    reg0 = (u64)sqrtf((f32)result);
                                    break;
                                }
                            }
                        } else {
                            switch (vi.size) {
                                case 1: {
                                    i8 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((i8*)a)[c] * ((i8*)a)[c];
                                    *((i8*)&reg0) = (i8)sqrtf((i8)result);
                                    break;
                                }
                                case 2: {
                                    i16 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((i16*)a)[c] * ((i16*)a)[c];
                                    *((i16*)&reg0) = (i16)sqrtf((i16)result);
                                    break;
                                }
                                case 4: {
                                    i32 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((i32*)a)[c] * ((i32*)a)[c];
                                    *((i32*)&reg0) = (i32)sqrtf(f32((i32)result));
                                    break;
                                }
                                case 8: {
                                    i64 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((i64*)a)[c] * ((i64*)a)[c];
                                    *((i64*)&reg0) = (i64)sqrtf(f32((i64)result));
                                    break;
                                }
                            }
                        }
                    } else {
                        switch (vi.size) {
                            case 4: {
                                f32 result = 0.0f;
                                for (u8 c = 0;c < compCnt;c++) result += ((f32*)a)[c] * ((f32*)a)[c];
                                *((f32*)&reg0) = sqrtf(result);
                                break;
                            }
                            case 8: {
                                f64 result = 0.0;
                                for (u8 c = 0;c < compCnt;c++) result += ((f64*)a)[c] * ((f64*)a)[c];
                                *((f64*)&reg0) = sqrt(result);
                                break;
                            }
                        }
                    }

                    break;
                }
                case OpCode::vmagsq: {
                    void* a = reinterpret_cast<void*>(v1);
                    DataType* vtp = ((PointerType*)tp0)->getDestinationType();
                    const type_meta& vi = vtp->getInfo();
                    u8 compCnt = i.options.vmagsq.componentCount;

                    if (vi.is_integral) {
                        if (vi.is_unsigned) {
                            switch (vi.size) {
                                case 1: {
                                    u8 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((u8*)a)[c] * ((u8*)a)[c];
                                    reg0 = result;
                                    break;
                                }
                                case 2: {
                                    u16 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((u16*)a)[c] * ((u16*)a)[c];
                                    reg0 = result;
                                    break;
                                }
                                case 4: {
                                    u32 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((u32*)a)[c] * ((u32*)a)[c];
                                    reg0 = result;
                                    break;
                                }
                                case 8: {
                                    u64 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((u64*)a)[c] * ((u64*)a)[c];
                                    reg0 = result;
                                    break;
                                }
                            }
                        } else {
                            switch (vi.size) {
                                case 1: {
                                    i8 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((i8*)a)[c] * ((i8*)a)[c];
                                    *((i8*)&reg0) = result;
                                    break;
                                }
                                case 2: {
                                    i16 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((i16*)a)[c] * ((i16*)a)[c];
                                    *((i16*)&reg0) = result;
                                    break;
                                }
                                case 4: {
                                    i32 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((i32*)a)[c] * ((i32*)a)[c];
                                    *((i32*)&reg0) = result;
                                    break;
                                }
                                case 8: {
                                    i64 result = 0;
                                    for (u8 c = 0;c < compCnt;c++) result += ((i64*)a)[c] * ((i64*)a)[c];
                                    *((i64*)&reg0) = result;
                                    break;
                                }
                            }
                        }
                    } else {
                        switch (vi.size) {
                            case 4: {
                                f32 result = 0.0f;
                                for (u8 c = 0;c < compCnt;c++) result += ((f32*)a)[c] * ((f32*)a)[c];
                                *((f32*)&reg0) = result;
                                break;
                            }
                            case 8: {
                                f64 result = 0.0;
                                for (u8 c = 0;c < compCnt;c++) result += ((f64*)a)[c] * ((f64*)a)[c];
                                *((f64*)&reg0) = result;
                                break;
                            }
                        }
                    }

                    break;
                }
                case OpCode::vnorm: {
                    void* a = reinterpret_cast<void*>(v0);
                    DataType* vtp = ((PointerType*)tp0)->getDestinationType();
                    const type_meta& vi = vtp->getInfo();
                    u8 compCnt = i.options.vnorm.componentCount;

                    if (vi.is_integral) {
                        if (vi.is_unsigned) {
                            switch (vi.size) {
                                case 1: { vnorm<u8>(a, compCnt); break; }
                                case 2: { vnorm<u16>(a, compCnt); break; }
                                case 4: { vnorm<u32>(a, compCnt); break; }
                                case 8: { vnorm<u64>(a, compCnt); break; }
                            }
                        } else {
                            switch (vi.size) {
                                case 1: { vnorm<i8>(a, compCnt); break; }
                                case 2: { vnorm<i16>(a, compCnt); break; }
                                case 4: { vnorm<i32>(a, compCnt); break; }
                                case 8: { vnorm<i64>(a, compCnt); break; }
                            }
                        }
                    } else {
                        switch (vi.size) {
                            case 4: { vnorm<f32>(a, compCnt); break; }
                            case 8: { vnorm<f64>(a, compCnt); break; }
                        }
                    }

                    break;
                }
                case OpCode::vcross: {
                    void* r = reinterpret_cast<void*>(v0);
                    void* a = reinterpret_cast<void*>(v1);
                    void* b = reinterpret_cast<void*>(v2);
                    DataType* vtp = ((PointerType*)tp0)->getDestinationType();
                    const type_meta& vi = vtp->getInfo();
                    u8 compCnt = i.options.vcross.componentCount;

                    if (vi.is_integral) {
                        if (vi.is_unsigned) {
                            switch (vi.size) {
                                case 1: { vcross<u8>(r, a, b); break; }
                                case 2: { vcross<u16>(r, a, b); break; }
                                case 4: { vcross<u32>(r, a, b); break; }
                                case 8: { vcross<u64>(r, a, b); break; }
                            }
                        } else {
                            switch (vi.size) {
                                case 1: { vcross<i8>(r, a, b); break; }
                                case 2: { vcross<i16>(r, a, b); break; }
                                case 4: { vcross<i32>(r, a, b); break; }
                                case 8: { vcross<i64>(r, a, b); break; }
                            }
                        }
                    } else {
                        switch (vi.size) {
                            case 4: { vcross<f32>(r, a, b); break; }
                            case 8: { vcross<f64>(r, a, b); break; }
                        }
                    }

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