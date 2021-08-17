include(FetchContent)

macro(FetchCMakeUtilities)
    FetchContent_Declare(
        cmake_utility
        GIT_REPOSITORY https://github.com/Hoshiningen/cmake_utility
    )

    FetchContent_GetProperties(cmake_utility)

    if (NOT cmake_utility_POPULATED)
        FetchContent_Populate(cmake_utility)
    endif()
endmacro()