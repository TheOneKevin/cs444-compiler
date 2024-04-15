#pragma once

#define _MAKE_ENUM(VAR) VAR,
#define _MAKE_STRINGS(VAR) #VAR,
#define _MAKE_ENUM_2(VAR1, VAR2) VAR1,
#define _MAKE_STRINGS_2(VAR1, VAR2) #VAR2,

/**
 * @brief Declare an enum class with a list of members. First, you must declare
 *        the list of enum values through a macro. Like so:
 *        #define ENUM_NAME_LIST(F) F(First) F(Second) F(LAST_MEMBER)
 *        Then, use this macro to declare the enum class.
 */
#define DECLARE_ENUM(ENUM_NAME, LIST) \
   enum class ENUM_NAME { LIST(_MAKE_ENUM) LAST_MEMBER };

/**
 * @brief See DECLARE_ENUM, except the list functions accept a tuple of
 * the form (enum name, enum name unquoted string).
 */
#define DECLARE_ENUM_2(ENUM_NAME, LIST) \
   enum class ENUM_NAME { LIST(_MAKE_ENUM_2) LAST_MEMBER };

/**
 * @brief Declare a string table for an enum class. First, you must declare the
 *        list of enum values through a macro (see DECLARE_ENUM). Then, use this
 *        macro to declare the string table. Use ENUM_NAME##_to_string to get
 *        the string representation of an enum value.
 */
#define DECLARE_STRING_TABLE(ENUM_NAME, TABLE_NAME, LIST)                    \
   static constexpr char const* TABLE_NAME[]{LIST(_MAKE_STRINGS)};           \
   static char const* ENUM_NAME##_to_string(ENUM_NAME type,                  \
                                            char const* default_value) {     \
      /* Check if the type is within the range of the type string list */    \
      if(static_cast<int>(type) >= static_cast<int>(ENUM_NAME::LAST_MEMBER)) \
         return default_value;                                               \
      return TABLE_NAME[static_cast<int>(type)];                             \
   }

/**
 * @brief See DECLARE_STRING_TABLE and DECLARE_ENUM_2 for more info.
 */
#define DECLARE_STRING_TABLE_2(ENUM_NAME, TABLE_NAME, LIST)                  \
   static constexpr char const* TABLE_NAME[]{LIST(_MAKE_STRINGS_2)};         \
   static char const* ENUM_NAME##_to_string(ENUM_NAME type,                  \
                                            char const* default_value) {     \
      /* Check if the type is within the range of the type string list */    \
      if(static_cast<int>(type) >= static_cast<int>(ENUM_NAME::LAST_MEMBER)) \
         return default_value;                                               \
      return TABLE_NAME[static_cast<int>(type)];                             \
   }
