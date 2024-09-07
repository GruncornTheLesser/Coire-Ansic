#pragma once
#include <type_traits>

#define EXPAND(...) __VA_ARGS__
#define DECL_ATTRIB_NAMESPACE \
template<typename T, typename Attrib_T, typename=std::void_t<>> struct get_attribute; \
template<typename T, typename Attrib_T> using get_attribute_t = typename get_attribute<T, Attrib_T>::type; \
template<typename T, typename Attrib_T> static constexpr auto get_attribute_v = get_attribute<T, Attrib_T>::value; \
template<typename T, typename Attrib_T, typename=std::void_t<>> struct has_attribute; \
template<typename T, typename Attrib_T> static constexpr bool has_attribute_v = has_attribute<T, Attrib_T>::value; \
template<typename T, typename Attrib_T, typename=std::void_t<>> struct get_template_attribute; \
template<typename T, typename Attrib_T> using get_template_attribute_t = typename get_template_attribute<T, Attrib_T>::type; \
template<typename T, typename Attrib_T, typename=std::void_t<>> struct has_template_attribute; \
template<typename T, typename Attrib_T> static constexpr bool has_template_attribute_v = has_template_attribute<T, Attrib_T>::value;

#define DECL_TYPE_ATTRIB(NAME, DEFAULT) namespace attribute { struct NAME { }; } \
template<typename T, typename void_T> struct get_attribute<T, attribute::NAME, void_T> { using type = DEFAULT; }; \
template<typename T> struct get_attribute<T, attribute::NAME, std::void_t<typename T::NAME>> { using type = typename T::NAME; }; \
template<typename T, typename void_T> struct has_attribute<T, attribute::NAME, void_T> : std::false_type { }; \
template<typename T> struct has_attribute<T, attribute::NAME, std::void_t<typename T::NAME>> : std::true_type { }; 

#define DECL_VALUE_ATTRIB(TYPE, NAME, DEFAULT) namespace attribute { struct NAME { }; } \
template<typename T, typename void_T> struct get_attribute<T, attribute::NAME, void_T> { static constexpr TYPE value = DEFAULT; }; \
template<typename T> struct get_attribute<T, attribute::NAME, std::void_t<decltype(T::NAME)>> { static constexpr TYPE value = T::NAME; }; \
template<typename T, typename void_T> struct has_attribute<T, attribute::NAME, void_T> : std::false_type { }; \
template<typename T> struct has_attribute<T, attribute::NAME, std::void_t<decltype(T::NAME)>> : std::true_type { };

#define DECL_TEMPLATE_ATTRIB(NAME, DEFAULT) namespace attribute { template<typename U> struct NAME { }; } \
template<typename T, typename U, typename void_T> struct get_template_attribute<T, attribute::NAME<U>, void_T> { using type = DEFAULT; }; \
template<typename T, typename U> struct get_template_attribute<T, attribute::NAME<U>, std::void_t<typename T::template NAME<U>>> { using type = typename T::template NAME<U>; }; \
template<typename T, typename U, typename void_T> struct has_template_attribute<T, attribute::NAME<U>, void_T> : std::false_type { }; \
template<typename T, typename U> struct has_template_attribute<T, attribute::NAME<U>, std::void_t<typename T::template NAME<U>>> : std::true_type { }; 


