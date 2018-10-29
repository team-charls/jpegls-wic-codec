#pragma once

#include <charls/publictypes.h>

class charls_category : public std::error_category
{
public:
    const char* name() const noexcept override
    {
        return "charls";
    }

    std::string message(int /* error_value */) const override
    {
        return "CharLS error";
    }
};

class charls_error : public std::system_error
{
public:
    explicit charls_error(charls::ApiResult errorCode)
        : system_error(static_cast<int>(errorCode), CharLSCategoryInstance())
    {
    }

    charls_error(charls::ApiResult errorCode, const std::string& message)
        : system_error(static_cast<int>(errorCode), CharLSCategoryInstance(), message)
    {
    }

private:
    static const std::error_category& CharLSCategoryInstance() noexcept
    {
        static charls_category instance;
        return instance;
    }
};

