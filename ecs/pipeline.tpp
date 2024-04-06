#ifndef ECS_PIPELINE_TPP
#define ECS_PIPELINE_TPP

#include "pipeline.h"
#include "util/tuple_util.h"
#include "util/type_name.h"

template<typename ... Ts>
struct ecs::pipeline_builder
{
	using type = 
		util::tuple_sort_t<util::cmp<util::alpha_lt, std::remove_const>::type, 	// sort for consistent locking order 
		util::tuple_union_t<util::cmp<std::is_same, std::remove_const>::type,	// remove repeats
		util::tuple_sort_t<util::inv_cmp_if<std::is_const>::type,				// prioritizes mutable over const
		util::tuple_concat_t<ecs::pipeline_t<>, 								// cast to pipeline
		std::conditional_t<std::is_const_v<Ts>, 								// if const
			util::tuple_transform_t<std::add_const, 							
			ecs::traits::get_resource_set_t<Ts>>, 								// cast const to resource set
			ecs::traits::get_resource_set_t<Ts>>...>>>>; 						// get resource set
};

template<typename resource_set, typename base_T>
struct resource_set_constructor;

template<typename ... Ts, typename base_T>
struct resource_set_constructor<std::tuple<Ts*...>, base_T> {
	static std::tuple<Ts*...> construct(base_T& base) {
		return std::make_tuple(&base.template get_resource<Ts>()...);
	}
};

template<typename ... Ts>
template<typename base_T> 
ecs::pipeline_t<Ts...>::pipeline_t(base_T& base)
	: set(resource_set_constructor<pipeline_set, base_T>::construct(base)) 
{ }

template<typename ... Ts>
ecs::pipeline_t<Ts...>::pipeline_t(pipeline_t<Ts...>&& other)
	 : set(other.set) 
{ }

template<typename ... Ts>
ecs::pipeline_t<Ts...>& 
ecs::pipeline_t<Ts...>::operator=(pipeline_t<Ts...>&& other) 
{ set = other.set; return *this; }

template<typename ... Ts>
void ecs::pipeline_t<Ts...>::lock() {
	(get_resource<Ts>().acquire(), ...);
}

template<typename ... Ts>
void ecs::pipeline_t<Ts...>::unlock() {
	sync();
	(get_resource<Ts>().release(), ...);
}

template<typename ... Ts>
template<typename U>
U& ecs::pipeline_t<Ts...>::get_resource() requires (is_accessible_v<U>) {
	if constexpr (traits::has_resource_container_v<U>)
		return std::get<typename traits::get_resource_container_t<U>*>(set)->template get_resource<U>();
	else
		return *std::get<U*>(set);
}

template<typename ... Ts>
template<typename U, template<typename> typename Policy_U, typename ... Arg_Us>
U& ecs::pipeline_t<Ts...>::emplace(entity e, Arg_Us&& ... args) requires (util::tuple_allof_v<is_accessible, traits::get_resource_set_t<Policy_U<U>>>) 
{
	return Policy_U<U>::template emplace(*this, e, std::forward<Arg_Us>(args)...);
}

template<typename ... Ts>
template<typename U, template<typename> typename Policy_U>
void ecs::pipeline_t<Ts...>::erase(entity e) requires (util::tuple_allof_v<is_accessible, traits::get_resource_set_t<Policy_U<U>>>) {
	Policy_U<U>::template erase(*this, e);
}

template<typename ... Ts>
template<typename ... Us, typename from_T, typename where_T>
ecs::view<ecs::select<Us...>, from_T, where_T, ecs::pipeline_t<Ts...>&> 
ecs::pipeline_t<Ts...>::view(from_T, where_T) {
	return *this;
}

template<typename Resource_Set_T, typename Storage_T>
struct storage_syncer;

template<typename ... Resource_Set_Ts, typename Storage_T>
struct storage_syncer<std::tuple<Resource_Set_Ts...>, Storage_T> {
	static inline void sync(Storage_T* storage) { storage->template sync<Resource_Set_Ts...>(); }
};

template<typename Resource_Set_T, typename Storage_T>
void sync_storage(Storage_T* storage) {
	storage_syncer<util::tuple_intersect_t<util::cmp<std::is_same, std::remove_const>::type, 
		Resource_Set_T, ecs::traits::get_synchronization_set_t<Storage_T>>, Storage_T>::sync(storage);
}

template<typename ... Ts>
void ecs::pipeline_t<Ts...>::sync() {
	std::apply([](auto* ... storage) { (sync_storage<resource_container_set>(storage), ...); }, set);
}


#endif