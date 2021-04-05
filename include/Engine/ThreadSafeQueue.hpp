#pragma once

// STD
#include <mutex>
#include <queue>


namespace Engine {
	template<class T>
	class ThreadSafeQueue {
		private:
			std::condition_variable condv;
			std::mutex mutex;

			// TODO: There are a number of improvements that could be made if we did our own impl.
			// CONT: Such as simultaneous push/pop. Probably use unrolled linked list since objects are usually going to be small.
			std::queue<T> storage;

		public:
			ThreadSafeQueue() = default;
			ThreadSafeQueue(const ThreadSafeQueue&) = delete;

			void notify() {
				condv.notify_all();
			}

			decltype(auto) lock() {
				return std::unique_lock{mutex};
			}

			template<class T>
			void push(T&& obj) {
				std::scoped_lock lock{mutex};
				storage.push(std::forward<T>(obj));
				condv.notify_one();
			}

			template<class... Ts>
			void emplace(Ts&&... args) {
				std::scoped_lock lock{mutex};
				storage.emplace(std::forward<Ts>(args)...);
				condv.notify_one();
			}

			
			template<class... Ts>
			void unsafeEmplace(Ts&&... args) {
				storage.emplace(std::forward<Ts>(args)...);
			}

			bool pop(T& ret) {
				std::scoped_lock lock{mutex};
				if (storage.empty()) { return false; }
				ret = storage.front();
				storage.pop();
				return true;
			}

			bool popOrWait(T& ret) {
				std::unique_lock lock{mutex};

				if (storage.empty()) {
					condv.wait(lock);
					if (storage.empty()) { return false; }
				}

				ret = storage.front();
				storage.pop();
				return true;
			}
	};
}
