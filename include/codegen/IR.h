#pragma once
#include <codegen/types.h>
#include <codegen/Value.h>
#include <codegen/OpCodes.h>
#include <utils/String.h>

namespace codegen {
    class FunctionBuilder;

    enum class OperandType : u8 {
        /** operand unused */
        Unused,

        /** immediate value */
        Immediate,

        /** label id (immediate) */
        Label,

        /** virtual register id */
        Register,

        /** register or immediate */
        Value,

        /** function (immediate function id or closure pointer in vreg) */
        Function
    };

    struct opInfo {
        const char* name;
        u8 operandCount;
        OperandType operands[3];
        u8 assignsOperandIndex;
        unsigned hasExternalSideEffects : 1;
        unsigned hasSideEffectsForOp0 : 1;
        unsigned hasSideEffectsForOp1 : 1;
        unsigned hasSideEffectsForOp2 : 1;
    };
    
    class Instruction {
        public:
            Instruction();
            Instruction(OpCode code);

            OpCode op;
            Value operands[3];
            union {
                struct { u8 componentCount; } vset;
                struct { u8 componentCount; } vadd;
                struct { u8 componentCount; } vsub;
                struct { u8 componentCount; } vmul;
                struct { u8 componentCount; } vdiv;
                struct { u8 componentCount; } vmod;
                struct { u8 componentCount; } vneg;
                struct { u8 componentCount; } vdot;
                struct { u8 componentCount; } vmag;
                struct { u8 componentCount; } vmagsq;
                struct { u8 componentCount; } vnorm;
                struct { u8 componentCount; } vcross;
            } options;

            /**
             * @brief Returns pointer to Value that would be assigned by this instruction
             * 
             * @return Pointer to Value if op refers to an assigning instruction, otherwise null
             */
            const Value* assigns() const;

            /**
             * @brief Returns true if this instruction involves the specified vreg ID
             * 
             * @param reg vreg ID to check
             * @param excludeAssignment Whether to ignore involvement if the vreg ID is
             *                          only involved by being assigned to by operation
             * @return Whether or not the specified vreg ID is involved 
             */
            bool involves(vreg_id reg, bool excludeAssignment = false) const;

            Instruction& operator =(const Instruction& rhs);
            String toString() const;

            static const opInfo& Info(OpCode code);
    };

    class InstructionRef {
        public:
            Instruction* operator->();
        
        protected:
            friend class FunctionBuilder;

            InstructionRef(FunctionBuilder* cb, u32 index);

            FunctionBuilder* m_owner;
            u32 m_index;
    };
};