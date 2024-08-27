#pragma once
#include <codegen/types.h>

namespace codegen {
    //
    // 'vreg' refers to a val which represents a virtual register
    // 'imm' refers to a val which represents a hard-coded value
    // 'val' can mean either of the above two things
    //
    // The operands for all unary/binary operations must be of the
    // same data type, with the following exceptions:
    // - Result operand for boolean operators must be of boolean
    //   type
    // 
    enum class OpCode : u32 {
        // Do nothing. Should be ignored when processing and translating to target arch
        noop,

        // Labels address for instructions which represent jumps
        //
        // Operand 0 will be the label
        label,

        // Allocates space on the stack
        //
        // Operand 0 will be u32 imm for allocation size in bytes
        // Operand 1 will be u32 imm for allocation ID
        stack_alloc,

        // Gets pointer to stack allocation by ID
        //
        // Operand 0 will be vreg which will receive the pointer
        // Operand 1 will be u32 imm for allocation ID
        stack_ptr,

        // Frees space that was allocated on the stack
        //
        // Operand 0 will be u32 imm for allocation ID
        stack_free,

        // Gets a pointer to a global value by ID
        // Operand 0 will be vreg which will receive the pointer
        // Operand 1 will be u64 imm for the value ID
        value_ptr,

        // Reserves a virtual register which will be assigned later via resolve
        // This instruction counts as an assignment. The reserve/resolve instructions
        // exist exclusively to influence code optimization and register allocation.
        // 
        // Operand 0 will be vreg which will be assigned later
        reserve,

        // Resolves a pending assignment which was promised by the reserve instruction
        // This instruction is equivalent to the assign instruction, with the following
        // exceptions:
        // - The relationship with the reserve instruction
        // - This instruction does not count as an assignment
        //
        // Operand 0 will be vreg which was promised an assignment
        // Operand 1 will be val who's value will be copied
        resolve,

        // Load value from address
        //
        // Operand 0 will be vreg which will receive the loaded value
        // Operand 1 will be vreg which holds the address to load from
        // (optional) Operand 2 can be imm which holds offset for address
        load,

        // Store value at address
        //
        // Operand 0 will be val which holds value to store
        // Operand 1 will be vreg which holds address to store value in
        // (optional) Operand 2 can be imm which holds offset for address
        store,

        // Unconditional jump to a label
        //
        // Operand 0 will be the label to jump to
        jump,

        // Converts the value of a vreg from one data type to another
        // Note: Only applicable to primitive types
        //
        // Operand 0 will be the vreg which receives the converted value
        // Operand 1 will be the val to convert
        // Operand 2 will be u32 imm ID of the data type to convert to
        cvt,

        // Specifies a parameter to a function call
        // Multiple parameters will be specified in left to right order
        //
        // Operand 0 will be val representing the parameter to pass
        param,

        // Calls a function
        //
        // Operand 0 will be one of:
        // - Before compilation completes:
        //     - imm pointer to the Function being called
        //     - vreg pointer to the Closure being called
        // - After compilation completes:
        //     - imm function_id of the function being called
        //     - vreg pointer to the Closure being called
        //
        // In all cases, the data type of operand 0 will be the function
        // signature
        //
        // Operand 1 will be one of:
        //     - vreg which holds address of return value
        //     - vreg which will receive the return value, if the function
        //       returns a primitive
        //     - empty, if the function does not return a value
        // Operand 2 will be one of:
        //     - vreg which holds address of 'this'
        //     - empty, if the function is not a class method or pseudo-method
        call,

        // Returns from the current function
        //
        // All functions return via output parameters, so this instruction
        // needs no operand
        ret,

        // Branches to one of two labels based on the value of a vreg
        //
        // Operand 0 will be vreg which holds a boolean value
        // Operand 1 will be the label to jump to if op 0 is false
        branch,

        _not,       // op0 = !op1
        inv,        // op0 = ~op1
        shl,        // op0 = op1 << op2
        shr,        // op0 = op1 >> op2
        land,       // op0 = op1 && op2
        band,       // op0 = op1 & op2
        lor,        // op0 = op1 || op2
        bor,        // op0 = op1 | op2
        _xor,       // op0 = op1 ^ op2
        assign,     // op0 = op1

        vset,       // op0[0] = (op1[0] | op1), ..., op0[N] = (op1[N] | op1)
        vadd,       // op0[0] = op0[0] + (op1[0] | op1), ..., op0[N] = op0[N] + (op1[N] | op1)
        vsub,       // op0[0] = op0[0] - (op1[0] | op1), ..., op0[N] = op0[N] - (op1[N] | op1)
        vmul,       // op0[0] = op0[0] * (op1[0] | op1), ..., op0[N] = op0[N] * (op1[N] | op1)
        vdiv,       // op0[0] = op0[0] / (op1[0] | op1), ..., op0[N] = op0[N] / (op1[N] | op1)
        vmod,       // op0[0] = op0[0] % (op1[0] | op1), ..., op0[N] = op0[N] % (op1[N] | op1)
        vneg,       // op0[0] = -op0[0], ..., op0[N] = -op0[N]
        vdot,       // op0 = dot(op1[0], op2[0])
        vmag,       // op0 = length(op1)
        vmagsq,     // op0 = lengthSq(op1)
        vnorm,      // op0[0] = op0[0] / length(op0), ..., op0[N] = op0[N] / length(op0)
        vcross,     // op0[0] = cross(op1, op2)[0], ..., op0[N] = cross(op1, op2)[N]
        
        iadd,      // op0 = op1 + op2 (signed integer)
        uadd,      // op0 = op1 + op2 (unsigned integer)
        fadd,      // op0 = op1 + op2 (32 bit floating point)
        dadd,      // op0 = op1 + op2 (64 bit floating point)
        isub,      // op0 = op1 - op2 (signed integer)
        usub,      // op0 = op1 - op2 (unsigned integer)
        fsub,      // op0 = op1 - op2 (32 bit floating point)
        dsub,      // op0 = op1 - op2 (64 bit floating point)
        imul,      // op0 = op1 * op2 (signed integer)
        umul,      // op0 = op1 * op2 (unsigned integer)
        fmul,      // op0 = op1 * op2 (32 bit floating point)
        dmul,      // op0 = op1 * op2 (64 bit floating point)
        idiv,      // op0 = op1 / op2 (signed integer)
        udiv,      // op0 = op1 / op2 (unsigned integer)
        fdiv,      // op0 = op1 / op2 (32 bit floating point)
        ddiv,      // op0 = op1 / op2 (64 bit floating point)
        imod,      // op0 = op1 % op2 (signed integer)
        umod,      // op0 = op1 % op2 (unsigned integer)
        fmod,      // op0 = op1 % op2 (32 bit floating point)
        dmod,      // op0 = op1 % op2 (64 bit floating point)
        ineg,      // op0 = -op1 (signed integer)
        fneg,      // op0 = -op1 (32 bit floating point)
        dneg,      // op0 = -op1 (64 bit floating point)

        iinc,      // op0++ (signed integer)
        uinc,      // op0++ (unsigned integer)
        finc,      // op0++ (32 bit floating point)
        dinc,      // op0++ (64 bit floating point)
        idec,      // op0-- (signed integer)
        udec,      // op0-- (unsigned integer)
        fdec,      // op0-- (32 bit floating point)
        ddec,      // op0-- (64 bit floating point)

        ilt,       // op0 = op1 < op2 (signed integer)
        ult,       // op0 = op1 < op2 (unsigned integer)
        flt,       // op0 = op1 < op2 (32 bit floating point)
        dlt,       // op0 = op1 < op2 (64 bit floating point)
        ilte,      // op0 = op1 <= op2 (signed integer)
        ulte,      // op0 = op1 <= op2 (unsigned integer)
        flte,      // op0 = op1 <= op2 (32 bit floating point)
        dlte,      // op0 = op1 <= op2 (64 bit floating point)
        igt,       // op0 = op1 > op2 (signed integer)
        ugt,       // op0 = op1 > op2 (unsigned integer)
        fgt,       // op0 = op1 > op2 (32 bit floating point)
        dgt,       // op0 = op1 > op2 (64 bit floating point)
        igte,      // op0 = op1 >= op2 (signed integer)
        ugte,      // op0 = op1 >= op2 (unsigned integer)
        fgte,      // op0 = op1 >= op2 (32 bit floating point)
        dgte,      // op0 = op1 >= op2 (64 bit floating point)
        ieq,       // op0 = op1 == op2 (signed integer)
        ueq,       // op0 = op1 == op2 (unsigned integer)
        feq,       // op0 = op1 == op2 (32 bit floating point)
        deq,       // op0 = op1 == op2 (64 bit floating point)
        ineq,      // op0 = op1 != op2 (signed integer)
        uneq,      // op0 = op1 != op2 (unsigned integer)
        fneq,      // op0 = op1 != op2 (32 bit floating point)
        dneq       // op0 = op1 != op2 (64 bit floating point)
    };
};