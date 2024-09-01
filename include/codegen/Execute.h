#pragma once
#include <codegen/types.h>
#include <codegen/Value.h>
#include <bind/interfaces/ICallHandler.h>
#include <utils/Array.h>
#include <type_traits>
#include <unordered_map>

namespace bind {
    class Function;
};

namespace codegen {
    class CodeHolder;

    class TestExecuterCallHandler : public ICallHandler {
        public:
            TestExecuterCallHandler(CodeHolder* ch);

            virtual void call(Function* target, void* retDest, void** args);
        
        protected:
            CodeHolder* m_code;
    };

    class TestExecuter {
        public:
            TestExecuter(CodeHolder* ch);
            ~TestExecuter();

            void setArg(u32 index, bool value);
            void setArg(u32 index, u8 value);
            void setArg(u32 index, u16 value);
            void setArg(u32 index, u32 value);
            void setArg(u32 index, u64 value);
            void setArg(u32 index, i8 value);
            void setArg(u32 index, i16 value);
            void setArg(u32 index, i32 value);
            void setArg(u32 index, i64 value);
            void setArg(u32 index, f32 value);
            void setArg(u32 index, f64 value);
            void setArg(u32 index, void* value);
            void setThisPtr(void* thisPtr);
            void setReturnValuePointer(void* retDest);
            void execute();

            template <typename T>
            std::enable_if_t<std::is_fundamental_v<T> || std::is_pointer_v<T> || std::is_reference_v<T>, T>
            getRegister(vreg_id regId) const {
                if constexpr (std::is_fundamental_v<T> || std::is_pointer_v<T>) {
                    return *(T*)&m_registers[regId];
                }
                
                if constexpr (std::is_reference_v<T>) {
                    return *(std::remove_reference_t<T>*)&m_registers[regId];
                }
            }

            template <typename T>
            std::enable_if_t<std::is_fundamental_v<T> || std::is_pointer_v<T> || std::is_reference_v<T>, T>
            getRegister(const Value& v) {
                return getRegister<T>(v.getRegisterId());
            }
        
            template <typename T>
            std::enable_if_t<std::is_fundamental_v<T> || std::is_pointer_v<T> || std::is_reference_v<T>, void>
            setRegister(vreg_id regId, T value) const {
                if constexpr (std::is_fundamental_v<T> || std::is_pointer_v<T>) {
                    *(T*)&m_registers[regId] = value;
                }
                
                if constexpr (std::is_reference_v<T>) {
                    *(std::remove_reference_t<T>*)&m_registers[regId] = value;
                }
            }

        protected:
            CodeHolder* m_code;
            FunctionBuilder* m_fb;
            Function* m_func;

            u8* m_stack;
            u64* m_registers;
            void* m_returnPtr;
            u32 m_stackOffset;
            i32 m_instructionIdx;
            std::unordered_map<stack_id, u32> m_stackAddrs;
            std::unordered_map<label_id, i32> m_labelAddrs;
            utils::Array<u64> m_nextCallParams;
    };
};