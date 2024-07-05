#pragma once
#include <mutex>
#include <condition_variable>

namespace util 
{
	enum priority {
		LOW = 0,
		MEDIUM = 2,
		HIGH = 4
	}; 
	// HIGH before MEDIUM and LOW and SHARED before EXCLUSIVE 
	struct priority_mutex 
	{
	private:
		enum state : uint32_t { NONE=0, EXCLUSIVE=static_cast<uint32_t>(-1), SHARED=1 };

	public:
		void exclusive_lock(priority p = priority::MEDIUM)
		{
			std::unique_lock lk(mtx);
			if (current != state::NONE) 
			{
				++queue_count[p+1]; // enqueue
				exclusive_queue[p/2].wait(lk);
				--queue_count[p+1]; // dequeue
			}
			current = state::EXCLUSIVE;
		}
		void exclusive_unlock() 
		{
			std::lock_guard lk(mtx);
			current = 0;
			notify_next();
			
		}

		void shared_lock(priority p = priority::MEDIUM)
		{
			std::unique_lock lk(mtx);
			if (current != state::NONE) 
			{
				++queue_count[p];
				shared_queue.wait(lk);
				--queue_count[p];
			}
			++current;
		}
		void shared_unlock()
		{ 
			std::lock_guard lk(mtx);
			--current;
			notify_next();
		}

	private:
		void notify_next() 
		{
			int i;
			for (i=5; i >= 0 && queue_count[i] == 0; --i);
			if (queue_count[i] == 0) return; // queues empty nothing to notify
			
			if (i % 2) // exclusive
				exclusive_queue[i / 2].notify_one();
			else // shared
				shared_queue.notify_all();
		}

		std::mutex mtx;
		uint32_t current { state::NONE };
		uint32_t queue_count[6]{ 0,0,0,0,0,0 };
		std::condition_variable exclusive_queue[3];
		std::condition_variable shared_queue;
	};


}