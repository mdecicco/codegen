#include <codegen/optimize/Optimize.h>
#include <codegen/PostProcessGroup.h>

#include <codegen/optimize/CommonSubexpressionElimination.h>
#include <codegen/optimize/ConstantFolding.h>
#include <codegen/optimize/DeadCodeElimination.h>
#include <codegen/optimize/CopyPropagation.h>
#include <codegen/optimize/ReduceMemoryAccessStep.h>

namespace codegen {
    IPostProcessStep* defaultOptimizations() {
        PostProcessGroup* outer = new PostProcessGroup();

        PostProcessGroup* inner = new PostProcessGroup();

        inner->addStep(new CopyPropagationStep());
        inner->addStep(new CommonSubexpressionEliminationStep());
        inner->addStep(new ReduceMemoryAccessStep());

        outer->addStep(inner);
        outer->addStep(new ConstantFoldingStep());
        outer->addStep(new DeadCodeEliminationStep());

        return outer;
    }
};