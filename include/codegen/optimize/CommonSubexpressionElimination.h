#pragma once
#include <codegen/interfaces/IPostProcessStep.h>

namespace codegen {
    class CommonSubexpressionEliminationStep : public IPostProcessStep {
        public:
            CommonSubexpressionEliminationStep();
            virtual ~CommonSubexpressionEliminationStep();

            virtual bool execute(CodeHolder* code, BasicBlock* block, u32 mask = 0xFFFFFFFF);
    };
};