#pragma once
#include <stdint.h>
#include <vector>
#include <bitset>

namespace ecslite 
{
	using entity = uint32_t;

	constexpr uint32_t tombstone = static_cast<uint32_t>(-1);

	template<typename T>
	class pool
	{
	public:
		entity operator[](int index) { return packed[index]; }
		bool has(entity e) { return e < sparse.size() && sparse[e] != tombstone; }
		T& get(entity e) { return storage[sparse[e]]; }
		size_t size() { return packed.size(); }
		
		T& emplace_back(entity e, T&& value) 
		{
			if (e >= sparse.size()) sparse.resize(e + 1);
			sparse[e] = packed.size();
			packed.push_back(e);
			return storage.emplace_back(std::move(value));
		}
		//T& insert(pool::iterator pos, entity e, T&& value); 
		
		void erase(entity e) 
		{
			size_t index = sparse[e];
			sparse[e] = -1;

			std::swap(storage.back(), storage[index]);
			storage.pop_back();
			
			packed[index] = packed.back();
			packed.pop_back();
		}

	private:
		std::vector<uint32_t> sparse;
		std::vector<entity> packed;
		std::vector<T> storage;
	};

	template<typename reg_T, typename T, typename ... Ts>
	class view_iterator
	{
	public:
		using return_set = std::tuple<ecslite::entity, T&, Ts&...>;

		view_iterator(reg_T& reg, uint32_t i) : reg(reg), index(i) { }
		view_iterator<reg_T, T, Ts...>& operator*() { return *this; }
		view_iterator<reg_T, T, Ts...>& operator++() 
		{
			do
			{
				if (++index == reg.template get_pool<T>().size()) 
				{
					break;
				}
					
				e = reg.template get_pool<T>()[index];
			} while (!(reg.template get_pool<Ts>().has(e) && ...));
			
			return *this;
		}
		bool operator==(view_iterator<reg_T, T, Ts...>& other)
		{
			return index == other.index; 
		}
		bool operator!=(view_iterator<reg_T, T, Ts...>& other)
		{
			return index != other.index;
		}

		template<size_t N>
		std::tuple_element_t<N, return_set> get() 
		{
			if constexpr (N == 0) 
				return e;
			else
				return reg.template get_pool<std::remove_reference_t<std::tuple_element_t<N, return_set>>>().get(e);
		}
	private:
		entity e;
		uint32_t index;
		reg_T& reg;
	};

	template<typename reg_T, typename T, typename ... Ts>
	class view
	{
	public:
		view(reg_T& reg) : reg(reg) { }
		view_iterator<reg_T, T, Ts...> begin() 
		{
			return ++view_iterator<reg_T, T, Ts...>{ reg, static_cast<uint32_t>(-1) };
		}
		view_iterator<reg_T, T, Ts...> end() 
		{
			return view_iterator<reg_T, T, Ts...>{ reg, reg.template get_pool<T>().size() };
		}

	private:
		reg_T& reg;
	};

	template<typename ... Ts>
	class registry : public handle_manager
	{
	public:
		registry() { }

		template<typename T>
		pool<T>& get_pool() { return std::get<pool<T>>(pools); }

		template<typename ... Us>
		view<registry<Ts...>, Us...> get_view() { return { *this }; }

		entity create() 
		{ 
			uint32_t e;
			if (inactive.size() > 0)
			{
				e = inactive.back();
				inactive.pop_back();
			}
			else 
			{
				if (next == active.size() >> 32) active.emplace_back(); // adds 512 capacity
				e = next++;
			}
		
			active[e / 32].set(e % 32);
			return entity{ e };
		}
	
		void destroy(entity e)
		{
			active[e / 32].set(e % 32, false);
			inactive.push_back(e);
		}
	
		bool alive(entity e) 
		{
			return active[(uint32_t)e / 32].test(e % 32);
		}
	
	private:
		std::tuple<pool<Ts>...> pools;
		std::vector<std::bitset<4096>> active;
		std::vector<entity> inactive;
		uint32_t next = 0;
	};
}

// view reference structured binding 
template<typename reg_T, typename T, typename ... Ts>
struct std::tuple_size<ecslite::view_iterator<reg_T, T, Ts...>>
 : std::tuple_size<typename ecslite::view_iterator<reg_T, T, Ts...>::return_set> { };

template<size_t N, typename reg_T, typename T, typename ... Ts>
struct std::tuple_element<N, ecslite::view_iterator<reg_T, T, Ts...>>
 : std::tuple_element<N, typename ecslite::view_iterator<reg_T, T, Ts...>::return_set> { };
