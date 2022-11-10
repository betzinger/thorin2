#ifndef THORIN_PASS_RW_LOWER_MATRIX_LOWLEVEL_H
#define THORIN_PASS_RW_LOWER_MATRIX_LOWLEVEL_H

#include <thorin/def.h>
#include <thorin/pass/pass.h>
#include <thorin/phase/phase.h>

namespace thorin::matrix {

class LowerMatrixLowLevel : public RWPhase {
public:
    LowerMatrixLowLevel(World& world)
        : RWPhase(world, "lower_matrix_lowlevel") {}

    /// custom rewrite function
    /// memoized version of rewrite_

    // const Def* rewrite_def(const Def*);
    // const Def* rewrite_def_(const Def*);

    // void enter() override;
    // void rewrite_lam(Lam* lam);

    const Def* rewrite_structural(const Def*) override;

    static PassTag* ID();

private:
    Def2Def rewritten;
};

// class LowerMatrixLowLevel : public RWPass<LowerMatrixLowLevel, Lam> {
// public:
//     LowerMatrixLowLevel(PassMan& man)
//         : RWPass(man, "lower_matrix_lowlevel") {}

//     void prepare() override { clos::LowerMatrixLowLevelPhase(world()).run(); }
// };

} // namespace thorin::matrix

#endif
