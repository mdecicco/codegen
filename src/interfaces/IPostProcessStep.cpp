#include <codegen/interfaces/IPostProcessStep.h>
#include <codegen/PostProcessGroup.h>

#include <utils/Array.hpp>

namespace codegen {
    IPostProcessStep::IPostProcessStep() {
        m_group = nullptr;
        m_mask = 0;
    }

    IPostProcessStep::~IPostProcessStep() {
    }

    void IPostProcessStep::setGroup(PostProcessGroup* group) {
        m_group = group;
    }
    
    void IPostProcessStep::setMask(u32 mask) {
        m_mask = mask;
    }
    
    u32 IPostProcessStep::getMask() const {
        return m_mask;
    }

    PostProcessGroup* IPostProcessStep::getGroup() const {
        return m_group;
    }

    bool IPostProcessStep::execute(CodeHolder* code, u32 mask) {
        return false;
    }
    
    bool IPostProcessStep::execute(CodeHolder* code, BasicBlock* block, u32 mask) {
        return false;
    }
};