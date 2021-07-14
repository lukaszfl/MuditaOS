add_library(platform STATIC)
target_include_directories(platform PUBLIC include)
add_subdirectory(${PROJECT_TARGET_NAME})
