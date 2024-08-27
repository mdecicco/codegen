#pragma once
#include <codegen/types.h>
#include <codegen/Immediate.h>
#include <codegen/OpCodes.h>
#include <utils/String.h>
#include <utils/Array.h>

namespace bind {
    class DataType;
};

namespace codegen {
    class FunctionBuilder;

    class Value {
        public:
            Value();
            Value(const Value& val);
            Value(FunctionBuilder* func, bool imm);
            Value(FunctionBuilder* func, u8 imm);
            Value(FunctionBuilder* func, u16 imm);
            Value(FunctionBuilder* func, u32 imm);
            Value(FunctionBuilder* func, u64 imm);
            Value(FunctionBuilder* func, i8 imm);
            Value(FunctionBuilder* func, i16 imm);
            Value(FunctionBuilder* func, i32 imm);
            Value(FunctionBuilder* func, i64 imm);
            Value(FunctionBuilder* func, f32 imm);
            Value(FunctionBuilder* func, f64 imm);
            Value(FunctionBuilder* func, ptr imm);

            void reset(const Value& v);
            FunctionBuilder* getOwner() const;
            vreg_id getRegisterId() const;
            const Immediate& getImm() const;
            Immediate& getImm();
            bool isImm() const;
            bool isReg() const;
            bool isLabel() const;
            bool isEmpty() const;

            Value convertedTo(DataType* to) const;

            Value operator +  (const Value& rhs) const;
            Value operator += (const Value& rhs) const;
            Value operator -  (const Value& rhs) const;
            Value operator -= (const Value& rhs) const;
            Value operator *  (const Value& rhs) const;
            Value operator *= (const Value& rhs) const;
            Value operator /  (const Value& rhs) const;
            Value operator /= (const Value& rhs) const;
            Value operator %  (const Value& rhs) const;
            Value operator %= (const Value& rhs) const;
            Value operator ^  (const Value& rhs) const;
            Value operator ^= (const Value& rhs) const;
            Value operator &  (const Value& rhs) const;
            Value operator &= (const Value& rhs) const;
            Value operator |  (const Value& rhs) const;
            Value operator |= (const Value& rhs) const;
            Value operator << (const Value& rhs) const;
            Value operator <<=(const Value& rhs) const;
            Value operator >> (const Value& rhs) const;
            Value operator >>=(const Value& rhs) const;
            Value operator != (const Value& rhs) const;
            Value operator && (const Value& rhs) const;
            Value operator || (const Value& rhs) const; 
            Value operator =  (const Value& rhs) const;
            Value operator == (const Value& rhs) const;
            Value operator <  (const Value& rhs) const;
            Value operator <= (const Value& rhs) const;
            Value operator >  (const Value& rhs) const;
            Value operator >= (const Value& rhs) const;
            Value operator [] (const Value& rhs) const;
            Value operator () (const Array<Value>& args, Value* self) const;
            Value operator -  () const;
            Value operator -- () const;
            Value operator -- (int) const;
            Value operator ++ () const;
            Value operator ++ (int) const;
            Value operator !  () const;
            Value operator ~  () const;
            Value operator *  () const;

            // a &&= b
            Value operator_logicalAndAssign(const Value& rhs) const;

            // a ||= b
            Value operator_logicalOrAssign(const Value& rhs) const;

            bool isEquivalentTo(const Value& v) const;

            String toString() const;
        
        protected:
            friend class FunctionBuilder;
            Value(vreg_id regId, FunctionBuilder* func, DataType* type);
            
            Value genBinaryOp(
                const Value& rhs,
                OpCode _i,
                OpCode _u,
                OpCode _f,
                OpCode _d,
                const char* overrideName,
                bool assignmentOp = false
            ) const;

            Value genUnaryOp(
                OpCode _i,
                OpCode _u,
                OpCode _f,
                OpCode _d,
                const char* overrideName,
                bool resultIsPreOp = false,
                bool assignmentOp = false,
                bool noResultReg = false
            ) const;

            FunctionBuilder* m_owner;
            DataType* m_type;
            bool m_isImm;
            bool m_isLabel;
            vreg_id m_regId;
            Immediate m_imm;
    };
};