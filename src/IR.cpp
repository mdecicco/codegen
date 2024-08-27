#include <codegen/IR.h>

namespace codegen {
    constexpr opInfo opcodeInfo[] = {
        // opcode name, operand count, { op[0] type, op[1] type, op[2] type }, assigns operand index, has external side effects, has side effects (op 1, 2, 3)
        { "noop"          , 0, { OperandType::Unused   , OperandType::Unused   , OperandType::Unused    }, 0xFF, 0, 0, 0, 0 },
        { "label"         , 1, { OperandType::Label    , OperandType::Unused   , OperandType::Unused    }, 0xFF, 0, 0, 0, 0 },
        { "stack_alloc"   , 2, { OperandType::Immediate, OperandType::Immediate, OperandType::Unused    }, 0xFF, 0, 0, 0, 0 },
        { "stack_ptr"     , 2, { OperandType::Register , OperandType::Immediate, OperandType::Unused    }, 0   , 0, 0, 0, 0 },
        { "stack_free"    , 1, { OperandType::Immediate, OperandType::Unused   , OperandType::Unused    }, 0xFF, 0, 0, 0, 0 },
        { "reserve"       , 1, { OperandType::Register , OperandType::Unused   , OperandType::Unused    }, 0   , 0, 0, 0, 0 },
        { "resolve"       , 2, { OperandType::Register , OperandType::Value    , OperandType::Unused    }, 0xFF, 0, 0, 0, 0 },
        { "load"          , 3, { OperandType::Register , OperandType::Register , OperandType::Immediate }, 0   , 0, 0, 0, 0 },
        { "store"         , 3, { OperandType::Value    , OperandType::Register , OperandType::Immediate }, 0xFF, 0, 0, 0, 0 },
        { "jump"          , 1, { OperandType::Label    , OperandType::Unused   , OperandType::Unused    }, 0xFF, 0, 0, 0, 0 },
        { "cvt"           , 3, { OperandType::Register , OperandType::Value    , OperandType::Immediate }, 0   , 0, 0, 0, 0 },
        { "param"         , 2, { OperandType::Value    , OperandType::Immediate, OperandType::Unused    }, 0xFF, 0, 0, 0, 0 },
        { "call"          , 1, { OperandType::Function , OperandType::Unused   , OperandType::Unused    }, 0xFF, 1, 0, 0, 0 },
        { "ret"           , 0, { OperandType::Unused   , OperandType::Unused   , OperandType::Unused    }, 0xFF, 0, 0, 0, 0 },
        { "branch"        , 3, { OperandType::Register , OperandType::Label    , OperandType::Label     }, 0xFF, 0, 0, 0, 0 },
        
        { "_not"          , 2, { OperandType::Register , OperandType::Value    , OperandType::Unused    }, 0   , 0, 0, 0, 0 },
        { "inv"           , 2, { OperandType::Register , OperandType::Value    , OperandType::Unused    }, 0   , 0, 0, 0, 0 },
        { "shl"           , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "shr"           , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "land"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "band"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "lor"           , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "bor"           , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "_xor"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "assign"        , 2, { OperandType::Register , OperandType::Value    , OperandType::Unused    }, 0   , 0, 0, 0, 0 },

        { "vset"          , 2, { OperandType::Register , OperandType::Register , OperandType::Unused    }, 0xFF, 0, 1, 0, 0 },
        { "vadd"          , 2, { OperandType::Register , OperandType::Register , OperandType::Unused    }, 0xFF, 0, 1, 0, 0 },
        { "vsub"          , 2, { OperandType::Register , OperandType::Register , OperandType::Unused    }, 0xFF, 0, 1, 0, 0 },
        { "vmul"          , 2, { OperandType::Register , OperandType::Register , OperandType::Unused    }, 0xFF, 0, 1, 0, 0 },
        { "vdiv"          , 2, { OperandType::Register , OperandType::Register , OperandType::Unused    }, 0xFF, 0, 1, 0, 0 },
        { "vmod"          , 2, { OperandType::Register , OperandType::Register , OperandType::Unused    }, 0xFF, 0, 1, 0, 0 },
        { "vneg"          , 1, { OperandType::Register , OperandType::Unused   , OperandType::Unused    }, 0xFF, 0, 1, 0, 0 },
        { "vdot"          , 3, { OperandType::Register , OperandType::Register , OperandType::Register  }, 0   , 0, 0, 0, 0 },
        { "vmag"          , 2, { OperandType::Register , OperandType::Register , OperandType::Unused    }, 0   , 0, 0, 0, 0 },
        { "vmagsq"        , 2, { OperandType::Register , OperandType::Register , OperandType::Unused    }, 0   , 0, 0, 0, 0 },
        { "vnorm"         , 1, { OperandType::Register , OperandType::Unused   , OperandType::Unused    }, 0xFF, 0, 1, 0, 0 },
        { "vcross"        , 3, { OperandType::Register , OperandType::Register , OperandType::Register  }, 0xFF, 0, 1, 0, 0 },

        { "iadd"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "uadd"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "fadd"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "dadd"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "isub"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "usub"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "fsub"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "dsub"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "imul"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "umul"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "fmul"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "dmul"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "idiv"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "udiv"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "fdiv"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "ddiv"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "imod"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "umod"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "fmod"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "dmod"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "ineg"          , 2, { OperandType::Register , OperandType::Value    , OperandType::Unused    }, 0   , 0, 0, 0, 0 },
        { "fneg"          , 2, { OperandType::Register , OperandType::Value    , OperandType::Unused    }, 0   , 0, 0, 0, 0 },
        { "dneg"          , 2, { OperandType::Register , OperandType::Value    , OperandType::Unused    }, 0   , 0, 0, 0, 0 },

        { "iinc"          , 1, { OperandType::Register , OperandType::Unused   , OperandType::Unused    }, 0   , 0, 0, 0, 0 },
        { "uinc"          , 1, { OperandType::Register , OperandType::Unused   , OperandType::Unused    }, 0   , 0, 0, 0, 0 },
        { "finc"          , 1, { OperandType::Register , OperandType::Unused   , OperandType::Unused    }, 0   , 0, 0, 0, 0 },
        { "dinc"          , 1, { OperandType::Register , OperandType::Unused   , OperandType::Unused    }, 0   , 0, 0, 0, 0 },
        { "idec"          , 1, { OperandType::Register , OperandType::Unused   , OperandType::Unused    }, 0   , 0, 0, 0, 0 },
        { "udec"          , 1, { OperandType::Register , OperandType::Unused   , OperandType::Unused    }, 0   , 0, 0, 0, 0 },
        { "fdec"          , 1, { OperandType::Register , OperandType::Unused   , OperandType::Unused    }, 0   , 0, 0, 0, 0 },
        { "ddec"          , 1, { OperandType::Register , OperandType::Unused   , OperandType::Unused    }, 0   , 0, 0, 0, 0 },

        { "ilt"           , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "ult"           , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "flt"           , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "dlt"           , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "ilte"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "ulte"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "flte"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "dlte"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "igt"           , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "ugt"           , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "fgt"           , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "dgt"           , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "igte"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "ugte"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "fgte"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "dgte"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "ieq"           , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "ueq"           , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "feq"           , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "deq"           , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "ineq"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "uneq"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "fneq"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 },
        { "dneq"          , 3, { OperandType::Register , OperandType::Value    , OperandType::Value     }, 0   , 0, 0, 0, 0 }
    };

    
    Instruction::Instruction() : op(OpCode::noop) {
    }

    Instruction::Instruction(OpCode code) : op(code) {
    }

    const Value* Instruction::assigns() const {
        u8 assignsIdx = opcodeInfo[u32(op)].assignsOperandIndex;
        if (assignsIdx == 0xFF) return nullptr;

        return &operands[assignsIdx];
    }

    bool Instruction::involves(vreg_id reg, bool excludeAssignment) const {
        if (!excludeAssignment) {
            return operands[0].getRegisterId() == reg || operands[1].getRegisterId() == reg || operands[2].getRegisterId() == reg;
        }

        u8 assignsIdx = opcodeInfo[u32(op)].assignsOperandIndex;
        if (operands[0].getRegisterId() == reg && assignsIdx != 0) return true;
        if (operands[1].getRegisterId() == reg && assignsIdx != 1) return true;
        if (operands[2].getRegisterId() == reg && assignsIdx != 2) return true;

        return false;
    }

    Instruction& Instruction::operator =(const Instruction& rhs) {
        op = rhs.op;
        operands[0].reset(rhs.operands[0]);
        operands[1].reset(rhs.operands[1]);
        operands[2].reset(rhs.operands[2]);
        return *this;
    }

    String Instruction::toString() const {
        return String();
    }

    const opInfo& Instruction::Info(OpCode code) {
        return opcodeInfo[u32(code)];
    }
};