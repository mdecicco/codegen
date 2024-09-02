#include <codegen/interfaces/IBackend.h>
#include <codegen/CodeHolder.h>
#include <codegen/FunctionBuilder.h>
#include <utils/Array.hpp>

namespace codegen {
    IBackend::IBackend() {
    }

    IBackend::~IBackend() {
    }

    void IBackend::addPostProcess(IPostProcessStep* process) {
        m_postProcesses.push(process);
    }

    void IBackend::process(FunctionBuilder* input, u32 postProcessMask) {
        CodeHolder ch(input->getCode());
        ch.owner = input;
        ch.rebuildAll();

        onBeforePostProcessing(&ch);
        
        for (IPostProcessStep* step : m_postProcesses) {
            for (BasicBlock& b : ch.cfg.blocks) {
                while (step->execute(&ch, &b, postProcessMask));
            }

            while (step->execute(&ch, postProcessMask));
        }

        onAfterPostProcessing(&ch);

        transform(&ch);
    }

    void IBackend::onBeforePostProcessing(CodeHolder* ch) { }
    void IBackend::onAfterPostProcessing(CodeHolder* ch) { }
};