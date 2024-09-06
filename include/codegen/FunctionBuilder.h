#pragma once
#include <codegen/types.h>
#include <codegen/IR.h>
#include <codegen/SourceLocation.h>
#include <codegen/SourceMap.h>
#include <codegen/Scope.h>
#include <bind/Registry.hpp>
#include <utils/Array.h>
#include <utils/interfaces/IWithLogging.h>

namespace bind {
    class DataType;
    class Function;
    class ISymbol;
    class ValuePointer;
};

namespace codegen {
    class FunctionBuilder : public IWithLogging {
        public:
            FunctionBuilder(Function* func);
            FunctionBuilder(Function* func, FunctionBuilder* parent);
            ~FunctionBuilder();

            InstructionRef add(const Instruction& i);

            
            /**
             * @brief Allocates a new label ID and optionally adds it to the code
             * 
             * @param doAddToCode Whether or not the generated label ID should be added to the code
             * @param name Name of the label
             */
            label_id label(bool doAddToCode = true, const String& name = String());

            /**
             * @brief Adds a label to the code. Each `label_id` must only be added to the code one time
             */
            InstructionRef label(label_id label);
            
            /**
             * @brief Allocates `size` bytes on the stack and returns the stack ID
             */
            stack_id stackAlloc(u32 size);
            
            /**
             * @brief Allocates `size` bytes on the stack for the provided `allocId`
             */
            InstructionRef stackAlloc(u32 size, stack_id allocId);
            
            /**
             * @brief Gets a pointer to the stack space referred to by `allocId` and stores it in `ptrDest`
             * 
             * @param ptrDest Must refer to a register, and must have a pointer type
             */
            InstructionRef stackPtr(const Value& ptrDest, stack_id allocId);
            
            /**
             * @brief Frees a stack allocation referred to by `allocId`
             */
            InstructionRef stackFree(stack_id allocId);
            
            /**
             * @brief Gets a pointer to the value represented by a `ValuePointer` with some `id` and stores it
             * in `ptrDest`
             * 
             * @param ptrDest Must refer to a register, and must have a pointer type that points to the same type
             * that the `ValuePointer` is
             * @param id A symbol ID that refers to a `ValuePointer` in `bind::Registry`. This is the same value
             * you would use to look up the pointer yourself via `bind::Registry::GetValue(symbol_id id)`
             */
            InstructionRef valuePtr(const Value& ptrDest, symbol_id id);
            
            /**
             * @brief Gets a pointer to the value represented by `val` and stores it in `ptrDest`
             * 
             * @param ptrDest Must refer to a register, and must have a pointer type that points to the same type
             * that the `ValuePointer` is
             * @param val The value to get a pointer to
             */
            InstructionRef valuePtr(const Value& ptrDest, ValuePointer* val);
            
            /**
             * @brief Gets a pointer to the `this` object of the function being compiled, if applicable, and stores
             * it in `ptrDest`
             * 
             * @param ptrDest Must refer to a register, and must have a type that is strictly equal to the `this`
             * type of the function being compiled
             */
            InstructionRef thisPtr(const Value& ptrDest);
            
            /**
             * @brief Gets a pointer to the stack space that the function being compiled should use for its return
             * value, if the function being compiled returns on the stack. The return pointer will be stored in
             * `ptrDest`
             * 
             * @param ptrDest Must refer to a register, and must have a type that is a pointer to the return type
             * of the function being compiled
             */
            InstructionRef retPtr(const Value& ptrDest);
            
            /**
             * @brief Gets the value of an argument to the function being compiled at a given index and stores it
             * in `result`
             * 
             * @param result Must refer to a register, and must have the same type as the argument being referred
             * to by `argIndex`
             * @param argIndex Index of the argument to get the value of
             */
            InstructionRef argument(const Value& result, u32 argIndex);
            
            /**
             * @brief Reserves a virtual register which will be assigned later via the `resolve` instruction. This
             * instruction counts as an assignment. The `reserve` and `resolve` instructions exist exclusively to
             * influence code optimization and register allocation
             * 
             * @param reg Value to reserve a register for, must refer to a register, and must be assigned later
             * via the `resolve` instruction
             */
            InstructionRef reserve(const Value& reg);
            
            /**
             * @brief Resolves a pending assignment which was promised by the `reserve` instruction. This
             * instruction is equivalent to the `assign` instruction, with the following exceptions:
             * 
             * - The relationship with the `reserve` instruction
             * 
             * - This instruction does not count as an assignment
             * 
             * @param reg Value to assign, must refer to a register, and must be the same type as `assignTo`
             * @param assignTo Value to assign to `reg`, must be the same type as `reg`
             */
            InstructionRef resolve(const Value& reg, const Value& assignTo);
            
            /**
             * @brief Load a value from a pointer with an optional offset
             * @note `dest = *(src + offset)`
             * 
             * @param dest Value to load the value of `src` into. Must refer to a register and must be the same
             * type as the one pointed to by `src`
             * @param src Pointer to load a value from. Must refer to a register, and must be a pointer to
             * `dest`'s type
             * @param offset Optional offset from `src` to load from
             */
            InstructionRef load(const Value& dest, const Value& src, u32 offset = 0);
            
            /**
             * @brief Stores a value at a pointer with an optional offset
             * @note `*(dest + offset) = src`
             * 
             * @param src Value to store in `dest`. Must be the same type as the one pointed to by `dest`
             * @param dest Pointer to store `src` at. Must refer to a register and must be a pointer to `src`'s
             * type
             * @param offset Optional offset from `dest` to store in
             */
            InstructionRef store(const Value& src, const Value& dest, u32 offset = 0);
            
            /**
             * @brief Jump to a label
             * @note If you know you need to jump before you know *where* you need to jump, use `label(false)` to
             * to generate a label to use with this function, then add that label to the code later via
             * `label(your_label)`
             * 
             * @param label label to jump to
             */
            InstructionRef jump(label_id label);
            
            /**
             * @brief Convert `src` to `dest`'s type and store it in `dest`
             * 
             * @param dest Value to store the converted value in. Must refer to a register, and must have a primitive
             * type
             * @param src Value to convert to `dest`'s type. Must have a primitive type
             */
            InstructionRef cvt(const Value& dest, const Value& src);
            
            /**
             * @brief Prepares a value to be passed as an argument to a function. When you are about to use the `call`
             * instruction, you have to emit one `param` instruction for each argument that call should take, and in
             * the order they should be provided in from left to right
             * 
             * @param val Value to pass as an argument
             */
            InstructionRef param(const Value& val);
            
            /**
             * @brief Calls a function with arguments provided via previously emitted `param` instructions
             * 
             * @param func Function to call
             * @param retDest Value that will have the return value stored in it, or a pointer to some memory that
             * will have the return value constructed within it, depending on how the function returns. Must refer to
             * a register. Must be empty if the function does not return a value
             * @param selfPtr Pointer to the `this` object to pass to the function, if it needs one
             */
            InstructionRef call(FunctionBuilder* func, const Value& retDest = Value(), const Value& selfPtr = Value());
            
            /**
             * @brief Calls a function with arguments provided via previously emitted `param` instructions
             * 
             * @param func Function to call
             * @param retDest Value that will have the return value stored in it, or a pointer to some memory that
             * will have the return value constructed within it, depending on how the function returns. Must refer to
             * a register. Must be empty if the function does not return a value
             * @param selfPtr Pointer to the `this` object to pass to the function, if it needs one
             */
            InstructionRef call(Function* func, const Value& retDest = Value(), const Value& selfPtr = Value());
            
            /**
             * @brief Calls a function with arguments provided via previously emitted `param` instructions
             * 
             * @param func Function to call
             * @param retDest Value that will have the return value stored in it, or a pointer to some memory that
             * will have the return value constructed within it, depending on how the function returns. Must refer to
             * a register. Must be empty if the function does not return a value
             * @param selfPtr Pointer to the `this` object to pass to the function, if it needs one
             */
            InstructionRef call(const Value& func, const Value& retDest = Value(), const Value& selfPtr = Value());
            
            /**
             * @brief Returns from the function
             * 
             * @param val Value to return, if the function returns a primitive value and doesn't return on the stack
             */
            InstructionRef ret(const Value& val = Value());
            
            /**
             * @brief Branches on `cond`
             * 
             * @param cond Value to branch on, should have a boolean type
             * @param destOnFalse Label to jump to if `cond` is falsy
             */
            InstructionRef branch(const Value& cond, label_id destOnFalse);
            
            /**
             * @brief `result = !val`
             * @param result Value to store the result in. Must represent a register, must have a boolean type
             */
            InstructionRef _not(const Value& result, const Value& val);
            
            /**
             * @brief `result = ~val`
             * @param result Value to store the result in. Must represent a register, must have primitive type
             */
            InstructionRef inv(const Value& result, const Value& val);
            
            /**
             * @brief `result = val << bits`
             * @param result Value to store the result in. Must represent a register, must have integral type
             */
            InstructionRef shl(const Value& result, const Value& val, const Value& bits);
            
            /**
             * @brief `result = val >> bits`
             * @param result Value to store the result in. Must represent a register, must have integral type
             */
            InstructionRef shr(const Value& result, const Value& val, const Value& bits);
            
            /**
             * @brief `result = a && b`
             * @param result Value to store the result in. Must represent a register, must have a boolean type
             */
            InstructionRef land(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a & b`
             * @param result Value to store the result in. Must represent a register, must have integral type
             */
            InstructionRef band(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a || b`
             * @param result Value to store the result in. Must represent a register, must have a boolean type
             */
            InstructionRef lor(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a | b`
             * @param result Value to store the result in. Must represent a register, must have integral type
             */
            InstructionRef bor(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a ^ b`
             * @param result Value to store the result in. Must represent a register, must have integral type
             */
            InstructionRef _xor(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `dest = src`
             * @param dest Value to store `src` in. Must represent a register, must have the same type as `src`
             * @param src Value to store in `dest`. Must have the same type as `dest`
             */
            InstructionRef assign(const Value& dest, const Value& src);
            
            /**
             * @brief Vector assignment
             * 
             * @param dest Pointer to vector to assign. Must refer to a register, must be a pointer type
             * @param src Value to assign to `dest`. Must either have the same type as `dest`, or have the type
             * that `dest`'s type points to
             * @param componentCount Vector component count
             */
            InstructionRef vset(const Value& dest, const Value& src, u8 componentCount);
            
            /**
             * @brief Vector addition
             * 
             * @param a Pointer to vector to add to. Must refer to a register, must be a pointer type
             * @param b Value to add to `dest`. Must either have the same type as `dest`, or have the type that
             * `dest`'s type points to
             * @param componentCount Vector component count
             */
            InstructionRef vadd(const Value& a, const Value& b, u8 componentCount);
            
            /**
             * @brief Vector subtraction
             * 
             * @param a Pointer to vector to subtract from. Must refer to a register, must be a pointer type
             * @param b Value to subtract from `dest`. Must either have the same type as `dest`, or have the
             * type that `dest`'s type points to
             * @param componentCount Vector component count
             */
            InstructionRef vsub(const Value& a, const Value& b, u8 componentCount);
            
            /**
             * @brief Vector multiplication
             * 
             * @param a Pointer to vector to multiply. Must refer to a register, must be a pointer type
             * @param b Value to multiply `dest` by. Must either have the same type as `dest`, or have the
             * type that `dest`'s type points to
             * @param componentCount Vector component count
             */
            InstructionRef vmul(const Value& a, const Value& b, u8 componentCount);
            
            /**
             * @brief Vector division
             * 
             * @param a Pointer to vector to divide. Must refer to a register, must be a pointer type
             * @param b Value to divide `dest` by. Must either have the same type as `dest`, or have the
             * type that `dest`'s type points to
             * @param componentCount Vector component count
             */
            InstructionRef vdiv(const Value& a, const Value& b, u8 componentCount);
            
            /**
             * @brief Vector modulo
             * 
             * @param a Pointer to vector to modulo. Must refer to a register, must be a pointer type
             * @param b Value to modulo `dest` by. Must either have the same type as `dest`, or have the
             * type that `dest`'s type points to
             * @param componentCount Vector component count
             */
            InstructionRef vmod(const Value& a, const Value& b, u8 componentCount);
            
            /**
             * @brief Vector negation
             * 
             * @param val Pointer to vector to negate. Must refer to a register, must be a pointer type
             * @param componentCount Vector component count
             */
            InstructionRef vneg(const Value& val, u8 componentCount);
            
            /**
             * @brief Vector dot product
             * 
             * @param result Result value. Must refer to a register, must have the type that the types of `a`
             * and `b` point to
             * @param a Left-hand vector, Must have pointer type, must have the same type as `b`
             * @param b Right-hand vector, Must have pointer type, must have the same type as `a`
             * @param componentCount Vector component count
             */
            InstructionRef vdot(const Value& result, const Value& a, const Value& b, u8 componentCount);
            
            /**
             * @brief Vector magnitude
             * 
             * @param result Result value. Must refer to a register, must have the type that the types of `a`
             * and `b` point to
             * @param val Vector to get the magnitude of, must have pointer type
             * @param componentCount Vector component count
             */
            InstructionRef vmag(const Value& result, const Value& val, u8 componentCount);
            
            /**
             * @brief Vector squared magnitude
             * 
             * @param result Result value. Must refer to a register, must have the type that the types of `a`
             * and `b` point to
             * @param val Vector to get the magnitude of, must have pointer type
             * @param componentCount Vector component count
             */
            InstructionRef vmagsq(const Value& result, const Value& val, u8 componentCount);
            
            /**
             * @brief Vector normalize
             * 
             * @param val Pointer to vector to normalize. Must be a pointer type
             * @param componentCount Vector component count
             */
            InstructionRef vnorm(const Value& val, u8 componentCount);
            
            /**
             * @brief Vector dot product
             * 
             * @param result Result value. Must have the same type as `a` and `b`
             * @param a Left-hand vector, Must have pointer type, must have the same type as `result`
             * @param b Right-hand vector, Must have pointer type, must have the same type as `result`
             * @param componentCount Vector component count, must be 3
             */
            InstructionRef vcross(const Value& result, const Value& a, const Value& b, u8 componentCount);
            
            /**
             * @brief `result = a + b`, Signed integer
             * @param result Value to store the result in. Must represent a register, must have signed integral type
             */
            InstructionRef iadd(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a + b`, Unsigned integer
             * @param result Value to store the result in. Must represent a register, must have unsigned integral type
             */
            InstructionRef uadd(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a + b`, 32-Bit floating point
             * @param result Value to store the result in. Must represent a register, must be f32
             */
            InstructionRef fadd(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a + b`, 64-Bit floating point
             * @param result Value to store the result in. Must represent a register, must be f64
             */
            InstructionRef dadd(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a - b`, Signed integer
             * @param result Value to store the result in. Must represent a register, must have signed integral type
             */
            InstructionRef isub(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a - b`, Unsigned integer
             * @param result Value to store the result in. Must represent a register, must have unsigned integral type
             */
            InstructionRef usub(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a - b`, 32-Bit floating point
             * @param result Value to store the result in. Must represent a register, must be f32
             */
            InstructionRef fsub(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a - b`, 64-Bit floating point
             * @param result Value to store the result in. Must represent a register, must be f64
             */
            InstructionRef dsub(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a * b`, Signed integer
             * @param result Value to store the result in. Must represent a register, must have signed integral type
             */
            InstructionRef imul(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a * b`, Unsigned integer
             * @param result Value to store the result in. Must represent a register, must have unsigned integral type
             */
            InstructionRef umul(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a * b`, 32-Bit floating point
             * @param result Value to store the result in. Must represent a register, must be f32
             */
            InstructionRef fmul(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a * b`, 64-Bit floating point
             * @param result Value to store the result in. Must represent a register, must be f64
             */
            InstructionRef dmul(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a / b`, Signed integer
             * @param result Value to store the result in. Must represent a register, must have signed integral type
             */
            InstructionRef idiv(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a / b`, Unsigned integer
             * @param result Value to store the result in. Must represent a register, must have unsigned integral type
             */
            InstructionRef udiv(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a / b`, 32-Bit floating point
             * @param result Value to store the result in. Must represent a register, must be f32
             */
            InstructionRef fdiv(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a / b`, 64-Bit floating point
             * @param result Value to store the result in. Must represent a register, must be f64
             */
            InstructionRef ddiv(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a % b`, Signed integer
             * @param result Value to store the result in. Must represent a register, must have signed integral type
             */
            InstructionRef imod(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a % b`, Unsigned integer
             * @param result Value to store the result in. Must represent a register, must have unsigned integral type
             */
            InstructionRef umod(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a % b`, 32-Bit floating point
             * @param result Value to store the result in. Must represent a register, must be f32
             */
            InstructionRef fmod(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a % b`, 64-Bit floating point
             * @param result Value to store the result in. Must represent a register, must be f64
             */
            InstructionRef dmod(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = -val`, Signed integer
             * @param result Value to store the result in. Must represent a register, must have signed integral type
             */
            InstructionRef ineg(const Value& result, const Value& val);
            
            /**
             * @brief `result = -val`, 32-Bit floating point
             * @param result Value to store the result in. Must represent a register, must be f32
             */
            InstructionRef fneg(const Value& result, const Value& val);
            
            /**
             * @brief `result = -val`, 64-Bit floating point
             * @param result Value to store the result in. Must represent a register, must be f64
             */
            InstructionRef dneg(const Value& result, const Value& val);
            
            /**
             * @brief `val++`, Signed integer
             */
            InstructionRef iinc(const Value& val);
            
            /**
             * @brief `val++`, Unsigned integer
             */
            InstructionRef uinc(const Value& val);
            
            /**
             * @brief `val++`, 32-Bit floating point
             */
            InstructionRef finc(const Value& val);
            
            /**
             * @brief `val++`, 64-Bit floating point
             */
            InstructionRef dinc(const Value& val);
            
            /**
             * @brief `val--`, Signed integer
             */
            InstructionRef idec(const Value& val);
            
            /**
             * @brief `val--`, Unsigned integer
             */
            InstructionRef udec(const Value& val);
            
            /**
             * @brief `val--`, 32-Bit floating point
             */
            InstructionRef fdec(const Value& val);
            
            /**
             * @brief `val--`, 64-Bit floating point
             */
            InstructionRef ddec(const Value& val);
            
            /**
             * @brief `result = a < b`, Signed integer
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef ilt(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a < b`, Unsigned integer
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef ult(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a < b`, 32-Bit floating point
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef flt(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a < b`, 64-Bit floating point
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef dlt(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a <= b`, Signed integer
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef ilte(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a <= b`, Unsigned integer
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef ulte(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a <= b`, 32-Bit floating point
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef flte(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a <= b`, 64-Bit floating point
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef dlte(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a > b`, Signed integer
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef igt(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a > b`, Unsigned integer
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef ugt(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a > b`, 32-Bit floating point
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef fgt(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a > b`, 64-Bit floating point
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef dgt(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a >= b`, Signed integer
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef igte(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a >= b`, Unsigned integer
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef ugte(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a >= b`, 32-Bit floating point
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef fgte(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a >= b`, 64-Bit floating point
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef dgte(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a == b`, Signed integer
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef ieq(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a == b`, Unsigned integer
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef ueq(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a == b`, 32-Bit floating point
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef feq(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a == b`, 64-Bit floating point
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef deq(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a != b`, Signed integer
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef ineq(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a != b`, Unsigned integer
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef uneq(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a != b`, 32-Bit floating point
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef fneq(const Value& result, const Value& a, const Value& b);
            
            /**
             * @brief `result = a != b`, 64-Bit floating point
             * @param result Value to store result in. Must represent a register, must have a boolean type
             */
            InstructionRef dneq(const Value& result, const Value& a, const Value& b);

            /**
             * @brief Essentially returns ptr + offset. By default, the type of the result
             * will be the same as the type of `ptr`. If `destType` is specified, then
             * that will be the type of the result.
             * 
             * @param ptr Pointer to offset
             * @param offset Amount to offset the pointer in bytes. Must be integral,
             * can be immediate
             * @param destType Optional data type to assign to the result Value
             * @return The offset pointer
             */
            Value ptrOffset(const Value& ptr, const Value& offset, DataType* destType = nullptr);

            /**
             * @brief Essentially returns ptr + offset. By default, the type of the result
             * will be the same as the type of `ptr`. If `destType` is specified, then
             * that will be the type of the result.
             * 
             * @param ptr Pointer to offset
             * @param offset Amount to offset the pointer in bytes
             * @param destType Optional data type to assign to the result Value
             * @return The offset pointer
             */
            Value ptrOffset(const Value& ptr, i64 offset, DataType* destType = nullptr);

            /**
             * @brief Generates IR code for calling a function
             * 
             * @param func Function to call
             * @param args Arguments to pass to the function
             * @param selfPtr 'this' pointer, if the function being called needs one
             * @return The function's return value. May be a pointer to memory on the stack
             */
            Value generateCall(
                FunctionBuilder* func,
                const Array<Value>& args = Array<Value>(),
                const Value& selfPtr = Value()
            );
            
            /**
             * @brief Generates IR code for calling a function
             * 
             * @param func Function to call
             * @param args Arguments to pass to the function
             * @param selfPtr 'this' pointer, if the function being called needs one
             * @return The function's return value. May be a pointer to memory on the stack
             */
            Value generateCall(
                Function* func,
                const Array<Value>& args = Array<Value>(),
                const Value& selfPtr = Value()
            );
            
            /**
             * @brief Generates IR code for calling a function
             * 
             * @param func Function to call
             * @param args Arguments to pass to the function
             * @param selfPtr 'this' pointer, if the function being called needs one
             * @return The function's return value. May be a pointer to memory on the stack
             */
            Value generateCall(
                const Value& func,
                const Array<Value>& args = Array<Value>(),
                const Value& selfPtr = Value()
            );

            /**
             * @brief This will attempt to construct an instance of `destPtr`'s pointed-to type in the memory
             * pointed to by `destPtr`, with the arguments specified by `args`.
             * 
             * The behavior of this function is as follows:
             * 
             * - If `destPtr` points to a primitive type, and `args` is empty, nothing will be done
             * 
             * - If `destPtr` points to a primitive type, and `args` contains one value that is convertible to
             * that type, then code will be emitted to convert that value to the destination type if necessary
             * and the result will be copied directly to the memory pointed to by `destPtr`
             * 
             * - If `destPtr` points to a primitive type, and the provided arguments can't be used to directly
             * initialize the value at `destPtr`, this function will search for a "pseudo" constructor on the
             * destination type which can accept the provided arguments (or that the provided arguments can be
             * coerced into, preferring a strict match). If a suitable constructor is found and access is granted
             * by the `accessMask` parameter, the constructor will be called for `destPtr` with the provided
             * arguments
             * 
             * - If `destPtr` points to a non-primitive type this function will search for a constructor on the
             * destination type which can accept the provided arguments (or that the provided arguments can be
             * coerced into, preferring a strict match). If a suitable constructor is found and access is granted
             * by the `accessMask` parameter, the constructor will be called for `destPtr` with the provided
             * arguments
             * 
             * - If `destPtr` points to a non-primitive type that is trivially copyable and no suitable constructor
             * is found, and if `args` contains just one argument that is the same type as (or is convertible to)
             * the destination type, then the argument will be converted (if necessary) to the destination type and
             * copied byte-for-byte into the memory pointed to by `destPtr`
             * 
             * @param destPtr Pointer to some memory where construction (or copying) should take place
             * @param args Arguments to attempt to construct `destPtr` with
             * @param accessMask User-defined access mask. If a constructor has to be searched for, this will
             * determine which ones can be used
             */
            void generateConstruction(const Value& destPtr, const Array<Value>& args, AccessFlags accessMask = FullAccessRights);

            /**
             * @brief Generates instructions for the destruction of an object
             * 
             * @param objPtr Object to destroy
             * @param accessMask User-defined access mask. If a destructor has to be called, this will determine
             * if it can be used
             */
            void generateDestruction(const Value& objPtr, AccessFlags accessMask = FullAccessRights);

            /**
             * @brief Generates instructions for returning from the function. This will emit IR code that does
             * the following things:
             * 
             * - If the function returns on the stack, then code will be emitted to copy-construct `val` in the
             * memory pointed to by the return pointer
             * 
             * - Code will be emitted to destroy all stack objects in the current scope and all parent scopes
             * 
             * - Code will be emitted to free all active stack allocations in the current scope and all parent
             * scopes
             * 
             * @param val Value to return, if any
             */
            void generateReturn(const Value& val = Value());

            /**
             * @brief Emits the IR code for an if statement
             * 
             * @param cond Boolean value to branch on
             * @param emitThenBody Function that will emit the code for the if statement's body when called
             */
            template <typename ThenFn>
            std::enable_if_t<std::is_invocable_v<ThenFn>, void>
            generateIf(const Value& cond, ThenFn&& emitThenBody) {
                label_id afterBody = label(false, "IF_END");
                branch(cond, afterBody);
                // if (cond) {
                    emitThenBody();
                // }
                label(afterBody); // jump to here if !cond
            }
            
            /**
             * @brief Emits the IR code for an if/else statement
             * 
             * @param cond Boolean value to branch on
             * @param emitThenBody Function that will emit the code for the if statement's truthy-path body when called
             * @param emitElseBody Function that will emit the code for the if statement's falsy-path body when called
             */
            template <typename ThenFn, typename ElseFn>
            std::enable_if_t<std::is_invocable_v<ThenFn> && std::is_invocable_v<ElseFn>, void>
            generateIf(const Value& cond, ThenFn&& emitThenBody, ElseFn&& emitElseBody) {
                label_id elseBody = label(false, "ELSE_BEGIN");
                label_id afterElse = label(false, "IF_END");

                branch(cond, elseBody);
                // if (cond) {
                    emitThenBody();
                    jump(afterElse);
                // } else {
                    label(elseBody); // jump to here if !cond
                    emitElseBody();
                // }

                label(afterElse); // jump to here at the end of the 'then' body
            }
            
            /**
             * @brief Emits the IR code for a for loop, excluding the initializer which can be handled externally
             * 
             * @param emitCondition Function that will emit the code for the condition and return the condition variable.
             * The value returned by the condition emitter function should have a boolean type.
             * @param emitModifier Function that will emit the code for the modification step of the for loop
             * @param emitBody Function that will emit the code for the loop body
             */
            template <typename CondFn, typename ModifyFn, typename BodyFn>
            std::enable_if_t<std::is_invocable_r_v<Value, CondFn> && std::is_invocable_v<ModifyFn> && std::is_invocable_v<BodyFn>, void>
            generateFor(CondFn&& emitCondition, ModifyFn&& emitModifier, BodyFn&& emitBody) {
                label_id afterLoop = label(false, "LOOP_END");
                label_id continueLbl = label(false, "LOOP_CONT");
                label_id loopBegin = label(true, "LOOP_START");
                Value cond = emitCondition();
                branch(cond, afterLoop);

                Scope s(this);
                    s.setLoopContinueLabel(continueLbl);
                    s.setLoopBreakLabel(afterLoop);
                    emitBody();
                s.escape();

                label(continueLbl);
                emitModifier();
                jump(loopBegin);
                label(afterLoop);
            }
            
            /**
             * @brief Emits the IR code for a while loop
             * 
             * @param emitCondition Function that will emit the code for the condition and return the condition variable.
             * The value returned by the condition emitter function should have a boolean type.
             * @param emitBody Function that will emit the code for the loop body
             */
            template <typename CondFn, typename BodyFn>
            std::enable_if_t<std::is_invocable_v<CondFn> && std::is_invocable_v<BodyFn>, void>
            generateWhile(CondFn&& emitCondition, BodyFn&& emitBody) {
                label_id afterLoop = label(false, "LOOP_END");
                label_id loopBegin = label(true, "LOOP_START");

                branch(emitCondition(), afterLoop);

                Scope s(this);
                    s.setLoopContinueLabel(loopBegin);
                    s.setLoopBreakLabel(afterLoop);
                    emitBody();
                s.escape();

                jump(loopBegin);
                label(afterLoop);
            }

            /**
             * @brief Emits the IR code for a do...while loop
             * 
             * @param emitCondition Function that will emit the code for the condition and return the condition variable.
             * The value returned by the condition emitter function should have a boolean type.
             * @param emitBody Function that will emit the code for the loop body
             */
            template <typename BodyFn, typename CondFn>
            std::enable_if_t<std::is_invocable_v<BodyFn> && std::is_invocable_r_v<Value, CondFn>, void>
            generateDoWhile(BodyFn&& emitBody, CondFn&& emitCondition) {
                label_id loopBegin = label(true, "LOOP_START");
                label_id loopEnd = label(false, "LOOP_END");

                Scope s(this);
                    s.setLoopContinueLabel(loopBegin);
                    s.setLoopBreakLabel(loopEnd);
                    emitBody();
                s.escape();

                Value cond = emitCondition();
                branch(!cond, loopBegin);
                label(loopEnd);
            }

            /**
             * @brief Generates a Value that represents a given `label_id`
             */
            Value labelVal(label_id label);

            /**
             * @brief Allocates a register (and maybe stack space) for a given `DataType`
             * 
             * @note
             * If `DataType` is non-primitive then this will allocate space on the stack for it within the current `Scope`.
             * The stack space will not be initialized and this function will return a value with a type that is a pointer
             * to `tp`. To clarify, if `tp` refers to `SomeObject`, the type of the return value will refer to `SomeObject*`
             * 
             * @param tp Data type to allocate a register (and maybe stack space) for
             * @return A `Value` that represents a virtual register, with either the provided type or a pointer to that type
             * if the value must be allocated on the stack
             */
            Value val(DataType* tp);

            /**
             * @brief Allocates a register (and maybe stack space) for the given type `T`
             * 
             * @note
             * If `T` is non-primitive then this will allocate space on the stack for it within the current `Scope`. The
             * stack space will not be initialized and this function will return a value with a type that is a pointer to
             * `T`. To clarify, if `T` is `SomeObject`, the type of the return value will refer to `SomeObject*`
             * 
             * @tparam T Data type to allocate a register (and maybe stack space) for
             * @return A `Value` that represents a virtual register, with either the provided type or a pointer to that type
             * if the value must be allocated on the stack
             */
            template <typename T>
            Value val() {
                return val(Registry::GetType<T>());
            }

            /**
             * @brief Allocates a register that will contain a pointer to the value represented by `value`.
             * 
             * @param value Some value in host memory to get a pointer to
             * @return A `Value` that represents a virtual register which will contain a pointer to the value represented
             * by `value`
             */
            Value val(ValuePointer* value);

            /** @brief Creates a boolean immediate `Value` */
            Value val(bool imm);
            /** @brief Creates a u8 immediate `Value` */
            Value val(u8 imm);
            /** @brief Creates a u16 immediate `Value` */
            Value val(u16 imm);
            /** @brief Creates a u32 immediate `Value` */
            Value val(u32 imm);
            /** @brief Creates a u64 immediate `Value` */
            Value val(u64 imm);
            /** @brief Creates a i8 immediate `Value` */
            Value val(i8 imm);
            /** @brief Creates a i16 immediate `Value` */
            Value val(i16 imm);
            /** @brief Creates a i32 immediate `Value` */
            Value val(i32 imm);
            /** @brief Creates a i64 immediate `Value` */
            Value val(i64 imm);
            /** @brief Creates a f32 immediate `Value` */
            Value val(f32 imm);
            /** @brief Creates a f64 immediate `Value` */
            Value val(f64 imm);

            /**
             * @brief Creates a pointer immediate `Value`
             * @note This should only be used if you're generating and executing code during the same runtime, unless you're
             * using pointers that are relative to some address that will be determined by the runtime and handled by whatever
             * backend you use to process this code
             */
            Value val(ptr imm);

            /** @brief Gets the function that this code is being generated for */
            Function* getFunction() const;

            /** @brief Gets all the code that's been generated so far, mutable */
            Array<Instruction>& getCode();
            
            /** @brief Gets all the code that's been generated so far, immutable */
            const Array<Instruction>& getCode() const;

            /** @brief If this is a method of a DataType, this will return a pointer to the 'this' object */
            Value getThis() const;

            /** @brief Returns a `Value` that contains the argument at a given index */
            Value getArg(u32 index) const;

            /**
             * @brief If this function returns on the stack, this will be a pointer to the memory the return value should be
             * copied to / constructed at
             */
            Value getRetPtr();

            /** @brief Gets the ID of the next stack allocation */
            stack_id getNextAllocId() const;

            /** @brief Allocates a stack ID and returns it, does nothing else */
            stack_id reserveAllocId();

            /**
             * @brief Sets info about the current source location being compiled, if applicable. See `SourceLocation` for more
             * details
             */
            void setCurrentSourceLocation(const SourceLocation& src);

            /** @brief Returns a source map for the generated code. See `SourceMap` for more details */
            const SourceMap* getSourceMap() const;

            /** @brief Returns the current `Scope` */
            Scope* getCurrentScope() const;

            /**
             * @brief Enables input validation for all the instruction emitter functions. This will cause exceptions to be
             * thrown if any invalid inputs are provided
             */
            void enableValidation();

            void setName(Value& v, const String& name);
            const String& getString(i32 stringId) const;
            const String& getLabelName(label_id label) const;

        protected:
            friend class InstructionRef;
            friend class Scope;

            void emitPrologue();
            void enterScope(Scope* s);
            void exitScope(Scope* s);
            i32 addString(const String& str);

            Function* m_function;
            FunctionBuilder* m_parent;
            Array<Instruction> m_code;
            Array<String> m_strings;
            std::unordered_map<label_id, u32> m_labelNameStringIds;
            label_id m_nextLabel;
            vreg_id m_nextReg;
            stack_id m_nextAlloc;
            SourceLocation m_currentSrcLoc;
            SourceMap m_srcMap;
            bool m_validationEnabled;

            Value m_thisPtr;
            Array<Value> m_args;
            Scope* m_currentScope;
            Scope m_ownScope;
    };
};