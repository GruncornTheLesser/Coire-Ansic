#pragma once
#include "resource.h"
#include "entity.h"

#include "versioner.h"
#include <vector>

namespace ecs {
	class entity_manager : public resource 
	{
	public:
		// TODO: finish entity manager create/destroy
		// I wanted to use version view to create a linked list embedded in a vector, however I dont need the value I need the index
		// its stupid to store the index as the value -> I would need version_view to store an iterator, which is kinda annoying.
		// ? maybe instead store an index in the version pair
		// ? maybe I can embbed the info in the pointer -> enforce the allocator goes to a power of 2 to get the page index and lookup the page index + offset from page index
		
		// ? less stupid way overall, do a std::swap to back of the array as a destroyed list
		// ! problem with that is that 
		ecs::entity create() 
		{
		}
		void destroy(ecs::entity e) 
		{
		}
		bool active(ecs::entity e) 
		{ 
		}
	private:
		
	};
}