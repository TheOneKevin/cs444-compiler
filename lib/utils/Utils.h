#pragma once

#include <type_traits>
#include <typeinfo>
#include <vector>

namespace utils {

template <typename T>
void move_vector(std::pmr::vector<T>& from, std::pmr::vector<T>& to) {
   to.reserve(from.size());
   to.insert(to.end(),
             std::make_move_iterator(from.begin()),
             std::make_move_iterator(from.end()));
}

} // namespace utils

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

// Transfer pointer or reference from one type to another.
template <typename To, typename From>
using transfer_ptr_or_ref_t = std::conditional_t<
      /* Condition */ std::is_pointer_v<From>,
      /* From* -> To* */ std::add_pointer_t<To>,
      std::conditional_t<
            /* Condition */ std::is_reference_v<From>,
            /* From& -> To& */ std::add_lvalue_reference_t<To>,
            /* From -> To */ To>>;

/**
 * @brief Checks if type From can be cast to type To. The checks are:
 *    1. From must be a (possibly ref or ptr to) class
 *    2. To must be a (possibly ref or ptr to) class
 *    3. From must be either a pointer or a reference
 *       To must not be a reference (for now...)
 *    4. Illegal cast: From* -> To&, From* -> To is allowed
 *    5. Illegal cast: From& -> To*, From& -> To is allowed
 */
template <typename To, typename From>
constexpr bool is_valid_cast_type_v =
      std::is_class_v<remove_ptr_or_ref_t<To>> &&
      std::is_class_v<remove_ptr_or_ref_t<From>> &&
      (std::is_pointer_v<From> || std::is_reference_v<From>) &&
      !std::is_reference_v<To> &&
      /* Illegal: From* -> To& */
      !(std::is_pointer_v<From> && std::is_reference_v<To>)&&
      /* Illegal: From& -> To* */
      !(std::is_reference_v<From> && std::is_pointer_v<To>);

/**
 * @brief Given a type To and a type From, returns the canonicalized type.
 * Strips To and From of their pointer and reference types, and transfers
 * const and volatile from From to To.
 */
template <typename To, typename From>
using canonicalize_t =
      transfer_cv_t<remove_ptr_or_ref_t<To>, remove_ptr_or_ref_t<From>>;

/**
 * @brief Casts a pointer of type From to a pointer type of: To or To*.
 * Will throw std::bad_cast if the cast is invalid.
 *
 * @tparam To A class or pointer to a class type.
 * @tparam From A pointer to a class type.
 */
template <typename To, typename From>
/* To* */ std::enable_if_t<std::is_pointer_v<From>, canonicalize_t<To, From>*>
cast(From from) {
   constexpr bool is_valid = is_valid_cast_type_v<To, From&>;
   static_assert(is_valid, "Cannot cast from From to To");
   if constexpr(is_valid) {
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
/* To* */ std::enable_if_t<!std::is_pointer_v<From&>, canonicalize_t<To, From&>*>
cast(From& from) {
   return cast<To>(&from);
}

/**
 * @brief Casts a pointer of type From to a pointer type of: To or To*.
 * Will throw std::bad_cast if the from pointer is nullptr. Will return
 * nullptr if the cast is invalid.
 * 
 * @tparam To A class or pointer to a class type.
 * @tparam From A pointer to a class type.
 */
template <typename To, typename From>
/* To* */ std::enable_if_t<std::is_pointer_v<From>, canonicalize_t<To, From>*>
dyn_cast(From from) {
   constexpr bool is_valid = is_valid_cast_type_v<To, From>;
   if constexpr(is_valid) {
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
/* To* */ std::enable_if_t<!std::is_pointer_v<From&>, canonicalize_t<To, From&>*>
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
   constexpr bool is_valid = is_valid_cast_type_v<To, From>;
   if constexpr(is_valid) {
      return dynamic_cast<canonicalize_t<To, From>*>(from);
   }
   return nullptr;
}
