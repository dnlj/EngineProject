#pragma once

#include <vector>
#include <chrono>
#include <thread>
#include <iostream>


namespace Bench {
	using namespace Engine::Types;

	using Clock = std::chrono::high_resolution_clock;
	using Duration = Clock::duration;
	using TimePoint = Clock::time_point;

	class Context;

	/**
	 * Memory clobber.
	 */
	ENGINE_INLINE inline void clobber() noexcept {
		// Cost: none
		std::atomic_signal_fence(std::memory_order_acq_rel);
	}

	/**
	 * Observes a value as a side effect for the purposes of optimization.
	 * From now on every clobber() will act as a read from @p o for
	 * the purposes of optimization.
	 * 
	 * @param o The expression to observe.
	 */
	template<class T>
	ENGINE_INLINE void observe(T&& o) noexcept {
		#ifdef _MSC_VER
			// Costs: lea, mov
			[[maybe_unused]] void* volatile unused = &o;
			clobber();
		#else
			// Costs: lea
			asm volatile (""::"g"(&o):"memory");

			// Cost: none
			// This has no cost but has a slightly different effect of only observing only
			// the current value but not future values when clobbered.
			// There is no equivalent or workaround on MSVC that i am aware of.
			//asm volatile ("":"+r"(o));
		#endif
	}

	class SystemInfo {
		public:
			std::string cpu;
			std::string os;
	};

	SystemInfo getSystemInfo();

	class BenchmarkId {
		public:
			std::string name;
			std::string dataset; // TODO: Dataset*

			friend decltype(auto) operator<<(std::ostream& os, const BenchmarkId& id) {
				return os << id.name << "/" << id.dataset;
			}

			ENGINE_INLINE bool operator==(const BenchmarkId& rhs) const noexcept {
				return name == rhs.name && dataset == rhs.dataset;
			}
	};
}

template<>
struct Engine::Hash<Bench::BenchmarkId> {
	size_t operator()(const Bench::BenchmarkId& val) const {
		auto seed = Engine::hash(val.name);
		Engine::hashCombine(seed, Engine::hash(val.dataset));
		return seed;
	}
};

namespace Bench {
	class Benchmark {
		public:
			using Func = void(*)();

		public:
			Func iterFunc;
			Func singleFunc;
			int64 size;
	};
	
	class Group {
		public: // TODO: private
			Engine::FlatHashMap<BenchmarkId, Benchmark> benchmarks;

		public:
			int add(BenchmarkId id, Benchmark bench) {
				const auto found = benchmarks.find(id);
				if (found != benchmarks.end()) {
					ENGINE_WARN("Benchmark \"", id, "\" already exists.");
					id.name += "~DUPLICATE~";
					return add(id, bench);
				} else {
					benchmarks.emplace(id, bench);
				}
				return 0;
			}

	};
	
	enum class StatInterp {
		Calc,
		Flat,
	};

	class StoredValueBase {
		public:
			StoredValueBase() = default;
			StoredValueBase(const StoredValueBase&) = delete;

			virtual auto fmt_format(fmt::dynamic_formatter<>& formatter, fmt::format_context& ctx) const -> decltype(ctx.out()) = 0;
			virtual const void* get() const = 0;
	};

	template<class T>
	class StoredValue : public StoredValueBase {
		private:
			static_assert(std::same_as<T, std::remove_cvref_t<T>>);
			T value;

		public:
			template<class U>
			StoredValue(U&& val) : value{std::forward<U>(val)} {
			};

			virtual auto fmt_format(fmt::dynamic_formatter<>& formatter, fmt::format_context& ctx) const -> decltype(ctx.out()) override {
				return formatter.format(value, ctx);
			}

			virtual const void* get() const override {
				return &value;
			}
	};

	template<class T>
	class SampleProperties {
		public:
			T min = {};
			T max = {};
			T mean = {};
			T stddev = {};
	};

	template<class Range>
	auto calcSampleProperties(const Range& input) {
		using T = std::remove_cvref_t<decltype(*std::ranges::data(input))>;
		SampleProperties<T> props = {};
		if (std::ranges::empty(input)) { return props; }

		props.min = *std::ranges::begin(input);
		props.max = props.min;
		for (T c = {}; const auto val : input) {
			props.min = std::min(props.min, val);
			props.max = std::max(props.max, val);

			// Kahan Summation
			const auto y = val - c;
			const auto t = props.mean + y;
			c = (t - props.mean) - y;
			props.mean = t;
		}

		const auto size = std::ranges::size(input);
		props.mean /= size;

		for (T c = {}; const auto& val : input) {
			const auto diff = val - props.mean;
			
			// Kahan Summation
			const auto y = diff * diff - c;
			const auto t = props.stddev + y;
			c = (t - props.stddev) - y;
			props.stddev = y;
		}

		props.stddev = std::sqrt(props.stddev / input.size());

		return props;
	}

	class Context {
		private:
			Engine::FlatHashMap<std::string, Group> groups;
			Engine::FlatHashMap<std::string, std::unique_ptr<StoredValueBase>> custom;
			TimePoint sampleStart;
			TimePoint sampleStop;
			std::vector<Duration> samples;

		public:
			Context() {};

			ENGINE_INLINE bool hasGroup(const std::string& name) { return groups.contains(name); }
			ENGINE_INLINE auto& getGroup(const std::string& name) { return groups[name]; }

			/*
			 * We usually want to sample the whole dataset at once
			 * because the cost of `Clock::now()` could actually out weigh our operation we are benchmarking.
			 * For example, on MSVC `now` performs: 2x call, 4x idiv, 2x imul, 8x other
			 * whereas just doing there vector iteration is just an: add, cmp, jne
			 * So timing the loop as a whole should give us more accurate numbers.
			 */
			ENGINE_INLINE void startSample() noexcept {
				std::atomic_thread_fence(std::memory_order_acq_rel);
				sampleStart = Clock::now();
				std::atomic_thread_fence(std::memory_order_acq_rel);
			}
			
			ENGINE_INLINE void stopSample() noexcept {
				std::atomic_thread_fence(std::memory_order_acq_rel);
				sampleStop = Clock::now();
				std::atomic_thread_fence(std::memory_order_acq_rel);
				samples.push_back(sampleStop - sampleStart);
			}

			ENGINE_INLINE static Context& instance() { static Context inst; return inst; }

			template<class T>
			ENGINE_INLINE static auto& getDataset() { static T inst; return inst; }

			void runGroup(const std::string& name);

			template<class T>
			void set(const std::string& col, T&& value, StatInterp interp = {}) {
				// TODO: different interp modes
				custom[col] = std::make_unique<StoredValue<std::remove_cvref_t<T>>>(value);
			}
	};
}


template <> struct fmt::formatter<Bench::StoredValueBase> : dynamic_formatter<> {
	constexpr auto format(const Bench::StoredValueBase& value, format_context& ctx) {
		return value.fmt_format(*this, ctx);
	}
};


#define BENCH_CONCAT_IMPL(a, b) a##b
#define BENCH_CONCAT(a, b) BENCH_CONCAT_IMPL(a, b)

#define BENCH_DEFINE_COMPILE_TYPE_DEF_CHECK(Name, Negate, Message)\
	template<class T, unsigned long long line, class = void> struct _bench_compile_check_##Name { static_assert(Negate || (line != line), Message); };\
	template<class T, unsigned long long line> struct _bench_compile_check_##Name<T, line, std::void_t<decltype(sizeof(T))>> { static_assert(!Negate || (line != line), Message); };

#define BENCH_USE_COMPILE_CHECK(Name, Type)\
	static _bench_compile_check_##Name<Type, __LINE__> BENCH_CONCAT(_bench_compile_check_##Name##_var, __LINE__);

/** Check for BENCH_GROUP */
BENCH_DEFINE_COMPILE_TYPE_DEF_CHECK(single_group_per_unit, true, "You many only have one benchmark group per translation unit.");

/**
 * TODO: doc
 */
#define BENCH_GROUP(Name, ...)\
	BENCH_USE_COMPILE_CHECK(single_group_per_unit, struct _bench_group_id)\
	struct _bench_group_id { constexpr static char name[] = Name; };

/**
 * TODO: doc
 */
#define BENCH(Name)\
	template<class D, bool SinglePass> void Name(Bench::Context& ctx = Bench::Context::instance(), const D& dataset = Bench::Context::getDataset<D>())

/**
 * TODO: doc
 */
#define BENCH_USE_GROUP(Group, Name, Dataset)\
	static auto BENCH_CONCAT(_bench_##Name##_var_, __LINE__) = Bench::Context::instance().getGroup(Group).add(\
		{#Name, #Dataset},\
		{[]{ Name<Dataset, false>(); }, []{ Name<Dataset, true>(); }, Dataset ::size()}\
	);

/** Check for BENCH_USE */
BENCH_DEFINE_COMPILE_TYPE_DEF_CHECK(use_group_defined, false, "You must define a group before using BENCH_USE or specify a group with BENCH_USE_GROUP.");

/**
 * TODO: doc
 */
#define BENCH_USE(Name, Dataset)\
	BENCH_USE_COMPILE_CHECK(use_group_defined, struct _bench_group_id)\
	BENCH_USE_GROUP(_bench_group_id::name, Name, Dataset)

