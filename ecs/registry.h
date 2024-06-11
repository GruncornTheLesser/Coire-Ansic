#pragma once
#include <shared_mutex>
#include <functional>
#include <typeindex>
#include <memory>

#include "traits.h"
#include "resource.h"
#include "view.h" 
#include "pipeline.h"
#include "util/erased_ptr.h"

namespace ecs {
	class registry {
	public:
		template<typename T, typename ... Args>
		ecs::traits::get_resource_type_t<T>& get_resource(Args&& ... args);

		template<typename T>
		void erase_resource();

		template<typename ... Us>
		pipeline<Us...> pipeline();

		template<typename ... Us, 
			typename from_T = ecs::view_from_builder_t<ecs::select<Us...>>, 
			typename where_T = ecs::view_where_builder_t<ecs::select<Us...>, from_T>>
		view<select<Us...>, from_T, where_T> view(from_T from = {}, where_T where = {});
	private:
		std::unordered_map<std::type_index, util::erased_unique_ptr> data;
	};
}



template<typename T, typename ... Args>
ecs::traits::get_resource_type_t<T>& ecs::registry::get_resource(Args&& ... args) {
	using res_type = ecs::traits::get_resource_type_t<std::remove_const_t<T>>;
	using alias_type = ecs::traits::get_resource_alias_t<std::remove_const_t<T>>;
	std::type_index key = typeid(alias_type);
	auto it = data.find(key);
	
	if (it == data.end()) {
		if constexpr (std::is_constructible_v<T, Args...>)
			it = data.emplace_hint(it, key, new res_type{ std::forward<Args>(args)... });
		else
			throw "";
			//throw std::runtime_error(std::string("registry does not contain resource/could not construct '")
			// + std::string(util::type_name<T>()) + "' from args '" + (std::string(util::type_name<Args>()) + ...)  + "'.");
	}
	return it->second.get<res_type>();
}

template<typename T>
void ecs::registry::erase_resource() {
	auto it = data.find(typeid(ecs::traits::get_resource_type_t<T>));
	if (it != data.end()) data.erase(it);
}

template<typename ... Us>
ecs::pipeline<Us...> ecs::registry::pipeline() {
	return ecs::pipeline<Us...>{ *this };
}

template<typename ... Us, typename from_T, typename where_T>
ecs::view<ecs::select<Us...>, from_T, where_T> ecs::registry::view(from_T, where_T) {
	return { *this };
}
