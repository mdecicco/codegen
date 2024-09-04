#include <codegen/TestBackend.h>
#include <codegen/CodeHolder.h>
#include <codegen/FunctionBuilder.h>
#include <bind/Function.h>
#include <utils/Array.hpp>

namespace codegen {
    TestBackend::TestBackend() {
    }

    TestBackend::~TestBackend() {
        for (TestExecuterCallHandler* h : m_callHandlers) delete h;
    }

    void TestBackend::transform(CodeHolder* processedCode) {
        processedCode->owner->getFunction()->setCallHandler(new TestExecuterCallHandler(processedCode));
    }
};