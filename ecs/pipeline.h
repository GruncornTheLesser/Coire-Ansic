#include "view.h" 
// pipeline requires view.h but view.tpp requires pipeline.h
// this is probably bad

#pragma once
#include "util/tuple_util.h"
#include "util/type_name.h"
#include "traits.h"

namespace ecs {
	template<typename> struct deferred;
	template<typename> struct immediate;

	template<ecs::traits::resource_class ... Ts>
	struct pipeline_t {
		// the set of resources locked by this pipeline
		using resource_set = std::tuple<Ts...>;
		// the set of pointers to containers required for all resource set  
		using pipeline_set = util::set_t<std::tuple<Ts...>, 
			util::build::each<traits::get_resource_container>::template type,
			util::build::wrap<std::tuple>::template type,
			util::build::concat::template type,
			util::build::unique<util::cmp::build::transformed<std::is_same, std::remove_const>::template type>::template type,
			util::build::each<std::add_pointer>::template type
			>;

		template<typename base_T>
		pipeline_t(base_T& base);

		pipeline_t(pipeline_t<Ts...>& other) = delete;
		pipeline_t(pipeline_t<Ts...>&& other);

		pipeline_t& operator=(pipeline_t<Ts...>& other) = delete;
		pipeline_t& operator=(pipeline_t<Ts...>&& other);

		void lock();
		void unlock();
		void sync();

		template<traits::accessible_resource_class<pipeline_t<Ts...>> U>
		U& get_resource();

		template<typename U, template<typename> typename Policy_U = ecs::deferred, typename ... Arg_Us> 
			requires (traits::is_accessible_resource_v<Policy_U<U>, pipeline_t<Ts...>>) && 
			std::is_constructible_v<U, Arg_Us...>
		Policy_U<U>::emplace_return emplace(ecs::entity e, Arg_Us&& ... args);
		
		template<typename U, template<typename> typename Policy_U = ecs::deferred, typename It, typename ... Arg_Us> 
			requires (traits::is_accessible_resource_v<Policy_U<U>, pipeline_t<Ts...>>) && 
			std::is_constructible_v<U, Arg_Us...>
		Policy_U<U>::emplace_return emplace(It first, It last, Arg_Us&& ... args);

		template<typename U, template<typename> typename Policy_U = deferred> 
			requires (traits::is_accessible_resource_v<Policy_U<U>, pipeline_t<Ts...>>)
		void erase(ecs::entity e);

		template<typename U, template<typename> typename Policy_U = deferred, typename It> 
			requires (traits::is_accessible_resource_v<Policy_U<U>, pipeline_t<Ts...>>)
		void erase(It first, It last);

		template<typename ... select_Us, 
			typename from_T = ecs::view_from_builder<ecs::select<select_Us...>>::type, 
			typename where_T = ecs::view_where_builder<ecs::select<select_Us...>, from_T>::type>
			requires (traits::is_accessible_resource_v<std::tuple<ecs::select<select_Us...>, from_T, where_T>, pipeline_t<Ts...>>)
		view<select<select_Us...>, from_T, where_T, pipeline_t<Ts...>&> view(from_T from = {}, where_T where = {});

		

	private:
		pipeline_set set;
	};

	// gathers and manipulates the set and order of required resources, to maintain consistent locking order and prevent deadlocks
	template<typename ... Ts>
	struct pipeline_builder;

	template<typename ... Ts> // resource_class
	using pipeline = typename pipeline_builder<Ts...>::type;
}

template<typename ... Ts>
struct ecs::pipeline_builder
{
	using type = util::set_t<std::tuple<Ts...>, 
		util::build::each< // for each T
			util::build::propergate_const_each< // if const, add const to each element in tuple after
				util::build::set<   // if component, get_pool
					util::build::conditional<ecs::traits::is_component, ecs::traits::get_pool>::template type,
					ecs::traits::get_resource_set // get resource set
				>::template type // end transform set 
			>::template type // end transform post conditional
		>::template type, // end transform each
		util::build::concat::template type, // concat resource sets together
		util::build::sort<util::alpha_lt>::template type, // sort types by name
		util::build::sort<util::cmp::build::prioritize_if<std::is_const>::template negated>::template type, // sort type by const
		util::build::unique<util::cmp::build::transformed<std::is_same, std::remove_const>::template type>::template type, // remove duplicates
		util::build::rewrap<ecs::pipeline_t>::template type
	>;
};

//static_assert(std::is_same_v<ecs::pipeline<int>, ecs::pipeline<ecs::pool<int>>>, "pipeline test failed: component to pool conversion");

template<typename resource_set, typename base_T>
struct pipeline_set_constructor;

template<typename ... Ts, typename base_T>
struct pipeline_set_constructor<std::tuple<Ts*...>, base_T> {
	static std::tuple<Ts*...> construct(base_T& base) {
		return std::make_tuple(&base.template get_resource<Ts>()...);
	}
};

template<ecs::traits::resource_class ... Ts>
template<typename base_T> 
ecs::pipeline_t<Ts...>::pipeline_t(base_T& base)
	: set(pipeline_set_constructor<pipeline_set, base_T>::construct(base)) 
{ }

template<ecs::traits::resource_class ... Ts>
ecs::pipeline_t<Ts...>::pipeline_t(pipeline_t<Ts...>&& other)
	 : set(other.set) 
{ }

template<ecs::traits::resource_class ... Ts>
ecs::pipeline_t<Ts...>& 
ecs::pipeline_t<Ts...>::operator=(pipeline_t<Ts...>&& other) 
{ set = other.set; return *this; }

template<ecs::traits::resource_class ... Ts>
void ecs::pipeline_t<Ts...>::lock() {
	(get_resource<Ts>().acquire(), ...);
}

template<ecs::traits::resource_class ... Ts>
void ecs::pipeline_t<Ts...>::unlock() {
	sync();
	(get_resource<Ts>().release(), ...);
}

template<ecs::traits::resource_class ... Ts>
template<ecs::traits::accessible_resource_class<ecs::pipeline_t<Ts...>> U>
U& ecs::pipeline_t<Ts...>::get_resource() {
	if constexpr (traits::has_resource_container_v<U>)
		return std::get<typename traits::get_resource_container_t<U>*>(set)->template get_resource<U>();
	else
		return *std::get<U*>(set);
}

template<ecs::traits::resource_class ... Ts>
template<typename U, template<typename> typename Policy_U, typename ... Arg_Us>
	requires (ecs::traits::is_accessible_resource_v<Policy_U<U>, ecs::pipeline_t<Ts...>>) && 
	std::is_constructible_v<U, Arg_Us...>
Policy_U<U>::emplace_return ecs::pipeline_t<Ts...>::emplace(entity e, Arg_Us&& ... args)
{
	return Policy_U<U>::emplace(*this, e, std::forward<Arg_Us>(args)...);
}

template<ecs::traits::resource_class ... Ts>
template<typename U, template<typename> typename Policy_U>
	requires (ecs::traits::is_accessible_resource_v<Policy_U<U>, ecs::pipeline_t<Ts...>>)
void ecs::pipeline_t<Ts...>::erase(entity e)
{
	Policy_U<U>::template erase(*this, e);
}

template<ecs::traits::resource_class ... Ts>
template<typename ... Us, typename from_T, typename where_T>
	requires (ecs::traits::is_accessible_resource_v<std::tuple<ecs::select<Us...>, from_T, where_T>, ecs::pipeline_t<Ts...>>)
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
	storage_syncer<
		util::set_intersect_t<Resource_Set_T, 
			ecs::traits::get_synchronization_set_t<Storage_T>,
			util::cmp::build::transformed<std::is_same, std::remove_const>::template type>, 
		Storage_T>::sync(storage);
}

template<ecs::traits::resource_class ... Ts>
void ecs::pipeline_t<Ts...>::sync() {
	std::apply([](auto* ... container) { (sync_storage<std::tuple<Ts...>>(container), ...); }, set);
}