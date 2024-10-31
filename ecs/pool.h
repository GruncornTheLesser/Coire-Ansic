#pragma once
#include <stdexcept>
#include "traits.h"
#include "util\proxy_ref.h"
#include "resource.h"

namespace ecs {
	template<typename T> struct manager : restricted_resource, std::vector<typename handle_traits<get_handle_t<T>>::value_type> { };
	template<typename T> struct indexer : restricted_resource, std::vector<typename handle_traits<get_handle_t<T>>::value_type> { };
	template<typename T> struct storage : restricted_resource, std::vector<T> { };
}

namespace ecs {
	template<typename T, typename reg_T>
	class pool_iterator {
	public:
		using handle_type = typename component_traits<std::remove_const_t<T>>::handle_type;
		using manager_type = typename component_traits<std::remove_const_t<T>>::manager_type;
		using indexer_type = typename component_traits<std::remove_const_t<T>>::indexer_type;
		using storage_type = typename component_traits<std::remove_const_t<T>>::storage_type;

		using iterator_category = std::random_access_iterator_tag;
		using value_type = std::tuple<handle_type, std::remove_const_t<T>&>;
		using reference = util::proxy_ref<std::conditional_t<std::is_const_v<T>, const value_type, value_type>>;
		using difference_type = std::ptrdiff_t;

		pool_iterator() = default;
		pool_iterator(size_t pos, reg_T* reg) : pos(pos), reg(reg) { }
		
		pool_iterator(const pool_iterator& other) = default;
		pool_iterator& operator=(const pool_iterator& other) = default;
		pool_iterator(pool_iterator&& other) = default;
		pool_iterator& operator=(pool_iterator&& other) = default;

		operator pool_iterator<const T, reg_T>() const { return { const_cast<reg_T*>(reg), pos }; }

		constexpr reference operator*() const {
			return value_type{
				const_cast<reg_T*>(reg)->template get<manager_type>()[pos],
				const_cast<reg_T*>(reg)->template get<storage_type>()[pos]
			};
		}
		constexpr reference operator[](difference_type n) const { 
			n += pos;
			return value_type{
				const_cast<reg_T*>(reg)->template get<manager_type>()[n],
				const_cast<reg_T*>(reg)->template get<storage_type>()[n]
			}; 
		}

		constexpr inline bool operator==(const pool_iterator& other) const { return pos == other.pos; }
		constexpr inline bool operator!=(const pool_iterator& other) const { return pos != other.pos; }
		constexpr inline bool operator<(const pool_iterator& other) const  { return pos < other.pos; }
		constexpr inline bool operator>(const pool_iterator& other) const  { return pos > other.pos; }
		constexpr inline bool operator<=(const pool_iterator& other) const { return pos <= other.pos; }
		constexpr inline bool operator>=(const pool_iterator& other) const { return pos >= other.pos; }

		constexpr inline pool_iterator& operator++() { ++pos; return *this; }
		constexpr inline pool_iterator& operator--() { --pos; return *this; }
		constexpr inline pool_iterator operator++(int) { auto tmp = *this; ++pos; return tmp; }
		constexpr inline pool_iterator operator--(int) { auto tmp = *this; --pos; return tmp; }
		constexpr inline pool_iterator& operator+=(difference_type n) { pos += n; return *this; }
		constexpr inline pool_iterator& operator-=(difference_type n) { pos -= n; return *this; }
		
		constexpr inline pool_iterator operator+(difference_type n) const { auto tmp = *this; return tmp += n; }
		constexpr inline pool_iterator operator-(difference_type n) const { auto tmp = *this; return tmp -= n; }

		constexpr inline difference_type operator-(const pool_iterator& other) const { return pos - other.pos; }

		constexpr inline size_t index() const { return pos; }
	private:
		size_t pos;
		reg_T* reg;
	};

	template<typename T, typename reg_T>
	pool_iterator<T, reg_T> operator+(typename pool_iterator<T, reg_T>::difference_type n, const pool_iterator<T, reg_T>& it) { 
		return it.pos + n;
	}
}

namespace ecs
{
	template<typename T, typename reg_T>
	class pool {
	public:
		using registry_type = reg_T;
		
		using handle_type = typename component_traits<std::remove_const_t<T>>::handle_type;
		using manager_type = typename component_traits<std::remove_const_t<T>>::manager_type;
		using indexer_type = typename component_traits<std::remove_const_t<T>>::indexer_type;
		using storage_type = typename component_traits<std::remove_const_t<T>>::storage_type;
		
		using value_type = util::tuple<handle_type, T&>;
		using reference = util::proxy_ref<value_type>;
		using const_reference = util::proxy_ref<const value_type>;
		using iterator = pool_iterator<T, reg_T>;
		using const_iterator = pool_iterator<const T, const reg_T>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		
		constexpr inline pool(reg_T* reg) noexcept : reg(reg) { }

		// iterators
		constexpr inline iterator begin() noexcept { return { 0, reg }; }
		constexpr inline const_iterator begin() const noexcept { return { 0, reg }; }
		constexpr inline const_iterator cbegin() const noexcept { return { 0, reg }; }

		constexpr inline iterator end() noexcept { return { reg->template get<manager_type>().size(), reg }; }
		constexpr inline const_iterator end() const noexcept { return { reg->template get<manager_type>().size() }; }
		constexpr inline const_iterator cend() const noexcept { return { reg->template get<manager_type>().size(), reg }; }
		
		constexpr inline reverse_iterator rbegin() noexcept { return iterator{ reg->template get<manager_type>().size()-1 , reg}; } 
		constexpr inline const_reverse_iterator rbegin() const noexcept { return const_iterator{ reg->template get<manager_type>().size()-1, reg }; }
		constexpr inline const_reverse_iterator crbegin() const noexcept { return const_iterator{ reg->template get<manager_type>().size()-1, reg }; }
		
		constexpr inline reverse_iterator rend() noexcept { return iterator{ -1, reg }; }
		constexpr inline const_reverse_iterator rend() const noexcept { return const_iterator{ -1, reg }; }
		constexpr inline const_reverse_iterator crend() const noexcept { return const_iterator{ -1, reg }; }

		// capacity
		[[nodiscard]] constexpr inline bool empty() const noexcept { return reg->template get<manager_type>().size() == 0; }
		constexpr inline size_t max_size() const noexcept { return reg->template get<manager_type>().max_size(); }
		constexpr inline size_t size() const { return reg->template get<manager_type>().size(); }
		constexpr inline size_t capacity() const { return reg->template get<manager_type>().capacity(); }
		constexpr void reserve(size_t n) { 
			reg->template get<manager_type>().reserve(n);
			reg->template get<storage_type>().reserve(n);
		}
		constexpr void shrink_to_fit() {
			size_t n = size();
			reg->template get<manager_type>().shrink_to_fit(n);
			reg->template get<storage_type>().shrink_to_fit(n);
		}
		
		// element/page/data access
		constexpr inline reference operator[](size_t pos) {
			return { reg->template get<manager_type>()[pos], reg->template get<storage_type>()[pos] }; 
		}
		constexpr inline const reference operator[](size_t pos) const {
			return { reg->template get<manager_type>()[pos], reg->template get<storage_type>()[pos] }; 
		}
		
		constexpr inline reference operator[](handle_type ent) {
			auto key = handle_traits<handle_type>::get_data(ent);
			auto pos = handle_traits<handle_type>::get_data(reg->template get<indexer_type>()[key]);
			return { reg->template get<manager_type>()[pos], reg->template get<storage_type>()[pos] }; 
		}
		constexpr inline const reference operator[](handle_type ent) const {
			auto key = handle_traits<handle_type>::get_data(ent);
			auto pos = handle_traits<handle_type>::get_data(reg->template get<indexer_type>()[key]);
			return { reg->template get<manager_type>()[pos], reg->template get<storage_type>()[pos] }; 
		}
		
		constexpr reference at(size_t pos) {
			if (pos >= size()) throw std::out_of_range("");
			return { reg->template get<manager_type>()[pos], reg->template get<storage_type>()[pos] }; 
		}
		constexpr const reference at(size_t pos) const {
			if (pos >= size()) throw std::out_of_range("");
			return { reg->template get<manager_type>()[pos], reg->template get<storage_type>()[pos] }; 
		}

		constexpr T& get(handle_type ent) {
			auto pos = index_of(ent);
			return { reg->template get<manager_type>()[pos], reg->template get<storage_type>()[pos] }; 
		}
		constexpr const T& get(handle_type ent) const {
			auto pos = index_of(ent);
			return { reg->template get<manager_type>()[pos], reg->template get<storage_type>()[pos] }; 
		}

		constexpr reference front() {
			if (empty()) throw std::out_of_range("");
			return { reg->template get<manager_type>()[0], reg->template get<storage_type>()[0] }; 
		}
		constexpr const reference front() const {
			if (empty()) throw std::out_of_range("");
			return { reg->template get<manager_type>()[0], reg->template get<storage_type>()[0] };
		}
		
		constexpr reference back() {
			if (empty()) throw std::out_of_range("");
			size_t pos = size() - 1;
			return { reg->template get<manager_type>()[pos], reg->template get<storage_type>()[pos] };
		}
		constexpr const reference back() const {
			if (empty()) throw std::out_of_range("");
			size_t pos = size() - 1;
			return { reg->template get<manager_type>()[pos], reg->template get<storage_type>()[pos] };
		}
		
		constexpr size_t index_of(handle_type ent) const {
			auto key = handle_traits<handle_type>::get_data(ent);
			if (key >= reg->template get<indexer_type>().size()) throw std::out_of_range("");

			auto ind = reg->template get<indexer_type>()[key];
			if (handle_traits<handle_type>::vers_neq(ent, ind)) throw std::out_of_range("");

			auto pos = handle_traits<handle_type>::get_data(ind);
			if (pos >= size()) throw std::out_of_range("");

			return pos;
		}

		[[nodiscard]] bool contains(handle_type ent) const {
			auto key = handle_traits<handle_type>::get_data(ent);
			if (key >= reg->template get<indexer_type>().size()) return false;
			
			auto ind = reg->template get<indexer_type>()[key];
			if (handle_traits<handle_type>::vers_neq(ent, ind)) return false;
			
			auto pos = handle_traits<handle_type>::get_data(ind);
			if (pos >= size()) return false;

			return reg->template get<manager_type>()[pos] == ent;
		}

		// modifiers
		template<typename ... arg_Ts> requires (std::is_constructible_v<T, arg_Ts...>)
		constexpr T& init(handle_type ent, arg_Ts&&... args)
		{
			size_t key = handle_traits<handle_type>::get_data(ent);
			
			if (key >= reg->template get<indexer_type>().size())
				reg->template get<indexer_type>().resize(key + 1, handle_traits<handle_type>::create()); // creates empty reference
			
			reg->template get<indexer_type>()[key] = handle_traits<handle_type>::create(size(), ent);
			reg->template get<manager_type>().emplace_back(ent);

			T& component = reg->template get<storage_type>().emplace_back(std::forward<arg_Ts>(args)...);
			if constexpr (component_traits<T>::enable_events) reg->template on<ecs::init<T>>().invoke({ ent, component });
			return component;
		}
		
		template<typename ... arg_Ts> requires (std::is_constructible_v<T, arg_Ts...>)
		T& get_or_init(handle_type ent, arg_Ts&& ... args)
		{
			auto key = handle_traits<handle_type>::get_data(ent);
			if (key >= reg->template get<indexer_type>().size()) 
				return init(ent, std::forward<arg_Ts>(args)...);
			
			auto ind = reg->template get<indexer_type>()[key];
			if (handle_traits<handle_type>::vers_neq(ent, ind))
				return init(ent, std::forward<arg_Ts>(args)...);
			
			auto pos = handle_traits<handle_type>::get_data(ind);
			if (pos >= size()) 
				return init(ent, std::forward<arg_Ts>(args)...);

			return get(ent);
		}

		constexpr void terminate(handle_type ent) {
			size_t back = size() - 1;
			if constexpr (component_traits<T>::enable_events) reg->template on<ecs::terminate<T>>().invoke({ ent, reg->template get<storage_type>()[back] });
			
			auto& ind = reg->template get<indexer_type>()[ent];
			auto curr = handle_traits<handle_type>::get_data(ind);
			ind = handle_traits<handle_type>::create(curr);
			
			if (curr != back) {
				std::swap(reg->template get<storage_type>()[curr], reg->template get<storage_type>()[back]);
				std::swap(reg->template get<manager_type>()[curr], reg->template get<manager_type>()[back]);
			}
			reg->template get<storage_type>().pop_back();
			reg->template get<manager_type>().pop_back();
		}
		
		bool try_terminate(handle_type e) {
			if (!contains(e)) return false;
			terminate(e); 
			return true;
		}

		constexpr void clear() noexcept {
			if constexpr (component_traits<T>::enable_events) {
				for (size_t pos = 0; pos < size(); ++pos) {
					reg->template on<ecs::init<T>>().invoke({reg->template get<manager_type>()[pos], reg->template get<storage_type>()[pos]});
				}
			}
			for (size_t pos = 0; pos < size(); ++pos) {
				auto ent = reg->template get<manager_type>()[pos];
				auto key = handle_traits<handle_type>::get_data(ent);
				
				auto& ind = reg->template get<indexer_type>()[key];
				ind = handle_traits<handle_type>::create(handle_traits<handle_type>::tomb, ind);
			}
			reg->template get<storage_type>().clear();
			reg->template get<manager_type>().clear();
		}

	private:
		reg_T* reg;
	};
}
