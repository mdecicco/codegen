#pragma once
#include <codegen/types.h>
#include <utils/Array.h>

namespace codegen {
    class PostProcessGroup;
    class CodeHolder;
    struct BasicBlock;

    /**
     * @brief Interface for defining optimization steps for IR code. Each optimization
     * step is executed repeatedly until the execution function returns false. This is
     * useful for multipass optimizations. Each optimization can be executed on either
     * the code as a whole, or on each basic block of the control flow graph (in order
     * of execution). If both methods are implemented, the variation that works on the
     * basic blocks is executed first.
     */
    class IPostProcessStep {
        public:
            IPostProcessStep();
            virtual ~IPostProcessStep();

            void setGroup(PostProcessGroup* group);
            void setMask(u32 mask);
            u32 getMask() const;
            PostProcessGroup* getGroup() const;

            /**
             * @brief Executes the optimization pass
             * 
             * @param code An object which contains the code to optimize
             * @param mask User-defined bitmask for selectively including post process steps
             * 
             * @return Returns true if the pass should be executed again immediately
             */
            virtual bool execute(CodeHolder* code, u32 mask = 0xFFFFFFFF);

            /**
             * @brief Executes the optimization pass on a basic block. This is called
             *        one time per basic block
             * 
             * @param code An object which contains the code to optimize
             * @param block Pointer to a basic block of the control flow graph
             * @param mask User-defined bitmask for selectively including post process steps
             * 
             * @return Returns true if the pass should be executed again immediately
             */
            virtual bool execute(CodeHolder* code, BasicBlock* block, u32 mask = 0xFFFFFFFF);
        
        private:
            PostProcessGroup* m_group;
            u32 m_mask;
    };
};