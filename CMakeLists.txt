cmake_minimum_required(VERSION 3.1)
project(GodotShaders)

set(CMAKE_MODULE_PATH "./cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./bin)

# source code
set(SOURCES
	main.cpp
	GodotShaderTranscompiler/Godot/shader_types.cpp
	GodotShaderTranscompiler/Godot/shader_language.cpp
	GodotShaderTranscompiler/Godot/shader_compiler_gles3.cpp
	GodotShaderTranscompiler/Godot/misc.cpp	
	GodotShaderTranscompiler/ShaderTranscompiler.cpp	
)

# cmake toolchain 
if(CMAKE_TOOLCHAIN_FILE)
    include(${CMAKE_TOOLCHAIN_FILE})
endif(CMAKE_TOOLCHAIN_FILE)

# create executable
add_executable(GodotShaders ${SOURCES})

# include directories
target_include_directories(GodotShaders PRIVATE Transcompiler)

# link libraries
# target_link_libraries(GodotShaders ${GLM_LIBRARY_DIRS})

if (NOT MSVC)
	target_compile_options(GodotShaders PRIVATE -Wno-narrowing)
endif()
