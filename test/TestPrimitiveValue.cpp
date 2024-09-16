#include "Common.h"

template <typename T1, typename T2>
void doPrimitiveConversion(Function* fn) {
    DataType* dstTp = Registry::GetType<T2>();
    FunctionBuilder fb(fn);
    Value src = fb.val<T1>();
    Value dst = src.convertedTo(dstTp);

    // resulting value has the correct type
    REQUIRE(dst.getType()->isEqualTo(dstTp));

    if constexpr (std::is_same_v<T1, T2>) {
        // no conversion instruction emitted
        auto code = fb.getCode();
        REQUIRE(code.size() == 0);
    } else {
        // conversion instruction emitted
        auto code = fb.getCode();
        REQUIRE(code.size() == 1);
        REQUIRE(code[0].op == OpCode::cvt);
        REQUIRE(code[0].operands[0].isEquivalentTo(dst));
        REQUIRE(code[0].operands[1].isEquivalentTo(src));
        REQUIRE(code[0].operands[2].isImm());
        REQUIRE(code[0].operands[2].getImm().u == dstTp->getSymbolId());
    }
}

template <typename T>
void testPrimitiveConversion() {
    setupTest();

    Function fn("test", Registry::Signature<void>(), Registry::GlobalNamespace());
    doPrimitiveConversion<T, u8>(&fn);
    doPrimitiveConversion<T, u16>(&fn);
    doPrimitiveConversion<T, u32>(&fn);
    doPrimitiveConversion<T, u64>(&fn);
    doPrimitiveConversion<T, i8>(&fn);
    doPrimitiveConversion<T, i16>(&fn);
    doPrimitiveConversion<T, i32>(&fn);
    doPrimitiveConversion<T, i64>(&fn);
    doPrimitiveConversion<T, f32>(&fn);
    doPrimitiveConversion<T, f64>(&fn);
}

void testPrimitiveBinaryOperator(const Value& lhs, const Value& rhs, Value (Value::*op)(const Value&) const, OpCode expectedOp, bool isAssignment) {
    CAPTURE(lhs.getType()->getName().c_str());
    CAPTURE(rhs.getType()->getName().c_str());
    CAPTURE(Instruction::Info(expectedOp).name);
    CAPTURE(isAssignment);

    Array<Instruction>& code = lhs.getOwner()->getCode();
    u32 codeBegin = code.size();
    Value result = (lhs.*op)(rhs);

    Value effectiveRhs(rhs);

    if (!rhs.getType()->isEqualTo(lhs.getType())) {
        // a conversion instruction should've been emitted before the operation
        REQUIRE((code.size() - codeBegin) == 2);

        Instruction& cvt = code[codeBegin];
        CAPTURE(Instruction::Info(cvt.op).name);
        REQUIRE(cvt.op == OpCode::cvt);
        REQUIRE(cvt.operands[1].isEquivalentTo(rhs));
        REQUIRE(cvt.operands[2].isImm());
        REQUIRE(cvt.operands[2].getImm().u == lhs.getType()->getSymbolId());

        effectiveRhs.reset(cvt.operands[0]);
        codeBegin++;
    }

    if (expectedOp == OpCode::assign) {
        // result should be same register as lhs
        REQUIRE(result.isEquivalentTo(lhs));

        // and have the same type as lhs
        REQUIRE(result.getType()->isEqualTo(lhs.getType()));

        // operation should be the only instruction that follows
        REQUIRE((code.size() - codeBegin) == 1);

        Instruction& op = code[codeBegin];
        CAPTURE(Instruction::Info(op.op).name);
        REQUIRE(op.op == OpCode::assign);
        REQUIRE(op.operands[0].isEquivalentTo(lhs));
        REQUIRE(op.operands[1].isEquivalentTo(effectiveRhs));
    } else if (isAssignment) {
        // result should be same register as lhs
        REQUIRE(result.isEquivalentTo(lhs));

        // and have the same type as lhs
        REQUIRE(result.getType()->isEqualTo(lhs.getType()));

        // operation should be the only instruction that follows
        REQUIRE((code.size() - codeBegin) == 1);

        Instruction& op = code[codeBegin];
        CAPTURE(Instruction::Info(op.op).name);
        REQUIRE(op.op == expectedOp);
        REQUIRE(op.operands[0].isEquivalentTo(lhs));
        REQUIRE(op.operands[1].isEquivalentTo(lhs));
        REQUIRE(op.operands[2].isEquivalentTo(effectiveRhs));
    } else {
        // result should be different register from lhs
        REQUIRE(!result.isEquivalentTo(lhs));

        // but have the same type as lhs
        REQUIRE(result.getType()->isEqualTo(lhs.getType()));

        // operation should be the only instruction that follows
        REQUIRE((code.size() - codeBegin) == 1);

        Instruction& op = code[codeBegin];
        CAPTURE(Instruction::Info(op.op).name);
        REQUIRE(op.op == expectedOp);
        REQUIRE(op.operands[0].isEquivalentTo(result));
        REQUIRE(op.operands[1].isEquivalentTo(lhs));
        REQUIRE(op.operands[2].isEquivalentTo(effectiveRhs));
    }
}

template <typename T1, typename T2>
void testPrimitiveOperators() {
    setupTest();

    Function fn("test", Registry::Signature<void>(), Registry::GlobalNamespace());
    FunctionBuilder fb(&fn);
    Value lhs = fb.val<T1>();
    Value rhs = fb.val<T2>();

    if constexpr (std::is_integral_v<T1>) {
        if constexpr (std::is_unsigned_v<T1>) {
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator +  , OpCode::uadd  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator += , OpCode::uadd  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator -  , OpCode::usub  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator -= , OpCode::usub  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator *  , OpCode::umul  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator *= , OpCode::umul  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator /  , OpCode::udiv  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator /= , OpCode::udiv  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator %  , OpCode::umod  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator %= , OpCode::umod  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator != , OpCode::uneq  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator == , OpCode::ueq   , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator <  , OpCode::ult   , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator <= , OpCode::ulte  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator >  , OpCode::ugt   , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator >= , OpCode::ugte  , false);
        } else {
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator +  , OpCode::iadd  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator += , OpCode::iadd  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator -  , OpCode::isub  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator -= , OpCode::isub  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator *  , OpCode::imul  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator *= , OpCode::imul  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator /  , OpCode::idiv  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator /= , OpCode::idiv  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator %  , OpCode::imod  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator %= , OpCode::imod  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator == , OpCode::ieq   , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator <  , OpCode::ilt   , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator <= , OpCode::ilte  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator >  , OpCode::igt   , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator >= , OpCode::igte  , false);
        }

        testPrimitiveBinaryOperator(lhs, rhs, &Value::operator ^  , OpCode::_xor  , false);
        testPrimitiveBinaryOperator(lhs, rhs, &Value::operator ^= , OpCode::_xor  , true );
        testPrimitiveBinaryOperator(lhs, rhs, &Value::operator &  , OpCode::band  , false);
        testPrimitiveBinaryOperator(lhs, rhs, &Value::operator &= , OpCode::band  , true );
        testPrimitiveBinaryOperator(lhs, rhs, &Value::operator |  , OpCode::bor   , false);
        testPrimitiveBinaryOperator(lhs, rhs, &Value::operator |= , OpCode::bor   , true );
        testPrimitiveBinaryOperator(lhs, rhs, &Value::operator << , OpCode::shl   , false);
        testPrimitiveBinaryOperator(lhs, rhs, &Value::operator <<=, OpCode::shl   , true );
        testPrimitiveBinaryOperator(lhs, rhs, &Value::operator >> , OpCode::shr   , false);
        testPrimitiveBinaryOperator(lhs, rhs, &Value::operator >>=, OpCode::shr   , true );
    } else if (std::is_floating_point_v<T1>) {
        if constexpr (sizeof(T1) == sizeof(f32)) {
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator +  , OpCode::fadd  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator += , OpCode::fadd  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator -  , OpCode::fsub  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator -= , OpCode::fsub  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator *  , OpCode::fmul  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator *= , OpCode::fmul  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator /  , OpCode::fdiv  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator /= , OpCode::fdiv  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator %  , OpCode::fmod  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator %= , OpCode::fmod  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator ^  , OpCode::noop  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator ^= , OpCode::noop  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator &  , OpCode::noop  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator &= , OpCode::noop  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator |  , OpCode::noop  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator |= , OpCode::noop  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator << , OpCode::noop  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator <<=, OpCode::noop  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator >> , OpCode::noop  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator >>=, OpCode::noop  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator != , OpCode::fneq  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator == , OpCode::feq   , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator <  , OpCode::flt   , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator <= , OpCode::flte  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator >  , OpCode::fgt   , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator >= , OpCode::fgte  , false);
        } else {
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator +  , OpCode::dadd  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator += , OpCode::dadd  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator -  , OpCode::dsub  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator -= , OpCode::dsub  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator *  , OpCode::dmul  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator *= , OpCode::dmul  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator /  , OpCode::ddiv  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator /= , OpCode::ddiv  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator %  , OpCode::dmod  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator %= , OpCode::dmod  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator ^  , OpCode::noop  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator ^= , OpCode::noop  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator &  , OpCode::noop  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator &= , OpCode::noop  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator |  , OpCode::noop  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator |= , OpCode::noop  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator << , OpCode::noop  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator <<=, OpCode::noop  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator >> , OpCode::noop  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator >>=, OpCode::noop  , true );
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator != , OpCode::dneq  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator == , OpCode::deq   , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator <  , OpCode::dlt   , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator <= , OpCode::dlte  , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator >  , OpCode::dgt   , false);
            testPrimitiveBinaryOperator(lhs, rhs, &Value::operator >= , OpCode::dgte  , false);
        }
    }

    testPrimitiveBinaryOperator(lhs, rhs, &Value::operator && , OpCode::land  , false);
    testPrimitiveBinaryOperator(lhs, rhs, &Value::operator || , OpCode::lor   , false);
    testPrimitiveBinaryOperator(lhs, rhs, &Value::operator =  , OpCode::assign, true );
    testPrimitiveBinaryOperator(lhs, rhs, &Value::operator_logicalAndAssign, OpCode::land, true);
    testPrimitiveBinaryOperator(lhs, rhs, &Value::operator_logicalOrAssign, OpCode::lor, true);
}

template <typename T>
void testAllPrimitiveOperatorPermutations() {
    testPrimitiveOperators<T, u8>();
    testPrimitiveOperators<T, u16>();
    testPrimitiveOperators<T, u32>();
    testPrimitiveOperators<T, u64>();
    testPrimitiveOperators<T, i8>();
    testPrimitiveOperators<T, i16>();
    testPrimitiveOperators<T, i32>();
    testPrimitiveOperators<T, i64>();
    testPrimitiveOperators<T, f32>();
    testPrimitiveOperators<T, f64>();
}

TEST_CASE("Test Primitive Values", "[bind]") {
    SECTION("Primitive Value Conversion") {
        testPrimitiveConversion<u8>();
        testPrimitiveConversion<u16>();
        testPrimitiveConversion<u32>();
        testPrimitiveConversion<u64>();
        testPrimitiveConversion<i8>();
        testPrimitiveConversion<i16>();
        testPrimitiveConversion<i32>();
        testPrimitiveConversion<i64>();
        testPrimitiveConversion<f32>();
        testPrimitiveConversion<f64>();
    }
    
    SECTION("Primitive Value Operators") {
        testAllPrimitiveOperatorPermutations<u8>();
        testAllPrimitiveOperatorPermutations<u16>();
        testAllPrimitiveOperatorPermutations<u32>();
        testAllPrimitiveOperatorPermutations<u64>();
        testAllPrimitiveOperatorPermutations<i8>();
        testAllPrimitiveOperatorPermutations<i16>();
        testAllPrimitiveOperatorPermutations<i32>();
        testAllPrimitiveOperatorPermutations<i64>();
        testAllPrimitiveOperatorPermutations<f32>();
        testAllPrimitiveOperatorPermutations<f64>();
    }
}