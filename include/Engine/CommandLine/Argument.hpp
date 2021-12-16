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

		protected:
			bool set;

		public:
			ArgumentBase(const char abbr, std::string help, TypeId typeId, bool set = false)
				: abbr{abbr}, help{std::move(help)}, typeId{typeId}, set{set} {
			}

			virtual ~ArgumentBase() = default;
			virtual bool store(const std::string& arg) = 0;
			virtual void* get() = 0;
			bool hasBeenSet() const { return set; }
			void setHasBeenSet(bool s) { set = s; }
	};

	template<class T>
	class ArgumentGeneric : public ArgumentBase {
		protected:
			T storage;
			virtual void* get() override { return &storage; }

		public:
			ArgumentGeneric(const char abbr, T value, std::string help, bool set = false)
				: ArgumentBase(abbr, std::move(help), getTypeId<T>(), set)
				, storage{std::move(value)} {
			}
	};
}

namespace Engine::CommandLine {
	template<class T>
	class Argument : public detail::ArgumentGeneric<T> {
		public:
			using detail::ArgumentGeneric<T>::ArgumentGeneric;

			virtual bool store(const std::string& arg) override {
				if (ArgumentConverter<T>{}(arg, this->storage)) {
					this->set = true;
					return true;
				}
				return false;
			};
	};

	template<class T>
	class Argument<std::vector<T>> : public detail::ArgumentGeneric<std::vector<T>> {
		public:
			using detail::ArgumentGeneric<std::vector<T>>::ArgumentGeneric;

			virtual bool store(const std::string& arg) override {
				// TODO: impl
				static_assert(sizeof(T) != sizeof(T), "TODO: Not implemented.");

				// How do we want this to work?
				// `program.exe --arg=abcd --arg=1234`
				// Or
				// `program.exe --arg=abcd,1234`
				// Or both?

				this->set = true;
				return false;
			};
	};
}
