#include <algorithm>
#include <cassert>
#include <atomic>
#include <mutex>
#include <iterator>
#include <iostream>
#include "concurrent_queue.h"

using namespace flowTumn;

namespace flowTumn {
	// 何もしない。これを使用すると検証時点で失敗する。
	struct lockDummy : public flowTumn::ILock {
		bool lock() override { return true; }
		void unlock() override {}
	};

	struct lockTry : public flowTumn::ILock {
		// 今lockを取得できるか否か。
		bool lock() override {
			return mtx_.try_lock();
		}

		void unlock() override{
			mtx_.unlock();
		}

		std::mutex mtx_;
	};

	struct lockWeak : public flowTumn::ILock {
		//単純にfalseからtrueに変えられたらlock成功
		bool lock() override {
			bool expected = false;
			bool desired = true;
			return flg_.compare_exchange_weak(expected, desired);
		}

		void unlock() override {
			flg_ = false;
		}

		std::atomic <bool> flg_{ false };
	};

	// mutexをlockするまで待機する単純な実装。
	struct lockNormal : public flowTumn::ILock {
		bool lock() override {
			mutex_.lock();
			return true;
		}

		void unlock() override {
			mutex_.unlock();
		}

		std::mutex mutex_;
	};
};

namespace flowTumn {
	using counter_type = std::atomic <int64_t>;

	template <typename T, typename Lock, typename F>
	uint64_t queue_tester(
		F gen,
		uint32_t createGenCount,
		std::function <void(T)> notify,
		uint32_t createNotifyCount,
		std::chrono::milliseconds timeout) {
		using queue_type = flowTumn::concurrent_queue <int64_t, Lock>;

		queue_type queue{};
		std::atomic <uint64_t> counter{0};
		std::atomic <bool> g{ false };
		std::vector <std::thread> thr;

		auto push = [&gen, &queue, &counter, &g]() {
			while (!g) {}
			while (g) {
				queue.push(gen());
				++counter;
			}
		};

		auto pop = [&notify, &g, &counter, &queue]() {
			while (!g) {}
			while (g) {
				auto r = queue.pop();
				if (r.first == queue_type::Result::Success) {
					notify(r.second);
				}
				++counter;
			}
		};

		std::generate_n(std::back_inserter(thr), createGenCount, [push] { return std::thread{ push }; });
		std::generate_n(std::back_inserter(thr), createNotifyCount, [pop] { return std::thread{ pop }; });

		g = !g;
		std::this_thread::sleep_for(timeout);
		g = !g;

		for (auto& elem : thr) {
			elem.join();
		}

		return counter.load();
	}

	//queueがthread safeなのかを検証します。
	template <typename L>
	bool checkQueue() {
		std::atomic <int64_t> v1{0};
		std::atomic <int64_t> v2{0};
		std::atomic <bool> result{true};

		//cpuのコア数分詰める処理を追加し、popしたデータを検証するのは一つが担う。
		queue_tester <int64_t, L>(
			[&v1] {
#if 1
				//検証するデータを生成するfunctionを返す。(この処理はlockされた後に呼ばれる。)
				return std::function <int64_t()> {
					[&v1] {
						++v1;
						return v1.load();
					}
				};
#else
				// 詰めるスレッドが複数なら、詰められる値はバラバラになってしまう。
				++v1;
				return v1.load();
#endif
			},
			std::thread::hardware_concurrency(),
			[&v2, &result](int64_t n) {
				//queueから取得したデータの通知を受け検証する。
				++v2;
				if (v2.load() != n) {
					result = false;
					assert(false && "queue is bug.");
				}
			},
			1,
			std::chrono::seconds(5)
		);

		return result.load();
	}

	//スコアを計測。
	template <typename Lock>
	uint64_t score() {
		const auto core = std::max <uint32_t> (std::thread::hardware_concurrency() >> 1, 1);
		return queue_tester <uint64_t, Lock>(
			[] {
				//好きな値で詰める。重要なのは呼ばれた回数。
				return 1234;
			},
			core,
			[](uint64_t) {
				//popしたデータを受けても何もしない。重要なのは呼ばれた回数。
			},
			core,
			std::chrono::seconds(5)
		);
	}

	template <typename F, typename FN>
	auto avg(F f, uint32_t count, FN notify) -> decltype(f()) {
		auto r = decltype(f()){};
		for (auto i = UINT32_C(0); i < count; ++i) {
			auto rr = f();
			notify(rr);
			r += rr;
		}
		return r / count;
	}
};

//キューを検証した後にスコアを計測。
#define SCORE(lock, count) \
	std::cout << "TargetLock => " << ""#lock""  << std::endl; \
	if (checkQueue<lock>()) { \
		std::cout << "\t" << "checkQueue: Success." << std::endl; \
		auto scoreAvg = avg(score<lock>, count, [](decltype(score<lock>()) score) { std::cout << "\t\t" << "Score: " << score << std::endl; }); \
		std::cout << "\t" << "Score(Avg): " << scoreAvg << std::endl; \
	} else {  \
		std::cout << "\t" << "checkQueue: Failed." << std::endl; \
	} \
	std::cout << std::endl;

int main() {
	const auto TEST_COUNT = 3;

	//スコアを計測。lockの実装が不十分だと検証で弾かれる。
	SCORE(lockNormal, TEST_COUNT);
	SCORE(lockTry, TEST_COUNT);
	SCORE(lockWeak, TEST_COUNT);
	return 0;
}

