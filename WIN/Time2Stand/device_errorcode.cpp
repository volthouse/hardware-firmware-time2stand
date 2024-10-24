#include "device_errorcode.h"

#define DEVICE_EBASE -200000

std::error_code make_error_code(enum DEVICE_ErrorCode e) noexcept
{
    /* Singelton of the Category */
    static const struct : std::error_category
    {
        virtual const char* name() const noexcept override
        {
            return "Device";
        };

        virtual std::string message(int ev) const override
        {
            switch (static_cast<DEVICE_ErrorCode>(ev - DEVICE_EBASE)) {
            case DEVICE_ErrorCode::Success:
                return "Success";
            case DEVICE_ErrorCode::Error:
                return "General Error";
            case DEVICE_ErrorCode::Timeout:
                return "Communication";
            case DEVICE_ErrorCode::Checksum:
                return "Checksum";
            default:
                return "(unrecognized error)";
            }
        }

    } DEVICECategory;

    if (static_cast<int>(e)){
        return {static_cast<int>(e) DEVICE_EBASE, DEVICECategory};
    }

    return {static_cast<int>(e) , DEVICECategory};
};


