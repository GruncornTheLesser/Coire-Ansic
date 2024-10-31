#pragma once
#include <utility>
#include <vector>
#include <variant>
#include <stdint.h>
#include <functional>
#include <bit>
#include "fwd.h"
#include "util/priority_mutex.h"

namespace ecs::traits {
	template<typename T, typename> struct get_tag : std::type_identity<void> { };
	template<typename T> struct get_tag<T, std::void_t<typename T::ecs_tag>> : std::type_identity<typename T::ecs_tag> { };
}

// resource traits
namespace ecs {
	using priority = util::priority;
	
	template<typename T>
	struct resource_traits<T, std::void_t<
		decltype(std::declval<T>().release()),
		decltype(std::declval<const T>().release()),
		decltype(std::declval<T>().acquire(std::declval<priority>())), 
		decltype(std::declval<const T>().acquire(std::declval<priority>()))>>
	{ };
	// & destructor services eg using destructor_services = std::tuple<>
}

// event traits
namespace ecs {
	template<typename T>
	struct event_traits<T, std::void_t<typename T::handle_type, typename T::invoker_type, typename T::listener_type, decltype(T::enable_async), decltype(T::strict_order)>> { 
		using handle_type = typename T::handle_type;
		using invoker_type = typename T::invoker_type;
		using listener_type = typename T::listener_type;
		static constexpr bool enable_async = T::enable_async;
		static constexpr bool strict_order = T::strict_order;
	};

	template<typename T>
	struct event_traits<T, ecs::tag::event_sync> { 
		using handle_type = uint32_t;
		using invoker_type = invoker<T>;
		using listener_type = std::function<void(T&)>;
		static constexpr bool enable_async = false;
		static constexpr bool strict_order = true;
	};

	template<typename T>
	struct event_traits<T, ecs::tag::event_async> { 
		using handle_type = uint32_t;
		using invoker_type = invoker<T>;
		using listener_type = std::function<void(T&)>;
		static constexpr bool enable_async = true;
		static constexpr bool strict_order = false;
	};
}

// handle traits
namespace ecs {
	template<typename T, std::unsigned_integral value_T, size_t version_N>
	struct handle_traits<T, ecs::tag::handle_versioned<value_T, version_N>> {
	private:
		static constexpr size_t version_width = version_N;
		static constexpr value_T vers_mask = static_cast<value_T>(0xffffffffffffffffull << ((sizeof(T) * 8) - version_width));
		static constexpr value_T data_mask = ~vers_mask;
		static constexpr value_T increment = std::bit_floor(data_mask) << 1; // increment == 0 if version_width == 0
	public:
		static constexpr value_T tomb = data_mask;
		static constexpr value_T null = vers_mask;

		using value_type = value_T;
		using generator_type = generator<ecs::entity>;
		static constexpr bool versioned = (version_width == 0);
		static constexpr bool enable_events = true;

		static constexpr inline value_type get_data(value_type val) { 
			return val & data_mask; 
		}
		
		static constexpr inline bool vers_neq(value_type lhs, value_type rhs) { 
			return ((lhs ^ rhs) & vers_mask);
		}
		static constexpr inline value_type create(value_type data=data_mask, value_type prev=vers_mask) { 
			return data | (prev & vers_mask); 
		}
		static constexpr inline value_type resurrect(value_type val) { 
			return val += increment;
		}
		// ? destroy, set version to reserved null version
	};
}

// component traits
namespace ecs {
	template<typename T>
	struct component_traits<T, std::void_t<
		typename T::handle_type, 
		typename T::manager_type,
		typename T::indexer_type,
		typename T::storage_type, 
		decltype(T::enable_events)>>
	{ 
		using handle_type = typename T::handle_type;
		using manager_type = typename T::manager_type;
		using indexer_type = typename T::indexer_type;
		using storage_type = typename T::storage_type;
		static constexpr bool enable_events = T::enable_events;
	};

	template<typename T>
	struct component_traits<T, ecs::tag::component_basictype>
	{
		using handle_type = ecs::entity;
		using manager_type = manager<T>;
		using indexer_type = indexer<T>;
		using storage_type = storage<T>;
		static constexpr bool enable_events = true;
	};

	template<typename T, typename U, typename ... Ts>
	struct component_traits<T, ecs::tag::component_archetype<U, Ts...>>
	{
		using handle_type = ecs::entity;
		using manager_type = manager<U>;
		using indexer_type = indexer<U>;
		using storage_type = storage<T>;
		static constexpr bool enable_events = true;
	};

	template<typename T, typename U, typename ... Ts>
	struct component_traits<T, ecs::tag::component_uniontype<U, Ts...>>
	{
		using handle_type = ecs::entity;
		using manager_type = manager<U>;
		using indexer_type = indexer<U>;
		using storage_type = storage<std::variant<U, Ts...>>;
		static constexpr bool enable_events = true;
	};
}

// traits
namespace ecs::traits {
	template<typename T, typename> struct is_resource : std::false_type { };
	template<typename T> struct is_resource<T, std::void_t<decltype(resource_traits<std::remove_const_t<T>>{})>> : std::true_type { };

	template<typename T, typename> struct is_event : std::false_type { };
	template<typename T> struct is_event<T, std::void_t<decltype(event_traits<std::remove_const_t<T>>{})>> : std::true_type { };
	
	template<typename T, typename> struct is_handle : std::false_type { };
	template<typename T> struct is_handle<T, std::void_t<decltype(handle_traits<std::remove_const_t<T>>{})>> : std::true_type { };

	template<typename T, typename> struct is_component : std::false_type { };
	template<typename T> struct is_component<T, std::void_t<decltype(component_traits<std::remove_const_t<T>>{})>> : std::true_type { };
	
	template<typename T, typename> struct is_contract : std::false_type { };
	template<typename T> struct is_contract<T, std::void_t<decltype(get_dependency_set<std::remove_const_t<T>>{})>> : std::true_type { };

	template<typename T> struct get_dependency_set<T, std::void_t<typename T::dependency_set>>
	 : std::type_identity<typename T::dependency_set> { };
	
	template<typename T> struct get_dependency_set<T, std::enable_if_t<is_resource_v<T>>>
	 : std::type_identity<std::tuple<T>> { };
	
	template<typename T> struct get_dependency_set<T, std::enable_if_t<is_event_v<T>>>
	 : std::type_identity<std::tuple<typename event_traits<T>::invoker_type>> { };
	
	template<typename T> struct get_dependency_set<T, std::enable_if_t<is_handle_v<T>>>
	 : std::type_identity<std::tuple<typename handle_traits<T>::generator_type,
		std::conditional_t<handle_traits<T>::enable_events, get_invoker_t<create<T>>, void>,
	 	std::conditional_t<handle_traits<T>::enable_events, get_invoker_t<destroy<T>>, void>>> { };
	
	template<typename T> struct get_dependency_set<T, std::enable_if_t<is_component_v<T>>>
	 : std::type_identity<std::tuple<
		typename handle_traits<typename component_traits<T>::handle_type>::generator_type,
	 	std::conditional_t<handle_traits<get_handle_t<T>>::enable_events, get_invoker_t<create<get_handle_t<T>>>, void>,
		std::conditional_t<handle_traits<get_handle_t<T>>::enable_events, get_invoker_t<destroy<get_handle_t<T>>>, void>,
		typename component_traits<T>::manager_type,
		typename component_traits<T>::indexer_type,
		typename component_traits<T>::storage_type,
		std::conditional_t<component_traits<T>::enable_events, get_invoker_t<init<T>>, void>,
		std::conditional_t<component_traits<T>::enable_events, get_invoker_t<terminate<T>>, void>>> { };
}