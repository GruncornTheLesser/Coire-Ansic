#pragma once
#include "traits.h"
#include "event.h"

namespace ecs {
	template<typename T> struct acquire { using event_tag = ecs::tag::event::sync; };
	template<typename T> struct release { using event_tag = ecs::tag::event::sync; };

	template<typename T>
	struct resource_traits<T, ecs::tag::resource::custom> {
		using event_set = typename T::event_set;
		using component_set = typename T::component_set;
	};

	template<typename T>
	struct resource_traits<T, ecs::tag::resource::unrestricted> {
		using event_set = std::tuple<>;
		using component_set = std::tuple<>;
	};

	template<typename T>
	struct resource_traits<T, ecs::tag::resource::restricted> {
		using event_set = std::tuple<>;
		using component_set = std::tuple<>;
	};

	template<typename T>
	struct resource_traits<T, ecs::tag::resource::exclusive> {
		using event_set = std::tuple<>;
		using component_set = std::tuple<>;
	};
}