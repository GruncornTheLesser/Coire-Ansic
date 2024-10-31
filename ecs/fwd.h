#pragma once

#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define FORCE_INLINE __attribute__((always_inline)) inline
#else
#define FORCE_INLINE inline
#endif


#ifndef ECS_DEFAULT_TAG
// the default tag to interpret the resource dependency set
#define ECS_DEFAULT_TAG ecs::tag::component
#endif

#ifndef ECS_DEFAULT_COMPONENT_TAG
// default component 
#define ECS_DEFAULT_COMPONENT_TAG ecs::tag::component_basictype
#endif

#ifndef ECS_DEFAULT_EVENT_TAG
#define ECS_DEFAULT_EVENT_TAG ecs::tag::event_sync
#endif

#ifndef ECS_DEFAULT_RESOURCE_TAG
#define ECS_DEFAULT_RESOURCE_TAG ecs::tag::resource_restricted
#endif

#ifndef ECS_DEFAULT_HANDLE_TAG
#define ECS_DEFAULT_HANDLE_TAG ecs::tag::handle_versioned<uint32_t, 12>
#endif

namespace ecs {
	namespace traits {
		template<typename T, typename=std::void_t<>> struct get_tag;
		template<typename T> using get_tag_t = typename get_tag<T>::type;
	}

	// traits
	template<typename T, typename=traits::get_tag_t<T>> struct resource_traits;
	template<typename T, typename=traits::get_tag_t<T>> struct event_traits; 
	template<typename T, typename=traits::get_tag_t<T>> struct handle_traits;
	template<typename T, typename=traits::get_tag_t<T>> struct component_traits;

	namespace traits {
		template<typename T, typename=std::void_t<>> struct is_resource;
		template<typename T> static constexpr bool is_resource_v = is_resource<T>::value;
		template<typename T> concept resource_class = is_resource<T>::value;

		template<typename T, typename=std::void_t<>> struct is_event;
		template<typename T> static constexpr bool is_event_v = is_event<T>::value;
		template<typename T> concept event_class = is_event<T>::value;
		
		template<typename T, typename=std::void_t<>> struct is_handle;
		template<typename T> static constexpr bool is_handle_v = is_handle<T>::value;
		template<typename T> concept handle_class = is_handle<T>::value;

		template<typename T, typename=std::void_t<>> struct is_component;
		template<typename T> static constexpr bool is_component_v = is_component<T>::value;
		template<typename T> concept component_class = is_component<T>::value;

		template<typename T, typename=std::void_t<>> struct is_contract;
		template<typename T> static constexpr bool is_contract_v = is_contract<T>::value;
		template<typename T> concept contract_class = is_contract<T>::value;
		
		template<typename T, typename=std::void_t<>> struct is_accessor;
		template<typename T> static constexpr bool is_accessor_v = is_accessor<T>::value;
		template<typename T> concept accessor_class = is_accessor<T>::value;

		template<typename T, typename=std::void_t<>> struct get_dependency_set;
		template<typename T> using get_dependency_set_t = typename get_dependency_set<T>::type;
	}

	// container
	template<traits::contract_class ... Ts> struct registry;

	// resource types
	struct unrestricted_resource;
	struct priority_resource;
	struct restricted_resource;
	struct exclusive_priority_resource;
	struct exclusive_restricted_resource;
	
	
	// resources
	template<typename T> struct generator;
	template<typename T> struct manager;
	template<typename T> struct indexer;
	template<typename T> struct storage;
	template<typename T> struct invoker;
	
	// events
	template<traits::resource_class T>  struct acquire;
	template<traits::resource_class T>  struct release;
	template<traits::handle_class T>    struct create;
	template<traits::handle_class T>    struct destroy;
	template<traits::component_class T> struct init;
	template<traits::component_class T> struct terminate;
	
	// handles
	struct entity;
	template<typename T> struct event_handle;

	// services
	template<typename set_T, typename reg_T> struct pipeline;
	template<typename T, typename reg_T> struct pool;
	template<typename select_T, typename from_T, typename where_T, typename reg_T> struct view;

	// extras
	struct tombstone;
	template<typename T, typename reg_T> struct pool_iterator;
	struct view_sentinel;
	template<typename select_T, typename from_T, typename where_T, typename reg_T> struct view_iterator;
	template<typename ... Ts> struct select;
	template<typename ... Ts> struct from;
	template<typename ... Ts> struct where;
	template<typename ... Ts> struct include;
	template<typename ... Ts> struct exclude;

	// resource getters
	template<traits::component_class T> using get_handle_t = typename component_traits<T>::handle_type;
	template<traits::handle_class T>    using get_generator_t = typename handle_traits<T>::generator_type;
	template<traits::component_class T> using get_manager_t = typename component_traits<T>::manager_type;
	template<traits::component_class T> using get_indexer_t = typename component_traits<T>::indexer_type;
	template<traits::component_class T> using get_storage_t = typename component_traits<T>::storage_type;
	template<traits::event_class T>     using get_invoker_t = typename event_traits<T>::invoker_type;
}

namespace ecs::tag {
	// resource tags are 
	struct resource_unrestricted;
	struct resource_priority;
	struct resource_restricted;
	struct resource_exclusive_priority;
	struct resource_exclusive_restricted;
	using resource = ECS_DEFAULT_RESOURCE_TAG;

	struct event_sync;
	struct event_async;
	using event = ECS_DEFAULT_EVENT_TAG;

	template<std::unsigned_integral T, size_t version_width> struct handle_versioned;
	template<std::unsigned_integral T> using handle_unversioned = handle_versioned<T, 0>;
	// struct handle_scoped;

	struct component_basictype;
	template<typename ... Ts> struct component_archetype;
	template<typename ... Ts> struct component_uniontype;

	using handle = ECS_DEFAULT_HANDLE_TAG;
	using component = ECS_DEFAULT_COMPONENT_TAG;
}