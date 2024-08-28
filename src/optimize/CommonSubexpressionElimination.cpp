#include <codegen/optimize/CommonSubexpressionElimination.h>
#include <codegen/CodeHolder.h>
#include <codegen/PostProcessGroup.h>
#include <codegen/IR.h>
#include <codegen/Value.h>
#include <codegen/FunctionBuilder.h>
#include <bind/DataType.h>
#include <bind/Registry.h>
#include <bind/Function.h>

#include <utils/Array.hpp>

namespace codegen {
    CommonSubexpressionEliminationStep::CommonSubexpressionEliminationStep() : IPostProcessStep() {
    }

    CommonSubexpressionEliminationStep::~CommonSubexpressionEliminationStep() {
    }

    bool CommonSubexpressionEliminationStep::execute(CodeHolder* ch, BasicBlock* b, u32 mask) {
        IWithLogging* log = ch->owner;

        log->logDebug("CommonSubexpressionEliminationStep: Analyzing %d to %d of %s", b->begin, b->end, ch->owner->getFunction()->getSymbolName().c_str());

        bool hasChanges = false;
        Array<Instruction*> assignments;
        Array<address> assignmentAddrs;
        for (address i = b->begin;i < b->end;i++) {
            // If the var is being loaded from an address then do nothing...
            // Todo: if no instructions with side effects occur between two identical load instructions,
            //       they produce the same result
            if (ch->code[i].op == OpCode::load) continue;

            // if the var was not assigned with a binary expression then do nothing
            if (ch->code[i].op == OpCode::assign || ch->code[i].op == OpCode::reserve) continue;

            const Value* assigns = ch->code[i].assigns();
            if (!assigns) continue;

            for (u32 a = 0;a < assignments.size();a++) {
                const Instruction& expr = *assignments[a];
                const auto& einfo = Instruction::Info(expr.op);
                u8 aidx = einfo.assignsOperandIndex;
                if (expr.op != ch->code[i].op || expr.operands[aidx].isEquivalentTo(ch->code[i].operands[aidx])) continue;
        
                bool sameArgs = true;
                for (u8 o = 0;o < einfo.operandCount;o++) {
                    if (o == aidx) continue;
                    const Value& expOp = expr.operands[o];
                    const Value& thisOp = ch->code[i].operands[o];
                    if (expOp.isEmpty() != thisOp.isEmpty() || !expOp.isEquivalentTo(thisOp)) {
                        sameArgs = false;
                        break;
                    }
                }

                if (sameArgs) {
                    // if none of the args (including the assignment arg) were assigned between
                    // now and where _assignment_ is from then code[i] can be replaced with
                    // OpCode::assig assigns prev

                    bool doUpdate = true;
                    for (u8 o = 0;o < einfo.operandCount;o++) {
                        const Value& expOp = expr.operands[o];
                        if (expOp.isEmpty()) break;
                        if (expOp.isImm()) continue;

                        u32 begin = assignmentAddrs[a];
                        for (u32 c = begin + 1;c < i;c++) {
                            const Value* assigns = ch->code[c].assigns();
                            if (assigns && assigns->getRegisterId() == expOp.getRegisterId()) {
                                doUpdate = false;
                                break;
                            }
                        }
                    }
                
                    if (doUpdate) {
                        u32 begin = assignmentAddrs[a];
                        log->logDebug("Eliminating [%lu] %s", i, ch->code[i].toString().c_str());
                        log->logDebug("^ Previously [%lu] %s", begin, expr.toString().c_str());

                        const Value& prev = expr.operands[0];
                        ch->code[i].op = OpCode::assign;
                        ch->code[i].operands[1].reset(expr.operands[0]);
                        ch->code[i].operands[2].reset(Value());

                        log->logDebug("^ Updated to [%lu] %s", i, ch->code[i].toString().c_str());
                
                        hasChanges = true;
                    }
                }
            }

            assignments.push(&ch->code[i]);
            assignmentAddrs.push(i);
        }
        
        if (hasChanges) {
            ch->rebuildAll();
            getGroup()->setShouldRepeat(true);
        }

        return hasChanges;
    }
};