#pragma once
#include "traits.h"
#include "versioner.h"

namespace ecs::tags 
{
	template<typename T>
	struct basictype 
	{
		template<typename U> requires (std::is_same_v<U, T>) using manager_alias = basic_manager<T>;
		template<typename U> requires (std::is_same_v<U, T>) using indexer_alias = basic_indexer<T>;
		template<typename U> requires (std::is_same_v<U, T>) using storage_alias = basic_storage<T>;
		template<typename U> requires (std::is_same_v<U, T>) using handle_alias = entity;

		using resource_lockset = std::tuple<manager<T>, indexer<T>, storage<T>>;
	};

	template<typename ... Ts> 
	struct archetype
	{
		using shared_type = util::find_min_t<std::tuple<Ts...>, util::get_type_ID>;
		
		template<typename U> requires (std::is_same_v<U, Ts> || ...) using manager_alias = basic_manager<shared_type>;
		template<typename U> requires (std::is_same_v<U, Ts> || ...) using indexer_alias = basic_indexer<shared_type>;
		template<typename U> requires (std::is_same_v<U, Ts> || ...) using storage_alias = basic_storage<U>;
		template<typename U> requires (std::is_same_v<U, Ts> || ...) using handle_alias = entity;
		
		using resource_lockset = std::tuple<manager<shared_type>, indexer<shared_type>, storage<Ts>...>;
	};
	
	template<typename ... Ts> 
	struct uniontype
	{
		using shared_type = util::find_min_t<std::tuple<Ts...>, util::get_type_ID>; // get unique type ID for set
		using value_type = util::sort_t<std::variant<Ts...>, util::cmp::lt_<util::get_type_ID>::template type>; // sorted so consistent order
		
		template<typename U> requires (std::is_same_v<U, Ts> || ...) using manager_alias = basic_manager<shared_type>;
		template<typename U> requires (std::is_same_v<U, Ts> || ...) using indexer_alias = basic_indexer<shared_type>;
		template<typename U> requires (std::is_same_v<U, Ts> || ...) using storage_alias = basic_storage<value_type>;
		template<typename U> requires (std::is_same_v<U, Ts> || ...) using handle_alias = entity;

		using resource_lockset = std::tuple<manager<shared_type>, indexer<shared_type>, storage<shared_type>>;
	};

	template<typename T>
	struct handletype
	{
		template<typename U> requires (std::is_same_v<T, U>) using manager_alias = handle_manager<U>;
		template<typename U> requires (std::is_same_v<T, U>) using handle_alias = U;
		
		using resource_lockset = std::tuple<manager<T>>;
	};
}

namespace ecs
{
	template<typename T>
	struct basic_manager
	{
		using component_tag = tags::resource;
		using resource_type = util::paged_vector<versioned<entity>>;
	};

	template<typename T>
	struct basic_indexer
	{
		using component_tag = tags::resource;
		using resource_type = util::sparse_map<ecs::entity, uint32_t>;
	};

	template<typename T>
	struct basic_storage
	{ 
		using component_tag = tags::resource;
		using resource_type = util::paged_vector<T>;
	};
}

