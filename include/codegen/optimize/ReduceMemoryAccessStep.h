#pragma once
#include <codegen/interfaces/IPostProcessStep.h>

namespace codegen {
    class ReduceMemoryAccessStep : public IPostProcessStep {
        public:
            ReduceMemoryAccessStep();
            virtual ~ReduceMemoryAccessStep();

            virtual bool execute(CodeHolder* code, u32 mask = 0xFFFFFFFF);
    };
};