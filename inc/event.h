#pragma once
#include <functional>
#include <tuple>
#include <stdint.h>
#include "traits.h"

// traits:
// bool asynchronous = false; 

namespace ecs
{
	// enum class execution { IMMEDIATE, ASYNC, SYNC, LAZY };

	template<typename T> struct event_traits<T, ecs::tag::event::custom>
	{
		using listener_type = typename T::listener_type;
		using invoker_type = typename T::invoker_type;
		static constexpr bool is_ordered = T::is_ordered;
		//static constexpr execution execution_type = typename T::execution_type;
	};

	template<typename T> struct event_traits<T, ecs::tag::event::sync>
	{
		using listener_type = std::function<void(const T&)>;
		using invoker_type = invoker<T>;
		static constexpr bool is_ordered = true;
		//static constexpr execution execution_type = execution::SYNC;
	};

	template<typename T> struct event_traits<T, ecs::tag::event::lazy>
	{
		using listener_type = std::function<void(const T&)>;
		using invoker_type = invoker<T>;
		static constexpr bool is_ordered = false;
		// co-routines???
		//static constexpr execution execution_type = execution::LAZY;
	};

	template<typename T> struct event_traits<T, ecs::tag::event::async>
	{
		using listener_type = std::function<void(const T&)>;
		using invoker_type = invoker<T>;
		static constexpr bool is_ordered = false;
		//static constexpr execution execution_type = execution::ASYNC;
	};

}

namespace ecs {
	template<typename T>
	struct invoker {
	private:

		using handle_type = uint32_t;
		using listener_type = typename event_traits<T>::listener_type;
		struct element_type { handle_type handle; listener_type listener = nullptr; };
		// static constexpr execution_type = typename event_traits<T>::execution_type;
	public:
		using resource_tag = ecs::tag::resource::unrestricted;
		
		constexpr inline void operator()(const T& event) { invoke(event); }
		constexpr inline handle_type operator^=(listener_type listener) { return listen(std::forward<listener_type>(listener), true); }
		constexpr inline handle_type operator+=(listener_type listener) { return listen(std::forward<listener_type>(listener), false); }
		constexpr inline bool operator-=(handle_type handle) { return detach(handle); }
	
		constexpr void invoke(const T& event)
		{
			int i = 0;
			while (i < recurring)
			{
				data[i].listener(event); // recurring listeners
				++i;
			}
			while (i < fire_once)
			{
				data[i].listener(event); // fire_once listeners
				data[i].listener = nullptr;
				++i;
			}
			fire_once = recurring; // clear fire once sections
		}

		constexpr handle_type listen(listener_type listener, bool once)
		{
			handle_type handle;
			
			if (fire_once == data.size())
			{
				handle = data.size();
				data.resize(data.size() + 1);
			}
			else
			{
				handle = data[fire_once].handle;
			}

			if (once) {
				data[fire_once] = { handle, std::forward<listener_type>(listener) };
				++fire_once;
				return handle;
			}
			else
			{
				if constexpr (event_traits<T>::is_ordered)
					std::move_backward(data.begin() + recurring, data.begin() + fire_once, data.begin() + fire_once + 1);
				else
					data[fire_once] = std::move(data[recurring]);
				
				data[recurring] = { handle, std::forward<listener_type>(listener) };

				++fire_once;
				++recurring;

				return handle;
			}
		}
	
		constexpr bool detach(handle_type handle) 
		{
			int i = 0;
			while (i < fire_once && data[i].handle != handle);
			if (i == fire_once) return false;
			
			if constexpr (event_traits<T>::is_ordered)
			{
				std::move(data.begin() + i + 1, data.begin() + fire_once, data.begin() + i);
				if (i < recurring) --recurring;
				--fire_once;
			}
			else
			{
				if (i < recurring) {
					data[i] = data[--recurring];
					i = recurring;
				}
				data[i] = data[--fire_once];
			}
			data[fire_once].handle = handle;
			return true;
		}
	
		constexpr void clear() {
			fire_once = 0;
			recurring = 0; 
			data.clear();
		}

	private:
		size_t recurring = 0, fire_once = 0;
		std::vector<element_type> data;
	};
}
