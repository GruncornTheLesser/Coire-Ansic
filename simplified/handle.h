#pragma once
#include <concepts>
#include <stdint.h>
#include <vector>
#include "traits.h"

namespace ecs {	
	struct tombstone { };

	template<std::unsigned_integral T, std::unsigned_integral V>
	struct handle
	{
		using component_tag = ecs::tags::handletype<handle<T, V>>;

		template<typename,typename> friend class entity_manager;
		template<typename...> friend class static_registry;
		
		using value_type = T;
		using version_type = V;

	private:
	 	handle(value_type value, version_type version) : val(value), ver(version) { }
		handle(tombstone) : ver(0) { }
		handle& operator=(tombstone) { ver = 0; return *this; }
		void destroy() { ver = 0; }
	public:
		handle() : val(0), ver(0) { }
		handle(const handle& other) = default;
		handle& operator=(const handle& other) = default;
		operator uint32_t() const { 
			return val; 
		}

		bool operator==(const handle& other) const { 
			return val == other.val && ver == other.ver; 
		}

		bool operator!=(const handle& other) const { 
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
