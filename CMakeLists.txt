cmake_minimum_required(VERSION 3.15)

set(PATCH_VERSION "0" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})

project(main_allocator VERSION ${PROJECT_VESRION})

configure_file(version.h.in version.h)

add_executable(main_allocator main_allocator.cpp)


set_target_properties( main_allocator PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_include_directories(main_allocator
    PRIVATE "${CMAKE_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}"
)

# include_directories("${CMAKE_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}"
# )

if (MSVC)
    target_compile_options(main_allocator PRIVATE
        /W4
    )
else ()
    target_compile_options(main_allocator PRIVATE
        -Wall -Wextra -pedantic -Werror
    )
endif()



install(TARGETS main_allocator RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT guerchoig@gmail.com)

include(CPack)

# enable_testing()

# add_test(test_version test_version)
