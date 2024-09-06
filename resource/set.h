#pragma once 
#include "../tuple_util/tuple_util.h"
#include "traits.h"
#include "registry.h"
#include "pool.h"

namespace resource {
	// TODO: write lock and resource such that they wait on the same condition_variable when acquiring all resources
	// * dont know how this would work
	
	// TODO: remove registry from 
	// * pass resources directly to constructor
	// * store array/tuple of sorted resources
	
	// TODO: constructor to allow for explicit exclusive const and volatile for shared non const
	// // i dont really know why you would want either of these
	// unique qualifier for each:
	// T -> exclsuive_ptr<T>
	// const T -> shared_ptr<const T>
	// volatile T -> shared_ptr<T> 
	// - could use */& -> dont really like that not -> very clear and doesnt line up with functionality
	// - could use tags eg shared<>, exclusive<>, kinda hate that.
	//  -> might as well wrap it in the shared/exclusive ptr type, just too wordy
	


	template<typename ... Ts>
	struct set_t {
		using pointer_set = std::tuple<locked_ptr<Ts>...>;
		using resource_lockset = std::tuple<Ts...>; 
		
		template<typename ... Us>
		set_t(resource::registry<Us...>& base, priority p = priority::NONE) : ptrs(base.template get_resource<Ts>(p)...) { }

		void lock(priority p = priority::MEDIUM)
		{
			std::apply([](auto& ... ptr) { (ptr.lock(), ...); }, ptrs);
		}

		void unlock()
		{
			std::apply([](auto& ... ptr) { (ptr.unlock(), ...); }, ptrs);
		}

		template<typename T> // requires (traits::is_accessible_v<T, std::tuple<Ts...>>)
		traits::get_type_t<T>& get_resource() 
		{
			static constexpr int I = util::find_v<std::tuple<Ts...>, util::cmp::to_<T, 
				util::cmp::is_ignore_cv_same>::template type>;
			return *std::get<I>(ptrs);
		}

	private:
		pointer_set ptrs;
		priority p;
	
	};

	template<typename ... Ts>
	struct set_builder : util::eval<std::tuple<Ts...>, 
		util::eval_each_<traits::get_lockset>::template type, 
		util::concat, 
		util::unique_priority_<
			util::cmp::is_ignore_const_same, 
			util::cmp::lt_<std::is_const>::template type
		>::template type,
		util::sort_<util::cmp::priority_<
			util::cmp::lt_<traits::get_lock_level>::template type, 
			util::cmp::lt_<util::get_type_ID>::template type
		>::template type>::template type,
		util::rewrap_<set_t>::template type
		> { };

	template<typename ... Ts>	
	using set = set_builder<Ts...>::type;
}