#pragma once
#include <codegen/interfaces/IPostProcessStep.h>
#include <utils/Array.h>

namespace codegen {
    /**
     * @brief Contains a series of post process steps. Steps are executed in the order they are added in.
     *        If the group should be executed multiple times, one or more of the added steps should call
     *        `getGroup()->setShouldRepeat(true)`.
     * 
     */
    class PostProcessGroup : public IPostProcessStep {
        public:
            PostProcessGroup();
            virtual ~PostProcessGroup();

            void setShouldRepeat(bool doRepeat);
            bool willRepeat() const;

            /**
             * @brief Adds a new post process step to be executed
             *
             * @param step The step to execute
             */
            void addStep(IPostProcessStep* step, u32 mask = 0xFFFFFFFF);

            virtual bool execute(CodeHolder* code, u32 mask = 0xFFFFFFFF);
        
        protected:
            friend class IPostProcessStep;
            bool m_doRepeat;
            Array<IPostProcessStep*> m_steps;
    };
};