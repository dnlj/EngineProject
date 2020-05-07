#pragma once

// STD
#include <string>
#include <vector>

// Engine
#include <Engine/CommandLine/ArgumentConverter.hpp>


namespace Engine::CommandLine::detail {
	using TypeId = size_t;
	inline TypeId nextTypeId = 0;

	template<class T>
	TypeId getTypeId() { static const TypeId id = nextTypeId++; return id; }

	class ArgumentBase {
		public:
			const char abbr;
			const std::string help;
			const TypeId typeId;

		public:
			ArgumentBase(const char abbr, std::string help, TypeId typeId)
				: abbr{abbr}, help{std::move(help)}, typeId{typeId} {
			}

			virtual ~ArgumentBase() = default;
			virtual void store(const std::string& arg) = 0;
			virtual void* get() = 0;
	};

	template<class T>
	class ArgumentGeneric : public ArgumentBase {
		protected:
			T storage;
			virtual void* get() override { return &storage; }

		public:
			ArgumentGeneric(const char abbr, T value, std::string help)
				: ArgumentBase(abbr, std::move(help), getTypeId<T>())
				, storage{std::move(value)} {
			}
	};
}

namespace Engine::CommandLine {
	template<class T>
	class Argument : public detail::ArgumentGeneric<T> {
		public:
			using ArgumentGeneric::ArgumentGeneric;
			virtual void store(const std::string& arg) override {
				ArgumentConverter<T>{}(arg, storage);
			};
	};

	template<class T>
	class Argument<std::vector<T>> : public detail::ArgumentGeneric<std::vector<T>> {
		public:
			using ArgumentGeneric::ArgumentGeneric;

			virtual void store(const std::string&  arg) override {
				// TODO: impl
			};
	};
}
