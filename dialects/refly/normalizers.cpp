#include "thorin/world.h"

#include "dialects/refly/refly.h"

namespace thorin::refly {

static_assert(sizeof(const Def*) <= sizeof(u64), "pointer doesn't fit into Lit");

/// The trick is that we simply "box" the pointer of @p def inside a Lit of type `%refly.Code`.
static Ref do_reify(const Def* def) { return def->world().lit(type_code(def->world()), reinterpret_cast<u64>(def)); }

/// And here we are doing the reverse to retrieve the original pointer again.
static const Def* do_reflect(const Def* def) { return reinterpret_cast<const Def*>(def->as<Lit>()->get()); }

template<dbg id>
Ref normalize_dbg(Ref type, Ref callee, Ref arg) {
    auto& world = arg->world();
    debug_print(arg);
    return id == dbg::perm ? world.raw_app(type, callee, arg) : arg;
}

Ref normalize_reify(Ref, Ref, Ref arg) { return do_reify(arg); }

Ref normalize_reflect(Ref, Ref, Ref arg, const Def*) { return do_reflect(arg); }

Ref normalize_refine(Ref type, Ref callee, Ref arg) {
    auto& world       = arg->world();
    auto [code, i, x] = arg->projs<3>();
    if (auto l = isa_lit(i)) {
        auto def = do_reflect(code);
        return do_reify(def->refine(*l, do_reflect(x)));
    }

    return world.raw_app(type, callee, arg);
}

Ref normalize_gid(Ref, Ref, Ref arg) {
    return arg->world().lit_nat(arg->gid());
}

THORIN_refly_NORMALIZER_IMPL

} // namespace thorin::refly
