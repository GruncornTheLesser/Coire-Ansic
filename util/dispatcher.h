#pragma once
#include <stdint.h>
#include <vector>
#include <algorithm>
#include <functional>
#include <tuple>
#include <mutex>

// add option to resource to make resource atomic...

namespace util
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
	class dispatcher 
	{
	public:
		dispatcher() = default;
		dispatcher(const dispatcher&) = delete;
		dispatcher& operator=(const dispatcher&) = delete;
		dispatcher(dispatcher&&) = default;
		dispatcher& operator=(dispatcher&&) = default;

		template<typename event_T> 
		dispatcher<event_T>& on()
		{ 
			return std::get<event_T>(dispatchers); 
		}

		template<typename event_T, typename handler_T>
		uint32_t listen(const std::function<void(const event_T&)>& handler, bool once) 
		{ 
			return on<event_T>().listen(handler, once);
		}

		template<typename event_T, typename handler_T>
		void detach(uint32_t handle) 
		{ 
			on<event_T>().detach(handle); 
		}
		
		template<typename event_T>
		void invoke(event_T&& event) 
		{ 
			on<event_T>().dispatch(event); 
		}

	private:
		std::tuple<dispatcher<event_Ts>...> dispatchers;
	};

	template<typename event_T>
	class dispatcher<event_T>
	{
		using uint32_t = uint32_t;
		using queue_type = std::vector<listener<event_T>>;

	public:
		dispatcher() = default;
		dispatcher(const dispatcher&) = delete;
		dispatcher& operator=(const dispatcher&) = delete;
		dispatcher(dispatcher&&) = default;
		dispatcher& operator=(dispatcher&&) = default;

		uint32_t listen(std::function<void(const event_T&)> handler, bool once=false) 
		{
			std::lock_guard<std::mutex> lock(mutex);
			queue.emplace_back(current, handler, once);
			return current++; 
		}

		void detach(uint32_t handle)
		{
			std::lock_guard<std::mutex> lock(mutex);
			queue.erase(std::remove_if(queue.begin(), queue.end(), [=](auto& listener) { 
				return listener.first == handle;
			}), queue.end());
		}

		void detach_all()
		{ 
			std::lock_guard<std::mutex> lock(mutex);
			queue.clear();
		}

		void dispatch(event_T&& event)
		{
			std::lock_guard<std::mutex> lock(mutex);
			event_T event_data = event;
			queue.erase(std::remove_if(queue.begin(), queue.end(), [&](listener<event_T>& listener) {
				listener.callback(event_data);
				return listener.once; 
			}), queue.end());
		}
	private:
		uint32_t current;
		queue_type queue;
		std::mutex mutex; 
	};
}