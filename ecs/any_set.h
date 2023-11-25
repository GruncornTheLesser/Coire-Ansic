#pragma once
#include <unordered_map>
#include <functional>
#include <typeindex>
#include <memory>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-incomplete"
namespace util {
	/// @brief a map using type as key. stores values individually on the heap
	class any_set {
		using erased_delete = std::function<void(void*)>;
		struct erased_ptr : std::unique_ptr<void, erased_delete> {
			template<typename t>
			erased_ptr(t* ptr) : std::unique_ptr<void, erased_delete>(ptr, [](void* p) { delete reinterpret_cast<t*>(p); }) { }
			erased_ptr() { }
		};
	public:
		template<typename u, typename ... arg_ts>
		u& get_or_emplace(arg_ts ... args) {
			std::type_index key = typeid(u);

			auto it = m_data.find(key);
			if (it == m_data.end()) 
			{
				void* elem = new u{ args... };
				it = m_data.emplace_hint(it, std::pair(key, elem));
			}

			return *reinterpret_cast<u*>(it->second.get());
		}

		template<typename u>
		u& get() const {
			return *reinterpret_cast<u*>(m_data.at(typeid(u)).get());
		}

		template<typename u>
		void erase() {
			m_data.erase(typeid(u));
		}

		template<typename u>
		bool contains() const {
			return m_data.contains(typeid(u));
		}

	private:
		std::unordered_map<std::type_index, erased_ptr> m_data;
	};
}
#pragma GCC diagnostic pop