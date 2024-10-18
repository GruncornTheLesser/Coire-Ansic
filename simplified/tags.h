#pragma once
#include "traits.h"
#include "dispatcher.h"
#include "../tuple_util/tuple_util.h"
#include "../util/type_name.h"
#include <variant>

namespace ecs {
	template<typename T> struct component_manager : DEFAULT_MANAGER { };
	template<typename T> struct component_indexer : DEFAULT_INDEXER { };
	template<typename T> struct component_storage : DEFAULT_STORAGE { };
}

namespace ecs::event {
	template<typename T> struct init { handle<T> e; T& comp; }; // component construction
	template<typename T> struct terminate { handle<T> e; T& comp; }; // component destruction
	
	template<typename handle_T> struct create { handle_T e; }; // handle construction
	template<typename handle_T> struct destroy { handle_T e; }; // handle destruction
	// template<typename T> struct update { handle e; };
}

// defaults are wrapped to make them uniques identifiable
namespace ecs::tags
{
	template<typename T>
	struct basictype
	{
		using default_handle = DEFAULT_HANDLE;
		using default_events = DEFAULT_EVENTS;
		template<typename U> requires (std::is_same_v<U, T>) using default_manager = component_manager<T>;
		template<typename U> requires (std::is_same_v<U, T>) using default_indexer = component_indexer<T>;
		template<typename U> requires (std::is_same_v<U, T>) using default_storage = component_storage<T>;
	};

	template<typename ... Ts>
	struct archetype
	{
	private:
		using value_type = util::find_min_t<std::tuple<Ts...>, util::get_type_ID>;
	public:
		using default_handle = DEFAULT_HANDLE;
		using default_events = std::tuple<ecs::event::init<value_type>, ecs::event::terminate<value_type>>;
		template<typename U> requires (std::is_same_v<U, Ts> || ...) using default_manager = component_manager<value_type>;
		template<typename U> requires (std::is_same_v<U, Ts> || ...) using default_indexer = component_indexer<value_type>;
		template<typename U> requires (std::is_same_v<U, Ts> || ...) using default_storage = component_storage<U>;
	};

	template<typename ... Ts>
	struct uniontype
	{
	private:
		using shared_type = util::find_min_t<std::tuple<Ts...>, util::get_type_ID>; // get unique type ID for set
		using value_type = util::sort_t<std::variant<Ts...>, util::cmp::lt_<util::get_type_ID>::template type>; // sorted so consistent order
	public:
		using default_handle = DEFAULT_HANDLE;
		using default_events = std::tuple<ecs::event::init<value_type>, ecs::event::terminate<value_type>>;
		template<typename U> requires (std::is_same_v<U, Ts> || ...) using default_manager = component_manager<shared_type>;
		template<typename U> requires (std::is_same_v<U, Ts> || ...) using default_indexer = component_indexer<shared_type>;
		template<typename U> requires (std::is_same_v<U, Ts> || ...) using default_storage = component_storage<value_type>;
	};

	template<typename T>
	struct handletype
	{
		using default_handle = void;
		using default_events = std::tuple<ecs::event::create<T>, ecs::event::destroy<T>>;
		template<typename U> requires (std::is_same_v<T, U>) using default_manager = handle_manager<T>;
		template<typename U> requires (std::is_same_v<U, T>) using default_indexer = void;
		template<typename U> requires (std::is_same_v<U, T>) using default_storage = void;
	};
}