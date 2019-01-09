#ifndef THORIN_lam_H
#define THORIN_lam_H

#include <list>
#include <vector>
#include <queue>

#include "thorin/config.h"
#include "thorin/def.h"
#include "thorin/type.h"

namespace thorin {

class Lam;
class Param;

typedef std::vector<Lam*> Lams;

//------------------------------------------------------------------------------

class Peek {
public:
    Peek() {}
    Peek(const Def* def, Lam* from)
        : def_(def)
        , from_(from)
    {}

    const Def* def() const { return def_; }
    Lam* from() const { return from_; }

private:
    const Def* def_;
    Lam* from_;
};

size_t get_param_index(const Def* def);
Lam* get_param_lam(const Def* def);
std::vector<Peek> peek(const Def*);

//------------------------------------------------------------------------------

enum class Intrinsic : uint8_t {
    None,                       ///< Not an intrinsic.
    _Accelerator_Begin,
    CUDA = _Accelerator_Begin,  ///< Internal CUDA-Backend.
    NVVM,                       ///< Internal NNVM-Backend.
    OpenCL,                     ///< Internal OpenCL-Backend.
    AMDGPU,                     ///< Internal AMDGPU-Backend.
    HLS,                        ///< Internal HLS-Backend.
    Parallel,                   ///< Internal Parallel-CPU-Backend.
    Spawn,                      ///< Internal Parallel-CPU-Backend.
    Sync,                       ///< Internal Parallel-CPU-Backend.
    CreateGraph,                ///< Internal Flow-Graph-Backend.
    CreateTask,                 ///< Internal Flow-Graph-Backend.
    CreateEdge,                 ///< Internal Flow-Graph-Backend.
    ExecuteGraph,               ///< Internal Flow-Graph-Backend.
    Vectorize,                  ///< External vectorizer.
    _Accelerator_End,
    Reserve = _Accelerator_End, ///< Intrinsic memory reserve function
    Atomic,                     ///< Intrinsic atomic function
    CmpXchg,                    ///< Intrinsic cmpxchg function
    Undef,                      ///< Intrinsic undef function
    Branch,                     ///< branch(cond, T, F).
    Match,                      ///< match(val, otherwise, (case1, cont1), (case2, cont2), ...)
    PeInfo,                     ///< Partial evaluation debug info.
    EndScope,                   ///< Dummy function which marks the end of a @p Scope.
};

enum class CC : uint8_t {
    C,          ///< C calling convention.
    Device,     ///< Device calling convention. These are special functions only available on a particular device.
};

class App : public Def {
private:
    App(const Type* type, const Def* callee, const Def* arg, Debug dbg)
        : Def(Node_App, type, {callee, arg}, dbg)
    {}

public:
    const Def* callee() const { return op(0); }
    const Def* arg() const { return op(1); }

    size_t num_args() const;
    const Def* arg(size_t i) const;
    Array<const Def*> args() const;

    const Def* vrebuild(World&, const Type*, Defs) const override;

    friend class World;
};

/**
 * A function abstraction.
 * A @p Lam is always of function type @p FnTypeNode.
 */
class Lam : public Def {
private:
    Lam(const Pi* pi, CC cc, Intrinsic intrinsic, Debug dbg);

public:
    //@{ operands
    const Def* filter() const { return op(0); }
    const Def* filter(size_t i) const;
    const Def* body() const { return op(1); }
    const App* app() const { return body()->isa<App>(); }
    //@}

    //@{ params
    const Param* param(Debug dbg = {}) const;
    size_t num_params() const;
    const Def* param(size_t i, Debug dbg = {}) const;
    Array<const Def*> params() const;
    const Def* mem_param() const;
    const Def* ret_param() const;
    //@}

    //@{ setters
    void set_filter(const Def* filter) { update_op(0, filter); }
    void set_filter(Defs filter);
    void set_all_true_filter();
    void set_body(const Def* body) { update_op(1, body); }
    void destroy_filter();
    //@}

    //@{ type
    const Pi* type() const { return Def::type()->as<Pi>(); }
    const Type* domain() const { return type()->domain(); }
    const Type* codomain() const { return type()->codomain(); }
    //@}

    Def* vstub(World&, const Type*) const override;
    const Def* vrebuild(World&, const Type*, Defs) const override { THORIN_UNREACHABLE; }

    Lams preds() const;
    Lams succs() const;
    bool is_empty() const;
    Intrinsic& intrinsic() { return intrinsic_; }
    Intrinsic intrinsic() const { return intrinsic_; }
    CC& cc() { return cc_; }
    CC cc() const { return cc_; }
    void set_intrinsic(); ///< Sets @p intrinsic_ derived on this @p Lam's @p name.
    bool is_external() const;
    void make_external();
    void make_internal();
    bool is_basicblock() const;
    bool is_returning() const;
    bool is_intrinsic() const;
    bool is_accelerator() const;
    void destroy_body();

    std::ostream& stream_head(std::ostream&) const;
    std::ostream& stream_body(std::ostream&) const;
    void dump_head() const;
    void dump_body() const;

    // terminate

    void app(const Def* callee, const Def* arg, Debug dbg = {});
    void app(const Def* callee, Defs args, Debug dbg = {});
    void branch(const Def* cond, const Def* t, const Def* f, Debug dbg = {});
    void match(const Def* val, Lam* otherwise, Defs patterns, ArrayRef<Lam*> lams, Debug dbg = {});
    void verify() const {
#if THORIN_ENABLE_CHECKS
        //auto c = callee_fn_type();
        //auto a = arg_fn_type();
        //assertf(c == a, "lam '{}' calls '{}' of type '{}' but call has type '{}'\n", this, callee(), c, a);
#endif
    }

private:
    CC cc_;
    Intrinsic intrinsic_;

    friend class Cleaner;
    friend class Scope;
    friend class CFA;
    friend class World;
};

/**
 * A parameter of a @p Lam function.
 * A @p Param's op isits @p lam() it belongs to.
 */
class Param : public Def {
private:
    Param(const Type* type, const Lam* lam, Debug dbg)
        : Def(Node_Param, type, Defs{lam}, dbg)
    {
        assert(lam->is_nominal());
    }

public:
    Lam* lam() const { return op(0)->as_lam(); }
    const Def* vrebuild(World&, const Type*, Defs) const override;

    friend class World;
};

bool visit_uses(Lam*, std::function<bool(Lam*)>, bool include_globals);
bool visit_capturing_intrinsics(Lam*, std::function<bool(Lam*)>, bool include_globals = true);
bool is_passed_to_accelerator(Lam*, bool include_globals = true);
bool is_passed_to_intrinsic(Lam*, Intrinsic, bool include_globals = true);

void app_to_dropped_app(Lam* src, Lam* dst, const App*);

//------------------------------------------------------------------------------

template<class To>
using AppMap  = GIDMap<const App*, To>;
using AppSet  = GIDSet<const App*>;
using App2App = AppMap<const App*>;

template<class To>
using ParamMap    = GIDMap<const Param*, To>;
using ParamSet    = GIDSet<const Param*>;
using Param2Param = ParamMap<const Param*>;

template<class To>
using LamMap  = GIDMap<Lam*, To>;
using LamSet  = GIDSet<Lam*>;
using Lam2Lam = LamMap<Lam*>;

//------------------------------------------------------------------------------

}

#endif
