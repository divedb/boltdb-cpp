#ifndef BOLTDB_CPP_TRANSACTION_HPP_
#define BOLTDB_CPP_TRANSACTION_HPP_

#include <chrono>

#include "boltdb/util/common.hpp"

namespace boltdb {

using Duration = std::chrono::duration<f32>;

// TxStats represents statistics about the actions performed by the transaction.
struct TxStats {
 public:
  int page_count{};           // Number of page allocations
  int page_alloc{};           // Total bytes allocated
  int cursor_count{};         // Number of cursors created
  int node_count{};           // Number of node allocations
  int node_deref{};           // Number of node dereferences
  int rebalance{};            // Number of node rebalances
  Duration rebalance_time{};  // Total time spent on rebalancing
  int split{};                // Number of nodes split
  int spill{};                // Number of nodes spilled
  Duration spill_time{};      // Total time spent on spilling
  int write{};                // Number of writes performed
  Duration write_time{};      // Total time spent on writing to disk
};

inline TxStats operator+(const TxStats& lhs, const TxStats& rhs) {
  TxStats result;

#define ADD(sum, op1, op2, field) (sum).field = (op1).field + (op2).field

  ADD(result, lhs, rhs, page_count);
  ADD(result, lhs, rhs, page_alloc);
  ADD(result, lhs, rhs, cursor_count);
  ADD(result, lhs, rhs, node_count);
  ADD(result, lhs, rhs, node_deref);
  ADD(result, lhs, rhs, rebalance);
  ADD(result, lhs, rhs, rebalance_time);
  ADD(result, lhs, rhs, split);
  ADD(result, lhs, rhs, spill);
  ADD(result, lhs, rhs, spill_time);
  ADD(result, lhs, rhs, write);
  ADD(result, lhs, rhs, write_time);

#undef ADD

  return result;
}

}  // namespace boltdb

#endif  // BOLTDB_CPP_TRANSACTION_HPP_