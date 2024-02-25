#pragma once
#include "registry.h"
#include "traits.h"
#include "pool.h"
#include "pipeline.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-incomplete"

template<typename T>
ecs::registry::erased_ptr::erased_ptr(T* ptr)
	 : ptr(ptr, [](void* p) { delete reinterpret_cast<T*>(p); }) 
{ }

template<typename T>
T& ecs::registry::erased_ptr::get() { 
	if constexpr (ecs::traits::has_resource_container_v<T>)
		return reinterpret_cast<ecs::traits::get_resource_container_t<T>*>(ptr.get())->template get_resource<T>();
	else
		return *reinterpret_cast<T*>(ptr.get());
}

#pragma GCC diagnostic pop

template<ecs::traits::acquireable u, typename ... arg_us>
u& ecs::registry::try_emplace(arg_us ... args) {
	std::type_index key = typeid(ecs::traits::get_resource_container_t<u>);
	data_t::iterator it = data.find(key);

	if (it == data.end())
	{
		auto* ptr = new u{ std::forward<arg_us>(args)... };
		it = data.emplace_hint(it, std::pair(key, ptr));
	}

	return it->second.get<u>();
}

template<ecs::traits::acquireable u>
u& ecs::registry::get() const {
	std::type_index key = typeid(ecs::traits::get_resource_container_t<u>);
	auto it = data.find(key);
	it->second.get<u>();
}

template<ecs::traits::acquireable u>
void ecs::registry::erase() {
	data.erase(typeid(ecs::traits::get_resource_container_t<u>));
}

template<ecs::traits::acquireable u>
void ecs::registry::try_erase() {
	auto it = data.find(typeid(ecs::traits::get_resource_container_t<u>));
	if (it != data.end()) data.erase(it);
}

template<ecs::traits::acquireable u>
bool ecs::registry::contains() const {
	auto it = data.find(typeid(ecs::traits::get_resource_container_t<u>));
	return it != data.end();
}

template<ecs::traits::acquireable ... us>
ecs::pipeline<us...> ecs::registry::pipeline() {
	return ecs::pipeline<us...>{ *this };
}
