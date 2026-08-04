cmake_minimum_required(VERSION 3.28)
project(Vazza)

set(CMAKE_CXX_STANDARD	20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)


include_directories(SYSTEM $ENV{VULKAN_SDK/include})
find_library(Slang_LIBRARY NAMES slang slang-compiler HINTS "$ENV{VULKAN_SDK}/lib" REQUIRED)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/VMA)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glm)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/KTX-Software/ ktx)
if(TARGET gtest)
    message(STATUS "KTX provided gtest")
endif()

if(TARGET gtest_main)
    message(STATUS "KTX provided gtest_main")
endif()
get_target_property(GTEST_TYPE gtest TYPE)
get_target_property(GTEST_LIBS gtest LINK_LIBRARIES)
get_target_property(GTEST_INCLUDES gtest INTERFACE_INCLUDE_DIRECTORIES)

message(STATUS "gtest TYPE: ${GTEST_TYPE}")
message(STATUS "gtest LIBS: ${GTEST_LIBS}")
message(STATUS "gtest INCLUDES: ${GTEST_INCLUDES}")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/volk)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/tinyobjloader)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL)

set(SDL_LIB ${CMAKE_CURRENT_BINARY_DIR}/SDL/libSDL3.so)
set(VOLK_LIB ${CMAKE_CURRENT_BINARY_DIR}/volk/libvolk.a)
set(TINY_OBJ_LIB ${CMAKE_CURRENT_BINARY_DIR}/tinyobjloader/libtinyobjloader.a)
#set(KTX_READ_LIB ${CMAKE_CURRENT_BINARY_DIR}/KTX-Software/libktx_read.so)
#set(KTX_LIB ${CMAKE_CURRENT_BINARY_DIR}/KTX-Software/libktx.so)

#set(GLM_LIB ${CMAKE_CURRENT_BINARY_DIR}/glm/build/glm/libglm.a)
set(VMA_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/VMA/include)
set(SDL_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL/include)
set(VOLK_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/volk)
set(TINY_OBJ_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/tinyobjloader/)
set(GLM_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glm/)
set(KTX_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/KTX-Software/lib/include)
set(VULKAN_INCLUDE "$ENV{VULKAN_SDK}/include")

#add_subdirectory(thirdparty/SDL)
#dd_subdirectory(thirdparty/KTX-Software)
#dd_subdirectory(thirdparty/volk)
#dd_subdirectory(thirdparty/VMA)
#dd_subdirectory(thirdparty/glm)


file(GLOB SOURCES CONFIGURE_DEPENDS "src/*.cpp")
add_executable(${CMAKE_PROJECT_NAME} src/main.cpp ${SOURCES} assets/shader.slang)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE include)


#add_dependencies(${CMAKE_PROJECT_NAME} SDL)
add_dependencies(${CMAKE_PROJECT_NAME} volk)
add_dependencies(${CMAKE_PROJECT_NAME} tinyobjloader)
add_dependencies(${CMAKE_PROJECT_NAME} ktx)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
	${SDL_INCLUDE}
	${VMA_INCLUDE}
	${VOLK_INCLUDE}
	${TINY_OBJ_INCLUDE}
	${GLM_INCLUDE}
	${KTX_INCLUDE}
	${VULKAN_INCLUDE}
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
	SDL3::SDL3
	volk
	tinyobjloader
	${KTX_READ_LIB}
	ktx
	${Slang_LIBRARY}
	#	${KTX_LIB}
#	${GLM_LIB}
	Vulkan::Vulkan
)

enable_testing()
add_executable(run_tests src/tests/unit_tests.cpp assets/shader.slang)
target_include_directories(run_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/KTX-Software/tests/gtest/include
)
target_link_libraries(run_tests PRIVATE
	gtest
	gtest
)
add_dependencies(run_tests gtest)

include(GoogleTest)
gtest_discover_tests(run_tests)
