#ifndef BOLTDB_CPP_TRANSACTION_HPP_
#define BOLTDB_CPP_TRANSACTION_HPP_

#include <chrono>

#include "boltdb/util/common.hpp"

namespace boltdb {

class Txn {};

// Tx represents a read-only or read/write transaction on the database.
// Read-only transactions can be used for retrieving values for keys and
// creating cursors. Read/write transactions can create and remove buckets and
// create and remove keys.
//
// IMPORTANT: You must commit or rollback transactions when you are done with
// them. Pages can not be reclaimed by the writer until no more transactions
// are using them. A long running read transaction can cause the database to
// quickly grow.

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

// Sub calculates and returns the difference between two sets of transaction
// stats. This is useful when obtaining stats at two different points and time
// and you need the performance counters that occurred within that time span.
inline TxStats operator-(const TxStats& lhs, const TxStats& rhs) {
  TxStats result;

#define SUB(diff, op1, op2, field) (diff).field = (op1).field - (op2).field

  SUB(result, lhs, rhs, page_count);
  SUB(result, lhs, rhs, page_alloc);
  SUB(result, lhs, rhs, cursor_count);
  SUB(result, lhs, rhs, node_count);
  SUB(result, lhs, rhs, node_deref);
  SUB(result, lhs, rhs, rebalance);
  SUB(result, lhs, rhs, rebalance_time);
  SUB(result, lhs, rhs, split);
  SUB(result, lhs, rhs, spill);
  SUB(result, lhs, rhs, spill_time);
  SUB(result, lhs, rhs, write);
  SUB(result, lhs, rhs, write_time);

#undef ADD

  return result;
}

}  // namespace boltdb

#endif  // BOLTDB_CPP_TRANSACTION_HPP_