#pragma once

#include "Types.h"
#include <vulkan/vulkan_core.h>

namespace Luna
{
    class VkError
    {
    private:
        VkResult result;
        int32 lineNum;
        string message;
        string funcName;
        string fileName;
        
    public:
        explicit VkError() noexcept;
        explicit VkError(const VkResult res,
            const string_view func,
            const string_view file,
            const int32 line,
            const string_view message = "") noexcept;
        
        string ToString() const;
    };
    
    #ifndef VkThrowIfFailed
    #define VkThrowIfFailed(x)                                                      \
    {                                                                               \
        VkResult res = (x);                                                         \
        if(res != VK_SUCCESS) { throw VkError(res, __func__, __FILE__, __LINE__); } \
    }
    #endif

    #ifndef VkThrowIfFailure
    #define VkThrowIfFailure(x, msg)                                                     \
    {                                                                                    \
        VkResult res = (x);                                                              \
        if(res != VK_SUCCESS) { throw VkError(res, __func__, __FILE__, __LINE__, msg); } \
    }
    #endif

    #ifndef VkThrowIfError
    #define VkThrowIfError(x, condicional)                                  \
    {                                                                       \
        if(condicional) { throw VkError(x, __func__, __FILE__, __LINE__); } \
    }
    #endif

    #ifndef VkThrowIfErrorMessage
    #define VkThrowIfErrorMessage(x, condicional, msg)                           \
    {                                                                            \
        if(condicional) { throw VkError(x, __func__, __FILE__, __LINE__, msg); } \
    }
    #endif
}