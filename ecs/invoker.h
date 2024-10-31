#pragma once
#include <functional>
#include <tuple>
#include <stdint.h>
#include "traits.h"

// events
namespace ecs {
	template<traits::resource_class T> struct acquire {
		using ecs_tag = ecs::tag::event_sync;
		T& value;
	};

	template<traits::resource_class T> struct release {
		using ecs_tag = ecs::tag::event_sync; 
		T& value;
	};

	template<traits::handle_class T> struct create {
		using ecs_tag = ecs::tag::event;
		T handle; 
	};

	template<traits::handle_class T> struct destroy {
		using ecs_tag = ecs::tag::event; 
		T handle;
	};
	
	template<traits::component_class T> struct init {
		using ecs_tag = ecs::tag::event; 
		typename component_traits<T>::handle_type handle;
		typename component_traits<T>::storage_type::value_type& value;
	};
	
	template<traits::component_class T> struct terminate {
		using ecs_tag = ecs::tag::event; 
		typename component_traits<T>::handle_type handle;
		typename component_traits<T>::storage_type::value_type& value;
	};

}

namespace ecs {
	template<typename T>
	struct invoker : restricted_resource {
	private:
		using handle_type = event_traits<T>::handle_type;
		using listener_type = typename event_traits<T>::listener_type;
		static constexpr bool strict_order = event_traits<T>::strict_order;

		struct element_type { uint32_t handle; listener_type listener = nullptr; };
		// static constexpr execution_type = typename event_traits<T>::execution_type;
	public:
		constexpr inline void operator()(const T& event) { invoke(event); }
		constexpr inline handle_type operator^=(listener_type listener) { return listen(std::forward<listener_type>(listener), true); }
		constexpr inline handle_type operator+=(listener_type listener) { return listen(std::forward<listener_type>(listener), false); }
		constexpr inline bool operator-=(handle_type handle) { return detach(handle); }
	
		constexpr void invoke(T event)
		{
			size_t i = 0;
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
			uint32_t handle;
			
			if (fire_once == data.size()) // storage is full
			{
				// allocate and generate new handle
				handle = data.size();
				data.resize(data.size() + 1);
			}
			else
			{
				handle = data[fire_once].handle; // reuse handle
			}

			if (once) {
				// add listener to back of fire once partition
				data[fire_once] = { handle, std::forward<listener_type>(listener) };
				
				// increment offsets
				++fire_once;
				
				return handle;
			}
			else
			{
				// make space for new listener
				if constexpr (strict_order)
					std::move_backward(data.begin() + recurring, data.begin() + fire_once, data.begin() + fire_once + 1);
				else
					data[fire_once] = std::move(data[recurring]);
				
				// add listener to back of recurring section
				data[recurring] = { handle, std::forward<listener_type>(listener) };
				
				// increment offsets
				++fire_once;
				++recurring;

				return handle;
			}
		}
	
		constexpr bool detach(handle_type handle) 
		{
			// find index
			int i = 0;
			while (i < fire_once && data[i].handle != handle);
			
			if (i == fire_once) return false; // if i == end of active partition
			
			if constexpr (strict_order)
			{
				// move active after i back 1
				std::move(data.begin() + i + 1, data.begin() + fire_once, data.begin() + i);
				if (i < recurring) --recurring;
				--fire_once;
			}
			else
			{
				if (i < recurring) {
					data[i] = std::move(data[--recurring]);
					i = recurring;
				}
				data[i] = std::move(data[--fire_once]);
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
