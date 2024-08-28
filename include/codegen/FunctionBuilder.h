#pragma once
#include <codegen/types.h>
#include <codegen/IR.h>
#include <codegen/interfaces/IWithLogging.h>
#include <codegen/SourceLocation.h>
#include <codegen/SourceMap.h>

#include <utils/Array.h>

namespace bind {
    class DataType;
    class Function;
    class ISymbol;
    class ValuePointer;
};

namespace codegen {
    class FunctionBuilder;

    class InstructionRef {
        public:
            ~InstructionRef();
        
        protected:
            friend class FunctionBuilder;

            InstructionRef(FunctionBuilder* cb, u32 index);

            FunctionBuilder* m_owner;
            u32 m_index;
    };

    class FunctionBuilder : public IWithLogging {
        public:
            FunctionBuilder(Function* func);
            FunctionBuilder(Function* func, FunctionBuilder* parent);
            ~FunctionBuilder();

            InstructionRef add(const Instruction& i);
            label_id label();
            stack_id stackAlloc(u32 size);
            InstructionRef stackAlloc(u32 size, stack_id alloc);
            InstructionRef stackPtr(const Value& ptrDest, stack_id alloc);
            InstructionRef stackFree(stack_id alloc);
            InstructionRef valuePtr(const Value& reg, symbol_id id);
            InstructionRef valuePtr(const Value& reg, ValuePointer* sym);
            InstructionRef thisPtr(const Value& reg);
            InstructionRef retPtr(const Value& reg);
            InstructionRef argument(const Value& reg, u32 argIndex);
            InstructionRef reserve(const Value& reg);
            InstructionRef resolve(const Value& reg, const Value& assignTo);
            InstructionRef load(const Value& dest, const Value& src, u32 offset = 0);
            InstructionRef store(const Value& src, const Value& dest, u32 offset = 0);
            InstructionRef jump(label_id label);
            InstructionRef cvt(const Value& dest, const Value& src);
            InstructionRef cvt(const Value& dest, const Value& src, u64 destTypeHash);
            InstructionRef param(const Value& val);
            InstructionRef call(FunctionBuilder* func, const Value& retDest = Value(), const Value& selfPtr = Value());
            InstructionRef call(Function* func, const Value& retDest = Value(), const Value& selfPtr = Value());
            InstructionRef call(const Value& func, const Value& retDest = Value(), const Value& selfPtr = Value());
            InstructionRef ret(const Value& val = Value());
            InstructionRef branch(const Value& cond, label_id destOnFalse);
        
            InstructionRef _not(const Value& result, const Value& val);
            InstructionRef inv(const Value& result, const Value& val);
            InstructionRef shl(const Value& result, const Value& val, const Value& bits);
            InstructionRef shr(const Value& result, const Value& val, const Value& bits);
            InstructionRef land(const Value& result, const Value& a, const Value& b);
            InstructionRef band(const Value& result, const Value& a, const Value& b);
            InstructionRef lor(const Value& result, const Value& a, const Value& b);
            InstructionRef bor(const Value& result, const Value& a, const Value& b);
            InstructionRef _xor(const Value& result, const Value& a, const Value& b);
            InstructionRef assign(const Value& dest, const Value& src);

            InstructionRef vset(const Value& dest, const Value& src);
            InstructionRef vadd(const Value& dest, const Value& val);
            InstructionRef vsub(const Value& dest, const Value& val);
            InstructionRef vmul(const Value& dest, const Value& val);
            InstructionRef vdiv(const Value& dest, const Value& val);
            InstructionRef vmod(const Value& dest, const Value& val);
            InstructionRef vneg(const Value& val);
            InstructionRef vdot(const Value& result, const Value& a, const Value& b);
            InstructionRef vmag(const Value& result, const Value& val);
            InstructionRef vmagsq(const Value& result, const Value& val);
            InstructionRef vnorm(const Value& val);
            InstructionRef vcross(const Value& result, const Value& a, const Value& b);

            InstructionRef iadd(const Value& result, const Value& a, const Value& b);
            InstructionRef uadd(const Value& result, const Value& a, const Value& b);
            InstructionRef fadd(const Value& result, const Value& a, const Value& b);
            InstructionRef dadd(const Value& result, const Value& a, const Value& b);
            InstructionRef isub(const Value& result, const Value& a, const Value& b);
            InstructionRef usub(const Value& result, const Value& a, const Value& b);
            InstructionRef fsub(const Value& result, const Value& a, const Value& b);
            InstructionRef dsub(const Value& result, const Value& a, const Value& b);
            InstructionRef imul(const Value& result, const Value& a, const Value& b);
            InstructionRef umul(const Value& result, const Value& a, const Value& b);
            InstructionRef fmul(const Value& result, const Value& a, const Value& b);
            InstructionRef dmul(const Value& result, const Value& a, const Value& b);
            InstructionRef idiv(const Value& result, const Value& a, const Value& b);
            InstructionRef udiv(const Value& result, const Value& a, const Value& b);
            InstructionRef fdiv(const Value& result, const Value& a, const Value& b);
            InstructionRef ddiv(const Value& result, const Value& a, const Value& b);
            InstructionRef imod(const Value& result, const Value& a, const Value& b);
            InstructionRef umod(const Value& result, const Value& a, const Value& b);
            InstructionRef fmod(const Value& result, const Value& a, const Value& b);
            InstructionRef dmod(const Value& result, const Value& a, const Value& b);
            InstructionRef ineg(const Value& result, const Value& val);
            InstructionRef fneg(const Value& result, const Value& val);
            InstructionRef dneg(const Value& result, const Value& val);

            InstructionRef iinc(const Value& val);
            InstructionRef uinc(const Value& val);
            InstructionRef finc(const Value& val);
            InstructionRef dinc(const Value& val);
            InstructionRef idec(const Value& val);
            InstructionRef udec(const Value& val);
            InstructionRef fdec(const Value& val);
            InstructionRef ddec(const Value& val);

            InstructionRef ilt(const Value& result, const Value& a, const Value& b);
            InstructionRef ult(const Value& result, const Value& a, const Value& b);
            InstructionRef flt(const Value& result, const Value& a, const Value& b);
            InstructionRef dlt(const Value& result, const Value& a, const Value& b);
            InstructionRef ilte(const Value& result, const Value& a, const Value& b);
            InstructionRef ulte(const Value& result, const Value& a, const Value& b);
            InstructionRef flte(const Value& result, const Value& a, const Value& b);
            InstructionRef dlte(const Value& result, const Value& a, const Value& b);
            InstructionRef igt(const Value& result, const Value& a, const Value& b);
            InstructionRef ugt(const Value& result, const Value& a, const Value& b);
            InstructionRef fgt(const Value& result, const Value& a, const Value& b);
            InstructionRef dgt(const Value& result, const Value& a, const Value& b);
            InstructionRef igte(const Value& result, const Value& a, const Value& b);
            InstructionRef ugte(const Value& result, const Value& a, const Value& b);
            InstructionRef fgte(const Value& result, const Value& a, const Value& b);
            InstructionRef dgte(const Value& result, const Value& a, const Value& b);
            InstructionRef ieq(const Value& result, const Value& a, const Value& b);
            InstructionRef ueq(const Value& result, const Value& a, const Value& b);
            InstructionRef feq(const Value& result, const Value& a, const Value& b);
            InstructionRef deq(const Value& result, const Value& a, const Value& b);
            InstructionRef ineq(const Value& result, const Value& a, const Value& b);
            InstructionRef uneq(const Value& result, const Value& a, const Value& b);
            InstructionRef fneq(const Value& result, const Value& a, const Value& b);
            InstructionRef dneq(const Value& result, const Value& a, const Value& b);

            Value generateCall(
                FunctionBuilder* func,
                const Array<Value>& args = Array<Value>(),
                const Value& selfPtr = Value()
            );
            
            Value generateCall(
                Function* func,
                const Array<Value>& args = Array<Value>(),
                const Value& selfPtr = Value()
            );
            
            Value generateCall(
                const Value& func,
                const Array<Value>& args = Array<Value>(),
                const Value& selfPtr = Value()
            );

            void generateConstruction(const Value& destPtr, const Array<Value>& args);

            void generateReturn(const Value& val = Value());

            Value label(label_id label);
            Value val(DataType* tp);
            Value val(ValuePointer* value);
            Value val(bool imm);
            Value val(u8 imm);
            Value val(u16 imm);
            Value val(u32 imm);
            Value val(u64 imm);
            Value val(i8 imm);
            Value val(i16 imm);
            Value val(i32 imm);
            Value val(i64 imm);
            Value val(f32 imm);
            Value val(f64 imm);
            Value val(ptr imm);

            Function* getFunction() const;
            Array<Instruction>& getCode();
            const Array<Instruction>& getCode() const;
            Value getThis() const;
            Value getArg(u32 index) const;
            Value getRetPtr();

            stack_id getNextAllocId() const;
            stack_id reserveAllocId();
            void setCurrentSourceLocation(const SourceLocation& src);
            const SourceMap* getSourceMap() const;
            void enableValidation();

        protected:
            friend class InstructionRef;

            void emitPrologue();

            Function* m_function;
            FunctionBuilder* m_parent;
            Array<Instruction> m_code;
            label_id m_nextLabel;
            vreg_id m_nextReg;
            stack_id m_nextAlloc;
            SourceLocation m_currentSrcLoc;
            SourceMap m_srcMap;
            bool m_validationEnabled;

            Value m_thisPtr;
            Array<Value> m_args;
    };
};