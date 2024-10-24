#ifndef DEVICE_ERRORCODE_H
#define DEVICE_ERRORCODE_H

#include <system_error>

enum class DEVICE_ErrorCode
{
    Success  =  0, /* OK */
    Error    = -1, /* General error */
    Timeout  = -2, /* Communication timeout error */
    Checksum = -3, /* Checksum error */
};

std::error_code make_error_code(enum DEVICE_ErrorCode e) noexcept;

namespace std {
    template <> struct is_error_code_enum<DEVICE_ErrorCode> : true_type {};
} // namespace std

#endif // DEVICE_ERRORCODE_H
