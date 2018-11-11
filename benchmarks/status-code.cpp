#include <string>
#include <unordered_map>

#include <benchmark/benchmark.h>

#include "ArrayStatusStrings.h"
#include "../StatusStrings.h"

namespace {

using shitty::status_strings;
using shitty::benchmark::array_status_strings;
using shitty::benchmark::continuous_status_strings;

// If you're concerned about this one being 34× faster possibly due to being
// optimized away (or repeated calls optimizing away because it's
// __attribute__((const))), insert an output statement (eg. stderr) in the
// implementation to see that it really is being called — the
// __attribute__((const)) doesn't result in only a single call.
static void BM_StatusStrings(benchmark::State& state) {
    for (auto _: state) {
        benchmark::DoNotOptimize(status_strings.get(200u));
    }
}
BENCHMARK(BM_StatusStrings);

static void BM_std_tostring(benchmark::State& state) {
    for (auto _: state) {
        std::to_string(200u);
    }
}
BENCHMARK(BM_std_tostring);

// For comparison, see how fast a lookup into an unordered map would be.
class UMAPStatusStrings {
public:
    UMAPStatusStrings() {
        strs.reserve(500);

        for (unsigned code = 100; code < 600; ++code) {
            strs.emplace(code, std::to_string(code));
        }
    }

    std::unordered_map<unsigned, std::string> strs;
};

static UMAPStatusStrings umap_status_strings;

static void BM_unordered_map_code(benchmark::State& state) {
    for (auto _: state) {
        benchmark::DoNotOptimize(umap_status_strings.strs.at(200u));
    }
}
BENCHMARK(BM_unordered_map_code);

static void BM_ContinuousStatusStrings(benchmark::State& state) {
    for (auto _: state) {
        benchmark::DoNotOptimize(continuous_status_strings.str(200u));
    }
}
BENCHMARK(BM_ContinuousStatusStrings);

static void BM_ArrayStatusStrings(benchmark::State& state) {
    for (auto _: state) {
        benchmark::DoNotOptimize(array_status_strings.get(200u));
    }
}
BENCHMARK(BM_ArrayStatusStrings);

} // namespace anon

BENCHMARK_MAIN();
