#include <codegen/optimize/ConstantFolding.h>
#include <codegen/CodeHolder.h>
#include <codegen/PostProcessGroup.h>
#include <codegen/IR.h>
#include <codegen/Value.h>
#include <codegen/FunctionBuilder.h>
#include <bind/DataType.h>
#include <bind/Registry.h>
#include <bind/Function.h>
#include <utils/Exception.h>

namespace codegen {
    template <typename T, typename S> T add(T a, S b) { return T(a + b); }
    template <typename T, typename S> T sub(T a, S b) { return T(a - b); }
    template <typename T, typename S> T mul(T a, S b) { return T(a * b); }
    template <typename T, typename S> T div(T a, S b) { return T(a / b); }
    template <typename T, typename S> T mod(T a, S b) {
        if constexpr (std::is_integral_v<T>) {
            if constexpr (std::is_integral_v<S>) return T(a % b);
            else return T(fmod(a, b));
        }
        else return T(fmod(a, b));
    }
    
    #pragma warning(push, 0)
    template <typename T, typename S> bool lt(T a, S b) { return a < b; }
    template <typename T, typename S> bool lte(T a, S b) { return a <= b; }
    template <typename T, typename S> bool gt(T a, S b) { return a > b; }
    template <typename T, typename S> bool gte(T a, S b) { return a >= b; }
    #pragma warning(pop)

    template <typename T, typename S> bool eq(T a, S b) { return a == b; }
    template <typename T, typename S> bool neq(T a, S b) { return a != b; }
    template <typename T> T neg(T a) { return -a; }
    template <typename T> bool not_(T a) { return !a; }
    template <typename T> T inv(T a) {
        if constexpr (std::is_integral_v<T>) return ~a;
        else if constexpr (std::is_same_v<T, f64>) { u64 t = ~(*(u64*)&a); return *(f64*)&t; }
        else { u32 t = ~(*(u32*)&a); return *(f32*)&t; }
    }
    template <typename T, typename S> T shl(T a, S b) {
        if constexpr (std::is_integral_v<T>) return a << u32(b);
        else if constexpr (std::is_same_v<T, f64>) { u64 t = (*(u64*)&a) << u32(b); return *(f64*)&t; }
        else { u32 t = (*(u32*)&a) << u32(b); return *(f32*)&t; }
    }
    template <typename T, typename S> T shr(T a, S b) {
        if constexpr (std::is_integral_v<T>) return a >> u32(b);
        else if constexpr (std::is_same_v<T, f64>) { u64 t = (*(u64*)&a) >> u32(b); return *(f64*)&t; }
        else { u32 t = (*(u32*)&a) >> u32(b); return *(f32*)&t; }
    }
    template <typename T, typename S> bool land(T a, S b) { return a && b; }
    template <typename T, typename S> T band(T a, S b) {
        if constexpr (std::is_integral_v<T>) {
            if constexpr (std::is_integral_v<S>) return T(a & b);
            else if constexpr (std::is_same_v<T, f64>) return T(a & (*(u64*)&b));
            else return T(a & (*(u32*)&b));
        } else {
            if constexpr (std::is_integral_v<S>) return T((*(u64*)&a) & b);
            else if constexpr (std::is_same_v<T, f64>) return T((*(u64*)&a) & (*(u64*)&b));
            else return T((*(u32*)&a) & (*(u32*)&b));
        }
    }
    template <typename T, typename S> bool lor(T a, S b) { return a || b; }
    template <typename T, typename S> T bor(T a, S b) {
        if constexpr (std::is_integral_v<T>) {
            if constexpr (std::is_integral_v<S>) return T(a | b);
            else if constexpr (std::is_same_v<T, f64>) return T(a | (*(u64*)&b));
            else return T(a | (*(u32*)&b));
        } else {
            if constexpr (std::is_integral_v<S>) return T((*(u64*)&a) | b);
            else if constexpr (std::is_same_v<T, f64>) return T((*(u64*)&a) | (*(u64*)&b));
            else return T((*(u32*)&a) | (*(u32*)&b));
        }
    }
    template <typename T, typename S> T xor_(T a, S b) {
        if constexpr (std::is_integral_v<T>) {
            if constexpr (std::is_integral_v<S>) return a ^ b;
            else if constexpr (std::is_same_v<T, f64>) return a ^ (*(u64*)&b);
            else return a ^ (*(u32*)&b);
        } else {
            if constexpr (std::is_integral_v<S>) return T((*(u64*)&a) ^ b);
            else if constexpr (std::is_same_v<T, f64>) return T((*(u64*)&a) ^ (*(u64*)&b));
            else return T((*(u32*)&a) ^ (*(u32*)&b));
        }
    }
    template <typename T, typename S> T cvt(S v) {
        return T(v);
    }

    ConstantFoldingStep::ConstantFoldingStep() : IPostProcessStep() {
    }

    ConstantFoldingStep::~ConstantFoldingStep() {
    }

    bool ConstantFoldingStep::execute(CodeHolder* ch, u32 mask) {
        IWithLogging* log = ch->owner;
        bool didChange = false;

        log->logDebug("ConstantFoldingStep: Analyzing %s", ch->owner->getFunction()->getSymbolName().c_str());

        for (address c = 0;c < ch->code.size();c++) {
            Instruction& i = ch->code[c];
            const auto& info = Instruction::Info(i.op);
            bool foundMatch = false;

            Value result;

            if (info.operandCount == 3 && info.assignsOperandIndex == 0) {
                Value& a = i.operands[1];
                Value& b = i.operands[2];
                if (!a.isImm() || !b.isImm()) continue;

                #define doOp(func)                                                                                                \
                    const auto& ai = a.getType()->getInfo();                                                                      \
                    const auto& bi = b.getType()->getInfo();                                                                      \
                    if (ai.is_floating_point) {                                                                                   \
                        if (ai.size == sizeof(f32)) {                                                                             \
                            if (bi.is_floating_point) {                                                                           \
                                if (bi.size == sizeof(f32)) result.reset(ch->owner->val(func<f32, f32>(a.getImm(), b.getImm()))); \
                                else                        result.reset(ch->owner->val(func<f32, f64>(a.getImm(), b.getImm()))); \
                            } else if (bi.is_unsigned)      result.reset(ch->owner->val(func<f32, u64>(a.getImm(), b.getImm()))); \
                            else                            result.reset(ch->owner->val(func<f32, i64>(a.getImm(), b.getImm()))); \
                        } else {                                                                                                  \
                            if (bi.is_floating_point) {                                                                           \
                                if (bi.size == sizeof(f32)) result.reset(ch->owner->val(func<f64, f32>(a.getImm(), b.getImm()))); \
                                else                        result.reset(ch->owner->val(func<f64, f64>(a.getImm(), b.getImm()))); \
                            } else if (bi.is_unsigned)      result.reset(ch->owner->val(func<f64, u64>(a.getImm(), b.getImm()))); \
                            else                            result.reset(ch->owner->val(func<f64, i64>(a.getImm(), b.getImm()))); \
                        }                                                                                                         \
                    } else if (ai.is_unsigned) {                                                                                  \
                        if (bi.is_floating_point) {                                                                               \
                            if (bi.size == sizeof(f32))     result.reset(ch->owner->val(func<u64, f32>(a.getImm(), b.getImm()))); \
                            else                            result.reset(ch->owner->val(func<u64, f64>(a.getImm(), b.getImm()))); \
                        } else if (bi.is_unsigned)          result.reset(ch->owner->val(func<u64, u64>(a.getImm(), b.getImm()))); \
                        else                                result.reset(ch->owner->val(func<u64, i64>(a.getImm(), b.getImm()))); \
                    } else {                                                                                                      \
                        if (bi.is_floating_point) {                                                                               \
                            if (bi.size == sizeof(f32))     result.reset(ch->owner->val(func<i64, f32>(a.getImm(), b.getImm()))); \
                            else                            result.reset(ch->owner->val(func<i64, f64>(a.getImm(), b.getImm()))); \
                        } else if (bi.is_unsigned)          result.reset(ch->owner->val(func<i64, u64>(a.getImm(), b.getImm()))); \
                        else                                result.reset(ch->owner->val(func<i64, i64>(a.getImm(), b.getImm()))); \
                    }

                switch (i.op) {
                    case OpCode::cvt: {
                        DataType* destTp = Registry::GetType(b.getImm());
                        if (!destTp) throw Exception("ConstantFoldingStep::execute - invlaid type id specified to cvt instruction");

                        auto ai = a.getType()->getInfo();
                        auto bi = destTp->getInfo();

                        if (ai.is_floating_point) {
                            if (ai.size == sizeof(f32)) {
                                if (bi.is_floating_point) {
                                    if (bi.size == sizeof(f32)) result.reset(b);
                                    else                        result.reset(ch->owner->val(cvt<f32, f64>(b.getImm())));
                                } else if (bi.is_unsigned) {
                                    switch (bi.size) {
                                        case sizeof(u8 ): { result.reset(ch->owner->val(cvt<f32, u8 >(b.getImm()))); break; }
                                        case sizeof(u16): { result.reset(ch->owner->val(cvt<f32, u16>(b.getImm()))); break; }
                                        case sizeof(u32): { result.reset(ch->owner->val(cvt<f32, u32>(b.getImm()))); break; }
                                        case sizeof(u64): { result.reset(ch->owner->val(cvt<f32, u64>(b.getImm()))); break; }
                                    }
                                } else {
                                    switch (bi.size) {
                                        case sizeof(i8 ): { result.reset(ch->owner->val(cvt<f32, i8 >(b.getImm()))); break; }
                                        case sizeof(i16): { result.reset(ch->owner->val(cvt<f32, i16>(b.getImm()))); break; }
                                        case sizeof(i32): { result.reset(ch->owner->val(cvt<f32, i32>(b.getImm()))); break; }
                                        case sizeof(i64): { result.reset(ch->owner->val(cvt<f32, i64>(b.getImm()))); break; }
                                    }
                                }
                            } else {
                                if (bi.is_floating_point) {
                                    if (bi.size == sizeof(f32)) result.reset(ch->owner->val(cvt<f64, f32>(b.getImm())));
                                    else                        result.reset(b);
                                } else if (bi.is_unsigned) {
                                    switch (bi.size) {
                                        case sizeof(u8 ): { result.reset(ch->owner->val(cvt<f64, u8 >(b.getImm()))); break; }
                                        case sizeof(u16): { result.reset(ch->owner->val(cvt<f64, u16>(b.getImm()))); break; }
                                        case sizeof(u32): { result.reset(ch->owner->val(cvt<f64, u32>(b.getImm()))); break; }
                                        case sizeof(u64): { result.reset(ch->owner->val(cvt<f64, u64>(b.getImm()))); break; }
                                    }
                                } else {
                                    switch (bi.size) {
                                        case sizeof(i8 ): { result.reset(ch->owner->val(cvt<f64, i8 >(b.getImm()))); break; }
                                        case sizeof(i16): { result.reset(ch->owner->val(cvt<f64, i16>(b.getImm()))); break; }
                                        case sizeof(i32): { result.reset(ch->owner->val(cvt<f64, i32>(b.getImm()))); break; }
                                        case sizeof(i64): { result.reset(ch->owner->val(cvt<f64, i64>(b.getImm()))); break; }
                                    }
                                }
                            }
                        } else if (ai.is_unsigned) {
                            switch (ai.size) {
                                case sizeof(u8): {
                                    if (bi.is_floating_point) {
                                        if (bi.size == sizeof(f32)) result.reset(ch->owner->val(cvt<u8, f32>(b.getImm())));
                                        else                        result.reset(ch->owner->val(cvt<u8, f64>(b.getImm())));
                                    } else if (bi.is_unsigned) {
                                        switch (bi.size) {
                                            case sizeof(u8 ): { result.reset(b); break; }
                                            case sizeof(u16): { result.reset(ch->owner->val(cvt<u8, u16>(b.getImm()))); break; }
                                            case sizeof(u32): { result.reset(ch->owner->val(cvt<u8, u32>(b.getImm()))); break; }
                                            case sizeof(u64): { result.reset(ch->owner->val(cvt<u8, u64>(b.getImm()))); break; }
                                        }
                                    } else {
                                        switch (bi.size) {
                                            case sizeof(i8 ): { result.reset(ch->owner->val(cvt<u8, i8 >(b.getImm()))); break; }
                                            case sizeof(i16): { result.reset(ch->owner->val(cvt<u8, i16>(b.getImm()))); break; }
                                            case sizeof(i32): { result.reset(ch->owner->val(cvt<u8, i32>(b.getImm()))); break; }
                                            case sizeof(i64): { result.reset(ch->owner->val(cvt<u8, i64>(b.getImm()))); break; }
                                        }
                                    }
                                    break;
                                }
                                case sizeof(u16): {
                                    if (bi.is_floating_point) {
                                        if (bi.size == sizeof(f32)) result.reset(ch->owner->val(cvt<u16, f32>(b.getImm())));
                                        else                        result.reset(ch->owner->val(cvt<u16, f64>(b.getImm())));
                                    } else if (bi.is_unsigned) {
                                        switch (bi.size) {
                                            case sizeof(u8 ): { result.reset(ch->owner->val(cvt<u16, u8 >(b.getImm()))); break; }
                                            case sizeof(u16): { result.reset(b); break; }
                                            case sizeof(u32): { result.reset(ch->owner->val(cvt<u16, u32>(b.getImm()))); break; }
                                            case sizeof(u64): { result.reset(ch->owner->val(cvt<u16, u64>(b.getImm()))); break; }
                                        }
                                    } else {
                                        switch (bi.size) {
                                            case sizeof(i8 ): { result.reset(ch->owner->val(cvt<u16, i8 >(b.getImm()))); break; }
                                            case sizeof(i16): { result.reset(ch->owner->val(cvt<u16, i16>(b.getImm()))); break; }
                                            case sizeof(i32): { result.reset(ch->owner->val(cvt<u16, i32>(b.getImm()))); break; }
                                            case sizeof(i64): { result.reset(ch->owner->val(cvt<u16, i64>(b.getImm()))); break; }
                                        }
                                    }
                                    break;
                                }
                                case sizeof(u32): {
                                    if (bi.is_floating_point) {
                                        if (bi.size == sizeof(f32)) result.reset(ch->owner->val(cvt<u32, f32>(b.getImm())));
                                        else                        result.reset(ch->owner->val(cvt<u32, f64>(b.getImm())));
                                    } else if (bi.is_unsigned) {
                                        switch (bi.size) {
                                            case sizeof(u8 ): { result.reset(ch->owner->val(cvt<u32, u8 >(b.getImm()))); break; }
                                            case sizeof(u16): { result.reset(ch->owner->val(cvt<u32, u16>(b.getImm()))); break; }
                                            case sizeof(u32): { result.reset(b); break; }
                                            case sizeof(u64): { result.reset(ch->owner->val(cvt<u32, u64>(b.getImm()))); break; }
                                        }
                                    } else {
                                        switch (bi.size) {
                                            case sizeof(i8 ): { result.reset(ch->owner->val(cvt<u32, i8 >(b.getImm()))); break; }
                                            case sizeof(i16): { result.reset(ch->owner->val(cvt<u32, i16>(b.getImm()))); break; }
                                            case sizeof(i32): { result.reset(ch->owner->val(cvt<u32, i32>(b.getImm()))); break; }
                                            case sizeof(i64): { result.reset(ch->owner->val(cvt<u32, i64>(b.getImm()))); break; }
                                        }
                                    }
                                    break;
                                }
                                case sizeof(u64): {
                                    if (bi.is_floating_point) {
                                        if (bi.size == sizeof(f32)) result.reset(ch->owner->val(cvt<u64, f32>(b.getImm())));
                                        else                        result.reset(ch->owner->val(cvt<u64, f64>(b.getImm())));
                                    } else if (bi.is_unsigned) {
                                        switch (bi.size) {
                                            case sizeof(u8 ): { result.reset(ch->owner->val(cvt<u64, u8 >(b.getImm()))); break; }
                                            case sizeof(u16): { result.reset(ch->owner->val(cvt<u64, u16>(b.getImm()))); break; }
                                            case sizeof(u32): { result.reset(ch->owner->val(cvt<u64, u32>(b.getImm()))); break; }
                                            case sizeof(u64): { result.reset(b); break; }
                                        }
                                    } else {
                                        switch (bi.size) {
                                            case sizeof(i8 ): { result.reset(ch->owner->val(cvt<u64, i8 >(b.getImm()))); break; }
                                            case sizeof(i16): { result.reset(ch->owner->val(cvt<u64, i16>(b.getImm()))); break; }
                                            case sizeof(i32): { result.reset(ch->owner->val(cvt<u64, i32>(b.getImm()))); break; }
                                            case sizeof(i64): { result.reset(ch->owner->val(cvt<u64, i64>(b.getImm()))); break; }
                                        }
                                    }
                                    break;
                                }
                            }
                        } else {
                            switch (ai.size) {
                                case sizeof(i8): {
                                    if (bi.is_floating_point) {
                                        if (bi.size == sizeof(f32)) result.reset(ch->owner->val(cvt<i8, f32>(b.getImm())));
                                        else                        result.reset(ch->owner->val(cvt<i8, f64>(b.getImm())));
                                    } else if (bi.is_unsigned) {
                                        switch (bi.size) {
                                            case sizeof(u8 ): { result.reset(ch->owner->val(cvt<i8, u8 >(b.getImm()))); break; }
                                            case sizeof(u16): { result.reset(ch->owner->val(cvt<i8, u16>(b.getImm()))); break; }
                                            case sizeof(u32): { result.reset(ch->owner->val(cvt<i8, u32>(b.getImm()))); break; }
                                            case sizeof(u64): { result.reset(ch->owner->val(cvt<i8, u64>(b.getImm()))); break; }
                                        }
                                    } else {
                                        switch (bi.size) {
                                            case sizeof(i8 ): { result.reset(b); break; }
                                            case sizeof(i16): { result.reset(ch->owner->val(cvt<i8, i16>(b.getImm()))); break; }
                                            case sizeof(i32): { result.reset(ch->owner->val(cvt<i8, i32>(b.getImm()))); break; }
                                            case sizeof(i64): { result.reset(ch->owner->val(cvt<i8, i64>(b.getImm()))); break; }
                                        }
                                    }
                                    break;
                                }
                                case sizeof(i16): {
                                    if (bi.is_floating_point) {
                                        if (bi.size == sizeof(f32)) result.reset(ch->owner->val(cvt<i16, f32>(b.getImm())));
                                        else                        result.reset(ch->owner->val(cvt<i16, f64>(b.getImm())));
                                    } else if (bi.is_unsigned) {
                                        switch (bi.size) {
                                            case sizeof(u8 ): { result.reset(ch->owner->val(cvt<i16, u8 >(b.getImm()))); break; }
                                            case sizeof(u16): { result.reset(ch->owner->val(cvt<i16, u16>(b.getImm()))); break; }
                                            case sizeof(u32): { result.reset(ch->owner->val(cvt<i16, u32>(b.getImm()))); break; }
                                            case sizeof(u64): { result.reset(ch->owner->val(cvt<i16, u64>(b.getImm()))); break; }
                                        }
                                    } else {
                                        switch (bi.size) {
                                            case sizeof(i8 ): { result.reset(ch->owner->val(cvt<i16, i8 >(b.getImm()))); break; }
                                            case sizeof(i16): { result.reset(b); break; }
                                            case sizeof(i32): { result.reset(ch->owner->val(cvt<i16, i32>(b.getImm()))); break; }
                                            case sizeof(i64): { result.reset(ch->owner->val(cvt<i16, i64>(b.getImm()))); break; }
                                        }
                                    }
                                    break;
                                }
                                case sizeof(i32): {
                                    if (bi.is_floating_point) {
                                        if (bi.size == sizeof(f32)) result.reset(ch->owner->val(cvt<i32, f32>(b.getImm())));
                                        else                        result.reset(ch->owner->val(cvt<i32, f64>(b.getImm())));
                                    } else if (bi.is_unsigned) {
                                        switch (bi.size) {
                                            case sizeof(u8 ): { result.reset(ch->owner->val(cvt<i32, u8 >(b.getImm()))); break; }
                                            case sizeof(u16): { result.reset(ch->owner->val(cvt<i32, u16>(b.getImm()))); break; }
                                            case sizeof(u32): { result.reset(ch->owner->val(cvt<i32, u32>(b.getImm()))); break; }
                                            case sizeof(u64): { result.reset(ch->owner->val(cvt<i32, u64>(b.getImm()))); break; }
                                        }
                                    } else {
                                        switch (bi.size) {
                                            case sizeof(i8 ): { result.reset(ch->owner->val(cvt<i32, i8 >(b.getImm()))); break; }
                                            case sizeof(i16): { result.reset(ch->owner->val(cvt<i32, i16>(b.getImm()))); break; }
                                            case sizeof(i32): { result.reset(b); break; }
                                            case sizeof(i64): { result.reset(ch->owner->val(cvt<i32, i64>(b.getImm()))); break; }
                                        }
                                    }
                                    break;
                                }
                                case sizeof(i64): {
                                    if (bi.is_floating_point) {
                                        if (bi.size == sizeof(f32)) result.reset(ch->owner->val(cvt<i64, f32>(b.getImm())));
                                        else                        result.reset(ch->owner->val(cvt<i64, f64>(b.getImm())));
                                    } else if (bi.is_unsigned) {
                                        switch (bi.size) {
                                            case sizeof(u8 ): { result.reset(ch->owner->val(cvt<i64, u8 >(b.getImm()))); break; }
                                            case sizeof(u16): { result.reset(ch->owner->val(cvt<i64, u16>(b.getImm()))); break; }
                                            case sizeof(u32): { result.reset(ch->owner->val(cvt<i64, u32>(b.getImm()))); break; }
                                            case sizeof(u64): { result.reset(ch->owner->val(cvt<i64, u64>(b.getImm()))); break; }
                                        }
                                    } else {
                                        switch (bi.size) {
                                            case sizeof(i8 ): { result.reset(ch->owner->val(cvt<i64, i8 >(b.getImm()))); break; }
                                            case sizeof(i16): { result.reset(ch->owner->val(cvt<i64, i16>(b.getImm()))); break; }
                                            case sizeof(i32): { result.reset(ch->owner->val(cvt<i64, i32>(b.getImm()))); break; }
                                            case sizeof(i64): { result.reset(b); break; }
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                        
                        break;
                    }
                    case OpCode::iadd:
                    case OpCode::uadd:
                    case OpCode::fadd:
                    case OpCode::dadd: { doOp(add); break; }
                    case OpCode::isub:
                    case OpCode::usub:
                    case OpCode::fsub:
                    case OpCode::dsub: { doOp(sub); break; }
                    case OpCode::imul:
                    case OpCode::umul:
                    case OpCode::fmul:
                    case OpCode::dmul: { doOp(mul); break; }
                    case OpCode::idiv:
                    case OpCode::udiv:
                    case OpCode::fdiv:
                    case OpCode::ddiv: { doOp(div); break; }
                    case OpCode::imod:
                    case OpCode::umod:
                    case OpCode::fmod:
                    case OpCode::dmod: { doOp(mod); break; }
                    case OpCode::ilt:
                    case OpCode::ult:
                    case OpCode::flt:
                    case OpCode::dlt: { doOp(lt); break; }
                    case OpCode::ilte:
                    case OpCode::ulte:
                    case OpCode::flte:
                    case OpCode::dlte: { doOp(lte); break; }
                    case OpCode::igt:
                    case OpCode::ugt:
                    case OpCode::fgt:
                    case OpCode::dgt: { doOp(gt); break; }
                    case OpCode::igte:
                    case OpCode::ugte:
                    case OpCode::fgte:
                    case OpCode::dgte: { doOp(gte); break; }
                    case OpCode::ieq:
                    case OpCode::ueq:
                    case OpCode::feq:
                    case OpCode::deq: { doOp(eq); break; }
                    case OpCode::ineq:
                    case OpCode::uneq:
                    case OpCode::fneq:
                    case OpCode::dneq: { doOp(neq); break; }
                    case OpCode::shl: { doOp(shl); break; }
                    case OpCode::shr: { doOp(shr); break; }
                    case OpCode::land: { doOp(land); break; }
                    case OpCode::band: { doOp(band); break; }
                    case OpCode::lor: { doOp(lor); break; }
                    case OpCode::bor: { doOp(bor); break; }
                    case OpCode::_xor: { doOp(xor_); break; }
                    default: continue;
                }
                
                foundMatch = true;
            }

            if (info.operandCount == 2 && info.assignsOperandIndex == 0) {
                Value& v = i.operands[1];
                if (!v.isImm()) continue;

                const auto& ti = v.getType()->getInfo();

                switch (i.op) {
                    case OpCode::ineg:
                    case OpCode::fneg:
                    case OpCode::dneg: {
                        if (ti.is_floating_point) {
                            if (ti.size == sizeof(f32)) result.reset(ch->owner->val(neg<f32>(v.getImm())));
                            else                        result.reset(ch->owner->val(neg<f64>(v.getImm())));
                        } else if (ti.is_unsigned)      result.reset(ch->owner->val(   (u64) v.getImm()));
                        else                            result.reset(ch->owner->val(neg<i64>(v.getImm())));
                        break;
                    }
                    case OpCode::_not: {
                        if (ti.is_floating_point) {
                            if (ti.size == sizeof(f32)) result.reset(ch->owner->val(not_<f32>(v.getImm())));
                            else                        result.reset(ch->owner->val(not_<f64>(v.getImm())));
                        } else if (ti.is_unsigned)      result.reset(ch->owner->val(not_<u64>(v.getImm())));
                        else                            result.reset(ch->owner->val(not_<i64>(v.getImm())));
                        break;
                    }
                    case OpCode::inv: {
                        if (ti.is_floating_point) {
                            if (ti.size == sizeof(f32)) result.reset(ch->owner->val(inv<f32>(v.getImm())));
                            else                        result.reset(ch->owner->val(inv<f64>(v.getImm())));
                        } else if (ti.is_unsigned)      result.reset(ch->owner->val(inv<u64>(v.getImm())));
                        else                            result.reset(ch->owner->val(inv<i64>(v.getImm())));
                        break;
                    }
                    default: continue;
                }

                foundMatch = true;
            }

            if (!foundMatch) continue;

            log->logDebug("[%lu] %s <- constant-only operation", c, i.toString().c_str());

            i.op = OpCode::assign;
            i.operands[1].reset(result);
            i.operands[2].reset(Value());

            log->logDebug("^ [%lu] %s (updated)", c, i.toString().c_str());
        }

        if (didChange) getGroup()->setShouldRepeat(true);
        return false;
    }
};