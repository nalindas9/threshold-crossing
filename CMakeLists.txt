cmake_minimum_required(VERSION 3.18  FATAL_ERROR)

project(trial_project C CXX)

#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:main")

set(CUSTOM_BUILD_DIR "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

# GLEW setup (opengl function loader)
add_compile_definitions("GLEW_STATIC")
set(GLEW_INC ${PROJECT_SOURCE_DIR}/glew-2.1.0/include)
find_library(opengl32 NAMES opengl32)
find_library(GLEW_LIB glew32s PATHS ${PROJECT_SOURCE_DIR}/glew-2.1.0/lib/)

# GLFW setup (windowing system)
set(GLFW_INC ${PROJECT_SOURCE_DIR}/glfw/include)
find_library(GLFW_LIB NAMES glfw3 HINTS "${PROJECT_SOURCE_DIR}/glfw/lib-vc2010-64/")

# dear imgui setup
set(DEAR_IMGUI_ROOT "${PROJECT_SOURCE_DIR}/dear_imgui")
set(DEAR_IMGUI_INC 
	"${DEAR_IMGUI_ROOT}"
	"${DEAR_IMGUI_ROOT}/backends")
set(DEAR_IMGUI_SOURCES 
	"${DEAR_IMGUI_ROOT}/imgui.cpp"
	"${DEAR_IMGUI_ROOT}/imgui_draw.cpp"
	"${DEAR_IMGUI_ROOT}/imgui_widgets.cpp"
	"${DEAR_IMGUI_ROOT}/imgui_tables.cpp"
	"${DEAR_IMGUI_ROOT}/imgui_demo.cpp"
	"${DEAR_IMGUI_ROOT}/backends/imgui_impl_glfw.cpp"
	"${DEAR_IMGUI_ROOT}/backends/imgui_impl_opengl3.cpp"
	"${DEAR_IMGUI_ROOT}/addons/imguifilesystem/imguifilesystem.cpp")

set(SRC
	${DEAR_IMGUI_SOURCES} 
	"src/main.cpp")

add_executable(${PROJECT_NAME} ${SRC} "src/imgui_stuff.hpp")

# build all the things in one folder
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CUSTOM_BUILD_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CUSTOM_BUILD_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CUSTOM_BUILD_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CUSTOM_BUILD_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CUSTOM_BUILD_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CUSTOM_BUILD_DIR})

target_link_libraries(${PROJECT_NAME} ${opengl32} ${GLEW_LIB} ${GLFW_LIB} ${DEAR_IMGUI_LIB})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR} ${GLEW_INC} ${GLFW_INC} ${DEAR_IMGUI_INC})

# support c++ 20 standard
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(${PROJECT_NAME} PROPERTIES CUDA_STANDARD 20)
set_target_properties(${PROJECT_NAME} PROPERTIES CUDA_STANDARD_REQUIRED ON)

# copy fonts to output
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${PROJECT_SOURCE_DIR}/fonts" "${CUSTOM_BUILD_DIR}/fonts")
