#include <codegen/FunctionBuilder.h>
#include <bind/DataType.h>
#include <bind/Function.h>
#include <bind/FunctionType.h>
#include <bind/ValuePointer.h>
#include <bind/Registry.h>

namespace codegen {
    FunctionBuilder::FunctionBuilder(Function* func) : m_function(func), m_parent(nullptr), m_nextLabel(1), m_nextReg(1), m_nextAlloc(1) {
    }

    FunctionBuilder::FunctionBuilder(Function* func, FunctionBuilder* parent) : m_function(func), m_parent(parent), m_nextLabel(1), m_nextReg(1), m_nextAlloc(1) {
    }

    FunctionBuilder::~FunctionBuilder() {
    }

    InstructionRef FunctionBuilder::add(const Instruction& i) {
        m_code.push(i);
        return InstructionRef(this, m_code.size() - 1);
    }

    label_id FunctionBuilder::label() {
        Instruction i(OpCode::label);
        i.operands[0].reset(label(m_nextLabel++));
        m_code.push(i);
        return 0;
    }

    stack_id FunctionBuilder::stackAlloc(u32 size) {
        stack_id id = m_nextAlloc++;
        stackAlloc(size, id);
        return id;
    }

    InstructionRef FunctionBuilder::stackAlloc(u32 size, stack_id alloc) {
        Instruction i(OpCode::stack_alloc);
        i.operands[0].reset(val(size));
        i.operands[1].reset(val(alloc));
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::stackPtr(const Value& ptrDest, stack_id alloc) {
        Instruction i(OpCode::stack_ptr);
        i.operands[0].reset(ptrDest);
        i.operands[1].reset(val(alloc));
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::stackFree(stack_id alloc) {
        Instruction i(OpCode::stack_free);
        i.operands[0].reset(val(alloc));
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::valuePtr(const Value& reg, symbol_id id) {
        Instruction i(OpCode::value_ptr);
        i.operands[0].reset(reg);
        i.operands[1].reset(val(id));
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::valuePtr(const Value& reg, ValuePointer* sym) {
        Instruction i(OpCode::value_ptr);
        i.operands[0].reset(reg);
        i.operands[1].reset(val(sym->getSymbolId()));
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::reserve(const Value& reg) {
        Instruction i(OpCode::reserve);
        i.operands[0].reset(reg);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::resolve(const Value& reg, const Value& assignTo) {
        Instruction i(OpCode::resolve);
        i.operands[0].reset(reg);
        i.operands[1].reset(assignTo);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::load(const Value& dest, const Value& src, u32 offset) {
        Instruction i(OpCode::load);
        i.operands[0].reset(dest);
        i.operands[1].reset(src);
        i.operands[2].reset(val(offset));
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::store(const Value& src, const Value& dest, u32 offset) {
        Instruction i(OpCode::store);
        i.operands[0].reset(src);
        i.operands[1].reset(dest);
        i.operands[2].reset(val(offset));
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::jump(label_id _label) {
        Instruction i(OpCode::jump);
        i.operands[0].reset(label(_label));
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::cvt(const Value& dest, const Value& src) {
        return cvt(dest, src, dest.m_type->getSymbolId());
    }

    InstructionRef FunctionBuilder::cvt(const Value& dest, const Value& src, u64 destTypeHash) {
        Instruction i(OpCode::cvt);
        i.operands[0].reset(dest);
        i.operands[1].reset(src);
        i.operands[2].reset(val(destTypeHash));
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::param(const Value& val) {
        Instruction i(OpCode::param);
        i.operands[0].reset(val);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::call(FunctionBuilder* func, const Value& retDest, const Value& selfPtr) {
        Instruction i(OpCode::call);
        i.operands[0].reset(val(func->getFunction()));
        i.operands[1].reset(retDest);
        i.operands[2].reset(selfPtr);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::call(Function* func, const Value& retDest, const Value& selfPtr) {
        Instruction i(OpCode::call);
        i.operands[0].reset(val(func));
        i.operands[1].reset(retDest);
        i.operands[2].reset(selfPtr);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::call(const Value& func, const Value& retDest, const Value& selfPtr) {
        Instruction i(OpCode::call);
        i.operands[0].reset(func);
        i.operands[1].reset(retDest);
        i.operands[2].reset(selfPtr);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::ret() {
        Instruction i(OpCode::ret);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::branch(const Value& cond, label_id destOnFalse) {
        Instruction i(OpCode::branch);
        i.operands[0].reset(cond);
        i.operands[1].reset(label(destOnFalse));
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::_not(const Value& result, const Value& val) {
        Instruction i(OpCode::_not);
        i.operands[0].reset(result);
        i.operands[1].reset(val);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::inv(const Value& result, const Value& val) {
        Instruction i(OpCode::inv);
        i.operands[0].reset(result);
        i.operands[1].reset(val);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::shl(const Value& result, const Value& val, const Value& bits) {
        Instruction i(OpCode::shl);
        i.operands[0].reset(result);
        i.operands[1].reset(val);
        i.operands[2].reset(bits);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::shr(const Value& result, const Value& val, const Value& bits) {
        Instruction i(OpCode::shr);
        i.operands[0].reset(result);
        i.operands[1].reset(val);
        i.operands[2].reset(bits);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::land(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::land);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::band(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::band);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::lor(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::lor);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::bor(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::bor);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::_xor(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::_xor);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::assign(const Value& dest, const Value& src) {
        Instruction i(OpCode::assign);
        i.operands[0].reset(dest);
        i.operands[1].reset(src);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::vset(const Value& dest, const Value& src) {
        Instruction i(OpCode::vset);
        i.operands[0].reset(dest);
        i.operands[1].reset(src);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::vadd(const Value& dest, const Value& val) {
        Instruction i(OpCode::vadd);
        i.operands[0].reset(dest);
        i.operands[1].reset(val);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::vsub(const Value& dest, const Value& val) {
        Instruction i(OpCode::vsub);
        i.operands[0].reset(dest);
        i.operands[1].reset(val);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::vmul(const Value& dest, const Value& val) {
        Instruction i(OpCode::vmul);
        i.operands[0].reset(dest);
        i.operands[1].reset(val);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::vdiv(const Value& dest, const Value& val) {
        Instruction i(OpCode::vdiv);
        i.operands[0].reset(dest);
        i.operands[1].reset(val);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::vmod(const Value& dest, const Value& val) {
        Instruction i(OpCode::vmod);
        i.operands[0].reset(dest);
        i.operands[1].reset(val);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::vneg(const Value& val) {
        Instruction i(OpCode::vneg);
        i.operands[0].reset(val);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::vdot(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::vdot);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::vmag(const Value& result, const Value& val) {
        Instruction i(OpCode::vmag);
        i.operands[0].reset(result);
        i.operands[1].reset(val);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::vmagsq(const Value& result, const Value& val) {
        Instruction i(OpCode::vmagsq);
        i.operands[0].reset(result);
        i.operands[1].reset(val);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::vnorm(const Value& val) {
        Instruction i(OpCode::vnorm);
        i.operands[0].reset(val);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::vcross(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::vcross);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::iadd(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::iadd);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::uadd(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::uadd);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::fadd(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::fadd);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::dadd(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::dadd);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::isub(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::isub);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::usub(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::usub);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::fsub(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::fsub);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::dsub(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::dsub);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::imul(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::imul);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::umul(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::umul);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::fmul(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::fmul);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::dmul(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::dmul);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::idiv(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::idiv);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::udiv(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::udiv);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::fdiv(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::fdiv);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::ddiv(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::ddiv);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::imod(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::imod);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::umod(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::umod);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::fmod(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::fmod);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::dmod(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::dmod);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::ineg(const Value& result, const Value& val) {
        Instruction i(OpCode::ineg);
        i.operands[0].reset(result);
        i.operands[1].reset(val);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::fneg(const Value& result, const Value& val) {
        Instruction i(OpCode::fneg);
        i.operands[0].reset(result);
        i.operands[1].reset(val);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::dneg(const Value& result, const Value& val) {
        Instruction i(OpCode::dneg);
        i.operands[0].reset(result);
        i.operands[1].reset(val);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::iinc(const Value& val) {
        Instruction i(OpCode::iinc);
        i.operands[0].reset(val);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::uinc(const Value& val) {
        Instruction i(OpCode::uinc);
        i.operands[0].reset(val);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::finc(const Value& val) {
        Instruction i(OpCode::finc);
        i.operands[0].reset(val);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::dinc(const Value& val) {
        Instruction i(OpCode::dinc);
        i.operands[0].reset(val);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::idec(const Value& val) {
        Instruction i(OpCode::idec);
        i.operands[0].reset(val);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::udec(const Value& val) {
        Instruction i(OpCode::udec);
        i.operands[0].reset(val);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::fdec(const Value& val) {
        Instruction i(OpCode::fdec);
        i.operands[0].reset(val);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::ddec(const Value& val) {
        Instruction i(OpCode::ddec);
        i.operands[0].reset(val);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::ilt(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::ilt);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::ult(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::ult);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::flt(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::flt);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::dlt(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::dlt);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::ilte(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::ilte);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::ulte(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::ulte);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::flte(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::flte);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::dlte(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::dlte);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::igt(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::igt);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::ugt(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::ugt);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::fgt(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::fgt);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::dgt(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::dgt);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::igte(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::igte);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::ugte(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::ugte);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::fgte(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::fgte);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::dgte(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::dgte);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::ieq(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::ieq);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::ueq(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::ueq);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::feq(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::feq);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::deq(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::deq);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::ineq(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::ineq);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::uneq(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::uneq);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::fneq(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::fneq);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    InstructionRef FunctionBuilder::dneq(const Value& result, const Value& a, const Value& b) {
        Instruction i(OpCode::dneq);
        i.operands[0].reset(result);
        i.operands[1].reset(a);
        i.operands[2].reset(b);
        m_code.push(i);
        return add(i);
    }

    Value FunctionBuilder::generateCall(
        FunctionBuilder* func,
        const Array<Value>& args,
        const Value& selfPtr
    ) {
        return generateCall(func->getFunction(), args, selfPtr);
    }

    Value FunctionBuilder::generateCall(
        Function* func,
        const Array<Value>& args,
        const Value& selfPtr
    ) {
        FunctionType* sig = func->getSignature();
        auto argInfo = sig->getArgs();

        if (argInfo.size() != args.size()) {
            logError(
                "Incorrect number of arguments provided to function '%s'. Expected %u, got %u",
                func->getName().c_str(),
                argInfo.size(),
                args.size()
            );

            return Value();
        }

        for (u32 i = 0;i < args.size();i++) {
            Value arg = args[i].convertedTo(argInfo[i].type);
            param(arg);
        }

        Value result = val(sig->getReturnType());
        call(func, result, selfPtr);

        return result;
    }

    Value FunctionBuilder::generateCall(
        const Value& func,
        const Array<Value>& args,
        const Value& selfPtr
    ) {
        if (func.m_isImm) {
            return generateCall((Function*)func.m_imm.p, args, selfPtr);
        }

        FunctionType* sig = (FunctionType*)func.m_type;
        auto argInfo = sig->getArgs();

        if (argInfo.size() != args.size()) {
            logError(
                "Incorrect number of arguments provided to function. Expected %u, got %u",
                argInfo.size(),
                args.size()
            );

            return Value();
        }

        for (u32 i = 0;i < args.size();i++) {
            Value arg = args[i].convertedTo(argInfo[i].type);
            param(arg);
        }

        Value result = val(sig->getReturnType());
        call(func, result, selfPtr);

        return result;
    }

    Function* FunctionBuilder::getFunction() const {
        return m_function;
    }

    Array<Instruction>& FunctionBuilder::getCode() {
        return m_code;
    }

    const Array<Instruction>& FunctionBuilder::getCode() const {
        return m_code;
    }

    stack_id FunctionBuilder::getNextAllocId() const {
        return m_nextAlloc;
    }

    stack_id FunctionBuilder::reserveAllocId() {
        return m_nextAlloc++;
    }

    Value FunctionBuilder::label(label_id label) {
        Value ret(this, label);
        ret.m_isLabel = true;
        return ret;
    }

    Value FunctionBuilder::val(DataType* tp) { return Value(m_nextReg++, this, tp); }
    Value FunctionBuilder::val(ValuePointer* value) {
        Value ret = Value(m_nextReg++, this, (DataType*)value->getType()->getPointerType());
        valuePtr(ret, value);
        return ret;
    }
    Value FunctionBuilder::val(bool imm) { return Value(this, imm); }
    Value FunctionBuilder::val(u8   imm) { return Value(this, imm); }
    Value FunctionBuilder::val(u16  imm) { return Value(this, imm); }
    Value FunctionBuilder::val(u32  imm) { return Value(this, imm); }
    Value FunctionBuilder::val(u64  imm) { return Value(this, imm); }
    Value FunctionBuilder::val(i8   imm) { return Value(this, imm); }
    Value FunctionBuilder::val(i16  imm) { return Value(this, imm); }
    Value FunctionBuilder::val(i32  imm) { return Value(this, imm); }
    Value FunctionBuilder::val(i64  imm) { return Value(this, imm); }
    Value FunctionBuilder::val(f32  imm) { return Value(this, imm); }
    Value FunctionBuilder::val(f64  imm) { return Value(this, imm); }
    Value FunctionBuilder::val(ptr  imm) { return Value(this, imm); }
};