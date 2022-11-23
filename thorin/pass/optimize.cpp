#include "thorin/pass/optimize.h"

#include "thorin/dialects.h"

#include "thorin/pass/fp/beta_red.h"
#include "thorin/pass/fp/eta_exp.h"
#include "thorin/pass/fp/eta_red.h"
#include "thorin/pass/fp/tail_rec_elim.h"
#include "thorin/pass/pipelinebuilder.h"
#include "thorin/pass/rw/lam_spec.h"
#include "thorin/pass/rw/partial_eval.h"
#include "thorin/pass/rw/ret_wrap.h"
#include "thorin/pass/rw/scalarize.h"
#include "thorin/phase/phase.h"

namespace thorin {

/// The optimizations proceed in a pipeline ordered by priorities.
/// Each phase is a sequence of passes that are run interleaved.
/// The passes are added to the phase ordered by their priority.

/// Order:
/// * 1-10: initial passes
/// * 100: Main optimization phase (default for extend_opt_phase)
/// * 200: Pre-CodeGen Optimization
/// * 300: CodeGen Preparation (default for extend_codegen_prep_phase)
///
/// concrete phases:
/// * 0  : Scalarize
/// * 1  : EtaRed
/// * 2  : TailRecElim
/// * 100: Optimize (Priority 50)
///   * PartialEval
///   * BetaRed
///   * EtaRed
///   * EtaExp
///   * Scalarize
///   * TailRecElim
///   * + Custom (default priority 100)
/// * 200: LamSpec
/// * 300: RetWrap (Priority 50)
///   * + Custom (default priority 100)

/// See optimize.h for magic numbers
void optimize(World& world, Passes& passes, PipelineBuilder& builder) {
    if (auto compilation = world.lookup("_compile")) {
        // We found a compilation directive in the file and use it to build the compilation pipeline.
        // The general idea is that passes and phases are exposed as axioms.
        // Each pass/phase axiom is associated with a handler function operating on the PipelineBuilder in the passes
        // map. This registering is analogous to the normalizers (`code -> code`) but only operated using side effects
        // that change the pipeline.
        world.DLOG("compilation using {} : {}", compilation, compilation->type());
        compilation->make_internal();

        // We can not directly access compile axioms here.
        // But the compile dialect has not the necessary communication pipeline.
        // Therefore, we register the handlers and let the compile dialect call them.

        PipelineBuilder pipe_builder;
        // TODO: remove indirections of pipeline builder. Just add passes and phases directly to the pipeline.

        auto pipeline     = compilation->as<Lam>()->body();
        auto [ax, phases] = collect_args(pipeline);

        // handle pipeline like all other pass axioms
        auto pipeline_axiom = ax->as<Axiom>();
        auto pipeline_flags = pipeline_axiom->flags();
        assert(passes.contains(pipeline_flags));
        passes[pipeline_flags](world, pipe_builder, pipeline);

        Pipeline pipe(world);
        world.DLOG("Building pipeline");
        pipe_builder.buildPipeline(pipe);

        pipe.run();
        return;
    }

    builder.extend_opt_phase(0, [](thorin::PassMan& man) { man.add<Scalerize>(); });
    builder.extend_opt_phase(1, [](thorin::PassMan& man) { man.add<EtaRed>(); });
    builder.extend_opt_phase(2, [](thorin::PassMan& man) { man.add<TailRecElim>(); });

    // main phase
    builder.add_opt(Opt_Phase);
    builder.extend_opt_phase(200, [](thorin::PassMan& man) { man.add<LamSpec>(); });
    // codegen prep phase
    builder.extend_opt_phase(
        Codegen_Prep_Phase, [](thorin::PassMan& man) { man.add<RetWrap>(); }, Pass_Internal_Priority);

    Pipeline pipe(world);

    // auto passes = builder.passes();
    // for (auto p : passes) pipe.add<PassManPhase>(builder.opt_phase(p, world));
    builder.buildPipeline(pipe);

    pipe.run();
}

} // namespace thorin
