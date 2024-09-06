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
			using resource_lockset = std::tuple<manager<T>>;
			size_t index;
			entity value;

			emplace(size_t i, entity e) : entity(e), index(i) { }

			template<typename base_T> 
			void operator()(base_T& base) 
			{
				auto& mng = base.template get_resource<manager<T>>();
				
				mng.insert(mng.cbegin() + index, value);
				std::for_each(mng.begin() + index, mng.end(), [&](auto& elem) { 
					mng.update_queue.push_back(elem);
				});
			}
		};
		
		template<typename T>
		struct emplace_back
		{
			using resource_lockset = std::tuple<manager<T>>;
			
			entity value;
			emplace_back(entity e) : value(e) { }

			template<typename base_T>
			void operator()(base_T& base) 
			{ 
				auto& mng = base.template get_resource<manager<T>>();
				
				//mng.emplace_back(value);
				//mng.update_queue.push_back(mng.back());
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
		template<typename T, typename It> 
		struct emplacer 
		{ 
			using resource_lockset = std::tuple<manager<T>>;
			
			It first, last;
			size_t index;
			
			emplacer(It first, It last, size_t index) : first(first), last(last), index(index) { }

			template<typename base_T> 
			void operator()(base_T& base) 
			{
				auto& mng = base.template get_resource<manager<T>>();
				size_t n = last - first;
				mng.insert(mng.end(), mng.begin() + index, mng.begin() + index + n);
				std::move(first, last, mng.begin() + index);
			}
		};
		template<typename T> 
		struct eraser 
		{
			using resource_lockset = std::tuple<manager<T>>;
		};
	};
	struct grouped 
	{
		template<typename T> 
		struct emplacer 
		{ 
			using resource_lockset = std::tuple<manager<T>>; // add grouping resource???
		};
		template<typename T> struct eraser 
		{ 
			using resource_lockset = std::tuple<manager<T>>; // add grouping resource???
		};
	};

	// execution policy

	// updates in same pipeline
	struct immediate 
	{
		template<typename T> 
		struct syncer 
		{ 
			using resource_lockset = std::tuple<const manager<T>, indexer<T>, storage<T>>;
			template<typename base_T> void operator()(base_T& base) 
			{
				auto& mng = base.template get_resource<manager<T>>(); // packed entity array
				auto& ind = base.template get_resource<indexer<T>>(); // entity look_up
				auto& str = base.template get_resource<storage<T>>(); // packed component array
				/*
				for (auto& elem : versioner{ mng })			// iterate through starting index
				{
					//size_t pos = mng.end() - elem; 		// TODO: i need index from list
					size_t curr = ind[elem];				// index that entity at pos used to be stored at

					// if (curr == -1) continue;

					while (&mng[curr] != &elem)				// if curr == pos, component is in the correct index
					{
						size_t next = ind[elem];		// index that entity at curr used to be stored at

						ind[elem] = curr;				// set sparse lookup for curr to the new index
						std::swap(str[curr], str[next]);	// move component at curr to correct position

						curr = next;						// set curr to next
					}

					ind[elem] = pos;						// when I remove this it breaks -> must be important.
				}

				mng.update_queue.clear();
				*/
			}
		};

	};

	// updates on resource release
	struct deferred 
	{ 
		template<typename T>
		 struct syncer 
		 { 
			using resource_lockset = std::tuple<const manager<T>>; // , on_unlock_dispatcher<T>
			template<typename base_T> void operator()(base_T& base) 
			{
				
			}
		};
	};

	// updates on next resource acquire
	struct lazy 
	{
		template<typename T> 
		struct syncer 
		{
			using resource_lockset = std::tuple<const manager<T>>; // , on_lock_dispatcher<T>
			template<typename base_T> void operator()(base_T& base) 
			{
				// queue sync operation on unlock
			}
		};
	};

}