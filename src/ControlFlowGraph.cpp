#include <codegen/ControlFlowGraph.h>
#include <codegen/CodeHolder.h>
#include <codegen/LabelMap.h>
#include <codegen/IR.h>
#include <codegen/interfaces/IPostProcessStep.h>
#include <utils/Array.hpp>
#include <unordered_set>

namespace codegen {
    BasicBlock* BasicBlock::flowsFrom(u32 idx, ControlFlowGraph* graph) {
        return &graph->blocks[from[idx]];
    }

    BasicBlock* BasicBlock::flowsTo(u32 idx, ControlFlowGraph* graph) {
        return &graph->blocks[to[idx]];
    }

    bool eventuallyFlowsTo(BasicBlock* entry, BasicBlock* target, ControlFlowGraph* graph, std::unordered_set<BasicBlock*>& explored) {
        explored.insert(entry);

        for (u8 i = 0;i < entry->to.size();i++) {
            BasicBlock* to = &graph->blocks[entry->to[i]];
            if (to == target) return true;
            if (explored.count(to)) continue;
            if (eventuallyFlowsTo(to, target, graph, explored)) return true;
        }

        return false;
    }

    bool BasicBlock::isLoop(ControlFlowGraph* graph) {
        std::unordered_set<BasicBlock*> explored;
        return eventuallyFlowsTo(this, this, graph, explored);
    }

    ControlFlowGraph::ControlFlowGraph() {}

    ControlFlowGraph::ControlFlowGraph(CodeHolder* ch) {
        rebuild(ch);
    }

    void ControlFlowGraph::rebuild(CodeHolder* ch) {
        blocks.clear();

        if (ch->code.size() == 0) return;

        // generate blocks
        BasicBlock b = { 0, 0, {}, {} };
        bool push_b = true;
        for (address c = 0;c < ch->code.size();c++) {
            push_b = true;
            const Instruction& i = ch->code[c];
            b.end = c + 1;

            switch (i.op) {
                case OpCode::label: {
                    if (c == b.begin) break;
                    b.end = c;
                    blocks.push(b);
                    b.begin = c;
                    push_b = false;
                    break;
                }
                case OpCode::jump: [[fallthrough]];
                case OpCode::branch: {
                    blocks.push(b);
                    b.begin = b.end;
                    push_b = false;
                    break;
                }
                default: { break; }
            }
        }

        if (push_b) {
            blocks.push(b);
        }

        // generate edges
        for (u32 b = 0;b < blocks.size();b++) {
            BasicBlock& blk = blocks[b];
            const Instruction& end = ch->code[blk.end - 1];
            switch (end.op) {
                case OpCode::jump: {
                    u32 bidx = blockIdxAtAddr(ch->labels.get(end.operands[0].getImm()));
                    blocks[bidx].from.push(b);
                    blk.to.push(bidx);
                    break;
                }
                case OpCode::branch: {
                    u32 bidx;

                    bidx = blockIdxAtAddr(ch->labels.get(end.operands[1].getImm()));
                    blocks[bidx].from.push(b);
                    blk.to.push(bidx);
                    break;
                }
                default: {
                    if (b == blocks.size() - 1) {
                        // end of function
                        break;
                    }

                    blk.to.push(b + 1);
                    blocks[b + 1].from.push(b);
                    break;
                }
            }
        }
    }

    BasicBlock* ControlFlowGraph::blockAtAddr(address a) {
        for (u32 b = 0;b < blocks.size();b++) {
            if (blocks[b].begin == a) return &blocks[b];
        }
        
        return nullptr;
    }

    u32 ControlFlowGraph::blockIdxAtAddr(address a) {
        for (u32 b = 0;b < blocks.size();b++) {
            if (blocks[b].begin == a) return b;
        }
        
        return u32(-1);
    }
};