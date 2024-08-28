#include <codegen/optimize/CopyPropagation.h>
#include <codegen/CodeHolder.h>
#include <codegen/PostProcessGroup.h>
#include <codegen/IR.h>
#include <codegen/Value.h>
#include <codegen/FunctionBuilder.h>
#include <bind/Function.h>

#include <unordered_map>

namespace codegen {
    CopyPropagationStep::CopyPropagationStep() : IPostProcessStep() {
    }

    CopyPropagationStep::~CopyPropagationStep() {
    }

    bool CopyPropagationStep::execute(CodeHolder* ch, BasicBlock* b, u32 mask) {
        IWithLogging* log = ch->owner;

        log->logDebug("CopyPropagationStep: Analyzing %d to %d of %s", b->begin, b->end, ch->owner->getFunction()->getSymbolName().c_str());

        std::unordered_map<u32, Value*> assignMap;
        std::unordered_map<u32, u32> assignAddrMap;

        bool hasChanges = false;

        for (address i = b->begin;i < b->end;i++) {
            bool isAdd0 = ch->code[i].op == OpCode::uadd && ch->code[i].operands[2].isImm() && (u64)ch->code[i].operands[2].getImm() == 0;
            isAdd0 = isAdd0 || (ch->code[i].op == OpCode::iadd && ch->code[i].operands[2].isImm() && (i64)ch->code[i].operands[2].getImm() == 0);

            if (ch->code[i].op == OpCode::assign || isAdd0) {
                u32 r = ch->code[i].operands[0].getRegisterId();
                if (!ch->code[i].operands[1].isImm()) {
                    auto it = assignMap.find(ch->code[i].operands[1].getRegisterId());
                    if (it != assignMap.end()) {
                        Value *v = it->second;
                        address assAddr = assignAddrMap[ch->code[i].operands[1].getRegisterId()];
                        log->logDebug("Propagating...");
                        log->logDebug("^ (assignment) [%lu] %s...", assAddr, ch->code[assAddr].toString().c_str());
                        log->logDebug("^ (destination) [%lu] %s", i, ch->code[i].toString().c_str());

                        DataType* tp = ch->code[i].operands[1].getType();
                        ch->code[i].operands[1].reset(*v);
                        if (ch->code[i].op >= OpCode::vset && ch->code[i].op <= OpCode::vcross) {
                            // the vector instructions rely on the type information because
                            // the behavior is different for each vector type
                            ch->code[i].operands[1].setType(tp);
                        }
                        hasChanges = true;

                        log->logDebug("^ (result) [%lu] %s", i, ch->code[i].toString().c_str());

                        assignMap[r] = v;
                        assignAddrMap[r] = (u32)i;
                    } else {
                        assignMap[r] = &ch->code[i].operands[1];
                        assignAddrMap[r] = (u32)i;
                    }
                } else {
                    assignMap[r] = &ch->code[i].operands[1];
                    assignAddrMap[r] = (u32)i;
                }

                continue;
            }

            const Value* assigned = ch->code[i].assigns();

            for (u8 o = 0;o < 3;o++) {
                Value& v = ch->code[i].operands[o];
                if (v.isEmpty()) break;
                if (/* v.isArg() || */ v.isImm() /* || v.is_spilled()*/ || (assigned && v.getRegisterId() == assigned->getRegisterId())) continue;
                auto it = assignMap.find(v.getRegisterId());
                if (it != assignMap.end()) {
                    log->logDebug("Propagating...");
                    log->logDebug("^ (original) [%lu] %s", i, ch->code[i].toString().c_str());

                    DataType* tp = ch->code[i].operands[o].getType();
                    ch->code[i].operands[o].reset(*it->second);
                    if (ch->code[i].op >= OpCode::vset && ch->code[i].op <= OpCode::vcross) {
                        // the vector instructions rely on the type information because
                        // the behavior is different for each vector type
                        ch->code[i].operands[o].setType(tp);
                    }
                    hasChanges = true;

                    log->logDebug("^ (updated) [%lu] %s", i, ch->code[i].toString().c_str());
                }
            }

            if (assigned) {
                // Register assigned to expression result that is evaluated at runtime.
                // This means that the new value of the register can't be stored in assignMap.
                // It also means that the register will no longer hold whatever value assignMap
                // has for the register after this instruction, so that value must be forgotten.
                assignMap.erase(assigned->getRegisterId());
                assignAddrMap.erase(assigned->getRegisterId());
            }
        }

        if (hasChanges) {
            ch->rebuildAll();
            getGroup()->setShouldRepeat(true);
        }

        return hasChanges;
    }
};