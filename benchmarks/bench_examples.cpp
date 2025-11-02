#include <benchmark/benchmark.h>

// instructing bench to not optimize d out
void BM_malloc(benchmark::State& state) {
    for (auto _ : state) {
        constexpr size_t size{ 1024 };
        auto d = malloc(size);
        benchmark::DoNotOptimize(d);
        free(d);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_malloc);

// repeating very small work with a macro so that the numbers
//  are more meaningful
static void BM_tiny_work(benchmark::State& state) {
    size_t i{};
    for (auto _ : state) {
        ++i;
        benchmark::DoNotOptimize(i);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_tiny_work);

// repeating very small work with a macro
//  so that the benchmark overhead itself is minimized
#define EXECUTE2X(x) x x
#define EXECUTE4X(x) EXECUTE2X(x) EXECUTE2X(x)
#define EXECUTE8X(x) EXECUTE4X(x) EXECUTE4X(x)
#define EXECUTE16X(x) EXECUTE8X(x) EXECUTE8X(x)
#define EXECUTE32X(x) EXECUTE16X(x) EXECUTE16X(x)
static void BM_tiny_work_32x(benchmark::State& state) {
    size_t i{};
    for (auto _ : state) {
        EXECUTE32X(
            ++i;
            benchmark::DoNotOptimize(i);
        )
    }
    state.SetItemsProcessed(32*state.iterations());
}
BENCHMARK(BM_tiny_work_32x);