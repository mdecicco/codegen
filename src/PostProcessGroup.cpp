#include <codegen/PostProcessGroup.h>
#include <codegen/CodeHolder.h>
#include <utils/Array.hpp>

namespace codegen {
    PostProcessGroup::PostProcessGroup() : IPostProcessStep() {
        m_doRepeat = false;
    }

    PostProcessGroup::~PostProcessGroup() {
        for (u32 i = 0;i < m_steps.size();i++) delete m_steps[i];
    }

    void PostProcessGroup::setShouldRepeat(bool doRepeat) {
        m_doRepeat = doRepeat;
    }

    bool PostProcessGroup::willRepeat() const {
        return m_doRepeat;
    }

    void PostProcessGroup::addStep(IPostProcessStep* step, u32 mask) {
        m_steps.push(step);
        step->setGroup(this);
        step->setMask(mask);
    }

    bool PostProcessGroup::execute(CodeHolder* code, u32 mask) {
        m_doRepeat = false;

        for (u32 i = 0;i < m_steps.size();i++) {
            IPostProcessStep* step = m_steps[i];
            if (step->getMask() != 0 && (step->getMask() & mask) == 0) continue;

            for (u32 b = 0;b < code->cfg.blocks.size();b++) {
                while (step->execute(code, &code->cfg.blocks[b], mask));
            }
            
            while (step->execute(code, mask));
        }

        return m_doRepeat;
    }
};