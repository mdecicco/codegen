#pragma once
#include <codegen/types.h>
#include <unordered_map>

namespace codegen {
    class CodeHolder;
    class LabelMap {
        public:
            LabelMap();
            LabelMap(CodeHolder* ch);

            address get(label_id label) const;

            void rebuild(CodeHolder* ch);

        protected:
            std::unordered_map<label_id, address> m_map;
    };
};