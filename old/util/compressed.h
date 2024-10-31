#pragma once
#include <stdint.h>
#include <concepts>
#include <numeric>
#include <array>

namespace util {
	template<size_t width>
	struct sized_uint : std::conditional<(width <= 8), uint8_t,
						std::conditional_t<(width <= 16), uint16_t,
						std::conditional_t<(width <= 32), uint32_t,
						std::conditional_t<(width <= 64), uint64_t,
		void>>>> { };
	template<size_t width>
	using sized_uint_t = typename sized_uint<width>::type;


	template<size_t ... Ns>
	struct compressed
	{
	//private:
		using value_type = sized_uint_t<(Ns + ...)>;
		value_type value;
		
		struct traits_type
		{
			std::array<size_t, sizeof...(Ns)> width_arr{Ns...};
			std::array<value_type, sizeof...(Ns)> mask_arr{};
			std::array<size_t, sizeof...(Ns)> offset_arr{};
			constexpr traits_type()
			{
				size_t last_offset = 0;
				for (int i=0; i < sizeof...(Ns); ++i) 
				{
					offset_arr[i] = last_offset; 
					last_offset += width_arr[i];
					mask_arr[i] = ((1ull << width_arr[i]) - 1) << offset_arr[i];
				}
			}
		};
		static constexpr traits_type traits{};

	public:
		static constexpr value_type get_mask(size_t i) { return traits.mask_arr[i]; }


		explicit constexpr compressed(value_type vals) : value(vals) { }
		explicit constexpr compressed(std::initializer_list<value_type> values) : value(0) 
		{ 
			size_t i = 0;
			auto it = values.begin();
			while (it != values.end()) value |= (*it++ << traits.offset_arr[i]) & traits.mask_arr[i++]; 
		}
		
		template<size_t I>
		struct view 
		{
			constexpr view() { }
			constexpr view(const view& other) : value(other.value) { }
			constexpr view& operator=(const view& other) { value = (value & ~get_mask(I)) | (other.value & get_mask(I)); }
			
			explicit constexpr view(value_type val) : value((val << traits.offset_arr[I]) & traits.mask_arr[I]) { }
			explicit constexpr view(compressed val) : value(val.value) { }
			explicit constexpr operator compressed() const { return value; }
			explicit constexpr operator value_type() const { return (value & traits.mask_arr[I]) >> traits.offset_arr[I]; }
			
			constexpr view operator++() { value += (1 << traits.offset_arr[I]); }
			constexpr view operator--() { value -= (1 << traits.offset_arr[I]); }
			constexpr view operator++(int) { auto tmp = *this; ++(*this); return tmp; }
			constexpr view operator--(int) { auto tmp = *this; --(*this); return tmp; }

			constexpr bool operator==(compressed other) const { return ((value ^ other.value) & traits.mask_arr[I]) == 0; };
			constexpr bool operator!=(compressed other) const { return ((value ^ other.value) & traits.mask_arr[I]) != 0; };
			// constexpr bool operator<=(compressed other) { };
			// constexpr bool operator<(compressed other) { };
			// constexpr bool operator>=(compressed other) { };
			// constexpr bool operator>(compressed other) { };
			
		//private:
			value_type value;
		};
	};
}