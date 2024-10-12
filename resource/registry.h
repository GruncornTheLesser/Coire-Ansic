#pragma once
#include <unordered_map>
#include <typeindex>
#include "../tuple_util/tuple_util.h"
#include "../util/erased_ptr.h"
#include "../util/type_name.h"
#include "traits.h"
#include "pool.h"

// apparently we are using priority ceiling protocol

/*
TODO: arena allocation

TODO: resource synchronization and versioning
 * declare a resource initialization synchronization function
 * use versioning to check if resource requires update
 * when resources are acquired with write priorities it's version gets incremented
 * to check versioning the version must

TODO: resource ID generation at runtime & resource lock granularity
 * spent a long time working out how to do compile time resource IDs. so to go back on it would be quite sad.
 * however with dynamic resource containers it could be useful to lock individual pages or elements to control
 * resource lock granularity. This could actually extend instance_count. like std::dynamic_extent as a specialization
 * resource pool could then be a vector of resources.
 */

// concurrent resource management
namespace resource
{
	class dynamic_registry
	{
	public:
		template<traits::resource_class T, typename ... Args>
		pool<T>& get_resource_pool(Args&& ... args);

		template<traits::resource_class T>
		locked_ptr<T> get_resource(priority p = priority::MEDIUM);
	private:
		std::unordered_map<std::type_index, util::erased_unique_ptr> data;
	};

	template<typename ... Ts>
	class static_registry
	{
		template<typename T>
		static constexpr bool accessible_alias = util::pred::element_of_v<traits::get_alias_t<std::remove_volatile_t<T>>,
			std::tuple<traits::get_alias_t<std::remove_volatile_t<Ts>>...>, util::cmp::is_const_accessible>;

	public:
		template<traits::resource_class T, typename ... Args>
		pool<T>& get_resource_pool(Args&& ... args)
			requires (accessible_alias<T>);

		template<traits::resource_class T>
		locked_ptr<T> get_resource(priority p = priority::MEDIUM)
			requires (accessible_alias<T>);

	private:
		std::tuple<pool<Ts>...> pools;
	};

	template<typename ... Ts>
	using registry = std::conditional_t<std::tuple_size_v<std::tuple<Ts...>> == 0, dynamic_registry, static_registry<Ts...>>;
}

template<resource::traits::resource_class T, typename ... Args>
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
#ifdef _DEBUG
		else
			throw std::runtime_error("registry does not contain resource '" + std::string(util::type_name<T>().data()) + 
			"' and cannot construct with from arguments: " + ((util::type_name<Args>().data() + ", ") + ...) + "\b\b.");
#endif
	}
	return it->second.get<pool_type>();
}

template<resource::traits::resource_class T>
resource::locked_ptr<T> resource::dynamic_registry::get_resource(priority p)
{
	return { &get_resource_pool<T>(), p };
}

template<typename ... Ts>
template<resource::traits::resource_class T, typename ... Args>
resource::pool<T>& resource::static_registry<Ts...>::get_resource_pool(Args&& ... args)
	requires (accessible_alias<T>)
{
	return std::get<util::find_v<std::tuple<Ts...>, util::cmp::to_<std::remove_cv_t<T>, std::is_same, traits::get_alias>::template type>>(pools);
}

template<typename ... Ts>
template<resource::traits::resource_class T>
resource::locked_ptr<T> resource::static_registry<Ts...>::get_resource(priority p)
	requires (accessible_alias<T>)
{
	return { &get_resource_pool<T>(), p };
}
