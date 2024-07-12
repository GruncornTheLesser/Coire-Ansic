#pragma once
#include <unordered_map>
#include <typeindex>
#include "../util/tuple_util.h"
#include "../util/erased_ptr.h"
#include "../util/type_name.h"
#include "traits.h"
#include "pool.h"



/*
TODO: deadlock avoidance algorithms
 * Bankers/Graph Algorithm

TODO: interrupt
 * would allow priority locks, borrow/sharing return mechanics, 
 *    would need synchronizing event and at that point??? does it really make any sense???
 *    functions to check priority mutex queue for higher priority mutexes and lend out mutex before return

TODO: arena allocation
 */

// concurrent resource management
namespace resource 
{
	class dynamic_registry
	{
	public:
		template<traits::resource_key_class T, typename ... Args>
		pool<T>& get_resource_pool(Args&& ... args);

		template<traits::resource_key_class T>
		resource_ptr<T> get_resource(priority p = priority::MEDIUM);

	private:
		std::unordered_map<std::type_index, util::erased_unique_ptr> data;
	};

	template<typename ... Ts>
	class static_registry 
	{
		template<typename T>
		static constexpr bool accessible_alias = util::element_of_v<traits::get_alias_t<T>, 
			std::tuple<traits::get_alias_t<Ts>...>, util::cmp::is_const_accessible>;

	public:	
		template<traits::resource_key_class T, typename ... Args> 
		pool<T>& get_resource_pool(Args&& ... args) 
			requires (accessible_alias<T>);

		template<traits::resource_key_class T> 
		resource_ptr<T> get_resource(priority p = priority::MEDIUM) 
			requires (accessible_alias<T>);
			
	private:
		std::tuple<pool<Ts>...> pools;
	};
}

template<resource::traits::resource_key_class T, typename ... Args>
resource::pool<T>& resource::dynamic_registry::get_resource_pool(Args&& ... args)
{
	using pool_type = resource::pool<T>;
	using alias_type = resource::traits::get_alias_t<T>;

	std::type_index key = typeid(alias_type);
	auto it = data.find(key);
	if (it == data.end())
	{
		if constexpr (std::is_constructible_v<T, Args...>)
			it = data.emplace_hint(it, key, new pool_type{ std::forward<Args>(args)... });
		else
			throw std::runtime_error("registry does not contain resource '" + util::type_name<T>().data() + "' and cannot construct with Args="
			 + ((util::type_name<Args>().data() + ", ") + ...) + "\b\b.");
	}
	return it->second.get<pool_type>();
}

template<resource::traits::resource_key_class T>
resource::resource_ptr<T> resource::dynamic_registry::get_resource(priority p)
{
	return { &get_resource_pool<T>(), p };
}

template<typename ... Ts>
template<resource::traits::resource_key_class T, typename ... Args>
resource::pool<T>& resource::static_registry<Ts...>::get_resource_pool(Args&& ... args)
	requires (accessible_alias<T>)
{
	return std::get<util::find_v<std::tuple<Ts...>, 
		util::compare_to_<std::remove_cv_t<T>, std::is_same, traits::get_alias>::template type>>(pools);
}

template<typename ... Ts>
template<resource::traits::resource_key_class T>
resource::resource_ptr<T> resource::static_registry<Ts...>::get_resource(priority p)
	requires (accessible_alias<T>)
{
	return { &get_resource_pool<T>(), p };
}
