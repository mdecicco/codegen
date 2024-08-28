#include <codegen/optimize/DeadCodeElimination.h>
#include <codegen/LabelMap.h>
#include <codegen/LivenessData.h>
#include <codegen/CodeHolder.h>
#include <codegen/IR.h>
#include <codegen/Value.h>
#include <codegen/FunctionBuilder.h>
#include <bind/Function.h>

#include <utils/Array.hpp>

namespace codegen {
    DeadCodeEliminationStep::DeadCodeEliminationStep() : IPostProcessStep() {
    }

    DeadCodeEliminationStep::~DeadCodeEliminationStep() {
    }

    bool DeadCodeEliminationStep::execute(CodeHolder* ch, u32 mask) {
        IWithLogging* log = ch->owner;

        log->logDebug("DeadCodeEliminationStep: Analyzing %s", ch->owner->getFunction()->getSymbolName().c_str());

        Array<u32> deadAddrs;
        for (auto& r : ch->liveness.lifetimes) {
            if (r.usage_count == 0) {
                log->logDebug("dead: [%llu] %s\n", r.begin, ch->code[r.begin].toString().c_str());
                deadAddrs.push((u32)r.begin);
            }
        }

        if (deadAddrs.size() > 0) {
            std::sort(
                deadAddrs.begin(),
                deadAddrs.end(),
                [](u32 a, u32 b) {
                    return a > b;
                }
            );

            for (u32 addr : deadAddrs) {
                ch->code.remove(addr);
            }

            ch->rebuildAll();
                
            return true;
        }

        return false;
    }
};