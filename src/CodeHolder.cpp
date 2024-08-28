#include <codegen/CodeHolder.h>
#include <codegen/IR.h>

#include <utils/Array.hpp>

namespace codegen {
    CodeHolder::CodeHolder(const Array<Instruction>& _code) : code(_code) {
    }

    void CodeHolder::rebuildAll() {
        labels.rebuild(this);
        cfg.rebuild(this);
        liveness.rebuild(this);
    }

    void CodeHolder::rebuildLabels() {
        labels.rebuild(this);
    }

    void CodeHolder::rebuildCFG() {
        cfg.rebuild(this);
    }

    void CodeHolder::rebuildLiveness() {
        liveness.rebuild(this);
    }
};