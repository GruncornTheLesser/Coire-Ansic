#pragma once
#include "traits.h"
#include "util\proxy_ref.h"
#include "util\tuple.h"
#include "util\tuple_util\tuple_util.h"
namespace ecs {
	namespace traits {
		template<typename T> using get_component_manager = std::type_identity<typename component_traits<T>::manager_type>;
		
		
		template<typename select_T>
		using default_from_t = util::eval_t<select_T, util::filter_<traits::is_handle>::template inverse, util::get_front, util::wrap_<from>::template type>;

		template<typename select_T, typename from_T>
		using default_where_t = util::eval_t<select_T, util::filter_<traits::is_handle>::template inverse, util::filter_<
			util::cmp::to_<ecs::manager<util::unwrap_t<from_T>>, util::cmp::is_ignore_cv_same, traits::get_component_manager>::template type>::template inverse, 
			util::rewrap_<ecs::include>::template type, util::wrap_<ecs::where>::template type>;
	}

	template<typename ... Ts>
	struct select {
		using value_type = util::propagate_cv_each_t<select<Ts...>, util::eval_<util::eval_each_<util::eval_if_<
			traits::is_handle, std::remove_cv, std::add_lvalue_reference>::template type>::template type, 
			util::rewrap_<util::tuple>::template type>::template type>;

		template<typename reg_T, typename handle_T>
		constexpr inline value_type operator()(reg_T* reg, size_t pos, handle_T ent) const {
			return { get<std::remove_const_t<Ts>>(reg, pos, ent)... };
		}
	private:
		template<traits::component_class T, typename reg_T, typename handle_T>
		constexpr static inline T& get(reg_T* reg, size_t pos, handle_T ent) {
			return reg->template get<get_storage_t<T>>()[pos];
		}

		template<traits::handle_class handle_T, typename reg_T>
		constexpr static inline handle_T get(reg_T* reg, size_t pos, handle_T ent) {
			return ent;
		}
	};

	template<typename ... Ts>
	struct from {};

	template<typename ... Ts>
	struct include {
		template<typename reg_T, typename handle_T>
		constexpr inline bool operator()(reg_T* reg, handle_T ent) const {
			return (reg->template pool<Ts>().contains(ent) && ...);
		}
	};
	template<>
	struct include<> {
		template<typename reg_T, typename handle_T>
		constexpr inline bool operator()(reg_T* reg, handle_T ent) const {
			return true;
		}
	};


	template<typename ... Ts>
	struct exclude {
		template<typename reg_T, typename handle_T>
		constexpr inline bool operator()(reg_T* reg, handle_T ent) const {
			return !(reg->template pool<Ts>().contains(ent) || ...);
		}
	};
	template<>
	struct exclude<> {
		template<typename reg_T, typename handle_T>
		constexpr inline bool operator()(reg_T* reg, handle_T ent) const {
			return true;
		}
	};

	template<typename ... Ts>
	struct where {
		template<typename reg_T, typename handle_T>
		constexpr inline bool operator()(reg_T* reg, handle_T handle) {
			return (Ts{}(reg, handle) && ...);
		}
	};
}
namespace ecs {
	template<typename select_T, typename from_T, typename where_T, typename reg_T>
	class view_iterator {
	public:
		using from_type = typename view<select_T, from_T, where_T, reg_T>::from_type;
		using handle_type = typename view<select_T, from_T, where_T, reg_T>::handle_type;
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = typename select_T::value_type;
		using reference = util::proxy_ref<value_type>;
		using difference_type = std::ptrdiff_t;
		using sentinel_type = view_sentinel;
		
		view_iterator() : reg(nullptr), pos(-1), ent() { }
		view_iterator(reg_T* reg, size_t pos) : reg(reg), pos(pos), ent() { }
		view_iterator(const view_iterator& other) : reg(other.reg), pos(other.pos), ent(other.ent) { }
		view_iterator& operator=(const view_iterator& other) { reg = other.reg; pos = other.pos; ent = other.ent; }
		view_iterator(view_iterator&& other) : reg(other.reg), pos(other.pos), ent(other.ent) { }
		view_iterator& operator=(view_iterator&& other) { reg = other.reg; pos = other.pos; ent = other.ent; }

		constexpr reference operator*() const {
			return select_T{}(reg, pos, ent);
		}
		
		constexpr view_iterator& operator++() {
			while (++pos != reg->template count<from_type>()) {
				ent = reg->template get<get_manager_t<from_type>>()[pos];
				if (where_T{}(reg, ent))
					return *this;
			}
			pos = static_cast<size_t>(-1);
			return *this;
		}
		constexpr view_iterator& operator--() {
			while (++pos != 0) { 
				ent = reg->template get<get_manager_t<from_type>>()[pos]; 
				if (where_T{}(reg, ent))
					return *this;
			}
			pos = static_cast<size_t>(-1);
			return *this;
		}
		
		constexpr view_iterator operator++(int) { auto tmp = *this; ++pos; return tmp; }
		constexpr view_iterator operator--(int) { auto tmp = *this; --pos; return tmp; }

		constexpr difference_type operator-(const view_iterator& other) const { return pos - other.pos; }

		constexpr bool operator==(const view_iterator& other) const { return pos == other.pos; }
		constexpr bool operator!=(const view_iterator& other) const { return pos != other.pos; }

		constexpr bool operator==(const view_sentinel& other) const { return pos == static_cast<size_t>(-1); }
		constexpr bool operator!=(const view_sentinel& other) const { return pos != static_cast<size_t>(-1); }

	private:
		reg_T*      reg;
		size_t      pos;
		handle_type ent;
	};

	struct view_sentinel {
		constexpr bool operator==(const view_sentinel& other) const { return true; }
		constexpr bool operator!=(const view_sentinel& other) const { return false; }
		template<typename T> constexpr bool operator==(const T& other) const { return other == *this; }
		template<typename T> constexpr bool operator!=(const T& other) const { return other != *this; }
	};
}


namespace ecs {
	template<typename select_T, typename from_T, typename where_T, typename reg_T>
	class view {
	public:
		using from_type = std::remove_const_t<util::unwrap_t<from_T>>;
		using handle_type = get_handle_t<from_type>;

		using iterator = view_iterator<select_T, from_T, where_T, reg_T>;
		using const_iterator = view_iterator<util::eval_each_t<select_T, std::add_const>, from_T, where_T, reg_T>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using sentinel_type = view_sentinel;

		view(reg_T* reg) : reg(reg) { }

		constexpr iterator begin() noexcept { return ++iterator{ reg, static_cast<size_t>(-1) }; }
		constexpr const_iterator begin() const noexcept { return ++const_iterator{ reg, static_cast<size_t>(-1) }; }
		constexpr const_iterator cbegin() const noexcept { return ++const_iterator{ reg, static_cast<size_t>(-1) }; }
		constexpr view_sentinel end() noexcept { return { }; }
		constexpr view_sentinel end() const noexcept { return { }; }
		constexpr view_sentinel cend() const noexcept { return { }; }
		constexpr reverse_iterator rbegin() noexcept { return --iterator{ reg, reg->template pool<from_type>().size() }; } 
		constexpr const_reverse_iterator rbegin() const noexcept { return --const_iterator{ reg, reg->template pool<from_type>().size() }; }
		constexpr const_reverse_iterator crbegin() const noexcept { return --const_iterator{ reg, reg->template pool<from_type>().size() }; }
		constexpr view_sentinel rend() noexcept { return { }; }
		constexpr view_sentinel rend() const noexcept { return { }; }
		constexpr view_sentinel crend() const noexcept { return { }; }
	private:
		reg_T* reg;
	};
}