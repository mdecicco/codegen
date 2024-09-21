#include "Common.h"
#include <codegen/TestBackend.h>

namespace conversion {
    struct not_convertible {
        i32 dummy;
    };

    struct convertible_via_ctor {
        i32 dummy;
    };

    struct dest_type {
        i32 dummy;
        dest_type() { dummy = 0; }
        dest_type(const convertible_via_ctor& rhs) { dummy = rhs.dummy; }
    };

    struct convertible_via_cast {
        i32 dummy;

        operator dest_type() const {
            return dest_type({ dummy });
        }
    };

    void setupConversionTest() {
        setupTest();

        auto dt = type<dest_type>("dest");
        auto nc = type<not_convertible>("not_convertible");
        auto c0 = type<convertible_via_ctor>("convertible_via_ctor");
        auto c1 = type<convertible_via_cast>("convertible_via_cast");

        dt.ctor();
        dt.ctor<const convertible_via_ctor&>();
        dt.prop("_", &dest_type::dummy);
        c1.opCast<dest_type>();
    }

    void testObjectConversion() {
        setupConversionTest();

        SECTION("When no conversion is available, no value should be returned and no code emitted") {
            Function fn("test", Registry::Signature<void>(), Registry::GlobalNamespace());
            FunctionBuilder fb(&fn);

            Value src = fb.val<not_convertible>();
            u32 afterDecl = fb.getCode().size();

            Value dst = src.convertedTo(Registry::GetType<dest_type>());
            REQUIRE(dst.isEmpty());
            REQUIRE(fb.getCode().size() == afterDecl);
        }

        SECTION("Conversion via copy ctor") {
            Function fn("test", Registry::Signature<void>(), Registry::GlobalNamespace());
            FunctionBuilder fb(&fn);

            Value src = fb.val<convertible_via_ctor>();
            u32 afterDecl = fb.getCode().size();

            Value dst = src.convertedTo(Registry::GetType<dest_type>());
            REQUIRE(!dst.isEmpty());
            REQUIRE(fb.getCode().size() > afterDecl);

            Array<Value> params;
            Value self;
            Function* ctor = nullptr;
            for (u32 i = afterDecl;i < fb.getCode().size();i++) {
                Instruction& op = fb.getCode()[i];
                if (op.op == OpCode::param) params.push(op.operands[0]);
                else if (op.op == OpCode::call) {
                    ctor = (Function*)op.operands[0].getImm().p;
                    self.reset(op.operands[2]);
                    break;
                }
            }

            REQUIRE(ctor != nullptr);
            REQUIRE(ctor->getName() == ConstructorName);
            REQUIRE(params.size() == 1);
            REQUIRE(params[0].isEquivalentTo(src));
            REQUIRE(self.isEquivalentTo(dst));
        }

        SECTION("Conversion via cast operator") {
            Function fn("test", Registry::Signature<void>(), Registry::GlobalNamespace());
            FunctionBuilder fb(&fn);

            Value src = *fb.val<convertible_via_cast>();
            u32 afterDecl = fb.getCode().size();

            Value dst = src.convertedTo(Registry::GetType<dest_type>());
            REQUIRE(!dst.isEmpty());
            REQUIRE(fb.getCode().size() > afterDecl);

            Array<Value> params;
            Value result;
            Value selfPtr;
            Function* cast = nullptr;
            for (u32 i = afterDecl;i < fb.getCode().size();i++) {
                Instruction& op = fb.getCode()[i];
                if (op.op == OpCode::param) params.push(op.operands[0]);
                else if (op.op == OpCode::call) {
                    cast = (Function*)op.operands[0].getImm().p;
                    result.reset(op.operands[1]);
                    selfPtr.reset(op.operands[2]);
                    break;
                }
            }

            REQUIRE(cast != nullptr);
            REQUIRE(cast->getName() == CastOperatorName);
            REQUIRE(params.size() == 0);
            REQUIRE(result.isEquivalentTo(dst));
            REQUIRE(selfPtr.isEquivalentTo(src));
        }
    }
};

namespace operators {
    struct test_type {
        i32 dummy;

        i32 operator +(i32) { return 0; }
        i32 operator -(i32) { return 1; }
        i32 operator *(i32) { return 2; }
        i32 operator /(i32) { return 3; }
        i32 operator %(i32) { return 4; }
        i32 operator +=(i32) { return 5; }
        i32 operator -=(i32) { return 6; }
        i32 operator *=(i32) { return 7; }
        i32 operator /=(i32) { return 8; }
        i32 operator %=(i32) { return 9; }
        i32 operator &&(i32) { return 10; }
        i32 operator ||(i32) { return 11; }
        i32 operator <<(i32) { return 12; }
        i32 operator >>(i32) { return 13; }
        i32 operator &(i32) { return 14; }
        i32 operator |(i32) { return 15; }
        i32 operator ^(i32) { return 16; }
        i32 operator &=(i32) { return 17; }
        i32 operator |=(i32) { return 18; }
        i32 operator ^=(i32) { return 19; }
        i32 operator =(i32) { return 20; }
        i32 operator ==(i32) { return 21; }
        i32 operator !=(i32) { return 22; }
        i32 operator <(i32) { return 23; }
        i32 operator <=(i32) { return 24; }
        i32 operator >(i32) { return 25; }
        i32 operator >=(i32) { return 26; }
        i32 operator ++() { return 27; }
        i32 operator ++(i32) { return 28; }
        i32 operator --() { return 29; }
        i32 operator --(i32) { return 30; }
        i32 operator -() { return 31; }
        i32 operator !() { return 32; }
        i32 operator ~() { return 33; }
    };

    void testOperatorNotPresent(const char* op, Value (*doOp)(const Value& lhs, const Value& rhs)) {
        Function fn("testOp", Registry::Signature<void>(), Registry::GlobalNamespace());
        FunctionBuilder fb(&fn);

        Value lhs = fb.val<test_type>();
        Value rhs = fb.val<i32>();
        u32 afterDecl = fb.getCode().size();
        
        CAPTURE(op);
        REQUIRE(!fb.didError());

        Value result = doOp(*lhs, rhs);

        REQUIRE(result.isEmpty());
        REQUIRE(afterDecl == fb.getCode().size());
        REQUIRE(fb.didError());
    }

    void testOperatorPresent(const char* op, i32 expectedResult, Value (*doOp)(const Value& lhs, const Value& rhs)) {
        Function fn("testOp", Registry::Signature<i32>(), Registry::GlobalNamespace());
        FunctionBuilder fb(&fn);

        Value lhs = fb.val<test_type>();
        Value rhs = fb.val<i32>();
        u32 afterDecl = fb.getCode().size();

        CAPTURE(op);
        REQUIRE(!fb.didError());

        Value result = doOp(*lhs, rhs);

        REQUIRE(!result.isEmpty());
        REQUIRE(result.getType()->isEqualTo(Registry::GetType<i32>()));
        REQUIRE(afterDecl != fb.getCode().size());
        REQUIRE(!fb.didError());

        auto code = fb.getCode();
        for (u32 i = afterDecl;i < code.size();i++) {
            auto inst = code[i];
            if (inst.op == OpCode::call) {
                Function* fn = (Function*)inst.operands[0].getImm().p;
                REQUIRE(fn->getName() == op);
                break;
            }
        }

        fb.ret(result);

        i32 resultValue = -1;

        {
            TestBackend tb;
            tb.process(&fb);
            fn.call(&resultValue, nullptr);
        }

        REQUIRE(resultValue == expectedResult);
    }

    void testOperatorsNotPresent() {
        setupTest();

        auto dt = type<test_type>("test");

        testOperatorNotPresent("+", +[](const Value& lhs, const Value& rhs) { return lhs + rhs; });
        testOperatorNotPresent("-", +[](const Value& lhs, const Value& rhs) { return lhs - rhs; });
        testOperatorNotPresent("*", +[](const Value& lhs, const Value& rhs) { return lhs * rhs; });
        testOperatorNotPresent("/", +[](const Value& lhs, const Value& rhs) { return lhs / rhs; });
        testOperatorNotPresent("%", +[](const Value& lhs, const Value& rhs) { return lhs % rhs; });
        testOperatorNotPresent("+=", +[](const Value& lhs, const Value& rhs) { return lhs += rhs; });
        testOperatorNotPresent("-=", +[](const Value& lhs, const Value& rhs) { return lhs -= rhs; });
        testOperatorNotPresent("*=", +[](const Value& lhs, const Value& rhs) { return lhs *= rhs; });
        testOperatorNotPresent("/=", +[](const Value& lhs, const Value& rhs) { return lhs /= rhs; });
        testOperatorNotPresent("%=", +[](const Value& lhs, const Value& rhs) { return lhs %= rhs; });
        testOperatorNotPresent("&&", +[](const Value& lhs, const Value& rhs) { return lhs && rhs; });
        testOperatorNotPresent("||", +[](const Value& lhs, const Value& rhs) { return lhs || rhs; });
        testOperatorNotPresent("<<", +[](const Value& lhs, const Value& rhs) { return lhs << rhs; });
        testOperatorNotPresent(">>", +[](const Value& lhs, const Value& rhs) { return lhs >> rhs; });
        testOperatorNotPresent("&", +[](const Value& lhs, const Value& rhs) { return lhs & rhs; });
        testOperatorNotPresent("|", +[](const Value& lhs, const Value& rhs) { return lhs | rhs; });
        testOperatorNotPresent("^", +[](const Value& lhs, const Value& rhs) { return lhs ^ rhs; });
        testOperatorNotPresent("&=", +[](const Value& lhs, const Value& rhs) { return lhs &= rhs; });
        testOperatorNotPresent("|=", +[](const Value& lhs, const Value& rhs) { return lhs |= rhs; });
        testOperatorNotPresent("^=", +[](const Value& lhs, const Value& rhs) { return lhs ^= rhs; });
        testOperatorNotPresent("=", +[](const Value& lhs, const Value& rhs) { return lhs = rhs; });
        testOperatorNotPresent("==", +[](const Value& lhs, const Value& rhs) { return lhs == rhs; });
        testOperatorNotPresent("!=", +[](const Value& lhs, const Value& rhs) { return lhs != rhs; });
        testOperatorNotPresent(">", +[](const Value& lhs, const Value& rhs) { return lhs > rhs; });
        testOperatorNotPresent(">=", +[](const Value& lhs, const Value& rhs) { return lhs >= rhs; });
        testOperatorNotPresent("<", +[](const Value& lhs, const Value& rhs) { return lhs < rhs; });
        testOperatorNotPresent("<=", +[](const Value& lhs, const Value& rhs) { return lhs <= rhs; });
        testOperatorNotPresent("++", +[](const Value& lhs, const Value& rhs) { return ++lhs; });
        testOperatorNotPresent("--", +[](const Value& lhs, const Value& rhs) { return --lhs; });
        testOperatorNotPresent("++", +[](const Value& lhs, const Value& rhs) { return lhs++; });
        testOperatorNotPresent("--", +[](const Value& lhs, const Value& rhs) { return lhs--; });
        testOperatorNotPresent("-", +[](const Value& lhs, const Value& rhs) { return -lhs; });
        testOperatorNotPresent("!", +[](const Value& lhs, const Value& rhs) { return !lhs; });
        testOperatorNotPresent("~", +[](const Value& lhs, const Value& rhs) { return ~lhs; });
    }

    void testOperatorsPresent() {
        setupTest();

        auto dt = type<test_type>("test");

        dt.opAdd<i32, i32>();
        dt.opSub<i32, i32>();
        dt.opMul<i32, i32>();
        dt.opDiv<i32, i32>();
        dt.opMod<i32, i32>();
        dt.opAddEq<i32, i32>();
        dt.opSubEq<i32, i32>();
        dt.opMulEq<i32, i32>();
        dt.opDivEq<i32, i32>();
        dt.opModEq<i32, i32>();
        dt.opLogicalAnd<i32, i32>();
        dt.opLogicalOr<i32, i32>();
        dt.opShiftLeft<i32, i32>();
        dt.opShiftRight<i32, i32>();
        dt.opAnd<i32, i32>();
        dt.opOr<i32, i32>();
        dt.opXOr<i32, i32>();
        dt.opAndEq<i32, i32>();
        dt.opOrEq<i32, i32>();
        dt.opXOrEq<i32, i32>();
        dt.opAssign<i32, i32>();
        dt.opEquality<i32, i32>();
        dt.opInequality<i32, i32>();
        dt.opGreater<i32, i32>();
        dt.opGreaterEq<i32, i32>();
        dt.opLess<i32, i32>();
        dt.opLessEq<i32, i32>();
        dt.opPreInc<i32>();
        dt.opPostInc<i32>();
        dt.opPreDec<i32>();
        dt.opPostDec<i32>();
        dt.opNegate<i32>();
        dt.opNot<i32>();
        dt.opInvert<i32>();
        
        testOperatorPresent("+" , 0,  +[](const Value& lhs, const Value& rhs) { return lhs +  rhs; });
        testOperatorPresent("-" , 1,  +[](const Value& lhs, const Value& rhs) { return lhs -  rhs; });
        testOperatorPresent("*" , 2,  +[](const Value& lhs, const Value& rhs) { return lhs *  rhs; });
        testOperatorPresent("/" , 3,  +[](const Value& lhs, const Value& rhs) { return lhs /  rhs; });
        testOperatorPresent("%" , 4,  +[](const Value& lhs, const Value& rhs) { return lhs %  rhs; });
        testOperatorPresent("+=", 5,  +[](const Value& lhs, const Value& rhs) { return lhs += rhs; });
        testOperatorPresent("-=", 6,  +[](const Value& lhs, const Value& rhs) { return lhs -= rhs; });
        testOperatorPresent("*=", 7,  +[](const Value& lhs, const Value& rhs) { return lhs *= rhs; });
        testOperatorPresent("/=", 8,  +[](const Value& lhs, const Value& rhs) { return lhs /= rhs; });
        testOperatorPresent("%=", 9,  +[](const Value& lhs, const Value& rhs) { return lhs %= rhs; });
        testOperatorPresent("&&", 10, +[](const Value& lhs, const Value& rhs) { return lhs && rhs; });
        testOperatorPresent("||", 11, +[](const Value& lhs, const Value& rhs) { return lhs || rhs; });
        testOperatorPresent("<<", 12, +[](const Value& lhs, const Value& rhs) { return lhs << rhs; });
        testOperatorPresent(">>", 13, +[](const Value& lhs, const Value& rhs) { return lhs >> rhs; });
        testOperatorPresent("&" , 14, +[](const Value& lhs, const Value& rhs) { return lhs &  rhs; });
        testOperatorPresent("|" , 15, +[](const Value& lhs, const Value& rhs) { return lhs |  rhs; });
        testOperatorPresent("^" , 16, +[](const Value& lhs, const Value& rhs) { return lhs ^  rhs; });
        testOperatorPresent("&=", 17, +[](const Value& lhs, const Value& rhs) { return lhs &= rhs; });
        testOperatorPresent("|=", 18, +[](const Value& lhs, const Value& rhs) { return lhs |= rhs; });
        testOperatorPresent("^=", 19, +[](const Value& lhs, const Value& rhs) { return lhs ^= rhs; });
        testOperatorPresent("=" , 20, +[](const Value& lhs, const Value& rhs) { return lhs =  rhs; });
        testOperatorPresent("==", 21, +[](const Value& lhs, const Value& rhs) { return lhs == rhs; });
        testOperatorPresent("!=", 22, +[](const Value& lhs, const Value& rhs) { return lhs != rhs; });
        testOperatorPresent("<" , 23, +[](const Value& lhs, const Value& rhs) { return lhs <  rhs; });
        testOperatorPresent("<=", 24, +[](const Value& lhs, const Value& rhs) { return lhs <= rhs; });
        testOperatorPresent(">" , 25, +[](const Value& lhs, const Value& rhs) { return lhs >  rhs; });
        testOperatorPresent(">=", 26, +[](const Value& lhs, const Value& rhs) { return lhs >= rhs; });
        testOperatorPresent("++", 27, +[](const Value& lhs, const Value& rhs) { return ++lhs;      });
        testOperatorPresent("++", 28, +[](const Value& lhs, const Value& rhs) { return lhs++;      });
        testOperatorPresent("--", 29, +[](const Value& lhs, const Value& rhs) { return --lhs;      });
        testOperatorPresent("--", 30, +[](const Value& lhs, const Value& rhs) { return lhs--;      });
        testOperatorPresent("-" , 31, +[](const Value& lhs, const Value& rhs) { return -lhs;       });
        testOperatorPresent("!" , 32, +[](const Value& lhs, const Value& rhs) { return !lhs;       });
        testOperatorPresent("~" , 33, +[](const Value& lhs, const Value& rhs) { return ~lhs;       });
    }
};

TEST_CASE("Test Object Values", "[codegen]") {
    SECTION("Object Value Conversion") {
        conversion::testObjectConversion();
    }
    
    SECTION("Object Value Operators When Not Bound") {
        operators::testOperatorsNotPresent();
    }
    
    SECTION("Object Value Operators When Bound") {
        operators::testOperatorsPresent();
    }
}