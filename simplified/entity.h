#pragma once
#include <concepts>
#include <stdint.h>
#include <vector>
#include "traits.h"

namespace ecs {	
	struct tombstone { };

	//template<std::unsigned_integral T, std::unsigned_integral V>
	struct entity
	{	
using component_tag = ecs::tags::handletype<entity>;

		template<typename,typename> friend class handle_manager;
		template<typename, typename> friend class pool;
		
		using value_type = uint32_t;
		using version_type = uint32_t;

	private:
	 	entity(value_type value, version_type version) : val(value), ver(version) { }
		entity(tombstone) : ver(0) { }
		entity& operator=(tombstone) { ver = 0; return *this; }
		void destroy() { ver = 0; }
	public:
		entity() : val(0), ver(0) { }
		entity(const entity& other) = default;
		entity& operator=(const entity& other) = default;
		operator uint32_t() const { 
			return val; 
		}

		bool operator==(const entity& other) const { 
			return val == other.val && ver == other.ver; 
		}

		bool operator!=(const entity& other) const { 
			return val == other.val && ver == other.ver; 
		}

		bool operator==(tombstone other) const { 
			return ver == 0;
		}

		bool operator!=(tombstone other) const { 
			return ver != 0; 
		}
	
		value_type value() const {
			return val;
		}

		version_type version() const {
			return ver;
		}
	private:
		value_type val;
		version_type ver;
	};
}
