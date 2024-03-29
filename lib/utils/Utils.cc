#include "utils/Utils.h"

/* ===--------------------------------------------------------------------=== */
// Static asserts to make sure canonicalize_t is working as expected.
/* ===--------------------------------------------------------------------=== */

static_assert(std::is_same_v<canonicalize_t<int, int>, int>);
static_assert(std::is_same_v<canonicalize_t<int const*, int*>, int const>);
static_assert(std::is_same_v<canonicalize_t<int const, int*>, int const>);
static_assert(std::is_same_v<canonicalize_t<int, int const*>, int const>);
static_assert(std::is_same_v<canonicalize_t<int*, int const*>, int const>);
