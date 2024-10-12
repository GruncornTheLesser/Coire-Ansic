#pragma once
#include "../tuple_util/tuple_util.h"
#include "traits.h"
#include "registry.h"
#include "pool.h"

namespace resource {
	// TODO: write lock and resource such that they wait on the same condition_variable when acquiring all resources
	// * dont know how this would work
	
	template<typename ... Ts>
	class set_t {
	private:
		using pointer_set = std::tuple<locked_ptr<Ts>...>;
	public:
		using resource_lockset = std::tuple<Ts...>;
		
		template<typename ... Us>
		set_t(registry<Us...>& base, priority p = priority::NONE) : ptrs(base.template get_resource<Ts>(p)...) { }

		void lock(priority p = priority::MEDIUM)
		{
			std::apply([&](auto& ... ptr) { (ptr.lock(p), ...); }, ptrs);
		}

		void unlock()
		{
			std::apply([](auto& ... ptr) { (ptr.unlock(), ...); }, ptrs);
		}

		template<typename T>//requires (traits::is_accessible_v<T, set_t>)
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

	/*
	the set builder manipulates the sequence of types to create a set of types where:
	 - non const, non volatile is preferred over const or volatile types
	 - types are consistently ordered to be used as the locking order
	 - locking order is determined by the static attribute member lock_level
	*/
	template<typename ... Ts>
	struct set_builder : util::eval<std::tuple<Ts...>,
		util::eval_each_<traits::get_lockset>::template type,
		util::concat,
		util::unique_priority_<
			util::cmp::is_ignore_cv_same,
			util::cmp::attrib_<std::is_const, std::is_volatile>::template type
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