#pragma once
#include <type_traits>

namespace std {
	template<class... _Types>
	class tuple;

	template<class _Ty1,
		class _Ty2>
		struct pair;
}

template <class T>
struct is_memcpy_safe 
	: std::bool_constant<std::is_trivially_copyable_v<T>>
{
};

template <>
struct is_memcpy_safe<std::tuple<>> : std::true_type {
};

template<class _Ty>
constexpr bool is_memcpy_safe_v = is_memcpy_safe<_Ty>::value;

template <class... Head>
struct are_types_memcpy_safe;

template <class Head>
struct are_types_memcpy_safe<Head> 
	: std::bool_constant<is_memcpy_safe_v<Head>>
{
};

template <class Head, class... Tail>
struct are_types_memcpy_safe<Head, Tail...>
	: std::bool_constant<
		are_types_memcpy_safe<Head>::value
		&& are_types_memcpy_safe<Tail...>::value
	>
{
};

template<class... _Ty>
constexpr bool are_types_memcpy_safe_v = are_types_memcpy_safe<_Ty...>::value;

template <class A>
bool trivial_compare(const A& a, const A& b) {
	static_assert(is_memcpy_safe_v<A>, "Type can't be trivially compared!");
	return !std::memcmp(&a, &b, sizeof(A));
}