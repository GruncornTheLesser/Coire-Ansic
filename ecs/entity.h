#pragma once
#include <cstdint>
#include <vector>
#include <bitset>
#include <concepts>
#include "../util/chain_vector.h"
#include "traits.h"
namespace ecs {
	template<typename T>
	class handle_generator;

	template<std::unsigned_integral T=uint32_t>
	struct entity
	{
		friend class handle_generator;
		using component_tag = tags::handletype<entity>;

	private:
	 	entity(T value, T version) : value(value), version(version) { }
	public:
		entity() : version(0) { }
		operator uint32_t() { return value; }
		bool operator==(const entity& other) const { return value == other.value && version == other.version; }
	private:
		T value;
		T version;
	};
	const entity tombstone{};


	template<typename T>
	class handle_generator
	{
	public:
		using component_tag = tags::resource;

		[[nodiscard]] entity create()
		{
			uint32_t value;
			uint32_t version;

			if (entities.chain().empty())
			{
				value = entities.size();
				version = 1;
				entities.emplace_back(version);
			}
			else
			{
				value = (entities.chain().begin() - entities.begin());
				version = entities.chain().front();
				entities.chain().pop_front();
			}

			return { value, version };
		}
		void destroy(entity e)
		{
			auto it = entities.begin() + e.value;
			++(*it);
			entities.chain().insert(it);
		}
		bool alive(entity e) const
		{
			return e.version == entities[e.value];
		}
	private:
		util::chain_vector<uint32_t> entities;
	};
}
