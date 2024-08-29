#include <codegen/IR.h>
#include <codegen/FunctionBuilder.h>
#include <bind/Registry.h>
#include <bind/DataType.h>
#include <bind/ValuePointer.h>
#include <bind/FunctionType.h>
#include <bind/PointerType.h>

namespace codegen {
    constexpr opInfo opcodeInfo[] = {
        // opcode name, operand count, { op[0] type, op[1] type, op[2] type }, assigns operand index, has external side effects, has side effects (op 1, 2, 3)
        { "noop"          , 0, { OperandType::Unused   , OperandType::Unused   , OperandType::Unused    }, 0xFF, 0, 0, 0, 0 },
        { "label"         , 1, { OperandType::Label    , OperandType::Unused   , OperandType::Unused    }, 0xFF, 0, 0, 0, 0 },
        { "stack_alloc"   , 2, { OperandType::Immediate, OperandType::Immediate, OperandType::Unused    }, 0xFF, 0, 0, 0, 0 },
        { "stack_ptr"     , 2, { OperandType::Register , OperandType::Immediate, OperandType::Unused    }, 0   , 0, 0, 0, 0 },
        { "stack_free"    , 1, { OperandType::Immediate, OperandType::Unused   , OperandType::Unused    }, 0xFF, 0, 0, 0, 0 },
        { "value_ptr"     , 2, { OperandType::Register , OperandType::Immediate, OperandType::Unused    }, 0   , 0, 0, 0, 0 },
        { "this_ptr"      , 1, { OperandType::Register , OperandType::Unused   , OperandType::Unused    }, 0   , 0, 0, 0, 0 },
        { "ret_ptr"       , 1, { OperandType::Register , OperandType::Unused   , OperandType::Unused    }, 0   , 0, 0, 0, 0 },
        { "argument"      , 2, { OperandType::Register , OperandType::Immediate, OperandType::Unused    }, 0   , 0, 0, 0, 0 },
        { "reserve"       , 1, { OperandType::Register , OperandType::Unused   , OperandType::Unused    }, 0   , 0, 0, 0, 0 },
        { "resolve"       , 2, { OperandType::Register , OperandType::Value    , OperandType::Unused    }, 0xFF, 0, 0, 0, 0 },
        { "load"          , 3, { OperandType::Register , OperandType::Register , OperandType::Immediate }, 0   , 0, 0, 0, 0 },
        { "store"         , 3, { OperandType::Value    , OperandType::Register , OperandType::Immediate }, 0xFF, 0, 0, 0, 0 },
        { "jump"          , 1, { OperandType::Label    , OperandType::Unused   , OperandType::Unused    }, 0xFF, 0, 0, 0, 0 },
        { "cvt"           , 3, { OperandType::Register , OperandType::Value    , OperandType::Immediate }, 0   , 0, 0, 0, 0 },
        { "param"         , 1, { OperandType::Value    , OperandType::Unused   , OperandType::Unused    }, 0xFF, 0, 0, 0, 0 },
        { "call"          , 2, { OperandType::Function , OperandType::Register , OperandType::Unused    }, 1   , 1, 0, 0, 0 },
        { "ret"           , 1, { OperandType::Value    , OperandType::Unused   , OperandType::Unused    }, 0xFF, 0, 0, 0, 0 },
        { "branch"        , 2, { OperandType::Register , OperandType::Label    , OperandType::Unused    }, 0xFF, 0, 0, 0, 0 },
        
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

    String getPropPath(DataType* tp, u32 offset) {
        auto& props = tp->getProps();

        for (u32 i = 0;i < props.size();i++) {
            auto& p = props[i];
            if (p.offset == -1) continue;
            u32 pOff = u32(p.offset);

            if (pOff == offset && (p.type->getInfo().is_primitive || p.type->getInfo().is_pointer)) {
                return p.name;
            }
            
            if (pOff <= offset && pOff + p.type->getInfo().size > offset && p.type->getInfo().is_pointer == 0) {
                utils::String path = getPropPath(p.type, offset - u32(pOff));
                if (path.size() > 0) return p.name + "." + path;
            }
        }

        return "";
    }



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
        auto& info = opcodeInfo[u32(op)];
        utils::String s = info.name;

        bool commentStarted = false;
        
        for (u8 o = 0;o < info.operandCount;o++) {
            if (op == OpCode::cvt && o == 2 && operands[2].isImm()) {
                DataType* tp = Registry::GetType(operands[2].getImm());
                if (tp) s += String::Format(" <Type %s>", tp->getSymbolName().c_str());
                else s += " <Invalid Type ID>";
                continue;
            } else if (op == OpCode::value_ptr) {
                if (o == 1 && operands[1].isImm() && operands[2].isImm()) {
                    ValuePointer* p = Registry::GetValue(operands[1].getImm());
                    if (p) s += String::Format(" <Global '%s'>", p->getSymbolName().c_str());
                    else s += " <Invalid Value ID>";
                    break;
                }
            } else if ((op == OpCode::load || op == OpCode::store) && o == 1 && !operands[2].isEmpty() && operands[2].isImm()) {
                s += String(" ") + operands[2].toString() + "(" + operands[1].toString() + ")";
                break;
            } else if (op == OpCode::call && o == 1) {
                DataType* retTp = ((FunctionType*)operands[0].getType())->getReturnType();
                if (retTp->getInfo().size > 0) {
                    s += String(" -> ") + operands[1].toString();
                }
                break;
            }

            s += String(" ") + operands[o].toString();
        }

        if (
            op == OpCode::uadd && operands[1].isReg() &&
            operands[1].getType()->getInfo().is_pointer &&
            operands[2].isImm() &&
            operands[2].getType()->getInfo().is_integral
        ) {
            // Is likely a property offset
            DataType* pointedTp = ((PointerType*)operands[1].getType())->getDestinationType();

            u32 offset = operands[2].getImm();
            utils::String path = getPropPath(pointedTp, offset);

            if (path.size() > 0) {
                // Yup
                const String& name = operands[1].getName();
                s += String(" ; ") + (name.size() > 0 ? name : pointedTp->getFullName()) + "." + path;
                commentStarted = true;
            }
        } else if (op == OpCode::load || op == OpCode::store) {
            // Is likely a property offset
            DataType* pointedTp = ((PointerType*)operands[1].getType())->getDestinationType();
            u32 offset = operands[2].getImm();
            utils::String path = getPropPath(pointedTp, offset);

            if (path.size() > 0) {
                // Yup
                const String& name = operands[1].getName();
                s += String(" ; ") + (name.size() > 0 ? name : pointedTp->getFullName()) + "." + path;
                commentStarted = true;
            }
        }

        // if (comment.size() > 0) {
        //     if (!commentStarted) s += " ; ";
        //     else s += ", ";
        //     s += comment;
        // }

        return s;
    }

    const opInfo& Instruction::Info(OpCode code) {
        return opcodeInfo[u32(code)];
    }


    InstructionRef::InstructionRef(FunctionBuilder* fn, u32 idx) : m_owner(fn), m_index(idx) {
    }

    Instruction* InstructionRef::operator->() {
        return &m_owner->m_code[m_index];
    }
};