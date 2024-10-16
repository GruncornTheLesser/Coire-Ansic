#pragma once
#include <stdint.h>
#include <vector>
#include <algorithm>
#include <functional>
#include <tuple>
#include <mutex>

// add option to resource to make resource atomic...

namespace ecs
{
	// traits:
	// bool strict_event_order = false;
	// bool overflow_protection = false;
	// bool asynchronous = false; 
	
	template<typename event_T>
	struct listener
	{
		uint32_t handle;
		std::function<void(const event_T&)> callback;
		bool once;
	};

	template<typename ... event_Ts>
	class dispatcher : private dispatcher<event_Ts>...
	{
	public:
		dispatcher() = default;
		dispatcher(const dispatcher&) = delete;
		dispatcher& operator=(const dispatcher&) = delete;
		dispatcher(dispatcher&&) = default;
		dispatcher& operator=(dispatcher&&) = default;

		template<typename event_T> requires (std::is_same_v<event_T, event_Ts> || ...)
		dispatcher<event_T>& on() { return *static_cast<dispatcher<event_T>*>(this); }

		template<typename event_T> requires (std::is_same_v<event_T, event_Ts> || ...)
		uint32_t listen(dispatcher<event_T>::handler handler, bool once=false) 
		{ 
			return on<event_T>().listen(handler, once);
		}

		template<typename event_T> requires (std::is_same_v<event_T, event_Ts> || ...)
		void detach(uint32_t handle) 
		{ 
			on<event_T>().detach(handle); 
		}
		
		template<typename event_T> requires (std::is_same_v<event_T, event_Ts> || ...)
		void invoke(event_T&& event)
		{ 
			on<event_T>().invoke(event);
		}
	};

	template<typename event_T>
	class dispatcher<event_T>
	{
		using uint32_t = uint32_t;
		using queue_type = std::vector<listener<event_T>>;
		using handler_type = std::function<void(const event_T&)>;
	public:
		dispatcher() = default;
		dispatcher(const dispatcher&) = delete;
		dispatcher& operator=(const dispatcher&) = delete;
		dispatcher(dispatcher&&) = default;
		dispatcher& operator=(dispatcher&&) = default;

		uint32_t operator+=(handler_type handler) { return listen(handler, false); }
		uint32_t operator^=(handler_type handler) { return listen(handler, true); }

		uint32_t listen(handler_type handler, bool once=false) 
		{
			std::lock_guard<std::mutex> lock(mutex);
			queue.emplace_back(current, handler, once);
			return current++; 
		}

		void detach(uint32_t handle)
		{
			std::lock_guard<std::mutex> lock(mutex);
			queue.erase(std::remove_if(queue.begin(), queue.end(), [=](auto& listener) { 
				return listener.handle == handle;
			}), queue.end());
		}

		void detach_all()
		{ 
			std::lock_guard<std::mutex> lock(mutex);
			queue.clear();
		}

		void invoke(event_T&& event)
		{
			std::lock_guard<std::mutex> lock(mutex);
			event_T event_data = std::move(event);
			queue.erase(std::remove_if(queue.begin(), queue.end(), [&](listener<event_T>& listener) {
				listener.callback(event_data);
				return listener.once; 
			}), queue.end());
		}

		template<typename T> dispatcher<event_T>& on() requires (std::is_same_v<T, event_T>) { return *this; }
		template<typename T> const dispatcher<event_T>& on() const requires (std::is_same_v<T, event_T>) { return *this; }
	private:
		uint32_t current = 0;
		queue_type queue;
		std::mutex mutex; 
	};
}