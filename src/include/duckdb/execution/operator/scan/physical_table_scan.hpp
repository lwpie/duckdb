//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/execution/operator/scan/physical_table_scan.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/extra_operator_info.hpp"
#include "duckdb/execution/physical_operator.hpp"
#include "duckdb/function/table_function.hpp"
#include "duckdb/planner/table_filter.hpp"
#include "duckdb/storage/data_table.hpp"

#include <tbb/enumerable_thread_specific.h>

namespace duckdb {

//! Represents a scan of a base table
class PhysicalTableScan : public PhysicalOperator {
public:
	static constexpr const PhysicalOperatorType TYPE = PhysicalOperatorType::TABLE_SCAN;

public:
	//! Table scan that immediately projects out filter columns that are unused in the remainder of the query plan
	PhysicalTableScan(vector<LogicalType> types, TableFunction function, unique_ptr<FunctionData> bind_data,
	                  vector<LogicalType> returned_types, vector<column_t> column_ids, vector<idx_t> projection_ids,
	                  vector<string> names, unique_ptr<TableFilterSet> table_filters, idx_t estimated_cardinality,
	                  ExtraOperatorInfo extra_info, vector<Value> parameters);

	//! The table function
	TableFunction function;
	//! Bind data of the function
	unique_ptr<FunctionData> bind_data;
	//! The types of ALL columns that can be returned by the table function
	vector<LogicalType> returned_types;
	//! The column ids used within the table function
	vector<column_t> column_ids;
	//! The projected-out column ids
	vector<idx_t> projection_ids;
	//! The names of the columns
	vector<string> names;
	//! The table filters
	unique_ptr<TableFilterSet> table_filters;
	//! Currently stores info related to filters pushed down into MultiFileLists
	ExtraOperatorInfo extra_info;
	//! Parameters
	vector<Value> parameters;
	//! Contains a reference to dynamically generated table filters (through e.g. a join up in the tree)
	shared_ptr<DynamicTableFilterSet> dynamic_filters;
	//! The file name of the current scan operator
	mutable tbb::enumerable_thread_specific<std::string> file_name;

public:
	string GetName() const override;
	InsertionOrderPreservingMap<string> ParamsToString() const override;

	bool Equals(const PhysicalOperator &other) const override;

public:
	unique_ptr<LocalSourceState> GetLocalSourceState(ExecutionContext &context,
	                                                 GlobalSourceState &gstate) const override;
	unique_ptr<GlobalSourceState> GetGlobalSourceState(ClientContext &context) const override;
	SourceResultType GetData(ExecutionContext &context, DataChunk &chunk, OperatorSourceInput &input) const override;
	idx_t GetBatchIndex(ExecutionContext &context, DataChunk &chunk, GlobalSourceState &gstate,
	                    LocalSourceState &lstate) const override;

	bool IsSource() const override {
		return true;
	}
	bool ParallelSource() const override;

	bool SupportsBatchIndex() const override {
		return function.get_batch_index != nullptr;
	}

	double GetProgress(ClientContext &context, GlobalSourceState &gstate) const override;
};

} // namespace duckdb
