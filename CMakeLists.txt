cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(OpenGL_Framework)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(SHADERS_PATH ${CMAKE_SOURCE_DIR}/shaders CACHE PATH "Project specific path. Set manually if it was not found.")

set(RESOURCES_PATH ${CMAKE_SOURCE_DIR}/recources CACHE PATH "Project specific path. Set manually if it was not found.")

set(OPENCL_KERNEL_PATH ${CMAKE_SOURCE_DIR}/openCL CACHE PATH "Project specific path. Set manually if it was not found.")

set(DEPENDENCIES_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/dependencies CACHE PATH "Project specific path. Set manually if it was not found.")

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp")

set_target_properties(PROPERTIES LINK_FLAGS "$<$<CONFIG:DEBUG>:/NODEFAULTLIB:MSVCRT")

add_definitions(-DSHADERS_PATH="${SHADERS_PATH}")
add_definitions(-DOPENCL_KERNEL_PATH="${OPENCL_KERNEL_PATH}")
add_definitions(-DRESOURCES_PATH="${RESOURCES_PATH}")
add_definitions(-DGLFW_INCLUDE_GLCOREARB)
add_definitions(-DGLEW_STATIC)
add_definitions(-DGL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED)
add_definitions(-DGLEW_NO_GLU)

## include libraries
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(OpenCL REQUIRED)
find_package(TinyOBJ REQUIRED)
find_package(ASSIMP REQUIRED)


include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${OPENGL_INCLUDE_PATH} ${GLEW_INCLUDE_PATH} ${GLM_INCLUDE_PATH} ${GLFW3_INCLUDE_PATH} ${OpenCL_INCLUDE_PATH} ${TinyOBJ_INCLUDE_PATH} ${ASSIMP_INCLUDE_PATH})
link_libraries(${GLEW_LIBRARY} ${GLFW3_LIBRARY} ${OpenCL_LIBRARY} ${OPENGL_LIBRARIES} ${TinyOBJ_LIBRARY} ${TinyOBJ_LIBRARY} ${ASSIMP_LIBRARY})

add_subdirectory(Src)
add_subdirectory(openCL)
add_subdirectory(shaders)
add_subdirectory(recources)
