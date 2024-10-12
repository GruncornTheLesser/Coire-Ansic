#pragma once
#include "entity.h"
#include "traits.h"

// this needs indexer to have been executed
// event queuer must automatically queue events to execute thing in the correct order


namespace ecs::policy
{
	struct stable
	{
		template<typename T>
		struct emplace
		{
			using component_tag = ecs::tags::resource;
			using resource_lockset = std::tuple<manager<T>>;

			size_t index;
			entity value;

			emplace(size_t i, entity e) : entity(e), index(i) { }

			template<typename base_T, typename ... Arg_Ts>
			void operator()(base_T& base, Arg_Ts&& ... args)
			{
				auto& mng = base.template get_resource<manager<T>>();

				mng.insert(mng.cbegin() + index, value);

				auto it = mng.begin() + index, end = mng.end();
				while (it != end) { mng.chain().update(it++); }
			}
		};

		template<typename T>
		struct emplace_back
		{
			using component_tag = ecs::tags::resource;
			using resource_lockset = std::tuple<manager<T>>;
			handle<T> e;

			template<typename base_T, typename ... Arg_Ts>
			void operator()(base_T& base)
			{
				auto& mng = base.template get_resource<manager<T>>();
				mng.push_back(e);
			}
		};


		template<typename T, typename It>
		struct erase
		{
			using resource_lockset = std::tuple<manager<T>, indexer<T>>;

			It first, last;

			erase(It first, It last) : first(first), last(last) { }

			template<typename base_T>
			void operator()(base_T& base)
			{
				//auto& mng = base.template get_resource<manager<T>>();
				//auto& ind = base.template get_resource<indexer<T>>();

				//ensure indexer up to date
				//
				//std::move_backward(first, last, mng.begin() + );
			}
		};

		template<typename T>
		struct erase_at
		{
			using resource_lockset = std::tuple<manager<T>>;
			uint32_t index;

			erase_at(uint32_t index) : index(index) { }

			template<typename base_T>
			void operator()(base_T& base)
			{
				auto mng = base.template get_resource<manager<T>>();
			}

		};


	};
	struct swap
	{
		template<typename T>
		struct emplacer
		{
			using resource_lockset = std::tuple<manager<T>>;
		};
		template<typename T>
		struct eraser
		{
			using resource_lockset = std::tuple<manager<T>>;
		};
	};


	// execution policy

	// updates in same pipeline
	struct immediate
	{
		template<typename T, typename func_T>
		struct dispatcher {
			using resource_set = std::tuple<func_T>;
		};
	};

	// updates on resource release
	struct deferred
	{

	};

	// updates resource on next acquire
	struct lazy
	{

	};
}