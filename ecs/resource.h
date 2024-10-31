#pragma once
#include "traits.h"
#include <shared_mutex>
namespace ecs {
	struct unrestricted_resource {
		constexpr inline void acquire(priority p=priority::MEDIUM) const { }
		constexpr inline void release() const { }
	};

	struct priority_resource {
		static constexpr bool enable_priority = true;

		inline void acquire(priority p=priority::MEDIUM) { mtx.lock(p); }
		inline void acquire(priority p=priority::MEDIUM) const { mtx.lock_shared(p); }

		inline void release() { mtx.unlock(); }
		inline void release() const { mtx.unlock_shared(); }
	private:
		mutable util::priority_shared_mutex mtx;
	};

	struct restricted_resource {
		static constexpr bool enable_priority = false;

		inline void acquire(priority p=priority::MEDIUM) { mtx.lock(); }
		inline void acquire(priority p=priority::MEDIUM) const { mtx.lock_shared(); }

		inline void release() { mtx.unlock(); }
		inline void release() const { mtx.unlock_shared(); }
	private:
		mutable std::shared_mutex mtx;
	};

	struct exclusive_priority_resource {
		static constexpr bool enable_priority = true;

		inline void acquire(priority p=priority::MEDIUM) const { mtx.lock(p); }
		inline void release(priority p=priority::MEDIUM) const { mtx.unlock(); }
	private:
		mutable util::priority_mutex mtx;
	};

	struct exclusive_restricted_resource {
		static constexpr bool enable_priority = false;

		inline void acquire() const { mtx.lock(); }
		inline void release() const { mtx.unlock(); }
	private:
		mutable std::mutex mtx;
	};
}