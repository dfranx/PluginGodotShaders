cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 17)
project(GodotShaders)

set(CMAKE_MODULE_PATH "./cmake")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ./bin)

# source code
set(SOURCES
	dllmain.cpp
	GodotShaders.cpp
	src/CanvasMaterial.cpp
	src/Sprite.cpp
	src/ResourceManager.cpp
	src/ShaderCompiler.cpp

# UI
	src/UIHelper.cpp

# shader transcompiler
	libs/GodotShaderTranscompiler/Godot/shader_types.cpp
	libs/GodotShaderTranscompiler/Godot/shader_language.cpp
	libs/GodotShaderTranscompiler/Godot/shader_compiler_gles3.cpp
	libs/GodotShaderTranscompiler/Godot/misc.cpp	
	libs/GodotShaderTranscompiler/ShaderTranscompiler.cpp

# libraries
	libs/imgui/imgui_draw.cpp
	libs/imgui/imgui_widgets.cpp
	libs/imgui/imgui.cpp
	libs/pugixml/src/pugixml.cpp
)

# cmake toolchain 
if(CMAKE_TOOLCHAIN_FILE)
    include(${CMAKE_TOOLCHAIN_FILE})
endif(CMAKE_TOOLCHAIN_FILE)

# opengl
find_package(OpenGL REQUIRED)

# glew
find_package(GLEW REQUIRED)

# glm
find_package(GLM REQUIRED)

# glslang
set(ENABLE_SPVREMAPPER OFF CACHE BOOL "Enables building of SPVRemapper" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "Builds glslangValidator and spirv-remap" FORCE)
add_subdirectory(libs/glslang)

# create executable
add_library(GodotShaders SHARED ${SOURCES})

# properties
set_target_properties(GodotShaders PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

set_target_properties(GodotShaders PROPERTIES OUTPUT_NAME "plugin")
set_target_properties(GodotShaders PROPERTIES PREFIX "")

# include directories
target_include_directories(GodotShaders PRIVATE ${GLEW_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS})
target_include_directories(GodotShaders PRIVATE libs inc libs/glslang)

# link libraries
target_link_libraries(GodotShaders ${GLM_LIBRARY_DIRS} ${OPENGL_LIBRARIES} glslang SPIRV)

if(WIN32)
	# link specific win32 libraries
	target_link_libraries(GodotShaders GLEW::GLEW)
else()
	# link linux libraries
	target_link_libraries(GodotShaders ${GLEW_LIBRARIES})
endif()

if (NOT MSVC)
	target_compile_options(GodotShaders PRIVATE -Wno-narrowing)
endif()