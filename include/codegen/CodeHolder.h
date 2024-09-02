#pragma once
#include <codegen/types.h>
#include <codegen/IR.h>
#include <codegen/LabelMap.h>
#include <codegen/ControlFlowGraph.h>
#include <codegen/LivenessData.h>
#include <utils/Array.h>

namespace codegen {
    class FunctionBuilder;

    class CodeHolder {
        public:
            CodeHolder(const Array<Instruction>& code);

            void rebuildAll();
            void rebuildLabels();
            void rebuildCFG();
            void rebuildLiveness();

            FunctionBuilder* owner;

            LabelMap labels;
            ControlFlowGraph cfg;
            LivenessData liveness;
            Array<Instruction> code;
    };
}