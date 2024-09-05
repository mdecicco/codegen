#include <codegen/FunctionBuilder.h>
#include <codegen/CodeHolder.h>
#include <codegen/Execute.h>
#include <codegen/TestBackend.h>
#include <utils/interfaces/ILogHandler.h>
#include <bind/bind.h>
using namespace codegen;
using namespace bind;
using namespace utils;

struct test_struct_1 {
    i32 a, b, c;
};

struct test_struct_2 {
    i32 a, b, c;
    test_struct_1 d;
};

class ErrHandler : public ILogHandler {
    virtual void onDebug(const char* msg) {
        printf("Debug: %s\n", msg);
        fflush(stdout);
    }
    virtual void onInfo(const char* msg) {
        printf("Info: %s\n", msg);
        fflush(stdout);
    }

    virtual void onWarn(const char* msg) {
        printf("Warn: %s\n", msg);
        fflush(stdout);
    }

    virtual void onError(const char* msg) {
        printf("Error: %s\n", msg);
        fflush(stdout);
    }
};

int main(int argc, const char** argv) {
    Registry::Create();

    {
        type<void>("void");
        type<bool>("bool");
        type<i8>("i8");
        type<i16>("i16");
        type<i32>("i32");
        type<i64>("i64");
        type<u8>("u8");
        type<u16>("u16");
        type<u32>("u32");
        type<u64>("u64");
        type<f32>("f32");
        type<f64>("f64");

        auto ts1 = type<test_struct_1>("test1");
        ts1.prop("a", &test_struct_1::a);
        ts1.prop("b", &test_struct_1::b);
        ts1.prop("c", &test_struct_1::c);

        auto ts2 = type<test_struct_2>("test2");
        ts2.prop("a", &test_struct_2::a);
        ts2.prop("b", &test_struct_2::b);
        ts2.prop("c", &test_struct_2::c);
        ts2.prop("d", &test_struct_2::d);

        ErrHandler h;

        Function fn = Function("test", Registry::Signature<i32, i32, i32>(), Registry::GlobalNamespace());

        FunctionBuilder fb = FunctionBuilder(&fn);
        fb.setLogHandler(&h);

        fb.enableValidation();
        Value arg1 = fb.getArg(0);
        Value arg2 = fb.getArg(1);
        Value something = arg1 + arg2;

        arg1.setName("arg1");
        arg2.setName("arg2");
        something.setName("something");

        Value v2 = fb.val(ts2.getType());
        fb.generateConstruction(v2, {});
        v2.setName("vec2");
        fb.store(fb.val(0), v2, offsetof(test_struct_2, a));
        fb.store(fb.val(1), v2, offsetof(test_struct_2, b));
        fb.store(fb.val(2), v2, offsetof(test_struct_2, c));
        
        fb.generateIf(something > fb.val(10), [&](){
            Scope s (&fb);
            something += arg2;

            Value i = fb.val(Registry::GetType<u32>());
            i.setName("i");
            i = fb.val(u32(0));
            fb.generateFor([&](){
                return i < fb.val(10);
            }, [&](){
                i++;
            }, [&](){
                something += i;
            });

            Value v1 = fb.val(ts1.getType());
            v1.setName("vec1");
            fb.generateConstruction(v1, { v2 });
            fb.store(fb.val(0), v1, offsetof(test_struct_1, a));
            fb.store(fb.val(1), v1, offsetof(test_struct_1, b));
            fb.store(fb.val(2), v1, offsetof(test_struct_1, c));

            something -= fb.generateCall(&fb, { fb.val(i32(2)), fb.val(i32(3)) });
        });
        
        fb.generateReturn(something);

        auto code = fb.getCode();
        for (u32 i = 0;i < code.size();i++) {
            printf("0x%0.3X | %s\n", i, code[i].toString().c_str());
        }
        fflush(stdout);

        TestBackend be;
        be.process(&fb);

        i32 result = 0;
        i32 a1 = 10, a2 = 15;
        void* args1[] = { &a1, &a2 };
        fn.call(&result, args1);

        printf("result: %d\n", result);
        fflush(stdout);
    }

    Registry::Destroy();
    return 0;
}