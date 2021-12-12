// STD
#include <ranges>

// FMT
#include <fmt/ostream.h>

// Engine
#include <Engine/Unicode/UTF8.hpp>

// Bench
#include <Bench/bench.hpp>

namespace Bench {
	void Context::runGroup(const std::string& name) {
		using Seconds = std::chrono::duration<long double>;
		using OutDur = std::chrono::duration<long double, std::micro>; // TODO: configurable output duration type (ms/us/ns) (infer based on times?)

		int32 warmups = 10; // TODO: configurable
		int32 iters = 100; // TODO: configurable

		using Cell = std::string;

		struct Row {
			Engine::FlatHashMap<std::string, Cell> cells;
		};

		struct Column {
			std::string title;

			/** The width in monospace units. Assumes every unicode code point is one unit wide. */
			int64 width = Engine::Unicode::length8(title);
		};

		std::vector<Row> rows;
		std::vector<Column> cols;

		cols.emplace_back("Name");
		cols.emplace_back("Dataset");
		cols.emplace_back("Avg");
		cols.emplace_back("Dataset Size");

		// Run benchmarks
		auto& ctx = Bench::Context::instance();
		auto& group = ctx.getGroup(name);
		ctx.samples.clear();

		for (auto& [id, bench] : group.benchmarks) {
			const auto start = Clock::now();
			for (int32 i = 0; i < warmups; ++i) {
				fmt::print("\r\033[0KRunning {} warm {}%", id, i*100/iters);
				ctx.samples.clear(); bench.func();
			}

			ctx.samples.clear();
			for (int32 i = 0; i < iters; ++i) {
				fmt::print("\r\033[0KRunning {} iter {}%", id, i*100/iters);
				bench.func();
			}
			const auto stop = Clock::now();
			fmt::print("\r\033[0K{} complete in {:.3}\n", id, Seconds{stop - start});

			const auto datasetSize = 100000; // TODO: query the dataset
			auto avg = std::chrono::duration_cast<OutDur>(std::reduce(ctx.samples.begin(), ctx.samples.end())) / datasetSize;

			auto& row = rows.emplace_back();
			row.cells["Name"] = id.name;
			row.cells["Dataset"] = id.dataset;
			row.cells["Avg"] = fmt::format("{:.3}", avg);
			row.cells["Dataset Size"] = fmt::format("{}", datasetSize);
		}

		// Output buffer
		std::string output;
		output.reserve(1024); // TOOD: how big to make this?
		auto out = std::back_inserter(output);

		// Figure out column widths
		for (auto& row : rows) {
			// TODO: switch to do for each row cell -> col so we can support custom cols
			for (auto& col : cols) {
				col.width = std::max(col.width, Engine::Unicode::length8(row.cells[col.title]));
			}
		}

		++cols.back().width; // Just looks nicer
		const auto totalWidth = (cols.size()-1)*3 + std::reduce(cols.cbegin(), cols.cend(), 0ll, [](auto s, const auto& o){ return s + o.width; });

		// Table header
		std::fill_n(out, totalWidth, '=');
		fmt::format_to(out, "\n= {}\n", name);
		std::fill_n(out, totalWidth, '-');
		out = '\n';

		if (auto col = cols.cbegin(), end = cols.cend(); col != end) { while (true) {
			fmt::format_to(out, col->title);

			auto diff = col->width - Engine::Unicode::length8(col->title);
			std::fill_n(out, diff, ' ');

			if (++col == end) { break; }
			out = ' '; out = '|'; out = ' ';
		}}

		out = '\n';
		std::fill_n(out, totalWidth, '-');
		out = '\n';

		// Table rows
		for (auto& row : rows) {
			if (auto col = cols.cbegin(), end = cols.cend(); col != end) { while (true) {
				const auto& cell = row.cells[col->title];
				fmt::format_to(out, cell);

				auto diff = col->width - static_cast<int64>(cell.size());
				std::fill_n(out, diff, ' ');

				if (++col == end) { break; }
				out = ' '; out = '|'; out = ' ';
			}}
			out = '\n';
		}
		out = '\n';

		fmt::print("\033[{}A\033[0J", group.benchmarks.size());
		fmt::print(output);
	}
}
