#pragma once
#include <codegen/types.h>
#include <utils/Array.h>

namespace codegen {
    class ControlFlowGraph;
    class CodeHolder;

    struct BasicBlock {
        address begin;
        address end;
        Array<address> from;
        Array<address> to;

        BasicBlock* flowsFrom(u32 idx, ControlFlowGraph* graph);
        BasicBlock* flowsTo(u32 idx, ControlFlowGraph* graph);
        bool isLoop(ControlFlowGraph* graph);
    };

    class ControlFlowGraph {
        public:
            ControlFlowGraph();
            ControlFlowGraph(CodeHolder* ch);

            void rebuild(CodeHolder* ch);
            BasicBlock* blockAtAddr(address a);
            u32 blockIdxAtAddr(address a);

            Array<BasicBlock> blocks;
    };
};