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
            void process(FunctionBuilder* input, u32 postProcessMask = 0xFFFFFFFF);

            virtual void onBeforePostProcessing(CodeHolder* ch);
            virtual void onAfterPostProcessing(CodeHolder* ch);
            virtual void transform(CodeHolder* processedCode) = 0;

        protected:
            Array<IPostProcessStep*> m_postProcesses;
    };
};