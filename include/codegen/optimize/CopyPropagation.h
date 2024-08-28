#pragma once
#include <codegen/interfaces/IPostProcessStep.h>

namespace codegen {
    class CopyPropagationStep : public IPostProcessStep {
        public:
            CopyPropagationStep();
            virtual ~CopyPropagationStep();

            virtual bool execute(CodeHolder* code, BasicBlock* block, u32 mask = 0xFFFFFFFF);
    };
};