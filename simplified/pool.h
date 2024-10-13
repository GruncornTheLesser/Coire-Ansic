#pragma once
#include "traits.h"
#include "registry.h"
#include "proxy_ref.h"
namespace ecs {
	template<typename T, typename reg_T>
	class pool_iterator {
	public:
		using iterator_category = std::random_access_iterator_tag;
		using value_type = std::tuple<entity<std::remove_const_t<T>>, std::remove_const_t<T>&>;
		using reference = proxy_ref<std::conditional_t<std::is_const_v<T>, const value_type, value_type>>;
		using difference_type = std::ptrdiff_t;

		pool_iterator() : reg(nullptr), ind(0) { }
		pool_iterator(reg_T* reg, size_t ind) : reg(reg), ind(ind) { }
		pool_iterator(const pool_iterator& other) : reg(other.reg), ind(other.ind) { }
		pool_iterator& operator=(const pool_iterator& other) { reg = other.reg; ind = other.ind; }
		pool_iterator(pool_iterator&& other) : reg(other.reg), ind(other.ind) { }
		pool_iterator& operator=(pool_iterator&& other) { reg = other.reg; ind = other.ind; }

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

		constexpr bool operator==(const pool_iterator& other) const { return ind == other.ind; }
		constexpr bool operator!=(const pool_iterator& other) const { return ind != other.ind; }
		constexpr bool operator<(const pool_iterator& other) const  { return ind < other.ind; }
		constexpr bool operator>(const pool_iterator& other) const  { return ind > other.ind; }
		constexpr bool operator<=(const pool_iterator& other) const { return ind <= other.ind; }
		constexpr bool operator>=(const pool_iterator& other) const { return ind >= other.ind; }

		constexpr pool_iterator& operator++() { ++ind; return *this; }
		constexpr pool_iterator& operator--() { --ind; return *this; }
		constexpr pool_iterator operator++(int) { auto tmp = *this; ++ind; return tmp; }
		constexpr pool_iterator operator--(int) { auto tmp = *this; --ind; return tmp; }
		constexpr pool_iterator& operator+=(difference_type n) { ind += n; return *this; }
		constexpr pool_iterator& operator-=(difference_type n) { ind -= n; return *this; }
		
		constexpr pool_iterator operator+(difference_type n) const { auto tmp = *this; return tmp += n; }
		constexpr pool_iterator operator-(difference_type n) const { auto tmp = *this; return tmp -= n; }

		constexpr difference_type operator-(const pool_iterator& other) const { return ind - other.ind; }

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
		using value_type = std::tuple<entity<std::remove_const_t<T>>, std::remove_const_t<T>&>;
		using reference = proxy_ref<value_type>;
		using const_reference = proxy_ref<const value_type>;

		using iterator = pool_iterator<T, reg_T>;
		using const_iterator = pool_iterator<const T, reg_T>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		pool(reg_T* reg) : reg(reg) { }

		template<std::input_iterator It>
		//void assign(It first, It last);
		//void assign(size_t n, const T& value);
		//void assign(std::initializer_list<std::pair<entity<T>, T>> ilist);

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
		[[nodiscard]] constexpr bool empty() const noexcept { return reg->template get_manager<T>().size() == 0; }
		constexpr size_t max_size() const noexcept { return reg->template get_manager<T>().max_size(); }
		constexpr size_t size() const { return reg->template get_manager<T>().size(); }
		constexpr size_t capacity() const { return reg->template get_manager<T>().capacity(); }
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
		
		constexpr reference operator[](entity<T> e) {
			auto index = reg->template get_indexer<T>()[e.value()].value();
			return { reg->template get_manager<T>()[index], reg->template get_storage<T>()[index] }; 
		}
		constexpr const reference operator[](entity<T> e) const {
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

		constexpr T& get(entity<T> e) {
			size_t index = index_of(e);
			return { reg->template get_manager<T>()[index], reg->template get_storage<T>()[index] }; 
		}
		constexpr const T& get(entity<T> e) const {
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
		
		constexpr size_t index_of(entity<T> e) const {
			auto val = e.value();
			if (e.value() > reg->template get_indexer<T>().size()) throw std::out_of_range("");
			if (e.version() != reg->template get_indexer<T>()[e.value()].version()) throw std::out_of_range("");
			return reg->template get_indexer<T>()[e.value()].value();
		}

		[[nodiscard]] bool contains(entity<T> e) const {
			return e.value() < reg->template get_indexer<T>().size() && e.version() == reg->template get_indexer<T>()[e.value()].version();
		}


		// modifiers
		template<typename ... arg_Ts>
		constexpr T& init(entity<T> e, arg_Ts&&... args)
		{
			auto& mng = reg->template get_manager<T>();
			auto& ind = reg->template get_indexer<T>();
			auto& str = reg->template get_storage<T>();
			
			ind.resize(e.value() + 1, entity<T>{ tombstone{} });
			ind[e.value()] = entity<T>{ (unsigned int)mng.size(), e.version() };
			mng.emplace_back(e);

			T& component = str.emplace_back(std::forward<arg_Ts>(args)...);
			reg->template on<event::init<T>>().invoke({ e, component });
			return component;
		}

		constexpr void terminate(entity<T> e) {
			auto& mng = reg->template get_manager<T>();
			auto& ind = reg->template get_indexer<T>();
			auto& str = reg->template get_storage<T>();

			auto back = mng.size() - 1;
			reg->template on<event::terminate<T>>().invoke({e, str[back]});
			
			auto curr = ind[e.value()].value();
			ind[e.value()] = tombstone{};
			
			if (curr != back) {
				str[curr] = std::move(str[back]);
				mng[curr] = std::move(mng[back]);
			}
			str.pop_back();
			mng.pop_back();
		}
		
		constexpr void clear() noexcept {
			auto& mng = reg->template get_manager<T>();
			auto& ind = reg->template get_indexer<T>();
			auto& str = reg->template get_storage<T>();
			
			for (int i=0; i<size(); ++i) {
				reg->template on<event::terminate<T>>().invoke({mng[i], str[i]});
				ind[i] = tombstone{};
			}
			mng.clear();
			str.clear();
		}

		template<typename ... arg_Ts>
		T* try_init(entity<T> e, arg_Ts&& ... args) {
			if (contains(e)) return nullptr;
			return &init(e, std::forward<arg_Ts>(args)...);
		}

		bool try_terminate(entity<T> e) {
			if (!contains(e)) return false;
			terminate(e); 
			return true;
		}

	private:
		reg_T* reg;
	};
}
