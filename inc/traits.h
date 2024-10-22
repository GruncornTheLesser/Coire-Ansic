#pragma once
#include "fwd.h"
#include <utility>
namespace ecs::traits {
	template<typename T, typename=std::void_t<>> struct is_component : std::false_type { };
	template<typename T> struct is_component<T, std::void_t<decltype(component_traits<T>{})>> : std::true_type { };
	template<typename T> static constexpr bool is_component_v = is_component<T>::value;
	template<typename T> concept component_class = is_component<T>::value;

	template<typename T, typename=std::void_t<>> struct is_registry : std::false_type { };
	template<typename T> struct is_registry<T, std::void_t<decltype(registry_traits<T>{})>> : std::true_type { };
	template<typename T> static constexpr bool is_registry_v = is_registry<T>::value;
	template<typename T> concept registry_class = is_registry<T>::value;

	template<typename T, typename=std::void_t<>> struct is_resource : std::false_type { };
	template<typename T> struct is_resource<T, std::void_t<decltype(resource_traits<T>{})>> : std::true_type { };
	template<typename T> static constexpr bool is_resource_v = is_resource<T>::value;
	template<typename T> concept resource_class = is_resource<T>::value;
	
	template<typename T, typename=std::void_t<>> struct is_handle : std::false_type { };
	template<typename T> struct is_handle<T, std::void_t<decltype(handle_traits<T>{})>> : std::true_type { };
	template<typename T> static constexpr bool is_handle_v = is_handle<T>::value;
	template<typename T> concept handle_class = is_handle<T>::value;

	template<typename T, typename=std::void_t<>> struct is_event : std::false_type { };
	template<typename T> struct is_event<T, std::void_t<decltype(event_traits<T>{})>> : std::true_type { };
	template<typename T> static constexpr bool is_event_v = is_event<T>::value;
	template<typename T> concept event_class = is_event<T>::value;



	template<typename T> using get_component_manager = std::type_identity<typename component_traits<T>::manager_type>;
	template<typename T> using get_component_manager_t = typename component_traits<T>::manager_type;
	
	template<typename T> using get_component_indexer = std::type_identity<typename component_traits<T>::indexer_type>;
	template<typename T> using get_component_indexer_t = typename component_traits<T>::indexer_type;
	
	template<typename T> using get_component_storage = std::type_identity<typename component_traits<T>::storage_type>;
	template<typename T> using get_component_storage_t = typename component_traits<T>::storage_type;
	
	template<typename T> using get_component_handle = std::type_identity<typename component_traits<T>::handle_type>;
	template<typename T> using get_component_handle_t = typename component_traits<T>::handle_type;
	
	template<typename T> using get_component_resource_set = std::type_identity<std::tuple<typename component_traits<T>::manager_type, typename component_traits<T>::indexer_type, typename component_traits<T>::storage_type>>;
	template<typename T> using get_component_resource_set_t = std::tuple<typename component_traits<T>::manager_type, typename component_traits<T>::indexer_type, typename component_traits<T>::storage_type>;
	


	template<typename T> using get_resource_component_set = std::type_identity<typename resource_traits<T>::component_set>;
	template<typename T> using get_resource_component_set_t = typename resource_traits<T>::component_set;
	
	template<typename T> using get_resource_event_set = std::type_identity<typename resource_traits<T>::event_set>;
	template<typename T> using get_resource_event_set_t = typename resource_traits<T>::event_set;
	
	template<typename T> using get_resource_mutex = std::type_identity<typename resource_traits<T>::mutex_type>;
	template<typename T> using get_resource_mutex_t = typename resource_traits<T>::mutex_type;
	


	template<typename T> using get_handle_generator = std::type_identity<typename handle_traits<T>::generator_type>;
	template<typename T> using get_handle_generator_t = typename handle_traits<T>::generator_type;
	


	template<typename T> using get_event_invoker = std::type_identity<typename event_traits<T>::invoker_type>;
	template<typename T> using get_event_invoker_t = typename event_traits<T>::invoker_type;
	
	template<typename T> using get_event_handle = std::type_identity<typename event_traits<T>::handle_type>;
	template<typename T> using get_event_handle_t = typename event_traits<T>::handle_type;
	
	

	template<typename T> using get_registry_component_set = std::type_identity<typename registry_traits<T>::component_set>;
	template<typename T> using get_registry_component_set_t = typename registry_traits<T>::component_set;

	template<typename T> using get_registry_event_set = std::type_identity<typename registry_traits<T>::event_set>;
	template<typename T> using get_registry_event_set_t = typename registry_traits<T>::event_set;

	template<typename T> using get_registry_handle_set = std::type_identity<typename registry_traits<T>::handle_set>;
	template<typename T> using get_registry_handle_set_t = typename registry_traits<T>::handle_set;

	template<typename T> using get_registry_resource_set = std::type_identity<typename registry_traits<T>::resource_set>;
	template<typename T> using get_registry_resource_set_t = typename registry_traits<T>::resource_set;
}

namespace ecs {
	template<traits::handle_class T> using generator_t = typename handle_traits<T>::generator_type;

	template<traits::component_class T> using handle_t = typename component_traits<T>::handle_type;
	template<traits::component_class T> using manager_t = typename component_traits<T>::manager_type;
	template<traits::component_class T> using indexer_t = typename component_traits<T>::indexer_type;
	template<traits::component_class T> using storage_t = typename component_traits<T>::storage_type;

	template<traits::event_class T> using invoker_t = typename event_traits<T>::invoker_type;

}