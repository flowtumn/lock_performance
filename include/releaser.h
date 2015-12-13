#ifndef RELEASER_H_INCLUDE
#define RELEASER_H_INCLUDE

#include <functional>

namespace flowTumn {
	struct releaser {
		using release_func = std::function <void()>;
		releaser(release_func f) : f_(std::move(f)) {}
		~releaser() {
			if (f_) {
				f_();
			}
		}

		operator bool() const {
			return (f_) ? true : false;
		}
		release_func f_;
	};
}

#endif
