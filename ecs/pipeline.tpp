#pragma once
#include "pipeline.h"

namespace ecs::traits {
	//DECL_HAS_ATTRIB_TYPE(try_emplace_func, template try_emplace<resource>());
}


template<typename base_t, typename ... ts>
void create_resource_set(std::tuple<ts*...>& set, base_t& base) {
	//if constexpr (ecs::traits::has_try_emplace_func_v<)
	set = std::make_tuple(&base.template try_emplace<ts>()...);
}

template<ecs::traits::resource_class ... ts>
template<typename base_t> 
ecs::pipeline_t<ts...>::pipeline_t(base_t& base) {
	create_resource_set(resource_set, base);
}

template<ecs::traits::resource_class ... ts>
void ecs::pipeline_t<ts...>::lock() { 
	(get_resource<ts>().acquire(), ...); 
}

template<ecs::traits::resource_class ... ts>
void ecs::pipeline_t<ts...>::unlock() { 
	(get_resource<ts>().release(), ...);
}

template<ecs::traits::resource_class ... ts>
template<ecs::traits::resource_class u>
u& ecs::pipeline_t<ts...>::get_resource() requires (is_accessible<u>) {
	if constexpr (traits::has_resource_container_v<u>)
		return (*std::get<typename traits::get_resource_container_t<u>*>(resource_set)).template get_resource<u>();
	else
		return *std::get<u*>(resource_set);
}
