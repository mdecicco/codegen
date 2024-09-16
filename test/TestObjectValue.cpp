#include "Common.h"

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

        c1.castOperator<dest_type>();
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
            Function* ctor = nullptr;
            for (u32 i = afterDecl;i < fb.getCode().size();i++) {
                Instruction& op = fb.getCode()[i];
                if (op.op == OpCode::param) params.push(op.operands[0]);
                else if (op.op == OpCode::call) {
                    ctor = (Function*)op.operands[0].getImm().p;
                    break;
                }
            }

            REQUIRE(ctor != nullptr);
            REQUIRE(ctor->getName() == ConstructorName);
            REQUIRE(params.size() == 2);
            REQUIRE(params[0].isEquivalentTo(dst));
            REQUIRE(params[1].isEquivalentTo(src));
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

TEST_CASE("Test Object Values", "[bind]") {
    SECTION("Object Value Conversion") {
        conversion::testObjectConversion();
    }
    
    SECTION("Object Value Operators") {
    }
    
    SECTION("Object Value Operators") {
    }
}