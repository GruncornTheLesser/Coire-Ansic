#pragma once
#include <type_traits>
#include <variant>
#include <tuple>
#include "../tuple_util/tuple_util.h"
#include "../util/type_name.h"
#include "../util/paged_vector.h"
#include "../util/sparse_map.h"
#include "../util/attribute.h"
#include "versioner.h"

// TODO: view args
// concepts - select, from, where
// builders - select, from, where, view
// view_indexable, view_iterable


// traits attribute

// default types
namespace ecs
{
	template<typename T> struct basic_manager;
	template<typename T> struct basic_indexer;
	template<typename T> struct basic_storage;
	template<typename T> struct handle_manager;
	struct entity;
}

namespace ecs::tags 
{
	struct resource;
	template<typename T>      struct handletype;
	template<typename T>      struct basictype;
	template<typename ... Ts> struct archetype;
	template<typename ... Ts> struct uniontype;
}

namespace ecs::traits 
{
	DECL_ATTRIB_NAMESPACE
	
	DECL_TYPE_ATTRIB(component_handle, void) //  entity
	DECL_TYPE_ATTRIB(component_manager, void) // basic_manager<T>
	DECL_TYPE_ATTRIB(component_indexer, void) // basic_indexer<T>
	DECL_TYPE_ATTRIB(component_storage, void) // basic_storage<T>

	DECL_TEMPLATE_ATTRIB(handle_alias, void)
	DECL_TEMPLATE_ATTRIB(manager_alias, void)
	DECL_TEMPLATE_ATTRIB(indexer_alias, void)
	DECL_TEMPLATE_ATTRIB(storage_alias, void)
	DECL_TYPE_ATTRIB(component_tag, tags::basictype<T>)
}

namespace ecs::traits 
{
	// TODO: finish me
	template<typename T> struct is_component : std::negation<std::is_void<T>> { };
	template<typename T> static constexpr bool is_component_v = is_component<T>::value;
	template<typename T> concept component_class = is_component<T>::value;

	template<typename T> struct is_sequence_policy : std::negation<std::is_void<T>> { };
	template<typename T> static constexpr bool is_sequence_policy_v = is_sequence_policy<T>::value;
	template<typename T> concept sequence_policy_class = is_sequence_policy<T>::value;

	template<typename T> struct is_execution_policy : std::negation<std::is_void<T>> { };
	template<typename T> static constexpr bool is_execution_policy_v = is_execution_policy<T>::value;
	template<typename T> concept execution_policy_class = is_execution_policy<T>::value;

	template<typename T> struct is_handle : std::negation<std::is_void<T>> { };
	template<typename T> static constexpr bool is_handle_v = is_handle<T>::value;
	template<typename T> concept handle_class = is_handle<T>::value;

	template<typename T> struct is_manager : std::negation<std::is_void<T>> { };
	template<typename T> static constexpr bool is_manager_v = is_manager<T>::value;
	
	template<typename T> struct is_indexer : std::negation<std::is_void<T>> { };
	template<typename T> static constexpr bool is_indexer_v = is_indexer<T>::value;
	
	template<typename T> struct is_storage : std::negation<std::is_void<T>> { };
	template<typename T> static constexpr bool is_storage_v = is_storage<T>::value;
}

namespace ecs::traits {
	template<typename T, typename=std::void_t<>> struct get_tag
	 : get_attribute<T, attribute::component_tag> { };
	template<typename T> struct get_tag<T, std::enable_if_t<std::is_same_v<get_attribute_t<T, attribute::component_tag>, tags::resource>>> : std::type_identity<T> { };
	template<typename T> using get_tag_t = typename get_tag<T>::type;



	template<typename T, typename=std::void_t<>> struct get_handle
	 : std::type_identity<ecs::entity> {
		static_assert(is_handle_v<ecs::entity>, "invalid default handle");
	};
	template<typename T> struct get_handle<T, std::enable_if_t<!has_attribute_v<T, attribute::component_handle> && has_attribute_v<T, attribute::component_tag>>>
	 : get_template_attribute<get_attribute_t<T, attribute::component_tag>, attribute::handle_alias<T>> {
		static_assert(is_handle_v<get_template_attribute_t<get_attribute_t<T, attribute::component_tag>, attribute::handle_alias<T>>>, "invalid component handle");
	};
	template<typename T> struct get_handle<T, std::enable_if_t<has_attribute_v<T, attribute::component_handle>>>
	 : get_attribute<T, attribute::component_handle> {
		static_assert(is_handle_v<get_attribute_t<T, attribute::component_handle>>, "invalid tag handle");
	};
	template<typename T> using get_handle_t = typename get_handle<T>::type;



	template<typename T, typename=std::void_t<>> struct get_manager
	 : std::type_identity<ecs::basic_manager<T>> {
		static_assert(is_manager_v<ecs::basic_manager<T>>, "invalid default manager");
	};
	template<typename T> struct get_manager<T, std::enable_if_t<!has_attribute_v<T, attribute::component_manager> && has_attribute_v<T, attribute::component_tag>>>
	 : get_template_attribute<get_attribute_t<T, attribute::component_tag>, attribute::manager_alias<T>> {
		static_assert(is_manager_v<get_template_attribute_t<get_attribute_t<T, attribute::component_tag>, attribute::manager_alias<T>>>, "invalid component manager");
	};
	template<typename T> struct get_manager<T, std::enable_if_t<has_attribute_v<T, attribute::component_manager>>>
	 : get_attribute<T, attribute::component_manager> {
		static_assert(is_manager_v<get_attribute_t<T, attribute::component_manager>>, "invalid tag manager");
	};
	template<typename T> using get_manager_t = typename get_manager<T>::type;



	template<typename T, typename=std::void_t<>> struct get_indexer
	 : std::type_identity<ecs::basic_indexer<T>> {
		static_assert(is_indexer_v<ecs::basic_indexer<T>>, "invalid default indexer");
	};
	template<typename T> struct get_indexer<T, std::enable_if_t<!has_attribute_v<T, attribute::component_indexer> && has_attribute_v<T, attribute::component_tag>>>
	 : get_template_attribute<get_attribute_t<T, attribute::component_tag>, attribute::indexer_alias<T>> {
		static_assert(is_indexer_v<get_template_attribute_t<get_attribute_t<T, attribute::component_tag>, attribute::indexer_alias<T>>>, "invalid component indexer");
	};
	template<typename T> struct get_indexer<T, std::enable_if_t<has_attribute_v<T, attribute::component_indexer>>>
	 : get_attribute<T, attribute::component_indexer> {
		static_assert(is_indexer_v<get_attribute_t<T, attribute::component_indexer>>, "invalid tag indexer");
	};
	template<typename T> using get_indexer_t = typename get_indexer<T>::type;




	template<typename T, typename=std::void_t<>> struct get_storage
	 : std::type_identity<ecs::basic_storage<T>> {
		static_assert(is_storage_v<ecs::basic_storage<T>>, "invalid default storage");
	};
	template<typename T> struct get_storage<T, std::enable_if_t<!has_attribute_v<T, attribute::component_storage> && has_attribute_v<T, attribute::component_tag>>>
	 : get_template_attribute<get_attribute_t<T, attribute::component_tag>, attribute::storage_alias<T>> {
		static_assert(is_storage_v<get_template_attribute_t<get_attribute_t<T, attribute::component_tag>, attribute::storage_alias<T>>>, "invalid component storage");
	};
	template<typename T> struct get_storage<T, std::enable_if_t<has_attribute_v<T, attribute::component_storage>>>
	 : get_attribute<T, attribute::component_storage> {
		static_assert(is_storage_v<get_attribute_t<T, attribute::component_storage>>, "invalid tag storage");
	};
	template<typename T> using get_storage_t = typename get_storage<T>::type;
}

namespace ecs 
{
	template<typename T> using manager = typename traits::get_manager<T>::type;
	template<typename T> using indexer = typename traits::get_indexer<T>::type;
	template<typename T> using storage = typename traits::get_storage<T>::type;
	template<typename T> using handle = typename traits::get_handle<T>::type;
}


#include "tags.h"

namespace ecs::test
{
	namespace {
		template<typename T>
		struct entity_alternative { 
			using component_tag = tags::handletype<T>;
		};
		template<typename T>
		struct test_tag { 
			template<typename U> using handle_alias = entity_alternative<T>;
			template<typename U> using manager_alias = ecs::basic_manager<T>;
			template<typename U> using indexer_alias = ecs::basic_indexer<T>;
			template<typename U> using storage_alias = ecs::basic_storage<T>;
		};
		

		struct A { };
		struct B { 
			using component_tag = test_tag<A>;
		};
		struct C {
			using component_handle = entity_alternative<A>;
			using component_manager = ecs::basic_manager<A>;
			using component_indexer = ecs::basic_indexer<A>;
			using component_storage = ecs::basic_storage<A>;
		};
		struct D {
			using component_tag = test_tag<int>;
			using component_handle = entity_alternative<A>;
			using component_manager = ecs::basic_manager<A>;
			using component_indexer = ecs::basic_indexer<A>;
			using component_storage = ecs::basic_storage<A>;
		};

		static_assert(std::is_same_v<ecs::traits::get_handle_t<A>, ecs::entity>, "failed default handle");
		static_assert(std::is_same_v<ecs::traits::get_manager_t<A>, ecs::basic_manager<A>>, "failed default manager");
		static_assert(std::is_same_v<ecs::traits::get_indexer_t<A>, ecs::basic_indexer<A>>, "failed default indexer");
		static_assert(std::is_same_v<ecs::traits::get_storage_t<A>, ecs::basic_storage<A>>, "failed default storage");

		static_assert(std::is_same_v<ecs::traits::get_handle_t<B>, entity_alternative<A>>, "failed tag handle");
		static_assert(std::is_same_v<ecs::traits::get_manager_t<B>, ecs::basic_manager<A>>, "failed tag manager");
		static_assert(std::is_same_v<ecs::traits::get_indexer_t<B>, ecs::basic_indexer<A>>, "failed tag indexer");
		static_assert(std::is_same_v<ecs::traits::get_storage_t<B>, ecs::basic_storage<A>>, "failed tag storage");

		static_assert(std::is_same_v<ecs::traits::get_handle_t<C>, entity_alternative<A>>, "failed tag handle");
		static_assert(std::is_same_v<ecs::traits::get_manager_t<C>, ecs::basic_manager<A>>, "failed tag manager");
		static_assert(std::is_same_v<ecs::traits::get_indexer_t<C>, ecs::basic_indexer<A>>, "failed tag indexer");
		static_assert(std::is_same_v<ecs::traits::get_storage_t<C>, ecs::basic_storage<A>>, "failed tag storage");

		static_assert(std::is_same_v<ecs::traits::get_handle_t<D>, entity_alternative<A>>, "failed tag override handle");
		static_assert(std::is_same_v<ecs::traits::get_manager_t<D>, ecs::basic_manager<A>>, "failed tag override manager");
		static_assert(std::is_same_v<ecs::traits::get_indexer_t<D>, ecs::basic_indexer<A>>, "failed tag override indexer");
		static_assert(std::is_same_v<ecs::traits::get_storage_t<D>, ecs::basic_storage<A>>, "failed tag override storage");
	}
}



















