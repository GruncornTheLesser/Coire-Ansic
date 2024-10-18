#pragma once
#include "proxy_ref.h"
#include "traits.h"

namespace ecs {
	template<typename ... Ts>
	struct select {
		using handle_type = util::eval_t<std::tuple<Ts...>, util::get_front, util::eval_if_<traits::is_handle, traits::get_handle>::template inverse, std::remove_cv>;
		using value_type = util::eval_each_t<std::tuple<Ts...>, util::eval_if_<traits::is_handle, std::remove_cv, std::add_lvalue_reference>::template type>;
		
		static_assert(util::pred::allof_v<std::tuple<Ts...>, util::pred::disj_<
			util::cmp::to_<handle_type, util::cmp::is_ignore_cv_same>::template type, 
			util::cmp::to_<handle_type, util::cmp::is_ignore_cv_same, traits::get_handle>::template type
			>::template type>);

		template<typename reg_T, typename from_T>
		static inline value_type retrieve(from_T, reg_T* reg, size_t ind, handle_type ent) {
			return { from_T::template get<Ts>(reg, ind, ent)... };
		}
	};

	template<typename T>
	struct from {
		using from_type = std::remove_cv_t<T>;
		using handle_type = traits::get_handle_t<T>;

		template<typename U, typename reg_T, typename ent_T> requires (!std::is_same_v<std::decay_t<U>, std::decay_t<ent_T>>)
		static inline U& get(reg_T* reg, size_t ind, ent_T ent) {
			return reg->template get_storage<U>()[ind];
		}

		template<typename U, typename reg_T>
		static inline U get(reg_T* reg, size_t ind, U ent) {
			return ent;
		}
	};

	template<typename ... Ts>
	struct where { 
		using handle_type = util::eval_t<std::tuple<Ts...>, util::get_front, traits::get_handle>;
		
		template<typename reg_T>
		static inline bool validate(reg_T* reg, handle_type e) {
			return (Ts::validate(reg, e) && ...);
		}
	};
	
	template<typename ... Ts>
	struct include {
		using handle_type = util::eval_t<std::tuple<Ts...>, util::get_front, traits::get_handle>;
		
		template<typename reg_T>
		static inline bool validate(reg_T* reg, handle_type e) {
			return (reg->template has<Ts>(e) && ...);
		}
	};

	template<typename ... Ts>
	struct exclude { 
		using handle_type = util::eval_t<std::tuple<Ts...>, util::get_front, traits::get_handle>;
		
		template<typename reg_T>
		static inline bool validate(reg_T* reg, handle_type e) {
			return !(reg->template has<Ts>(e) || ...);
		}
	};

	template<typename select_T>
	using default_from = util::eval_t<select_T, util::filter_<traits::is_handle>::template inverse, util::get_front, util::wrap_<from>::template type>;

	template<typename select_T, typename from_T>
	using default_where = util::eval_t<select_T, util::filter_<util::pred::disj_<traits::is_handle, 
		util::cmp::to_<ecs::manager<typename from_T::from_type>, util::cmp::is_ignore_cv_same, traits::get_manager>::template type
		>::template type>::template inverse, util::rewrap_<ecs::include>::template type>;
};

namespace ecs {
	struct view_sentinel;

	template<typename select_T, typename from_T, typename where_T, typename reg_T>
	class view_iterator {
		using handle_type = typename select_T::handle_type;
		using from_type = typename from_T::from_type;
	public:
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = typename select_T::value_type;
		using reference = proxy_ref<value_type>;
		using difference_type = std::ptrdiff_t;
		using sentinel_type = view_sentinel;
		
		view_iterator() : reg(nullptr), ind(-1), ent() { }
		view_iterator(reg_T* reg, size_t ind) : reg(reg), ind(ind), ent() { }
		view_iterator(const view_iterator& other) : reg(other.reg), ind(other.ind), ent(other.ent) { }
		view_iterator& operator=(const view_iterator& other) { reg = other.reg; ind = other.ind; ent = other.ent; }
		view_iterator(view_iterator&& other) : reg(other.reg), ind(other.ind), ent(other.ent) { }
		view_iterator& operator=(view_iterator&& other) { reg = other.reg; ind = other.ind; ent = other.ent; }

		constexpr reference operator*() const {
			return select_T::retrieve(from_T{}, reg, ind, ent);
		}
		
		constexpr view_iterator& operator++() {
			while (++ind != reg->template get_manager<from_type>().size()) {
				ent = reg->template get_manager<from_type>()[ind];
				if (where_T::validate(reg, ent)) 
					return *this;
			}
			ind = -1;
			return *this;
		}
		constexpr view_iterator& operator--() {
			while (++ind != 0) { 
				ent = reg->template get_manager<from_type>()[ind]; 
				if (where_T::validate(reg, ent)) 
					return *this;
			}
			ind = -1;
			return *this;
		}
		
		constexpr view_iterator operator++(int) { auto tmp = *this; ++ind; return tmp; }
		constexpr view_iterator operator--(int) { auto tmp = *this; --ind; return tmp; }

		constexpr difference_type operator-(const view_iterator& other) const { return ind - other.ind; }

		constexpr bool operator==(const view_iterator& other) const { return ind == other.ind; }
		constexpr bool operator!=(const view_iterator& other) const { return ind != other.ind; }

		constexpr bool operator==(const view_sentinel& other) const { return ind == -1; }
		constexpr bool operator!=(const view_sentinel& other) const { return ind != -1; }

	private:
		handle_type ent;
		size_t      ind;
		reg_T*      reg;
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
		using handle_type = handle<util::unwrap_t<from_T>>;

		static_assert(util::pred::allof_v<select_T, util::pred::disj_<
				util::pred::conj_<traits::is_handle, util::cmp::to_<handle_type, util::cmp::is_ignore_cv_same>::template type>::template type, 
				util::cmp::to_<handle_type, util::cmp::is_ignore_cv_same, traits::get_handle>::template type
			>::template type>);
	
		using iterator = view_iterator<select_T, from_T, where_T, reg_T>;
		using const_iterator = view_iterator<const select_T, from_T, where_T, reg_T>;
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
		constexpr reverse_iterator rbegin() noexcept { return --iterator{ reg, reg->template pool<from_T>().size() }; } 
		constexpr const_reverse_iterator rbegin() const noexcept { return --const_iterator{ reg, reg->template pool<from_T>().size() }; }
		constexpr const_reverse_iterator crbegin() const noexcept { return --const_iterator{ reg, reg->template pool<from_T>().size() }; }
		constexpr view_sentinel rend() noexcept { return { }; }
		constexpr view_sentinel rend() const noexcept { return { }; }
		constexpr view_sentinel crend() const noexcept { return { }; }
	private:
		reg_T* reg;
	};
}