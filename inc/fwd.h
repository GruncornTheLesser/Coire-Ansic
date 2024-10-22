#pragma once 
#include "macros.h"

namespace ecs::tag
{
	namespace component {
		struct custom;

		struct basictype; 
		template<typename ... Ts> struct archetype;
		template<typename ... Ts> struct uniontype;
	}
	namespace resource {
		struct custom;
		struct unrestricted;
		struct restricted;
		struct exclusive;
	}
	namespace handle {
		struct custom;
		struct versioned;
		struct unversioned;
	}
	namespace event {
		struct custom;
		struct sync;
		struct lazy;
		struct async;
	}
}

namespace ecs {
	enum class priority { NONE, LOW, MEDIUM, HIGH };

	// traits
	template<typename T, typename=typename T::resource_tag>
	struct resource_traits;
	
	template<typename T, typename=typename T::handle_tag>
	struct handle_traits;
	
	template<typename T, typename=typename T::event_tag>
	struct event_traits;

	template<typename T, typename=typename T::component_tag>
	struct component_traits;

	template<typename view_T>
	struct view_traits;

	template<typename T> 
	struct registry_traits;

	// handles
	struct tombstone { };

	struct entity;
	
	template<typename T>
	struct event_handle; // custom handle does not have a handle set
	
	// resources
	template<typename T>
	struct generator;
	
	template<typename T>
	struct manager;
	
	template<typename T>
	struct indexer;
	
	template<typename T>
	struct storage;
	
	template<typename T>
	struct invoker;
	
	// events
	template<typename T>
	struct init;

	template<typename T>
	struct terminate;

	template<typename T>
	struct create;

	template<typename T>
	struct destroy;

	// collections
	template<typename ... Ts>
	struct registry;
	
	template<typename T, typename reg_T>
	struct pool_iterator;

	template<typename T, typename reg_T>
	struct pool;
	
	template<typename ... Ts>
	struct select;

	template<typename ... Ts>
	struct from;

	template<typename ... Ts>
	struct where;

	template<typename ... Ts>
	struct include;

	template<typename ... Ts>
	struct exclude;

	template<typename select_T, typename from_T, typename where_T, typename reg_T> 
	struct view;

	

}