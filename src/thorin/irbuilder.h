#ifndef THORIN_JUMPTARGET_H
#define THORIN_JUMPTARGET_H

#include "thorin/def.h"
#include "thorin/util/array.h"
#include "thorin/util/autoptr.h"

namespace thorin {

class IRBuilder;
class Lambda;
class Param;
class Ref;
class Slot;
class Type;
class World;

//------------------------------------------------------------------------------

class Var {
public:
    enum Kind {
        Empty,
        ImmutableValRef,
        MutableValRef,
        SlotRef
    };

    Var()
        : kind_(Empty)
        , builder_(nullptr)
    {}
    Var(IRBuilder& builder, Def def);
    Var(IRBuilder& builder, size_t handle, const Type* type, const char* name);
    Var(IRBuilder& builder, const Slot* slot);

    const Kind kind() const { return kind_; }
    Def load() const;
    void store(Def val) const;
    operator bool() { return kind() != Empty; }

private:
    Kind kind_;
    IRBuilder* builder_;
    union {
        struct {
            size_t handle_;
            const Type* type_;
            const char* name_;
        };
        Def def_;
        struct {
            const Slot* slot_;
        };
    };
};

//------------------------------------------------------------------------------

class JumpTarget {
public:
    JumpTarget(const char* name = "")
        : lambda_(nullptr)
        , first_(false)
        , name_(name)
    {}
#ifndef NDEBUG
    ~JumpTarget();
#endif

    World& world() const;
    void seal();

private:
    void jump_from(Lambda* bb);
    Lambda* branch_to(World& world);
    Lambda* untangle();
    Lambda* enter();
    Lambda* enter_unsealed(World& world);

    Lambda* lambda_;
    bool first_;
    const char* name_;

    friend class IRBuilder;
};

//------------------------------------------------------------------------------

class IRBuilder {
public:
    IRBuilder(World& world)
        : cur_bb(nullptr)
        , world_(world)
    {}

    World& world() const { return world_; }
    bool is_reachable() const { return cur_bb != nullptr; }
    void set_unreachable() { cur_bb = nullptr; }
    Lambda* enter(JumpTarget& jt) { return cur_bb = jt.enter(); }
    Lambda* enter_unsealed(JumpTarget& jt) { return cur_bb = jt.enter_unsealed(world_); }
    void jump(JumpTarget& jt);
    void branch(Def cond, JumpTarget& t, JumpTarget& f);
    void mem_call(Def to, ArrayRef<Def> args, const Type* ret_type);
    void tail_call(Def to, ArrayRef<Def> args);
    void param_call(const Param* ret_param, ArrayRef<Def> args);
    Def get_mem();
    void set_mem(Def def);

    Lambda* cur_bb;

protected:
    World& world_;
};

//------------------------------------------------------------------------------

}

#endif
