#include <codegen/LabelMap.h>
#include <codegen/CodeHolder.h>
#include <codegen/IR.h>
#include <codegen/interfaces/IPostProcessStep.h>

namespace codegen {
    LabelMap::LabelMap() {}
    
    LabelMap::LabelMap(CodeHolder* ch) {
        rebuild(ch);
    }

    address LabelMap::get(label_id label) const {
        return m_map.at(label);
    }

    void LabelMap::rebuild(CodeHolder* ch) {
        for (address i = 0;i < ch->code.size();i++) {
            if (ch->code[i].op == OpCode::label) {
                label_id lbl = ch->code[i].operands[0].getImm();
                m_map[lbl] = i;
            }
        }
    }
};