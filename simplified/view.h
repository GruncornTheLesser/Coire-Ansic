#pragma once
#include "proxy_ref.h"
#include "registry.h"
#include "traits.h"

namespace ecs {
	
	template<typename ... Ts>
	struct select { };

	template<typename T>
	struct from { };

	template<typename ... T>
	struct where { };
	
	template<typename ... T>
	struct inc { };

	template<typename ... T>
	struct exc { };
	
	template<typename select_T, typename from_T, typename where_T, typename reg_T>
	class view_iterator;
	
	
	template<typename ... select_Ts, typename from_T, typename ... where_Ts, typename reg_T>
	class view_iterator<select<select_Ts...>, from<from_T>, where<where_Ts...>, reg_T> {
		using entity_type = entity<from_T>;
		static_assert((std::is_same_v<entity<select_Ts>, entity_type> && ...));
		
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = std::tuple<entity_type, select_Ts&...>;
		using reference = proxy_ref<value_type>;
		using difference_type = std::ptrdiff_t;

		view_iterator() : reg(nullptr), ind(0) { }
		view_iterator(reg_T* reg, size_t ind) : reg(reg), ind(ind) { }
		view_iterator(const view_iterator& other) : reg(other.reg), ind(other.ind) { }
		view_iterator& operator=(const view_iterator& other) { reg = other.reg; ind = other.ind; }
		view_iterator(view_iterator&& other) : reg(other.reg), ind(other.ind) { }
		view_iterator& operator=(view_iterator&& other) { reg = other.reg; ind = other.ind; }

		constexpr reference operator*() const;
		constexpr bool operator==(const view_iterator& other) const { return ind == other.ind; }
		constexpr bool operator!=(const view_iterator& other) const { return ind != other.ind; }
		
		constexpr view_iterator& operator++() { ++ind; return *this; }
		constexpr view_iterator& operator--() { --ind; return *this; }
		constexpr view_iterator operator++(int) { auto tmp = *this; ++ind; return tmp; }
		constexpr view_iterator operator--(int) { auto tmp = *this; --ind; return tmp; }
	
		constexpr difference_type operator-(const view_iterator& other) const { return ind - other.ind; }
	private:
		size_t ind;
		reg_T* reg;
	};
}

#ifdef _DEBUG
static_assert(std::bidirectional_iterator<ecs::view_iterator<ecs::select<int>, ecs::from<int>, ecs::inc<int>, ecs::registry<int>>>);
#endif

namespace ecs {
	template<typename select_T, typename from_T, typename where_T, typename reg_T>
	class view {
	public:
		using iterator = view_iterator<select_T, from_T, where_T, reg_T>;
		using const_iterator = view_iterator<util::eval_each_t<select_T, std::add_const>, from_T, where_T, reg_T>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	

		constexpr iterator begin() noexcept { return { reg, 0 }; }
		constexpr const_iterator begin() const noexcept { return { reg, 0 }; }
		constexpr const_iterator cbegin() const noexcept { return { reg, 0 }; }

		constexpr iterator end() noexcept { return { reg, reg->template pool<from_T>().size() }; }
		constexpr const_iterator end() const noexcept { return { reg, reg->template pool<from_T>().size() }; }
		constexpr const_iterator cend() const noexcept { return { reg, reg->template pool<from_T>().size() }; }
		
		constexpr reverse_iterator rbegin() noexcept { return iterator{ reg, reg->template pool<from_T>().size()-1 }; } 
		constexpr const_reverse_iterator rbegin() const noexcept { return const_iterator{ reg, reg->template pool<from_T>().size()-1 }; }
		constexpr const_reverse_iterator crbegin() const noexcept { return const_iterator{ reg, reg->template pool<from_T>().size()-1 }; }
		
		constexpr reverse_iterator rend() noexcept { return iterator{ reg, -1 }; }
		constexpr const_reverse_iterator rend() const noexcept { return const_iterator{ reg, -1 }; }
		constexpr const_reverse_iterator crend() const noexcept { return const_iterator{ reg, -1 }; }
	private:
		reg_T* reg;
	};





}