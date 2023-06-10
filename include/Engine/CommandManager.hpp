#pragma once


namespace Engine {
	enum class CommandId : uint32;
	ENGINE_BUILD_ALL_OPS(CommandId);
	ENGINE_BUILD_DECAY_ENUM(CommandId);

	using CommandFunc = std::function<void(CommandManager&)>;

	class CommandMeta {
		public:
			std::string_view name;
			CommandFunc func;
			// TODO: help text
			// TODO: params
	};

	class CommandManager {
		private:
			std::vector<CommandMeta> commands;
			std::vector<std::string> args;

		public:
			// getParam<type>(uint32 n);

			// TODO: args
			void exec(std::string_view str);

			void exec(CommandId cid) {
				ENGINE_DEBUG_ASSERT(+cid < commands.size(), "Attempting to execute invalid command.");
				auto& cmd = commands[+cid];
				cmd.func(*this);
			}
			
			CommandId registerCommand(std::string_view name, CommandFunc func) {
				ENGINE_DEBUG_ASSERT(func, "Attempting to add a command without a function.");
				const auto id = static_cast<CommandId>(commands.size());
				commands.emplace_back(name, func);
				return id;
			}

		private:
	};
}
