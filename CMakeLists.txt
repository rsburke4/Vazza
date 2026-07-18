cmake_minimum_required(VERSION 3.28)
project(LineDance)

set(CMAKE_CXX_STANDARD	20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

#Definitions
#add_definitions(-DVK_NO_PROTOTYPES)

find_package(Vulkan REQUIRED)

include(ExternalProject)
ExternalProject_Add(SDL3
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL
	BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/SDL
	INSTALL_COMMAND ""
	BUILD_ALWAYS 0
)
ExternalProject_Add(VOLK
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/volk
	BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/volk
	INSTALL_COMMAND ""
	BUILD_ALWAYS 0
)
ExternalProject_Add(TINY_OBJ
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/tinyobjloader
	BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/tinyobjloader
	INSTALL_COMMAND ""
	BUILD_ALWAYS 0
)
#ExternalProject_Add(KTX_SOFT
#	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/KTX-Software
#	BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/KTX-Software
#	INSTALL_COMMAND ""
#	BUILD_ALWAYS 0
#)

#ExternalProject_Add(VMA
#	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/VMA
#	BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/VMA
#	INSTALL_COMMAND ""
#	BUILD_ALWAYS 1
#)


include_directories(SYSTEM $ENV{VULKAN_SDK/include})
find_library(Slang_LIBRARY NAMES slang slang-compiler HINTS "$ENV{VULKAN_SDK}/lib" REQUIRED)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/VMA)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glm)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/KTX-Software/ ktx)

set(SDL_LIB ${CMAKE_CURRENT_BINARY_DIR}/SDL/libSDL3.so)
set(VOLK_LIB ${CMAKE_CURRENT_BINARY_DIR}/volk/libvolk.a)
set(TINY_OBJ_LIB ${CMAKE_CURRENT_BINARY_DIR}/tinyobjloader/libtinyobjloader.a)
#set(KTX_READ_LIB ${CMAKE_CURRENT_BINARY_DIR}/KTX-Software/libktx_read.so)
#set(KTX_LIB ${CMAKE_CURRENT_BINARY_DIR}/KTX-Software/libktx.so)

#set(GLM_LIB ${CMAKE_CURRENT_BINARY_DIR}/glm/build/glm/libglm.a)
#set(VMA_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/VMA/include)
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


#target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
	#thirdparty/SDL/SDL3
	#libvolk.a
	#libktx_read.so
	#libktx.so
	#)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(${CMAKE_PROJECT_NAME} src/main.cpp ${SOURCES} assets/shader.slang)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE include)

#set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
#	CMAKE_CXX_STANDARD 20
#	CMAKE_CXX_STANDARD_REQUIRED YES
#	CMAKE_CXX_EXTENSIONS OFF)


add_dependencies(${CMAKE_PROJECT_NAME} SDL3)
add_dependencies(${CMAKE_PROJECT_NAME} VOLK)
add_dependencies(${CMAKE_PROJECT_NAME} TINY_OBJ)
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
	${SDL_LIB}
	${VOLK_LIB}
	${TINY_OBJ_LIB}
	${KTX_READ_LIB}
	ktx
	${Slang_LIBRARY}
	#	${KTX_LIB}
#	${GLM_LIB}
	Vulkan::Vulkan
)
