#pragma once
#include <cstdint>
#include <vector>
#include <bitset>
#include "traits.h"
namespace ecs {
	template<typename T> class handle_manager;
	
	struct entity final 
	{
		template<typename T> friend class handle_manager;
		using component_tag = tags::handletype<entity>;

	private:
	 	entity(uint32_t v) : value(v) { }
	public:
		entity() : value(static_cast<uint32_t>(-1)) { }
		operator uint32_t() { return value; }

	private:
		uint32_t value; 
	};
	const entity tombstone{};

	template<typename handle_T>
	class handle_manager
	{
	public:
		using component_tag = tags::resource;
		using value_type = handle_T;

		handle_T create() 
		{ 
			uint32_t e;
			if (inactive.size() > 0)
			{
				e = inactive.back();
				inactive.pop_back();
			}
			else 
			{
				if (next == active.size() >> 32) active.emplace_back();
				e = next++;
			}
		
			active[e / 32].set(e % 32);
			return handle_T{ e };
		}
		void destroy(handle_T e)
		{
			active[e / 32].set(e % 32, false);
			inactive.push_back(e);
		}
		bool alive(handle_T e) 
		{
			return active[(uint32_t)e / 32].test(e % 32);
		}
	private:
		std::vector<std::bitset<4096>> active;
		std::vector<handle_T> inactive;
		uint32_t next = 0;
	};
}
