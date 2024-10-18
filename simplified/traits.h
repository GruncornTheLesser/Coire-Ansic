#pragma once
#include "../util/attribute.h"
#include "../tuple_util/tuple_util.h" // used for copy_cv
#include "dispatcher.h"


// TODO: view args
// concepts - select, from, where
// builders - select, from, where, view
// view_indexable, view_iterable

#ifndef DEFAULT_HANDLE
#define DEFAULT_HANDLE entity
#endif

#ifndef DEFAULT_MANAGER
#define DEFAULT_MANAGER std::vector<handle<T>>
#endif

#ifndef DEFAULT_INDEXER
#define DEFAULT_INDEXER std::vector<handle<T>>
#endif

#ifndef DEFAULT_STORAGE
#define DEFAULT_STORAGE std::vector<T>
#endif

#ifndef DEFAULT_EVENTS
#define DEFAULT_EVENTS std::tuple<event::init<T>, event::terminate<T>>
#endif

#ifndef DEFAULT_HANDLE_MANAGER
#define DEFAULT_HANDLE_MANAGER handle_manager<T>
#endif

namespace ecs
{
	//template<std::unsigned_integral T=uint32_t, std::unsigned_integral V=uint32_t> 
	struct entity;

	template<typename T, typename Alloc_T=std::allocator<T>> class handle_manager;
	template<typename T> struct component_manager;
	template<typename T> struct component_indexer;
	template<typename T> struct component_storage;
}

namespace ecs::tags
{
	template<typename T>
	struct handletype;
	
	template<typename T>
	struct basictype;
	
	template<typename ... Ts>
	struct archetype;
	
	template<typename ... Ts>
	struct uniontype;
}

namespace ecs::event {
	template<typename T> struct init;
	template<typename T> struct terminate;

	template<typename handle_T=DEFAULT_HANDLE> struct create;
	template<typename handle_T=DEFAULT_HANDLE> struct destroy;
}

namespace ecs::traits
{
	DECL_ATTRIB_NAMESPACE
	// tag 
	DECL_TYPE_ATTRIB(component_tag, tags::basictype<T>)

	// tag attributes
	DECL_TYPE_ATTRIB(default_handle, DEFAULT_HANDLE)
	DECL_TYPE_ATTRIB(default_events, DEFAULT_EVENTS)
	DECL_TEMPLATE_ATTRIB(default_manager, component_manager<T>)
	DECL_TEMPLATE_ATTRIB(default_indexer, component_storage<T>)
	DECL_TEMPLATE_ATTRIB(default_storage, component_indexer<T>)

	// component attributes
	DECL_TYPE_ATTRIB(component_handle, EXPAND(get_attribute_t<get_attribute_t<T, attribute::component_tag>, attribute::default_handle>))
	DECL_TYPE_ATTRIB(component_events, EXPAND(get_attribute_t<get_attribute_t<T, attribute::component_tag>, attribute::default_events>))
	DECL_TYPE_ATTRIB(component_manager, EXPAND(get_template_attribute_t<get_attribute_t<T, attribute::component_tag>, attribute::default_manager<T>>))
	DECL_TYPE_ATTRIB(component_indexer, EXPAND(get_template_attribute_t<get_attribute_t<T, attribute::component_tag>, attribute::default_indexer<T>>))
	DECL_TYPE_ATTRIB(component_storage, EXPAND(get_template_attribute_t<get_attribute_t<T, attribute::component_tag>, attribute::default_storage<T>>))
}

// getter traits
namespace ecs::traits
{
	template<typename T> struct get_handle : util::copy_cv<get_attribute_t<std::remove_cv_t<T>, attribute::component_handle>, T> { };
	template<typename T> using get_handle_t = typename get_handle<T>::type;

	template<typename T> struct get_events : get_attribute<std::remove_cv_t<T>, attribute::component_events> { };
	template<typename T> using get_events_t = typename get_events<T>::type;
	
	template<typename T> struct get_manager : util::copy_cv<get_attribute_t<std::remove_cv_t<T>, attribute::component_manager>, T> { };
	template<typename T> using get_manager_t = typename get_manager<T>::type;

	template<typename T> struct get_indexer : util::copy_cv<get_attribute_t<std::remove_cv_t<T>, attribute::component_indexer>, T> { };
	template<typename T> using get_indexer_t = typename get_indexer<T>::type;

	template<typename T> struct get_storage : util::copy_cv<get_attribute_t<std::remove_cv_t<T>, attribute::component_storage>, T> { };
	template<typename T> using get_storage_t = typename get_storage<T>::type;
}

// concept traits
namespace ecs::traits
{
	template<typename T> struct is_manager : std::negation<std::is_void<T>> { };
	template<typename T> static constexpr bool is_manager_v = is_manager<T>::value;

	template<typename T> struct is_indexer : std::negation<std::is_void<T>> { };
	template<typename T> static constexpr bool is_indexer_v = is_indexer<T>::value;

	template<typename T> struct is_storage : std::negation<std::is_void<T>> { };
	template<typename T> static constexpr bool is_storage_v = is_storage<T>::value;

	template<typename T> struct is_handle : std::is_same<traits::get_attribute_t<std::remove_cv_t<T>, attribute::component_tag>, tags::handletype<std::remove_cv_t<T>>> { };
	template<typename T> static constexpr bool is_handle_v = is_handle<T>::value;
	template<typename T> concept handle_class = is_handle<T>::value;

	template<typename T> struct is_component : std::conjunction<
		is_handle<get_handle_t<T>>, 
		is_manager<get_manager_t<T>>, 
		is_indexer<get_indexer_t<T>>, 
		is_storage<get_storage_t<T>>> { };
	template<typename T> static constexpr bool is_component_v = is_component<T>::value;
	template<typename T> concept component_class = is_component<T>::value;

}

namespace ecs
{
	/// @brief a handle is a unique ID to reference an handle and it's components.
	/// a component's handle declares what handle class the component belongs to.
	template<typename T> using handle = typename traits::get_handle<T>::type;

	/// @brief a component's manager is the container for the handles to reorder entities
	template<typename T> using manager = typename traits::get_manager<T>::type;
	
	/// @brief a component's indexer is the look up for the index of the components from the handle
	template<typename T> using indexer = typename traits::get_indexer<T>::type;
	
	/// @brief a component's storage stores the components
	template<typename T> using storage = typename traits::get_storage<T>::type;
}

#include "tags.h"

#ifdef _DEBUG
namespace ecs::test
{
	namespace {
		template<typename T>
		struct test_handle : handle<uint16_t, uint16_t> {
			using component_tag = tags::handletype<T>;
		};
		template<typename T>
		struct test_tag {
			using default_handle = test_handle<T>;
			template<typename U> using default_manager = ecs::component_manager<T>;
			template<typename U> using default_indexer = ecs::component_indexer<T>;
			template<typename U> using default_storage = ecs::component_storage<T>;
		};


		struct A { };
		struct B {
			using component_tag = test_tag<A>;
		};
		struct C {
			using component_handle = test_handle<A>;
			using component_manager = ecs::component_manager<A>;
			using component_indexer = ecs::component_indexer<A>;
			using component_storage = ecs::component_storage<A>;
		};
		struct D {
			using component_tag = test_tag<int>;
			using component_handle = test_handle<A>;
			using component_manager = ecs::component_manager<A>;
			using component_indexer = ecs::component_indexer<A>;
			using component_storage = ecs::component_storage<A>;
		};

		static_assert(std::is_same_v<ecs::traits::get_handle_t<A>, DEFAULT_HANDLE>, "failed default handle");
		static_assert(std::is_same_v<ecs::traits::get_manager_t<A>, ecs::component_manager<A>>, "failed default manager");
		static_assert(std::is_same_v<ecs::traits::get_indexer_t<A>, ecs::component_indexer<A>>, "failed default indexer");
		static_assert(std::is_same_v<ecs::traits::get_storage_t<A>, ecs::component_storage<A>>, "failed default storage");

		static_assert(std::is_same_v<ecs::traits::get_handle_t<B>, test_handle<A>>, "failed tag handle");
		static_assert(std::is_same_v<ecs::traits::get_manager_t<B>, ecs::component_manager<A>>, "failed tag manager");
		static_assert(std::is_same_v<ecs::traits::get_indexer_t<B>, ecs::component_indexer<A>>, "failed tag indexer");
		static_assert(std::is_same_v<ecs::traits::get_storage_t<B>, ecs::component_storage<A>>, "failed tag storage");

		static_assert(std::is_same_v<ecs::traits::get_handle_t<C>, test_handle<A>>, "failed tag handle");
		static_assert(std::is_same_v<ecs::traits::get_manager_t<C>, ecs::component_manager<A>>, "failed tag manager");
		static_assert(std::is_same_v<ecs::traits::get_indexer_t<C>, ecs::component_indexer<A>>, "failed tag indexer");
		static_assert(std::is_same_v<ecs::traits::get_storage_t<C>, ecs::component_storage<A>>, "failed tag storage");

		static_assert(std::is_same_v<ecs::traits::get_handle_t<D>, test_handle<A>>, "failed tag override handle");
		static_assert(std::is_same_v<ecs::traits::get_manager_t<D>, ecs::component_manager<A>>, "failed tag override manager");
		static_assert(std::is_same_v<ecs::traits::get_indexer_t<D>, ecs::component_indexer<A>>, "failed tag override indexer");
		static_assert(std::is_same_v<ecs::traits::get_storage_t<D>, ecs::component_storage<A>>, "failed tag override storage");
	}
}
#endif // DEBUG

















