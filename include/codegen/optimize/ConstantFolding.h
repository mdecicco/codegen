#pragma once
#include <codegen/interfaces/IPostProcessStep.h>

namespace codegen {
    class ConstantFoldingStep : public IPostProcessStep {
        public:
            ConstantFoldingStep();
            virtual ~ConstantFoldingStep();

            virtual bool execute(CodeHolder* code, u32 mask = 0xFFFFFFFF);
    };
};