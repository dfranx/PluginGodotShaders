# GodotShaderTranscompiler
I 'extracted' the source code for godot shader transcompiler from [Godot](https://www.github.com/godotengine/godot) into this simple library that converts Godot's shaders into GLSL. It is lightweight and has no dependencies.

It is super easy to use:
```c++
#include <GodotShaderTranscompiler/ShaderTranscompiler.h>

...
	gd::GLSLOutput out;
	gd::ShaderTranscompiler::Transcompile("GODOT SHADER HERE", out);
...
```

GLSLOutput structure will contain various information about the godot shader you've compiled, including the GLSL source code for vertex and fragment shaders.