#pragma once
#include <codegen/types.h>
#include <utils/Array.h>
#include <unordered_set>

namespace codegen {
    class Value;
    class FunctionBuilder;

    /**
     * @brief This class is a helper for managing the stack of a function. When you instantiate an
     * instance of this class, all stack objects that are allocated by the `FunctionBuilder` will
     * be added to that instance. When the instances of this class go out of scope or are otherwise
     * destroyed, it will automatically emit IR instructions destroy all the stack objects allocated
     * since its construction and free up the stack space used by them. However, if you call one of
     * the `escape` methods the stack will be cleaned up at that point and not when the scope is
     * destroyed. Each `Scope` can only be escaped one time
     * 
     * In addition to helping with stack cleanup, this class provides tools for managing control flow
     * with loops.
     */
    class Scope {
        public:
            Scope(FunctionBuilder* func);
            ~Scope();

            /**
             * @brief Escapes the current scope, destructing all the stack objects allocated within
             * it and freeing all the stack IDs. This can only be done once per scope instance.
             */
            void escape();
            
            /**
             * @brief Escapes the current scope, destructing all the stack objects allocated within
             * it and freeing all the stack IDs. This can only be done once per scope instance.
             * 
             * @param withValue Value to save from the stack cleanup. It will not be deconstructed
             * and its corresponding stack ID will not be freed. Additionally, the value and its
             * stack ID will be added to the parent scope
             */
            void escape(const Value& withValue);

            /**
             * @brief Whether or not this scope has escaped
             */
            bool didEscape() const;

            /**
             * @brief If this scope corresponds to a loop, this function will set the label that should
             * be jumped to when control should move back to the start of the loop
             * 
             * @param continueLbl Label to jump to when the control should move back to the start of the
             * loop
             */
            void setLoopContinueLabel(label_id continueLbl);

            /**
             * @brief If this scope or any parent scope corresponds to a loop, this function will get the
             * label that should be jumped to when control should move back to the start of the loop
             */
            label_id getLoopContinueLabel() const;

            /**
             * @brief If this scope or any parent scope corresponds to a loop, this function will emit IR
             * instructions that will cause control to move back to the start of the loop. This includes
             * stack cleanup instructions for all scopes between this one and the one that corresponds to
             * the loop
             */
            void loopContinue();

            /**
             * @brief If this scope corresponds to a loop, this function will set the label that should
             * be jumped to when control should break out of the loop and continue after it
             * 
             * @param breakLbl Label to jump to when the control should break out of the loop and continue
             * after it
             */
            void setLoopBreakLabel(label_id breakLbl);
            
            /**
             * @brief If this scope or any parent scope corresponds to a loop, this function will get the
             * label that should be jumped to when control should break out of the loop and continue after
             * it
             */
            label_id getLoopBreakLabel() const;

            /**
             * @brief If this scope or any parent scope corresponds to a loop, this function will emit IR
             * instructions that will cause control to break out of the loop and continue after it. This
             * includes stack cleanup instructions for all scopes between this one and the one that
             * corresponds to the loop
             */
            void loopBreak();
        
        protected:
            friend class FunctionBuilder;
            void add(const Value& stackPtr);
            void add(stack_id allocId);
            void remove(const Value& stackPtr);
            void remove(stack_id allocId);
            void emitEscapeInstructions();
            void emitPreReturnInstructions();

            FunctionBuilder* m_owner;
            Scope* m_parent;
            bool m_didEscape;
            label_id m_continueLbl;
            label_id m_breakLbl;

            std::unordered_set<stack_id> m_stackIds;
            Array<Value> m_stackPointers;
    };
};