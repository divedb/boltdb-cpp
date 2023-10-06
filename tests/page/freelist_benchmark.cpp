#include <benchmark/benchmark.h>

#include <algorithm>
#include <random>
#include <vector>

#include "boltdb/page/freelist.hpp"
#include "boltdb/page/page.hpp"

using namespace std;
using namespace boltdb;

static Page make_page(PageID pgid) { return {pgid, kFreeList, 4096}; }

static vector<PageID> random_pgids(int n) {
  vector<PageID> result;
  result.reserve(n);

  mt19937_64 rng(42);

  for (int i = 0; i < n; i++) {
    result.push_back(static_cast<PageID>(rng()));
  }

  sort(result.begin(), result.end());

  return result;
}

static void BM_freelist_release(benchmark::State& state) {
  auto size = state.range(0);
  vector<PageID> ids = random_pgids(size);
  vector<PageID> pending_ids = random_pgids(ids.size() / 400);
  vector<Page> pending_pages;
  pending_pages.reserve(pending_ids.size());
  transform(pending_ids.begin(), pending_ids.end(),
            back_inserter(pending_pages),
            [](PageID pgid) { return make_page(pgid); });

  TxnID txn_id = 1;

  for (auto _ : state) {
    FreeList freelist(ids);

    for (auto&& page : pending_pages) {
      freelist.free(txn_id, page);
    }

    freelist.release(1);
  }
}

BENCHMARK(BM_freelist_release)->RangeMultiplier(10)->Range(10000, 10000000);

BENCHMARK_MAIN();
