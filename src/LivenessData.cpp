#include <codegen/LivenessData.h>
#include <codegen/LabelMap.h>
#include <codegen/CodeHolder.h>
#include <codegen/Value.h>
#include <codegen/IR.h>
#include <codegen/interfaces/IPostProcessStep.h>
#include <bind/DataType.h>

#include <utils/Array.hpp>

namespace codegen {
    LivenessData::LivenessData() {}
    
    LivenessData::LivenessData(CodeHolder* ch) {
        rebuild(ch);
    }
    
    void LivenessData::rebuild(CodeHolder* ch) {
        lifetimes.clear();
        regLifetimeMap.clear();

        if (ch->code.size() == 0) return;

        for (address i = 0;i < ch->code.size();i++) {
            const Value* assigns = ch->code[i].assigns();
            // todo: the second part of this if statement came from the old compiler
            //       the new one does away with stack Values. Investigate if what this
            //       was originally intended to do. See what, if anything, is needed
            //       in its place
            if (!assigns /* || assigns->isStack() */) continue;

            if (isLive(assigns->getRegisterId(), i)) continue;

            RegisterLifetime l = { assigns->getRegisterId(), i, i, 0, assigns->getType()->getInfo().is_floating_point == 1 };

            bool do_calc = true;
            while (do_calc) {
                for (address i1 = l.end + 1;i1 < ch->code.size();i1++) {
                    const Value* assigned = ch->code[i1].assigns();
                    if (assigned && assigned->getRegisterId() == l.reg_id) {
                        if (ch->code[i1].involves(l.reg_id, true)) {
                            // if the instruction involves the register's value beyond just assigning it,
                            // it also depends on the value of the register.
                            l.usage_count++;
                            l.end = i1;
                            continue;
                        }

                        break;
                    }

                    if (ch->code[i1].involves(l.reg_id)) {
                        l.end = i1;
                        l.usage_count++;
                    }
                }

                do_calc = false;
                for (address i1 = l.end + 1;i1 < ch->code.size();i1++) {
                    const Instruction& instr1 = ch->code[i1];
                    // If a backwards jump goes into a live range,
                    // then that live range must be extended to fit
                    // the jump (if it doesn't already)

                    if (instr1.op == OpCode::jump) {
                        address jaddr = ch->labels.get(instr1.operands[0].getImm());
                        if (jaddr > i1) continue;
                        if (l.begin < jaddr && l.end >= jaddr && l.end < i1) {
                            l.end = i1;
                            do_calc = true;
                        }
                    } else if (instr1.op == OpCode::branch) {
                        address jaddr = ch->labels.get(instr1.operands[1].getImm());
                        if (jaddr > i1) continue;
                        if (l.begin < jaddr && l.end >= jaddr && l.end < i1) {
                            l.end = i1;
                            do_calc = true;
                        }
                    }
                }
            }

            regLifetimeMap[l.reg_id].push(lifetimes.size());
            lifetimes.push(l);
        }
    }

    utils::Array<RegisterLifetime*> LivenessData::rangesOf(const Value& v) {
        if (v.isEmpty() || v.isImm()) return {};
        auto it = regLifetimeMap.find(v.getRegisterId());
        if (it == regLifetimeMap.end()) return {};

        auto& rangeIndices = it->second;
        utils::Array<RegisterLifetime*> ranges;

        for (u32 i = 0;i < rangeIndices.size();i++) {
            ranges.push(&lifetimes[rangeIndices[i]]);
        }
        return ranges;
    }

    utils::Array<RegisterLifetime*> LivenessData::rangesOf(u32 reg_id) {
        auto it = regLifetimeMap.find(reg_id);
        if (it == regLifetimeMap.end()) return {};

        auto& rangeIndices = it->second;
        utils::Array<RegisterLifetime*> ranges;
        for (u32 i = 0;i < rangeIndices.size();i++) {
            ranges.push(&lifetimes[rangeIndices[i]]);
        }
        return ranges;
    }

    bool LivenessData::isLive(const Value& v, address at) {
        if (v.isEmpty() || v.isImm()) return false;
        auto it = regLifetimeMap.find(v.getRegisterId());
        if (it == regLifetimeMap.end()) return false;

        auto& rangeIndices = it->second;
        for (u32 i = 0;i < rangeIndices.size();i++) {
            auto& range = lifetimes[rangeIndices[i]];
            if (range.begin <= at && range.end > at) return true;
        }

        return false;
    }

    bool LivenessData::isLive(u32 reg_id, address at) {
        auto it = regLifetimeMap.find(reg_id);
        if (it == regLifetimeMap.end()) return false;

        auto& rangeIndices = it->second;
        for (u32 i = 0;i < rangeIndices.size();i++) {
            auto& range = lifetimes[rangeIndices[i]];
            if (range.begin <= at && range.end > at) return true;
        }

        return false;
    }

    RegisterLifetime* LivenessData::getLiveRange(const Value& v, address at) {
        if (v.isEmpty() || v.isImm()) return nullptr;
        auto it = regLifetimeMap.find(v.getRegisterId());
        if (it == regLifetimeMap.end()) return nullptr;

        auto& rangeIndices = it->second;
        for (u32 i = 0;i < rangeIndices.size();i++) {
            auto& range = lifetimes[rangeIndices[i]];
            if (range.begin <= at && range.end > at) return &range;
        }

        return nullptr;
    }

    RegisterLifetime* LivenessData::getLiveRange(u32 reg_id, address at) {
        auto it = regLifetimeMap.find(reg_id);
        if (it == regLifetimeMap.end()) return nullptr;

        auto& rangeIndices = it->second;
        for (u32 i = 0;i < rangeIndices.size();i++) {
            auto& range = lifetimes[rangeIndices[i]];
            if (range.begin <= at && range.end > at) return &range;
        }

        return nullptr;
    }
};