#include <codegen/Scope.h>
#include <codegen/FunctionBuilder.h>
#include <bind/DataType.h>
#include <bind/PointerType.h>
#include <utils/Exception.h>
#include <utils/Array.hpp>

namespace codegen {
    Scope::Scope(FunctionBuilder* func)
        : m_owner(func), m_parent(func->getCurrentScope()), m_didEscape(false), m_continueLbl(-1),
          m_breakLbl(-1)
    {
        func->enterScope(this);
    }
    
    Scope::~Scope() {
        if (!m_didEscape) escape();
    }

    void Scope::escape() {
        if (m_didEscape) throw Exception("Scope::escape - scope has already been escaped");
        
        emitEscapeInstructions();
        m_didEscape = true;
        m_owner->exitScope(this);
    }

    void Scope::escape(const Value& withValue) {
        if (m_didEscape) throw Exception("Scope::escape - scope has already been escaped");
        if (!m_parent) {
            throw Exception("Scope::escape - Attempt to escape from root scope with a value would result in the value not being deconstructed or freed from the stack");
        }

        stack_id stackRef = withValue.getStackRef();
        if (stackRef == NullStack || m_stackIds.count(stackRef) == 0) {
            escape();
            return;
        }

        m_parent->add(stackRef);

        for (u32 i = 0;i < m_stackPointers.size();i++) {
            if (m_stackPointers[i].getStackRef() == stackRef) {
                m_parent->add(m_stackPointers[i]);
                break;
            }
        }

        remove(stackRef);
        escape();
    }

    bool Scope::didEscape() const {
        return m_didEscape;
    }

    void Scope::setLoopContinueLabel(label_id continueLbl) {
        m_continueLbl = continueLbl;
    }
    
    label_id Scope::getLoopContinueLabel() const {
        if (m_parent && m_continueLbl == label_id(-1)) return m_parent->getLoopContinueLabel();
        return m_continueLbl;
    }
    
    void Scope::loopContinue() {
        if (m_parent && m_continueLbl == label_id(-1)) {
            emitEscapeInstructions();
            m_parent->loopContinue();
            return;
        }

        if (m_continueLbl == label_id(-1)) {
            throw Exception("Scope::loopContinue - continue label is unset in the current scope and all parent scopes");
        }

        emitEscapeInstructions();
        m_owner->jump(m_continueLbl);
    }

    void Scope::setLoopBreakLabel(label_id breakLbl) {
        m_breakLbl;
    }
    
    label_id Scope::getLoopBreakLabel() const {
        if (m_parent && m_breakLbl == label_id(-1)) return m_parent->getLoopBreakLabel();
        return m_breakLbl;
    }

    void Scope::loopBreak() {
        if (m_parent && m_breakLbl == label_id(-1)) {
            emitEscapeInstructions();
            m_parent->loopBreak();
            return;
        }

        if (m_breakLbl == label_id(-1)) {
            throw Exception("Scope::loopContinue - break label is unset in the current scope and all parent scopes");
        }

        emitEscapeInstructions();
        m_owner->jump(m_breakLbl);
    }
    
    void Scope::add(const Value& stackPtr) {
        stack_id stackRef = stackPtr.getStackRef();

        for (u32 i = 0;i < m_stackPointers.size();i++) {
            if (m_stackPointers[i].getStackRef() == stackRef) {
                // multiple stack pointers for the same stack_id should
                // all point to the same memory, we only need one here
                return;
            }
        }

        m_stackPointers.push(stackPtr);
    }
    
    void Scope::add(stack_id allocId) {
        m_stackIds.insert(allocId);
    }
    
    void Scope::remove(const Value& stackPtr) {
        stack_id stackRef = stackPtr.getStackRef();

        for (u32 i = 0;i < m_stackPointers.size();i++) {
            if (m_stackPointers[i].getStackRef() == stackRef) {
                m_stackPointers.remove(i);
                break;
            }
        }
    }
    
    void Scope::remove(stack_id allocId) {
        m_stackIds.erase(allocId);

        for (u32 i = 0;i < m_stackPointers.size();i++) {
            if (m_stackPointers[i].getStackRef() == allocId) {
                m_stackPointers.remove(i);
                break;
            }
        }
    }

    void Scope::emitEscapeInstructions() {
        std::unordered_set<stack_id> freedIds;

        for (i64 i = i64(m_stackPointers.size()) - 1;i >= 0;i--) {
            // todo: Some planning has to be done to figure out how what
            // access rights to use here
            m_owner->generateDestruction(m_stackPointers[u32(i)]);
            
            stack_id ref = m_stackPointers[u32(i)].getStackRef();
            m_owner->stackFree(ref);
            freedIds.insert(ref);
        }

        for (auto it : m_stackIds) {
            if (freedIds.count(it) != 0) continue;
            m_owner->stackFree(it);
        }
    }

    void Scope::emitPreReturnInstructions() {
        emitEscapeInstructions();
        if (m_parent) m_parent->emitPreReturnInstructions();
    }
};