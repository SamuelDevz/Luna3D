if(BUILD_DIRECT3D11)
    add_subdirectory(d3d11)
elseif(BUILD_DIRECT3D12)
    add_subdirectory(d3d12)
elseif(BUILD_VULKAN)
    add_subdirectory(vulkan)
else()
    message(FATAL_ERROR "Please choose a graphics API. Options:" 
        "   BUILD_DIRECT3D11"
        "   BUILD_DIRECT3D12)"
        "   BUILD_VULKAN")
endif()