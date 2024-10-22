#pragma once
#include "traits.h"
#include "proxy_ref.h"
#include "macros.h"
#include <stdexcept>

namespace ecs {
	template<typename T, typename reg_T>
	class pool_iterator {
	public:
		using handle_type = traits::get_component_handle_t<std::remove_const_t<T>>;

		using iterator_category = std::random_access_iterator_tag;
		using value_type = std::tuple<handle_type, std::remove_const_t<T>&>;
		using reference = proxy_ref<std::conditional_t<std::is_const_v<T>, const value_type, value_type>>;
		using difference_type = std::ptrdiff_t;

		operator pool_iterator<const T, reg_T>() const { return { const_cast<reg_T*>(reg), ind }; }

		constexpr reference operator*() const { 
			return value_type{
				const_cast<reg_T*>(reg)->template get_manager<std::remove_const_t<T>>()[ind],
				const_cast<reg_T*>(reg)->template get_storage<std::remove_const_t<T>>()[ind]
			};
		}
		constexpr reference operator[](difference_type n) const { 
			n += ind;
			return value_type{
				const_cast<reg_T*>(reg)->template get_manager<std::remove_const_t<T>>()[n],
				const_cast<reg_T*>(reg)->template get_storage<std::remove_const_t<T>>()[n]
			}; 
		}

		FORCE_INLINE constexpr bool operator==(const pool_iterator& other) const { return ind == other.ind; }
		FORCE_INLINE constexpr bool operator!=(const pool_iterator& other) const { return ind != other.ind; }
		FORCE_INLINE constexpr bool operator<(const pool_iterator& other) const  { return ind < other.ind; }
		FORCE_INLINE constexpr bool operator>(const pool_iterator& other) const  { return ind > other.ind; }
		FORCE_INLINE constexpr bool operator<=(const pool_iterator& other) const { return ind <= other.ind; }
		FORCE_INLINE constexpr bool operator>=(const pool_iterator& other) const { return ind >= other.ind; }

		FORCE_INLINE constexpr pool_iterator& operator++() { ++ind; return *this; }
		FORCE_INLINE constexpr pool_iterator& operator--() { --ind; return *this; }
		FORCE_INLINE constexpr pool_iterator operator++(int) { auto tmp = *this; ++ind; return tmp; }
		FORCE_INLINE constexpr pool_iterator operator--(int) { auto tmp = *this; --ind; return tmp; }
		FORCE_INLINE constexpr pool_iterator& operator+=(difference_type n) { ind += n; return *this; }
		FORCE_INLINE constexpr pool_iterator& operator-=(difference_type n) { ind -= n; return *this; }
		
		FORCE_INLINE constexpr pool_iterator operator+(difference_type n) const { auto tmp = *this; return tmp += n; }
		FORCE_INLINE constexpr pool_iterator operator-(difference_type n) const { auto tmp = *this; return tmp -= n; }

		FORCE_INLINE constexpr difference_type operator-(const pool_iterator& other) const { return ind - other.ind; }

		size_t index() const { return ind; }
	private:
		size_t ind;
		reg_T* reg;
	};

	template<typename T, typename reg_T>
	pool_iterator<T, reg_T> operator+(typename pool_iterator<T, reg_T>::difference_type n, const pool_iterator<T, reg_T>& it) { 
		return it.ind + n;
	}
}

namespace ecs {
	template<typename T, typename reg_T>
	class pool {
	public:


		using registry_type = reg_T;
		using handle_type = handle_t<std::remove_const_t<T>>;
		using manager_type = std::conditional_t<std::is_const_v<T>, 
			const traits::get_component_manager_t<std::remove_const_t<T>>, 
			traits::get_component_manager_t<T>>;
		using indexer_type = std::conditional_t<std::is_const_v<T>, 
			const traits::get_component_indexer_t<std::remove_const_t<T>>, 
			traits::get_component_indexer_t<T>>;
		using storage_type = std::conditional_t<std::is_const_v<T>, 
			const traits::get_component_storage_t<std::remove_const_t<T>>, 
			traits::get_component_storage_t<T>>;
		
		using value_type = std::tuple<handle_type, std::remove_const_t<T>&>;
		using reference = proxy_ref<value_type>;
		using const_reference = proxy_ref<const value_type>;
		using iterator = pool_iterator<T, reg_T>;
		using const_iterator = pool_iterator<const T, reg_T>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		
		constexpr pool(reg_T* reg) noexcept : reg(reg) { }

		// iterators
		constexpr iterator begin() noexcept { return { reg, 0 }; }
		constexpr const_iterator begin() const noexcept { return { reg, 0 }; }
		constexpr const_iterator cbegin() const noexcept { return { reg, 0 }; }

		constexpr iterator end() noexcept { return { reg, reg->template get_manager<T>().size() }; }
		constexpr const_iterator end() const noexcept { return { reg, reg->template get_manager<T>().size() }; }
		constexpr const_iterator cend() const noexcept { return { reg, reg->template get_manager<T>().size() }; }
		
		constexpr reverse_iterator rbegin() noexcept { return iterator{ reg, reg->template get_manager<T>().size()-1 }; } 
		constexpr const_reverse_iterator rbegin() const noexcept { return const_iterator{ reg, reg->template get_manager<T>().size()-1 }; }
		constexpr const_reverse_iterator crbegin() const noexcept { return const_iterator{ reg, reg->template get_manager<T>().size()-1 }; }
		
		constexpr reverse_iterator rend() noexcept { return iterator{ reg, -1 }; }
		constexpr const_reverse_iterator rend() const noexcept { return const_iterator{ reg, -1 }; }
		constexpr const_reverse_iterator crend() const noexcept { return const_iterator{ reg, -1 }; }

		// capacity
		[[nodiscard]] FORCE_INLINE constexpr bool empty() const noexcept { return reg->template get_manager<T>().size() == 0; }
		FORCE_INLINE constexpr size_t max_size() const noexcept { return reg->template get_manager<T>().max_size(); }
		FORCE_INLINE constexpr size_t size() const { return reg->template get_manager<T>().size(); }
		FORCE_INLINE constexpr size_t capacity() const { return reg->template get_manager<T>().capacity(); }
		constexpr void reserve(size_t n) { 
			reg->template get_manager<T>().reserve(n);
			reg->template get_storage<T>().reserve(n);
		}
		constexpr void shrink_to_fit() {
			size_t n = size();
			reg->template get_manager<T>().shrink_to_fit(n);
			reg->template get_storage<T>().shrink_to_fit(n);
		}
		
		// element/page/data access
		constexpr reference operator[](size_t index) {
			return { reg->template get_manager<T>()[index], reg->template get_storage<T>()[index] }; 
		}
		constexpr const reference operator[](size_t index) const {
			return { reg->template get_manager<T>()[index], reg->template get_storage<T>()[index] }; 
		}
		
		constexpr reference operator[](handle_type e) {
			auto index = reg->template get_indexer<T>()[e.value()].value();
			return { reg->template get_manager<T>()[index], reg->template get_storage<T>()[index] }; 
		}
		constexpr const reference operator[](handle_type e) const {
			auto index = reg->template get_indexer<T>()[e.value()].value();
			return { reg->template get_manager<T>()[index], reg->template get_storage<T>()[index] }; 
		}
		
		constexpr reference at(size_t pos) {
			if (size() >= pos) throw std::out_of_range("");
			return { reg->template get_manager<T>()[pos], reg->template get_storage<T>()[pos] }; 
		}
		constexpr const reference at(size_t pos) const {
			if (size() >= pos) throw std::out_of_range("");
			return { reg->template get_manager<T>()[pos], reg->template get_storage<T>()[pos] }; 
		}

		constexpr T& get(handle_type e) {
			if (!contains(e)) throw std::out_of_range("");
			size_t index = index_of(e);
			return { reg->template get_manager<T>()[index], reg->template get_storage<T>()[index] }; 
		}
		constexpr const T& get(handle_type e) const {
			if (!contains(e)) throw std::out_of_range("");
			size_t index = index_of(e);
			return { reg->template get_manager<T>()[index], reg->template get_storage<T>()[index] }; 
		}

		constexpr reference front() { 
			return { reg->template get_manager<T>()[0], reg->template get_storage<T>()[0] }; 
		}
		constexpr const reference front() const {
			return { reg->template get_manager<T>()[0], reg->template get_storage<T>()[0] };
		}
		
		constexpr reference back() {
			size_t index = size();
			return { reg->template get_manager<T>()[index], reg->template get_storage<T>()[index] };
		}
		constexpr const reference back() const {
			size_t index = size();
			return { reg->template get_manager<T>()[index], reg->template get_storage<T>()[index] };			
		}
		
		constexpr size_t index_of(handle_type e) const {
			auto val = e.value();
			if (e.value() >= reg->template get_indexer<T>().size()) throw std::out_of_range("");
			if (e.version() != reg->template get_indexer<T>()[e.value()].version()) throw std::out_of_range("");
			return reg->template get_indexer<T>()[e.value()].value();
		}

		[[nodiscard]] bool contains(handle_type e) const {
			return e.value() < reg->template get_indexer<T>().size() && e.version() == reg->template get_indexer<T>()[e.value()].version();
		}

		// modifiers
		template<typename ... arg_Ts> requires (std::is_constructible_v<T, arg_Ts...>)
		constexpr T& init(handle_type e, arg_Ts&&... args)
		{
			auto& mng = reg->template get_manager<T>();
			auto& ind = reg->template get_indexer<T>();
			auto& str = reg->template get_storage<T>();
			auto& inv = reg->template on<ecs::init<T>>();

			ind.resize(e.value() + 1, handle_type{ tombstone{} });
			ind[e.value()] = handle_type{ (unsigned int)mng.size(), e.version() };
			mng.emplace_back(e);

			T& component = str.emplace_back(std::forward<arg_Ts>(args)...);
			inv.invoke({ e, component });
			return component;
		}

		constexpr void terminate(handle_type e) {
			auto& mng = reg->template get_manager<T>();
			auto& ind = reg->template get_indexer<T>();
			auto& str = reg->template get_storage<T>();
			auto& inv = reg->template on<ecs::terminate<T>>();
			
			auto back = mng.size() - 1;
			inv.invoke({e, str[back]});
			
			auto curr = ind[e.value()].value();
			ind[e.value()] = tombstone{};
			
			if (curr != back) {
				std::swap(str[curr], str[back]);
				std::swap(mng[curr], mng[back]);
			}
			str.pop_back();
			mng.pop_back();
		}
		
		constexpr void clear() noexcept {
			auto& mng = reg->template get_manager<T>();
			auto& ind = reg->template get_indexer<T>();
			auto& str = reg->template get_storage<T>();
			auto& inv = reg->template on<ecs::terminate<T>>();
			
			for (int i=0; i<size(); ++i) {
				inv.invoke({mng[i], str[i]});
				ind[i] = tombstone{};
			}
			mng.clear();
			str.clear();
		}

		template<typename ... arg_Ts>
		T* try_init(handle_type e, arg_Ts&& ... args) {
			if (contains(e)) return nullptr;
			return &init(e, std::forward<arg_Ts>(args)...);
		}

		bool try_terminate(handle_type e) {
			if (!contains(e)) return false;
			terminate(e); 
			return true;
		}

	private:
		reg_T* reg;
	};
}
