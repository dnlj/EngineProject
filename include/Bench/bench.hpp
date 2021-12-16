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
			// TODO: rm - moved to id std::string dataset;
			Func func;
	};
	
	class Group {
		public: // TODO: private
			Engine::FlatHashMap<BenchmarkId, Benchmark> benchmarks;

		public:
			int add(std::string name, std::string dataset, Benchmark::Func func) {
				add2({name, dataset}, func);
				return 0;
			}

		private:
			void add2(BenchmarkId id, Benchmark::Func func) {
				const auto found = benchmarks.find(id);
				if (found != benchmarks.end()) {
					ENGINE_WARN("Benchmark \"", id, "\" already exists.");
					id.name += "~DUPLICATE~";
					return add2(std::move(id), std::move(func));
				} else {
					benchmarks.emplace(std::piecewise_construct,
						std::forward_as_tuple(std::move(id)),
						std::forward_as_tuple(std::move(func))
					);
				}
			}

	};


	class Context {
		public: // TODO: private
			Engine::FlatHashMap<std::string, Group> groups;
			TimePoint sampleStart;
			TimePoint sampleStop;
			std::vector<Duration> samples;

		public:
			Context() {};

			ENGINE_INLINE auto& getGroup(const std::string& name) { return groups[name]; }

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
	};
}



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
	template<class D> void Name(Bench::Context& ctx = Bench::Context::instance(), const D& dataset = Bench::Context::getDataset<D>())

/**
 * TODO: doc
 */
#define BENCH_USE_GROUP(Group, Name, Dataset)\
	static auto BENCH_CONCAT(_bench_##Name##_var_, __LINE__) = Bench::Context::instance().getGroup(Group).add(#Name, #Dataset, [](){ Name<Dataset>(); });

/** Check for BENCH_USE */
BENCH_DEFINE_COMPILE_TYPE_DEF_CHECK(use_group_defined, false, "You must define a group before using BENCH_USE or specify a group with BENCH_USE_GROUP.");

/**
 * TODO: doc
 */
#define BENCH_USE(Name, Dataset)\
	BENCH_USE_COMPILE_CHECK(use_group_defined, struct _bench_group_id)\
	BENCH_USE_GROUP(_bench_group_id::name, Name, Dataset)

