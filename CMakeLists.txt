include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
cmake_minimum_required(VERSION 4.2)

set(NAME Evoke)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project({NAME} VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)  

if(DEFINED VULKAN_SDK_PATH)
    set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include")
    set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib")
    set(Vulkan_FOUND "True")
else()
    find_package(Vulkan REQUIRED)
endif()
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Couldn't find vulkan library!")
endif()
include_directories(${Vulkan_INCLUDE_DIRS})
link_directories(${Vulkan_LIBRARIES})

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
list(FILTER SOURCES EXCLUDE REGEX ".*/external/.*")

add_executable(${NAME} ${SOURCES})

add_subdirectory(external/glfw)

# Link GLFW
target_link_libraries(${NAME} PRIVATE glfw)
target_include_directories(${NAME} PRIVATE external/glfw/include)

 target_include_directories(${NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}
    ${Vulkan_INCLUDE_DIRS}
    )
 
  target_link_directories(${NAME} PUBLIC
    ${Vulkan_LIBRARIES}
  )

target_include_directories(${NAME} PRIVATE external/glm)

target_link_libraries(${NAME} PRIVATE ${Vulkan_LIBRARIES}/vulkan-1.lib)