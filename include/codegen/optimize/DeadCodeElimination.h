#pragma once
#include <codegen/interfaces/IPostProcessStep.h>

namespace codegen {
    class DeadCodeEliminationStep : public IPostProcessStep {
        public:
            DeadCodeEliminationStep();
            virtual ~DeadCodeEliminationStep();

            virtual bool execute(CodeHolder* code, u32 mask = 0xFFFFFFFF);
    };
};