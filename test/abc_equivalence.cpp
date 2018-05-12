#include <cstdio>
#include <percy/percy.hpp>
#include <kitty/kitty.hpp>

#include "base/abc/abc.h"
#include "base/main/main.h"

#define MAX_TESTS 256

using namespace percy;
using namespace abc;

/*******************************************************************************
    Verifies that our synthesizers' results are equivalent to ABC's.
*******************************************************************************/
template<int NrIn>
void check_equivalence(bool full_coverage)
{
    synth_spec<static_truth_table<NrIn>> spec(NrIn, 1);
    auto synth = new_std_synth();

    spec.verbosity = 0;

    Abc_Start();

    // Don't run too many tests.
    auto max_tests = (1 << (1 << NrIn));
    if (!full_coverage) {
        max_tests = (std::min)(max_tests, MAX_TESTS);
    }
    static_truth_table<NrIn> tt;
    chain<2> c;
    for (auto i = 1; i < max_tests; i++) {
        kitty::create_from_words(tt, &i, &i+1);
        word abc_tt = 0;
        for (auto block = tt.begin(); block != tt.end(); block++) {
            abc_tt = *block;
        }

        // We skip the trivial functions
        if (is_trivial(tt)) {
            continue;
        }

        spec.functions[0] = &tt;
        auto res = synth->synthesize(spec, c);
        assert(res == success);
        auto chain_size = c.get_nr_vertices();
        assert(c.satisfies_spec(spec));

        auto abc_ntk = Abc_NtkFindExact(&abc_tt, NrIn, 1, -1, NULL, 0, 0, 0);
        auto abc_ntk_size = Abc_NtkNodeNum(abc_ntk);
        // Not a normal function, so we subtract 1 from abc's network size
        if (!is_normal(tt)) {
            abc_ntk_size -= 1;
        }
        assert(abc_ntk_size == chain_size);
        Abc_NtkDelete(abc_ntk);
        printf("(%d/%d)\r", i+1, max_tests);
        fflush(stdout);
    }
    printf("\n");

    Abc_Stop();
}

/*******************************************************************************
    By default, does not check for full equivalence of all n-input functions.
    Users can specify a arbitrary runtime argument, which removes the limit on
    the number of equivalence tests.
 *******************************************************************************/
int main(int argc, char **argv)
{
    bool full_coverage = false;
    if (argc > 1) {
        full_coverage = true;
    }
    if (full_coverage) {
        printf("Doing full equivalence check\n");
    } else {
        printf("Doing partial equivalence check\n");
    }

    check_equivalence<2>(full_coverage);
    check_equivalence<3>(full_coverage);
    check_equivalence<4>(full_coverage);

    return 0;
}

