#ifndef ECS_REGISTRY_H
#define ECS_REGISTRY_H
#include <shared_mutex>
#include <functional>
#include <typeindex>
#include <memory>

#include "traits.h"
#include "view.h" 
#include "pipeline.h"

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

#include "registry.tpp"
#endif