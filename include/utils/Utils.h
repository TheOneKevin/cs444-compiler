#pragma once

#include <type_traits>
#include <vector>

namespace utils {

template <typename T>
void move_vector(std::pmr::vector<T>& from, std::pmr::vector<T>& to) {
   to.reserve(from.size());
   to.insert(to.end(),
             std::make_move_iterator(from.begin()),
             std::make_move_iterator(from.end()));
}

// Remove a pointer or reference from a type.
template <typename T>
using remove_ptr_or_ref_t = std::conditional_t<
      /* Condition */ std::is_pointer_v<T>,
      /* T* -> T */ std::remove_pointer_t<T>,
      /* T& -> T, or T -> T */ std::remove_reference_t<T>>;

// Transfer const from one type to another.
template <typename To, typename From>
using transfer_const_t = std::conditional_t<
      /* Condition */ std::is_const_v<From>,
      /* From const -> To const */ std::add_const_t<To>, To>;

// Transfer volatile from one type to another.
template <typename To, typename From>
using transfer_volatile_t = std::conditional_t<
      /* Condition */ std::is_volatile_v<From>,
      /* From volatile -> To volatile */ std::add_volatile_t<To>, To>;

// Transfer const and volatile from one type to another.
template <typename To, typename From>
using transfer_cv_t = transfer_volatile_t<transfer_const_t<To, From>, From>;

// Merge const and volatile from two types into U.
template <typename U, typename V>
using union_cv_t = transfer_cv_t<transfer_cv_t<U, V>, U>;

// Transfer pointer or reference from one type to another.
template <typename To, typename From>
using transfer_ptr_or_ref_t = std::conditional_t<
      /* Condition */ std::is_pointer_v<From>,
      /* From* -> To* */ std::add_pointer_t<To>,
      std::conditional_t<
            /* Condition */ std::is_reference_v<From>,
            /* From& -> To& */ std::add_lvalue_reference_t<To>,
            /* From -> To */ To>>;

// Checks if type From can be cast to type To.
template <typename To, typename From>
consteval bool is_valid_type_cast_f() {
   constexpr bool c1 = std::is_class_v<remove_ptr_or_ref_t<To>>;
   static_assert(c1, "To must be a class or a pointer to a class");
   constexpr bool c2 = std::is_class_v<remove_ptr_or_ref_t<From>>;
   static_assert(c2, "From must be a class or a pointer to a class");
   constexpr bool c3 = std::is_pointer_v<From> || std::is_reference_v<From>;
   static_assert(c3, "From must be a pointer or a reference");
   constexpr bool c4 = !(std::is_pointer_v<From> && std::is_reference_v<To>);
   static_assert(c4, "From* -> To& is illegal");
   constexpr bool c5 = !(std::is_reference_v<From> && std::is_pointer_v<To>);
   static_assert(c5, "From& -> To* is illegal");
   constexpr bool c6 = !std::is_reference_v<To>;
   static_assert(c6, "Casting to a reference is illegal");
   return c1 && c2 && c3 && c4 && c5 && c6;
}

} // namespace utils

/**
 * @brief Given a type To and a type From, returns the canonicalized type.
 * Strips To and From of their pointer and reference types, and unions
 * const and volatile between From and To.
 */
template <typename To, typename From>
using canonicalize_t = utils::union_cv_t<utils::remove_ptr_or_ref_t<To>,
                                         utils::remove_ptr_or_ref_t<From>>;

/**
 * @brief Casts a pointer of type From to a pointer type of: To or To*.
 * Will assert, if the cast is invalid.
 *
 * @tparam To A class or pointer to a class type.
 * @tparam From A pointer to a class type.
 */
template <typename To, typename From>
/* To* */ std::enable_if_t<std::is_pointer_v<From>, canonicalize_t<To, From>*>
cast(From from) {
   if constexpr(utils::is_valid_type_cast_f<To, From>()) {
      assert(from && "Tried to cast a nullptr");
      auto ptr = dynamic_cast<canonicalize_t<To, From>*>(from);
      assert(ptr && "Invalid cast");
      return ptr;
   }
   return nullptr;
}

/**
 * @brief Casts a reference of type From to a pointer of type of: To or To*.
 * Same as cast, but for references.
 *
 * @tparam To A class or pointer to a class type.
 * @tparam From A reference to a class type.
 */
template <typename To, typename From>
/* To* */ std::enable_if_t<!std::is_pointer_v<From>, canonicalize_t<To, From>*>
cast(From& from) {
   return cast<To>(&from);
}

/**
 * @brief Casts a pointer of type From to a pointer type of: To or To*.
 * Will assert, if the from pointer is nullptr. Will return
 * nullptr if the cast is invalid.
 *
 * @tparam To A class or pointer to a class type.
 * @tparam From A pointer to a class type.
 */
template <typename To, typename From>
/* To* */ std::enable_if_t<std::is_pointer_v<From>, canonicalize_t<To, From>*>
dyn_cast(From from) {
   if constexpr(utils::is_valid_type_cast_f<To, From>()) {
      assert(from && "Tried to dyn_cast a nullptr");
      return dynamic_cast<canonicalize_t<To, From>*>(from);
   }
   return nullptr;
}

/**
 * @brief Casts a reference of type From to a pointer type of: To or To*.
 * Same as dyn_cast, but for references.
 *
 * @tparam To A class or pointer to a class type.
 * @tparam From A reference to a class type.
 */
template <typename To, typename From>
/* To* */ std::enable_if_t<!std::is_pointer_v<From>, canonicalize_t<To, From>*>
dyn_cast(From& from) {
   return dyn_cast<To>(&from);
}

/**
 * @brief Same as dyn_cast, but returns nullptr if the from pointer is nullptr.
 *
 * @tparam To A class or pointer to a class type.
 * @tparam From A pointer to a class type.
 */
template <typename To, typename From>
/* To* */ std::enable_if_t<std::is_pointer_v<From>, canonicalize_t<To, From>*>
dyn_cast_or_null(From from) {
   if constexpr(utils::is_valid_type_cast_f<To, From>()) {
      return dynamic_cast<canonicalize_t<To, From>*>(from);
   }
   return nullptr;
}

/* ===--------------------------------------------------------------------=== */
// Static asserts to make sure canonicalize_t is working as expected.
/* ===--------------------------------------------------------------------=== */

static_assert(std::is_same_v<canonicalize_t<int, int>, int>);
static_assert(std::is_same_v<canonicalize_t<int const*, int*>, int const>);
static_assert(std::is_same_v<canonicalize_t<int const, int*>, int const>);
static_assert(std::is_same_v<canonicalize_t<int, int const*>, int const>);
static_assert(std::is_same_v<canonicalize_t<int*, int const*>, int const>);
