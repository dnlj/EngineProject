// STD
#include <ranges>

// FMT
#include <fmt/ostream.h>

// Engine
#include <Engine/Unicode/UTF8.hpp>

// Bench
#include <Bench/bench.hpp>

namespace {
	extern const char* win32ProductTypeLookup[PRODUCT_XBOX_SCARLETTHOSTOS];

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
		using OutDur = std::chrono::duration<long double, std::micro>; // TODO: configurable output duration type (ms/us/ns) (infer based on times?)

		const auto sysInfo = getSystemInfo();
		fmt::print("{}\n{}\n", sysInfo.cpu, sysInfo.os);

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
		// C++23: std::ranges::reduce(cols, {}, &Column::width);
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

namespace {
	// Will need to be updated for future Windows versions. See: winnt.h
	const char* win32ProductTypeLookup[PRODUCT_XBOX_SCARLETTHOSTOS] = {
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
