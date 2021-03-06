#include <percy/percy.hpp>
#include <cassert>
#include <cstdio>
#include <fstream>

using namespace percy;
using kitty::dynamic_truth_table;

/*******************************************************************************
    Tests denormalization of Boolean chains. This ensures that no chain output
    is inverted, but may result in suboptimal chains.
*******************************************************************************/
int main(void)
{

    {
        chain c;
        spec spec;

        kitty::dynamic_truth_table tt(3);
        for (int i = 0; i < 256; i++) {
            kitty::create_from_words(tt, &i, &i + 1);
            spec[0] = tt;
            const auto result = synthesize(spec, c);
            assert(result == success);
            if (!is_normal(tt)) {
                assert(c.is_output_inverted(0));
            }
            const auto sim_tt = c.simulate()[0];
            assert(sim_tt == tt);
            const auto nr_steps = c.get_nr_steps();
            c.denormalize();
            // If there are no steps, denormalization does nothing
            if (nr_steps > 0) {
                assert(!c.is_output_inverted(0));
            }
            assert(c.get_nr_steps() == nr_steps);
            const auto dsim_tt = c.simulate()[0];
            assert(dsim_tt == tt);
        }
    }
    
    {
        spec spec(2);
        spec.verbosity = 0;
        chain c;

        dynamic_truth_table in1(2), in2(2), f1(2), f2(2);

        kitty::create_nth_var(in1, 0);
        kitty::create_nth_var(in2, 1);

        f1 = in1 | in2;
        f2 = in1 & in2;

        spec[0] = f1;
        spec[1] = f2;

        auto result = synthesize(spec, c);
        assert(result == success);

        // For one OR gate and one AND gate we need exactly 2 steps.
        assert(c.get_nr_steps() == 2);
        auto tts = c.simulate();
        assert(tts[0] == f1);
        assert(tts[1] == f2);

        // After normalization we should still have 2 steps, since we have no
        // inverted outputs.
        c.denormalize();
        assert(c.get_nr_steps() == 2);
        tts = c.simulate();
        assert(tts[0] == f1);
        assert(tts[1] == f2);

        // Let us now synthesize an OR and a NOR function. These can be shared
        // in a normalized chain.
        f2 = ~f1;
        spec[1] = f2;
        result = synthesize(spec, c);
        assert(result == success);
        assert(c.get_nr_steps() == 1);
        tts = c.simulate();
        assert(tts[0] == f1);
        assert(tts[1] == f2);

        // Now when we denormalize we should have 2 steps, since we the
        // inverted output should have been removed.
        c.denormalize();
        assert(c.get_nr_steps() == 2);
        tts = c.simulate();
        assert(tts[0] == f1);
        assert(tts[1] == f2);

        // Finally, suppose we have 3 outputs, where two are inverted but share
        // the same function. In this case we can still denormalize the chain,
        // but we only have to add one additional step instead of two.
        spec.set_nr_out(3);
        spec[0] = f1;
        spec[1] = f2;
        spec[2] = f2;

        result = synthesize(spec, c);
        assert(result == success);
        assert(c.get_nr_steps() == 1);
        tts = c.simulate();
        assert(tts[0] == f1);
        assert(tts[1] == f2);
        assert(tts[2] == f2);

        c.denormalize();
        assert(c.get_nr_steps() == 2);
        tts = c.simulate();
        assert(tts[0] == f1);
        assert(tts[1] == f2);
        assert(tts[2] == f2);
    }

          
    return 0;
}

