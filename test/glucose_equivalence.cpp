#include <cstdio>
#include <percy/percy.hpp>

#define MAX_TESTS 256

using namespace percy;
using kitty::static_truth_table;

/*******************************************************************************
    Verifies that our synthesizers' results are equivalent to each other.
*******************************************************************************/
template<int NrIn>
void check_std_equivalence()
{
    spec spec;

    bsat_wrapper solver1;
    glucose_wrapper solver2;

    ssv_encoder encoder1(solver1);
    ssv_encoder encoder2(solver2);

    spec.verbosity = 0;

    chain c1, c1_cegar, c2, c2_cegar;

    // Don't run too many tests.
    auto max_tests = (1 << (1 << NrIn));
    max_tests = std::min(max_tests, MAX_TESTS);
    static_truth_table<NrIn> tt;
    for (auto i = 1; i < max_tests; i++) {
        kitty::create_from_words(tt, &i, &i+1);

        spec[0] = tt;
        auto res1 = synthesize(spec, c1, solver1, encoder1);
        assert(res1 == success);
        auto sim_tts1 = c1.simulate();
        auto c1_nr_steps = c1.get_nr_steps();

        auto res1_cegar = synthesize(spec, c1_cegar, solver1, encoder1, SYNTH_STD_CEGAR);
        assert(res1_cegar == success);
        auto sim_tts1_cegar = 
            c1_cegar.simulate();
        auto c1_cegar_nr_steps = c1_cegar.get_nr_steps();

        auto res2 = synthesize(spec, c2, solver2, encoder2);
        assert(res2 == success);
        auto sim_tts2 = c2.simulate();
        auto c2_nr_steps = c2.get_nr_steps();

        /*
         * CEGAR doesn't work when using Glucose::MultiSolvers
        auto res2_cegar = synth2->cegar_synthesize(spec, c2_cegar);
        assert(res2_cegar == success);
        auto sim_tts2_cegar = 
            c2_cegar.simulate();
        auto c2_cegar_nr_steps = c2_cegar.get_nr_steps();
        */

        assert(c1_nr_steps == c2_nr_steps);
        assert(c1_nr_steps == c1_cegar_nr_steps);
        //assert(c1_cegar_nr_steps == c2_cegar_nr_steps);
        assert(sim_tts1[0] == sim_tts2[0]);
        assert(sim_tts1[0] == sim_tts1_cegar[0]);
        //assert(sim_tts1_cegar[0] == sim_tts2_cegar[0]);
        
        printf("(%d/%d)\r", i+1, max_tests);
        fflush(stdout);
    }
    printf("\n");
}

/*******************************************************************************
    By default, does not check for full equivalence of all n-input functions.
    Users can specify a arbitrary runtime argument, which removes the limit on
    the number of equivalence tests.
*******************************************************************************/
int main()
{
#ifndef DISABLE_GLUCOSE
#ifndef USE_SYRUP
    check_std_equivalence<2>();
    check_std_equivalence<3>();
    check_std_equivalence<4>();
#endif
#endif
    return 0;
}

