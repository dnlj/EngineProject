#pragma once


namespace Engine {
	enum class CommandId : uint32 {
		Invalid = 0,
	};
	ENGINE_BUILD_DECAY_ENUM(CommandId);

	using CommandFunc = std::function<void (CommandManager&)>;

	class CommandMeta {
		public:
			std::string_view name;
			CommandFunc func;
			// TODO: help text
			// TODO: params
	};

	class CommandManager {
		public:
			template<class T>
			using CVarGet = std::function<const T& (CommandManager& manager)>;

			template<class T>
			using CVarSet = std::function<const T& (CommandManager& manager, std::string_view value)>;

		private:
			FlatHashMap<std::string, CommandId> nameToCommand;
			std::vector<CommandMeta> commands;
			std::vector<std::string> arguments;

		public:
			CommandManager();

			const auto& args() const { return arguments; }
			void exec(CommandId id);

			void exec(std::string_view str);

			CommandId registerCommand(std::string_view name, CommandFunc func) {
				ENGINE_DEBUG_ASSERT(func, "Attempting to add a command without a function.");
				ENGINE_DEBUG_ASSERT(!name.empty(), "Attempting to add a command with an empty name.");
				return registerCommandUnchecked(name, func);
			}

			CommandId lookup(std::string_view name) const noexcept {
				const auto found = nameToCommand.find(name);
				if (found == nameToCommand.end()) { return CommandId::Invalid; }
				return found->second;
			}

		private:
			void parse(std::string_view str);
			
			CommandId registerCommandUnchecked(std::string_view name, CommandFunc func) {
				const auto id = static_cast<CommandId>(commands.size());
				commands.emplace_back(name, func);

				if (nameToCommand.contains(name)) {
					ENGINE_WARN("Attempting to add duplicate command: ", name);
					return CommandId::Invalid;
				}

				nameToCommand.emplace(name, id);
				return id;
			}
	};
}
