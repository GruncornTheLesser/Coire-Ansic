#pragma once
#include <variant>
#include "traits.h"
#include "handle.h"
#include "event.h"
#include "resource.h"

namespace ecs {
	template<typename T> struct init { using event_tag = ecs::tag::event::sync; };
	template<typename T> struct terminate { using event_tag = ecs::tag::event::sync; };

	template<typename T> struct manager { 
		using resource_tag = ecs::tag::resource::custom;
		using event_set = std::tuple<>;
		using component_set = std::tuple<>; // must be empty
	};

	template<typename T> struct indexer { 
		using resource_tag = ecs::tag::resource::custom;
		using event_set = std::tuple<>;
		using component_set = std::tuple<>; // must be empty
	};

	template<typename T> struct storage { 
		using resource_tag = ecs::tag::resource::custom;
		using event_set = std::tuple<init<T>, terminate<T>>;
		using component_set = std::tuple<>; // must be empty
	
	};

	template<typename T>
	struct component_traits<T, ecs::tag::component::custom>
	{ 
		using handle_type = typename T::handle_type;
		using manager_type = typename T::manager_type;
		using indexer_type = typename T::indexer_type;
		using storage_type = typename T::storage_type;
	};

	template<typename T>
	struct component_traits<T, ecs::tag::component::basictype>
	{ 
		using handle_type = ecs::entity;
		using manager_type = manager<T>;
		using indexer_type = indexer<T>;
		using storage_type = storage<T>;
	};

	template<typename T, typename U, typename ... Ts>
	struct component_traits<T, ecs::tag::component::archetype<U, Ts...>>
	{ 
		using handle_type = ecs::entity;
		using manager_type = manager<U>;
		using indexer_type = manager<U>;
		using storage_type = storage<T>;
	};

	template<typename T, typename U, typename ... Ts>
	struct component_traits<T, ecs::tag::component::uniontype<U, Ts...>>
	{
		using handle_type = ecs::entity;
		using manager_type = manager<U>;
		using indexer_type = indexer<U>;
		using storage_type = storage<std::variant<U, Ts...>>;
	};
}