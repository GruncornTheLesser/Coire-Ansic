#pragma once
#include <shared_mutex>
#include <functional>
#include <typeindex>
#include <memory>

#include "traits.h"
#include "view.h" 
#include "pipeline.h"

#include "pool.h"
namespace ecs {
	class registry {
		class erased_ptr;
		using resource_data_t = std::unordered_map<std::type_index, erased_ptr>;
	public:
		template<typename U, typename ... Arg_Us>
		U& get_resource(Arg_Us ... args);

		template<typename U>
		void erase_resource();

		template<typename ... Us>
		pipeline<Us...> pipeline();

		template<typename ... Us, 
			typename from_T = ecs::view_from_builder_t<ecs::select<Us...>>, 
			typename where_T = ecs::view_where_builder_t<ecs::select<Us...>, from_T>>
		view<select<Us...>, from_T, where_T> view(from_T from = {}, where_T where = {});
	private:
		resource_data_t data;
	};
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-incomplete"

class ecs::registry::erased_ptr
{
public:
	template<typename T>
	erased_ptr(T* ptr);
	
	template<typename T>
	T& get();
private:
	std::unique_ptr<void, std::function<void(void*)>> ptr;
};

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

template<typename U, typename ... Arg_Us>
U& ecs::registry::get_resource(Arg_Us ... args) {
	using container_U = traits::get_resource_container_t<U>;
	
	std::type_index key = typeid(container_U);
	auto it = data.find(key);
	
	if (it == data.end()) {
		if constexpr (!std::is_constructible_v<U, Arg_Us...>)
			throw std::runtime_error(std::string("registry does not contain data/could not construct '") + 
				std::string(util::type_name<U>()) + "'");
		
		it = data.emplace_hint(it, key, new U{ std::forward<Arg_Us>(args)... });
	}

	if constexpr (!util::cmp::transformed_v<U, container_U, std::is_same, std::remove_const>)
		return it->second.get<U>();
	else
		return it->second.get<U>();
}

template<typename U>
void ecs::registry::erase_resource() {
	auto it = data.find(typeid(ecs::traits::get_resource_container_t<U>));
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
