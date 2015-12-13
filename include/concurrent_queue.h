#ifndef CONCURRENT_QUEUE_H_INCLUDE
#define CONCURRENT_QUEUE_H_INCLUDE

#include <queue>
#include <thread>

#include "ILock.h"
#include "releaser.h"

namespace flowTumn {
	template <typename T, typename L>
	class concurrent_queue {
		static_assert(std::is_base_of <flowTumn::ILock, L>::value, "L must be a descendant of ILock.");

	public:
		enum struct Result {
			Success,
			Failed,
			Empty,
			Timeout,
		};

		// lockをRAIIに。
		struct locker {
			releaser lock() {
				if (l_.lock()) {
					return{ [this]{ l_.unlock(); } };
				}
				else {
					return{ nullptr };
				}
			}
			L l_;
		} lock_controller;

		concurrent_queue() = default;

		//queueにデータを詰め込み。
		Result push(const T& v) {
			for (;;) {
				if (auto l = lock_controller.lock()) {
					queue_.push(v);
					return Result::Success;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			return Result::Failed;
		}

		//fから取り出したデータをqueueに詰め込み。
		Result push(std::function <T()> f) {
			for (;;) {
				if (auto l = lock_controller.lock()) {
					queue_.push(f());
					return Result::Success;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			return Result::Failed;
		}

		//queueからデータを取得。
		std::pair <Result, T> pop() {
			for (;;) {
				if (auto l = lock_controller.lock()) {
					if (0 < queue_.size()) {
						auto r = queue_.front();
						queue_.pop();
						return{ Result::Success, r };
					} else {
						return{ Result::Empty, {} };
					}
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			return{ Result::Failed, {} };
		}

	private:
		std::queue <T> queue_;
	};
}

#endif
