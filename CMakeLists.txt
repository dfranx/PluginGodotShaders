cmake_minimum_required(VERSION 3.1)
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
	libs/nativefiledialog/nfd_common.c
)

# cmake toolchain 
if(CMAKE_TOOLCHAIN_FILE)
    include(${CMAKE_TOOLCHAIN_FILE})
endif(CMAKE_TOOLCHAIN_FILE)

# nfd
if(WIN32)
	set(SOURCES
		"${SOURCES}"
		libs/nativefiledialog/nfd_win.cpp
	)
elseif(APPLE)
	set(SOURCES
		"${SOURCES}"
		libs/nativefiledialog/nfd_cocoa.m
	)
else()
	set(SOURCES
		"${SOURCES}"
		libs/nativefiledialog/nfd_gtk.c
	)
	
	# link gtk on linux
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(GTK REQUIRED gtk+-3.0)
	link_directories(${GTK_LIBRARY_DIRS})
	add_definitions(${GTK_CFLAGS})
endif()

# sdl2
if(WIN32 OR APPLE OR NOT ${USE_PKG_CHECK_SDL})
  find_package(SDL2 REQUIRED)
else()
  # find sdl2 on linux
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(SDL2 REQUIRED sdl2)
endif()

# opengl
find_package(OpenGL REQUIRED)

# glew
find_package(GLEW REQUIRED)

# glm
find_package(GLM REQUIRED)

# create executable
add_library(GodotShaders SHARED ${SOURCES})

set_target_properties(GodotShaders PROPERTIES OUTPUT_NAME "plugin")
set_target_properties(GodotShaders PROPERTIES PREFIX "")

# include directories
target_include_directories(GodotShaders PRIVATE ${SDL2_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS})
target_include_directories(GodotShaders PRIVATE libs inc)

# link libraries
target_link_libraries(GodotShaders ${GLM_LIBRARY_DIRS} ${OPENGL_LIBRARIES})

if(WIN32)
	# link specific win32 libraries
	target_link_libraries(GodotShaders GLEW::GLEW SDL2::SDL2)
else()
	# link linux libraries
	target_link_libraries(GodotShaders ${GLEW_LIBRARIES} ${SDL2_LIBRARIES})
endif()

if (NOT MSVC)
	target_compile_options(GodotShaders PRIVATE -Wno-narrowing)
endif()