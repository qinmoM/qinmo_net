#pragma once

#include "detail/Common.h"

namespace qinmo
{
/// @namespace qinmo::detail
/// @note For internal use only, do NOT use it from outside the library
namespace detail
{

#if defined(__linux__)
    using ThreadIDType = pid_t;

    static constexpr ThreadIDType g_ThreadIDTypeEmpty = 0;
#elif defined(_WIN32)
    using ThreadIDType = DWORD;

    static constexpr ThreadIDType g_ThreadIDTypeEmpty = 0;
#endif



ThreadIDType getTid();
uint32_t getTid32();

} // namespace detail
} // namespace qinmo