find_package(LLVM REQUIRED)
message("Found LLVM ${LLVM_PACKAGE_VERSION}, build type ${LLVM_BUILD_TYPE}")
add_definitions(${LLVM_DEFINITIONS})
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})

add_subdirectory(lib)
add_subdirectory(tools)

if(ENABLE_TESTS)
    add_subdirectory(unittests)
endif()
