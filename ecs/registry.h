#pragma once
#include <shared_mutex>
#include <functional>
#include <typeindex>
#include <memory>
#include "traits.h"
#include "pipeline.h"
namespace ecs {
	class registry {
		class erased_ptr {
		public:
			template<typename T>
			erased_ptr(T* ptr);

			template<typename T>
			T& get();

		private:
			std::unique_ptr<void, std::function<void(void*)>> ptr;
		};

		using data_t = std::unordered_map<std::type_index, erased_ptr>;

	public:
		template<traits::acquireable u, typename ... arg_us>
		u& try_emplace(arg_us ... args);

		template<traits::acquireable u>
		u& get() const;

		template<traits::acquireable u>
		void erase();

		template<traits::acquireable u>
		void try_erase();

		template<traits::acquireable u>
		bool contains() const;

		template<traits::acquireable ... us>
		pipeline<us...> pipeline();

	private:
		data_t data;
	};
}

#include "registry.tpp"