#pragma once
#include <codegen/interfaces/IPostProcessStep.h>
#include <utils/Array.h>

namespace codegen {
    class FunctionBuilder;
    class CodeHolder;

    class IBackend {
        public:
            IBackend();
            virtual ~IBackend();

            void addPostProcess(IPostProcessStep* process);
            bool process(FunctionBuilder* input, u32 postProcessMask = 0xFFFFFFFF);

            virtual bool onBeforePostProcessing(CodeHolder* ch);
            virtual bool onAfterPostProcessing(CodeHolder* ch);
            virtual bool transform(CodeHolder* processedCode) = 0;

        protected:
            Array<IPostProcessStep*> m_postProcesses;
    };
};