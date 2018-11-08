#include <benchmark/benchmark.h>

#include "../StatusStrings.h"

using shitty::status_strings;

// If you're concerned about this one being 34× faster possibly due to being
// optimized away (or repeated calls optimizing away because it's
// __attribute__((const))), insert an output statement (eg. stderr) in the
// implementation to see that it really is being called — the
// __attribute__((const)) doesn't result in only a single call.
static void BM_StatusStrings(benchmark::State& state) {
    for (auto _: state) {
        benchmark::DoNotOptimize(status_strings.str(200u));
    }
}
BENCHMARK(BM_StatusStrings);

static void BM_std_tostring(benchmark::State& state) {
    for (auto _: state) {
        std::to_string(200u);
    }
}
BENCHMARK(BM_std_tostring);

BENCHMARK_MAIN();
