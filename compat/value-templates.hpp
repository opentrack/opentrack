#pragma once

#include <type_traits>
#include <memory>
#include <functional>
#include <system_error>
#include <chrono>
#include <ratio>

#if __cplusplus < 201703L

template <class T> constexpr bool is_void_v =	std::is_void<T>::value;
template <class T> constexpr bool is_null_pointer_v =	std::is_null_pointer<T>::value;
template <class T> constexpr bool is_integral_v =	std::is_integral<T>::value;
template <class T> constexpr bool is_floating_point_v =	std::is_floating_point<T>::value;
template <class T> constexpr bool is_array_v =	std::is_array<T>::value;
template <class T> constexpr bool is_pointer_v =	std::is_pointer<T>::value;
template <class T> constexpr bool is_lvalue_reference_v =	std::is_lvalue_reference<T>::value;
template <class T> constexpr bool is_rvalue_reference_v =	std::is_rvalue_reference<T>::value;
template <class T> constexpr bool is_member_object_pointer_v =	std::is_member_object_pointer<T>::value;
template <class T> constexpr bool is_member_function_pointer_v =	std::is_member_function_pointer<T>::value;
template <class T> constexpr bool is_enum_v =	std::is_enum<T>::value;
template <class T> constexpr bool is_union_v =	std::is_union<T>::value;
template <class T> constexpr bool is_class_v =	std::is_class<T>::value;
template <class T> constexpr bool is_function_v =	std::is_function<T>::value;

template <class T> constexpr bool is_reference_v =	std::is_reference<T>::value;
template <class T> constexpr bool is_arithmetic_v =	std::is_arithmetic<T>::value;
template <class T> constexpr bool is_fundamental_v =	std::is_fundamental<T>::value;
template <class T> constexpr bool is_object_v =	std::is_object<T>::value;
template <class T> constexpr bool is_scalar_v =	std::is_scalar<T>::value;
template <class T> constexpr bool is_compound_v =	std::is_compound<T>::value;
template <class T> constexpr bool is_member_pointer_v =	std::is_member_pointer<T>::value;

template <class T> constexpr bool is_const_v =	std::is_const<T>::value;
template <class T> constexpr bool is_volatile_v =	std::is_volatile<T>::value;
template <class T> constexpr bool is_trivial_v =	std::is_trivial<T>::value;
template <class T> constexpr bool is_trivially_copyable_v =	std::is_trivially_copyable<T>::value;
template <class T> constexpr bool is_standard_layout_v =	std::is_standard_layout<T>::value;
template <class T> constexpr bool is_pod_v =	std::is_pod<T>::value;
template <class T> constexpr bool is_literal_type_v =	std::is_literal_type<T>::value;
template <class T> constexpr bool is_empty_v =	std::is_empty<T>::value;
template <class T> constexpr bool is_polymorphic_v =	std::is_polymorphic<T>::value;
template <class T> constexpr bool is_abstract_v =	std::is_abstract<T>::value;
template <class T> constexpr bool is_final_v =	std::is_final<T>::value;
template <class T> constexpr bool is_signed_v =	std::is_signed<T>::value;
template <class T> constexpr bool is_unsigned_v =	std::is_unsigned<T>::value;

template <class T, class... Args> constexpr bool is_constructible_v =	std::is_constructible<T, Args...>::value;
template <class T, class... Args> constexpr bool is_trivially_constructible_v =	std::is_trivially_constructible<T, Args...>::value;
template <class T, class... Args> constexpr bool is_nothrow_constructible_v =	std::is_nothrow_constructible<T, Args...>::value;
template <class T> constexpr bool is_default_constructible_v =	std::is_default_constructible<T>::value;
template <class T> constexpr bool is_trivially_default_constructible_v =	std::is_trivially_default_constructible<T>::value;
template <class T> constexpr bool is_nothrow_default_constructible_v =	std::is_nothrow_default_constructible<T>::value;
template <class T> constexpr bool is_copy_constructible_v =	std::is_copy_constructible<T>::value;
template <class T> constexpr bool is_trivially_copy_constructible_v =	std::is_trivially_copy_constructible<T>::value;
template <class T> constexpr bool is_nothrow_copy_constructible_v =	std::is_nothrow_copy_constructible<T>::value;
template <class T> constexpr bool is_move_constructible_v =	std::is_move_constructible<T>::value;
template <class T> constexpr bool is_trivially_move_constructible_v =	std::is_trivially_move_constructible<T>::value;
template <class T> constexpr bool is_nothrow_move_constructible_v =	std::is_nothrow_move_constructible<T>::value;
template <class T, class U> constexpr bool is_assignable_v =	std::is_assignable<T, U>::value;
template <class T, class U> constexpr bool is_trivially_assignable_v =	std::is_trivially_assignable<T, U>::value;
template <class T, class U> constexpr bool is_nothrow_assignable_v =	std::is_nothrow_assignable<T, U>::value;
template <class T> constexpr bool is_copy_assignable_v =	std::is_copy_assignable<T>::value;
template <class T> constexpr bool is_trivially_copy_assignable_v =	std::is_trivially_copy_assignable<T>::value;
template <class T> constexpr bool is_nothrow_copy_assignable_v =	std::is_nothrow_copy_assignable<T>::value;
template <class T> constexpr bool is_move_assignable_v =	std::is_move_assignable<T>::value;
template <class T> constexpr bool is_trivially_move_assignable_v =	std::is_trivially_move_assignable<T>::value;
template <class T> constexpr bool is_nothrow_move_assignable_v =	std::is_nothrow_move_assignable<T>::value;
template <class T> constexpr bool is_destructible_v =	std::is_destructible<T>::value;
template <class T> constexpr bool is_trivially_destructible_v =	std::is_trivially_destructible<T>::value;
template <class T> constexpr bool is_nothrow_destructible_v =	std::is_nothrow_destructible<T>::value;
template <class T> constexpr bool has_virtual_destructor_v =	std::has_virtual_destructor<T>::value;

template <class T> constexpr std::size_t alignment_of_v =	std::alignment_of<T>::value;
template <class T> constexpr std::size_t rank_v =	std::rank<T>::value;
template <class T, unsigned I = 0> constexpr std::size_t extent_v =	std::extent<T, I>::value;

template <class T, class U> constexpr bool is_same_v =	std::is_same<T, U>::value;
template <class Base, class Derived> constexpr bool is_base_of_v =	std::is_base_of<Base, Derived>::value;
template <class From, class To> constexpr bool is_convertible_v =	std::is_convertible<From, To>::value;

template <class R1, class R2> constexpr bool ratio_equal_v =	std::ratio_equal<R1, R2>::value;
template <class R1, class R2> constexpr bool ratio_not_equal_v =	std::ratio_not_equal<R1, R2>::value;
template <class R1, class R2> constexpr bool ratio_less_v =	std::ratio_less<R1, R2>::value;
template <class R1, class R2> constexpr bool ratio_less_equal_v =	std::ratio_less_equal<R1, R2>::value;
template <class R1, class R2> constexpr bool ratio_greater_v =	std::ratio_greater<R1, R2>::value;
template <class R1, class R2> constexpr bool ratio_greater_equal_v =	std::ratio_greater_equal<R1, R2>::value;

template <class T> constexpr std::size_t tuple_size_v =	std::tuple_size<T>::value;
template <class Rep> constexpr bool treat_as_floating_point_v =	std::chrono::treat_as_floating_point<Rep>::value;

template <class T> constexpr bool is_error_code_enum_v =	std::is_error_code_enum<T>::value;
template <class T> constexpr bool is_error_condition_enum_v =	std::is_error_condition_enum<T>::value;

template <class T> constexpr bool is_bind_expression_v =	std::is_bind_expression<T>::value;
template <class T> constexpr bool is_placeholder_v =	std::is_placeholder<T>::value;

template <class T, class Alloc> constexpr bool uses_allocator_v =	std::uses_allocator<T, Alloc>::value;

#else

template <class T> constexpr bool is_void_v =	std::is_void_v<T>;
template <class T> constexpr bool is_null_pointer_v =	std::is_null_pointer_v<T>;
template <class T> constexpr bool is_integral_v =	std::is_integral_v<T>;
template <class T> constexpr bool is_floating_point_v =	std::is_floating_point_v<T>;
template <class T> constexpr bool is_array_v =	std::is_array_v<T>;
template <class T> constexpr bool is_pointer_v =	std::is_pointer_v<T>;
template <class T> constexpr bool is_lvalue_reference_v =	std::is_lvalue_reference_v<T>;
template <class T> constexpr bool is_rvalue_reference_v =	std::is_rvalue_reference_v<T>;
template <class T> constexpr bool is_member_object_pointer_v =	std::is_member_object_pointer_v<T>;
template <class T> constexpr bool is_member_function_pointer_v =	std::is_member_function_pointer_v<T>;
template <class T> constexpr bool is_enum_v =	std::is_enum_v<T>;
template <class T> constexpr bool is_union_v =	std::is_union_v<T>;
template <class T> constexpr bool is_class_v =	std::is_class_v<T>;
template <class T> constexpr bool is_function_v =	std::is_function_v<T>;

template <class T> constexpr bool is_reference_v =	std::is_reference_v<T>;
template <class T> constexpr bool is_arithmetic_v =	std::is_arithmetic_v<T>;
template <class T> constexpr bool is_fundamental_v =	std::is_fundamental_v<T>;
template <class T> constexpr bool is_object_v =	std::is_object_v<T>;
template <class T> constexpr bool is_scalar_v =	std::is_scalar_v<T>;
template <class T> constexpr bool is_compound_v =	std::is_compound_v<T>;
template <class T> constexpr bool is_member_pointer_v =	std::is_member_pointer_v<T>;

template <class T> constexpr bool is_const_v =	std::is_const_v<T>;
template <class T> constexpr bool is_volatile_v =	std::is_volatile_v<T>;
template <class T> constexpr bool is_trivial_v =	std::is_trivial_v<T>;
template <class T> constexpr bool is_trivially_copyable_v =	std::is_trivially_copyable_v<T>;
template <class T> constexpr bool is_standard_layout_v =	std::is_standard_layout_v<T>;
template <class T> constexpr bool is_pod_v =	std::is_pod_v<T>;
template <class T> constexpr bool is_literal_type_v =	std::is_literal_type_v<T>;
template <class T> constexpr bool is_empty_v =	std::is_empty_v<T>;
template <class T> constexpr bool is_polymorphic_v =	std::is_polymorphic_v<T>;
template <class T> constexpr bool is_abstract_v =	std::is_abstract_v<T>;
template <class T> constexpr bool is_final_v =	std::is_final_v<T>;
template <class T> constexpr bool is_signed_v =	std::is_signed_v<T>;
template <class T> constexpr bool is_unsigned_v =	std::is_unsigned_v<T>;

template <class T, class... Args> constexpr bool is_constructible_v =	std::is_constructible_v<T, Args...>;
template <class T, class... Args> constexpr bool is_trivially_constructible_v =	std::is_trivially_constructible_v<T, Args...>;
template <class T, class... Args> constexpr bool is_nothrow_constructible_v =	std::is_nothrow_constructible_v<T, Args...>;
template <class T> constexpr bool is_default_constructible_v =	std::is_default_constructible_v<T>;
template <class T> constexpr bool is_trivially_default_constructible_v =	std::is_trivially_default_constructible_v<T>;
template <class T> constexpr bool is_nothrow_default_constructible_v =	std::is_nothrow_default_constructible_v<T>;
template <class T> constexpr bool is_copy_constructible_v =	std::is_copy_constructible_v<T>;
template <class T> constexpr bool is_trivially_copy_constructible_v =	std::is_trivially_copy_constructible_v<T>;
template <class T> constexpr bool is_nothrow_copy_constructible_v =	std::is_nothrow_copy_constructible_v<T>;
template <class T> constexpr bool is_move_constructible_v =	std::is_move_constructible_v<T>;
template <class T> constexpr bool is_trivially_move_constructible_v =	std::is_trivially_move_constructible_v<T>;
template <class T> constexpr bool is_nothrow_move_constructible_v =	std::is_nothrow_move_constructible_v<T>;
template <class T, class U> constexpr bool is_assignable_v =	std::is_assignable_v<T, U>;
template <class T, class U> constexpr bool is_trivially_assignable_v =	std::is_trivially_assignable_v<T, U>;
template <class T, class U> constexpr bool is_nothrow_assignable_v =	std::is_nothrow_assignable_v<T, U>;
template <class T> constexpr bool is_copy_assignable_v =	std::is_copy_assignable_v<T>;
template <class T> constexpr bool is_trivially_copy_assignable_v =	std::is_trivially_copy_assignable_v<T>;
template <class T> constexpr bool is_nothrow_copy_assignable_v =	std::is_nothrow_copy_assignable_v<T>;
template <class T> constexpr bool is_move_assignable_v =	std::is_move_assignable_v<T>;
template <class T> constexpr bool is_trivially_move_assignable_v =	std::is_trivially_move_assignable_v<T>;
template <class T> constexpr bool is_nothrow_move_assignable_v =	std::is_nothrow_move_assignable_v<T>;
template <class T> constexpr bool is_destructible_v =	std::is_destructible_v<T>;
template <class T> constexpr bool is_trivially_destructible_v =	std::is_trivially_destructible_v<T>;
template <class T> constexpr bool is_nothrow_destructible_v =	std::is_nothrow_destructible_v<T>;
template <class T> constexpr bool has_virtual_destructor_v =	std::has_virtual_destructor_v<T>;

template <class T> constexpr std::size_t alignment_of_v =	std::alignment_of_v<T>;
template <class T> constexpr std::size_t rank_v =	std::rank_v<T>;
template <class T, unsigned I = 0> constexpr std::size_t extent_v =	std::extent_v<T, I>;

template <class T, class U> constexpr bool is_same_v =	std::is_same_v<T, U>;
template <class Base, class Derived> constexpr bool is_base_of_v =	std::is_base_of_v<Base, Derived>;
template <class From, class To> constexpr bool is_convertible_v =	std::is_convertible_v<From, To>;

template <class R1, class R2> constexpr bool ratio_equal_v =	std::ratio_equal_v<R1, R2>;
template <class R1, class R2> constexpr bool ratio_not_equal_v =	std::ratio_not_equal_v<R1, R2>;
template <class R1, class R2> constexpr bool ratio_less_v =	std::ratio_less_v<R1, R2>;
template <class R1, class R2> constexpr bool ratio_less_equal_v =	std::ratio_less_equal_v<R1, R2>;
template <class R1, class R2> constexpr bool ratio_greater_v =	std::ratio_greater_v<R1, R2>;
template <class R1, class R2> constexpr bool ratio_greater_equal_v =	std::ratio_greater_equal_v<R1, R2>;

template <class T> constexpr std::size_t tuple_size_v =	std::tuple_size_v<T>;
template <class Rep> constexpr bool treat_as_floating_point_v =	std::chrono::treat_as_floating_point_v<Rep>;

template <class T> constexpr bool is_error_code_enum_v =	std::is_error_code_enum_v<T>;
template <class T> constexpr bool is_error_condition_enum_v =	std::is_error_condition_enum_v<T>;

template <class T> constexpr bool is_bind_expression_v =	std::is_bind_expression_v<T>;
template <class T> constexpr bool is_placeholder_v =	std::is_placeholder_v<T>;

template <class T, class Alloc> constexpr bool uses_allocator_v =	std::uses_allocator_v<T, Alloc>;

#endif
