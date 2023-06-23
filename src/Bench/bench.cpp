// STD
#include <ranges>

// Engine
#include <Engine/Unicode/UTF8.hpp>

// Bench
#include <Bench/bench.hpp>

namespace {
	extern const char* win32ProductTypeLookup[PRODUCT_XBOX_SCARLETTHOSTOS + 1];

	const char* win32GetProductTypeString(DWORD pt) {
		if (pt >= std::size(win32ProductTypeLookup)) { return "Unknown"; }
		return win32ProductTypeLookup[pt];
	}
}

namespace Bench {
	SystemInfo getSystemInfo() {
		#ifndef ENGINE_OS_WINDOWS
			#error TODO: implement for non-windows
		#endif

		SystemInfo info;

		{
			// Input values for the cpuid instruction can be found in:
			// AMD: Processor Programming Reference (PPR)
			// Intel: Intel® Architecture Instruction Set Extensions and Future Features Programming Reference
			int32 cpuinfo[4]; // EAX, EBX, ECX, EDX outputs
			info.cpu.reserve(4 * sizeof(cpuinfo));

			// Get maximum extended input
			__cpuid(cpuinfo, 0x80000000);
			const auto maxExtendedId = std::min<uint32>(cpuinfo[0], 0x80000004);

			for (auto func : {0x80000002, 0x80000003, 0x80000004}) {
				if (func > maxExtendedId) { break; }
				__cpuid(cpuinfo,  func);
				info.cpu.append(reinterpret_cast<const char*>(cpuinfo), sizeof(cpuinfo));
			}
			info.cpu.resize(strlen(info.cpu.data()));
		}

		#ifdef ENGINE_OS_WINDOWS
		// TODO: What we really want is Win32_OperatingSystem (this is closer to systeminfo) - https://docs.microsoft.com/en-us/windows/win32/cimwin32prov/win32-operatingsystem
		// TODO: cont: powershell example: Get-WmiObject -query "SELECT Version FROM Win32_OperatingSystem"
		if (const auto hmod = LoadLibraryW(L"ntdll.dll")) {
			using RtlGetVersionFunc = LONG(*)(PRTL_OSVERSIONINFOW lpVersionInformation);
			auto RtlGetVersion = reinterpret_cast<RtlGetVersionFunc>(::GetProcAddress(hmod, "RtlGetVersion"));
			OSVERSIONINFOEXW osinfo = {};
			osinfo.dwOSVersionInfoSize = sizeof(osinfo);
			RtlGetVersion(reinterpret_cast<PRTL_OSVERSIONINFOW>(&osinfo));

			DWORD productType = {};
			GetProductInfo(osinfo.dwMajorVersion, osinfo.dwMinorVersion, osinfo.wServicePackMajor, osinfo.wServicePackMinor, &productType);
			FreeLibrary(hmod);

			info.os.clear();
			fmt::format_to(std::back_inserter(info.os), "Windows {}.{}.{} {}",
				osinfo.dwMajorVersion, osinfo.dwMinorVersion, osinfo.dwBuildNumber,
				win32GetProductTypeString(productType)
			);
		}
		#endif

		return info;
	}

	void Context::runGroup(const std::string& name) {
		using Seconds = std::chrono::duration<long double>;
		using MircoDur = std::chrono::duration<long double, std::micro>;
		using NanoDur = std::chrono::duration<long double, std::nano>;

		auto& ctx = Bench::Context::instance();
		if (!ctx.hasGroup(name)) {
			ENGINE_WARN("No benchmark group found with name: \"", name, '"');
			return;
		}

		const auto sysInfo = getSystemInfo();
		fmt::print("{}\n{}\n\n", sysInfo.cpu, sysInfo.os);

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

		// TODO: scale output duration type based on size: `< 0.1s` -> ms, `< 0.1ms` -> us, `< 0.1us` -> ns
		// TODO: cont. although make sure all outputs have same units. Probably determine based on largest?
		// TODO: cont. also probably configurable through cmd line args
		cols.emplace_back("Name");
		cols.emplace_back("Dataset");
		cols.emplace_back("Avg");
		cols.emplace_back("Dataset Size");

		// Run benchmarks
		auto& group = ctx.getGroup(name);
		ctx.samples.reserve(iters);

		for (auto& [id, bench] : group.getBenchmarks()) {
			const auto start = Clock::now();

			fmt::print("\r\033[0KRunning {}", id);
			bench.singleFunc();
			
			ctx.samples.clear();
			for (int32 i = 0; i < warmups; ++i) {
				 bench.iterFunc();
			}

			ctx.samples.clear();
			for (int32 i = 0; i < iters; ++i) {
				// Disabled because this had a significant impact benchmarks. Use to also have in warmup section.
				// fmt::print("\r\033[0KRunning {} iter {}%", id, i*100/iters);
				bench.iterFunc();
			}

			const auto stop = Clock::now();
			fmt::print("\r\033[0K{} complete in {:.3}\n", id, Seconds{stop - start});

			constexpr auto countify = [](const auto& x) { return static_cast<long double>(x.count()); };
			const auto propsRaw = calcSampleProperties(ctx.samples | std::views::transform(countify)).scaleN(static_cast<long double>(bench.size));
			const auto props = SampleProperties<MircoDur>(SampleProperties<NanoDur>(propsRaw));

			const auto avg = props.mean;

			auto& row = rows.emplace_back();
			row.cells["Name"] = id.name;
			row.cells["Dataset"] = id.dataset;
			row.cells["Avg"] = fmt::format("{:.6}", avg);
			row.cells["Dataset Size"] = fmt::format("{}", bench.size);
			// TODO: time to run

			/*
			for (auto& [k, v] : custom) {
				if (k != "E-avg") { continue; }
				char chars[256] = {};
				const long double dub = *(const long double*)v->get();
				std::to_chars(chars, chars + sizeof(chars), dub);
				fmt::print("Key({0}) = Value({1} {1:} {1:f} {1:g} {1:G} {1:e} {1:E} {1:6g} {1:6G} {2} {3})\n", k, *v, chars, bool(dub == 0.0L));
			}*/

			for (const auto& [col, value] : custom) {
				if (std::ranges::find(cols, col, &Column::title) == cols.end()) {
					cols.emplace_back(col);
				}
				row.cells[col] = value->str();
			}

			custom.clear();
		}

		// Sort rows
		std::ranges::sort(rows, [](auto& a, auto& b) {
			return std::tie(a.cells["Name"], a.cells["Dataset"]) < std::tie(b.cells["Name"], b.cells["Dataset"]);
		});

		// Output buffer
		std::string output;
		output.reserve(1024);
		auto out = std::back_inserter(output);

		// Figure out column widths
		for (auto& row : rows) {
			for (auto& col : cols) {
				col.width = std::max(col.width, Engine::Unicode::length8(row.cells[col.title]));
			}
		}

		++cols.back().width; // Just looks nicer
		// C++23: std::ranges::reduce(cols, {}, &Column::width);
		const auto totalWidth = (cols.size()-1)*3 + std::reduce(cols.cbegin(), cols.cend(), 0ll, [](auto s, const auto& o){ return s + o.width; });

		// Table header
		// TODO: Windows seems to break some stuff whenever it feels like it. If you change this to 4096 it still gets cut at edge of display. Will be easy to lose data
		std::fill_n(out, totalWidth, '=');
		out = '\n';
		std::ranges::copy(name, out);
		out = '\n';
		std::fill_n(out, totalWidth, '-');
		out = '\n';

		if (auto col = cols.cbegin(), end = cols.cend(); col != end) { while (true) {
			std::ranges::copy(col->title, out);

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
				std::ranges::copy(cell, out);

				auto diff = col->width - Engine::Unicode::length8(cell);
				std::fill_n(out, diff, ' ');

				if (++col == end) { break; }
				out = ' '; out = '|'; out = ' ';
			}}
			out = '\n';
		}
		out = '\n';

		fmt::print("\033[{}A\033[0J", group.getBenchmarks().size());
		std::cout << '\n' << output << "\n";
	}
}

namespace {
	// Will need to be updated for future Windows versions. See: winnt.h
	const char* win32ProductTypeLookup[] = {
		"UNDEFINED",
		"Ultimate",
		"Home Basic",
		"Home Premium",
		"Enterprise",
		"Home Basic N",
		"Business",
		"Standard Server",
		"Datacenter Server",
		"Small Business Server",
		"Enterprise Server",
		"Starter",
		"Datacenter Server Core",
		"Standard Server Core",
		"Enterprise Server Core",
		"Enterprise Server Ia64",
		"Business N",
		"Web Server",
		"Cluster Server",
		"Home Server",
		"Storage Express Server",
		"Storage Standard Server",
		"Storage Workgroup Server",
		"Storage Enterprise Server",
		"Server For Small Business",
		"Small Business Server Premium",
		"Home Premium N",
		"Enterprise N",
		"Ultimate N",
		"Web Server Core",
		"Medium Business Server Management",
		"Medium Business Server Security",
		"Medium Business Server Messaging",
		"Server Foundation",
		"Home Premium Server",
		"Server For Small Business V",
		"Standard Server V",
		"Datacenter Server V",
		"Enterprise Server V",
		"Datacenter Server Core V",
		"Standard Server Core V",
		"Enterprise Server Core V",
		"Hyperv",
		"Storage Express Server Core",
		"Storage Standard Server Core",
		"Storage Workgroup Server Core",
		"Storage Enterprise Server Core",
		"Starter N",
		"Professional",
		"Professional N",
		"Sb Solution Server",
		"Server For Sb Solutions",
		"Standard Server Solutions",
		"Standard Server Solutions Core",
		"Sb Solution Server Em",
		"Server For Sb Solutions Em",
		"Solution Embedded Server",
		"Solution Embedded Server Core",
		"Professional Embedded",
		"Essential Business Server Mgmt",
		"Essential Business Server Addl",
		"Essential Business Server Mgmtsvc",
		"Essential Business Server Addlsvc",
		"Small Business Server Premium Core",
		"Cluster Server V",
		"Embedded",
		"Starter E",
		"Home Basic E",
		"Home Premium E",
		"Professional E",
		"Enterprise E",
		"Ultimate E",
		"Enterprise Evaluation",
		"Multipoint Standard Server",
		"Multipoint Premium Server",
		"Standard Evaluation Server",
		"Datacenter Evaluation Server",
		"Enterprise N Evaluation",
		"Embedded Automotive",
		"Embedded Industry A",
		"Thinpc",
		"Embedded A",
		"Embedded Industry",
		"Embedded E",
		"Embedded Industry E",
		"Embedded Industry A E",
		"Storage Workgroup Evaluation Server",
		"Storage Standard Evaluation Server",
		"Core Arm",
		"Core N",
		"Core Country Specific",
		"Core Single Language",
		"Core",
		"Professional Wmc",
		"Embedded Industry Eval",
		"Embedded Industry E Eval",
		"Embedded Eval",
		"Embedded E Eval",
		"Nano Server",
		"Cloud Storage Server",
		"Core Connected",
		"Professional Student",
		"Core Connected N",
		"Professional Student N",
		"Core Connected Single Language",
		"Core Connected Country Specific",
		"Connected Car",
		"Industry Handheld",
		"Ppi Pro",
		"Arm64 Server",
		"Education",
		"Education N",
		"Iotuap",
		"Cloud Host Infrastructure Server",
		"Enterprise S",
		"Enterprise S N",
		"Professional S",
		"Professional S N",
		"Enterprise S Evaluation",
		"Enterprise S N Evaluation",
		"Holographic",
		"Holographic Business",
		"Pro Single Language",
		"Pro China",
		"Enterprise Subscription",
		"Enterprise Subscription N",
		"Datacenter Nano Server",
		"Standard Nano Server",
		"Datacenter A Server Core",
		"Standard A Server Core",
		"Datacenter Ws Server Core",
		"Standard Ws Server Core",
		"Utility Vm",
		"Datacenter Evaluation Server Core",
		"Standard Evaluation Server Core",
		"Pro Workstation",
		"Pro Workstation N",
		"Pro For Education",
		"Pro For Education N",
		"Azure Server Core",
		"Azure Nano Server",
		"Enterpriseg",
		"Enterprisegn",
		"Serverrdsh",
		"Cloud",
		"Cloudn",
		"Hubos",
		"Onecoreupdateos",
		"Cloude",
		"Andromeda",
		"Iotos",
		"Clouden",
		"Iotedgeos",
		"Iotenterprise",
		"Lite",
		"Iotenterprises",
		"Xbox Systemos",
		"Xbox Nativeos",
		"Xbox Gameos",
		"Xbox Eraos",
		"Xbox Durangohostos",
		"Xbox Scarletthostos",
	};
}
