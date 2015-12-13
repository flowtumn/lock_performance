#ifndef ILOCK_H_INCLUDE
#define ILOCK_H_INCLUDE

namespace flowTumn {
	struct ILock {
		virtual bool lock() = 0;
		virtual void unlock() = 0;
	};
}

#endif
