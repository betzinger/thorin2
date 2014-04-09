#include "thorin/irbuilder.h"

#include "thorin/lambda.h"
#include "thorin/literal.h"
#include "thorin/memop.h"
#include "thorin/world.h"

namespace thorin {

//------------------------------------------------------------------------------

Var::Var(IRBuilder& builder, Def def)
    : kind_(ImmutableValRef)
    , builder_(&builder)
    , def_(def)
{}

Var::Var(IRBuilder& builder, size_t handle, const Type* type, const char* name)
    : kind_(MutableValRef)
    , builder_(&builder)
    , handle_(handle)
    , type_(type)
    , name_(name)
{}

Var::Var(IRBuilder& builder, const Slot* slot)
    : kind_(SlotRef)
    , builder_(&builder)
    , slot_(slot)
{}

Def Var::load() const { 
    switch (kind()) {
        case Empty:           return Def();
        case ImmutableValRef: return def_;
        case MutableValRef:   return builder_->cur_bb->get_value(handle_, type_, name_); 
        case SlotRef:         return builder_->world().load(builder_->get_mem(), slot_);
        default: THORIN_UNREACHABLE;
    }
}

void Var::store(Def def) const { 
    switch (kind()) {
        case MutableValRef:  builder_->cur_bb->set_value(handle_, def); return;
        case SlotRef: builder_->set_mem(builder_->world().store(builder_->get_mem(), slot_, def)); return;
        default: THORIN_UNREACHABLE;
    }
}

//------------------------------------------------------------------------------

#ifndef NDEBUG
JumpTarget::~JumpTarget() { assert((!lambda_ || first_ || lambda_->is_sealed()) && "JumpTarget not sealed"); }
#endif

World& JumpTarget::world() const { assert(lambda_); return lambda_->world(); }
void JumpTarget::seal() { assert(lambda_); lambda_->seal(); }

Lambda* JumpTarget::untangle() {
    if (!first_)
        return lambda_;
    assert(lambda_);
    auto bb = world().basicblock(name_);
    lambda_->jump(bb, {});
    first_ = false;
    return lambda_ = bb;
}

void JumpTarget::jump_from(Lambda* bb) {
    if (!lambda_) {
        lambda_ = bb;
        first_ = true;
    } else
        bb->jump(untangle(), {});
}

Lambda* JumpTarget::branch_to(World& world) {
    auto bb = world.basicblock(lambda_ ? name_ + std::string(".crit") : name_);
    jump_from(bb);
    bb->seal();
    return bb;
}

Lambda* JumpTarget::enter() {
    if (lambda_ && !first_)
        lambda_->seal();
    return lambda_;
}

Lambda* JumpTarget::enter_unsealed(World& world) {
    return lambda_ ? untangle() : lambda_ = world.basicblock(name_);
}

//------------------------------------------------------------------------------

void IRBuilder::jump(JumpTarget& jt) {
    if (is_reachable()) {
        jt.jump_from(cur_bb);
        set_unreachable();
    }
}

void IRBuilder::branch(Def cond, JumpTarget& t, JumpTarget& f) {
    if (is_reachable()) {
        if (auto lit = cond->isa<PrimLit>())
            jump(lit->value().get_bool() ? t : f);
        else if (&t == &f)
            jump(t);
        else {
            auto tl = t.branch_to(world());
            auto fl = f.branch_to(world());
            cur_bb->branch(cond, tl, fl);
            set_unreachable();
        }
    }
}

void IRBuilder::mem_call(Def to, ArrayRef<Def> args, const Type* ret_type) {
    if (is_reachable())
        (cur_bb = cur_bb->mem_call(to, args, ret_type));
}

void IRBuilder::tail_call(Def to, ArrayRef<Def> args) {
    if (is_reachable()) {
        cur_bb->jump(to, args);
        set_unreachable();
    }
}

void IRBuilder::param_call(const Param* ret_param, ArrayRef<Def> args) {
    if (is_reachable()) {
        cur_bb->jump(ret_param, args);
        set_unreachable();
    }
}

Def IRBuilder::get_mem() { return cur_bb->get_value(0, world().mem(), "mem"); }
void IRBuilder::set_mem(Def def) { if (is_reachable()) cur_bb->set_value(0, def); }

//------------------------------------------------------------------------------

}
