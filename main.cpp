#include <stdio.h>
#include <fstream>

#include "GodotShaderTranscompiler/ShaderTranscompiler.h"

int main() {
	std::ifstream t("shader.gds");
	std::string shader((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
	t.close();


	gd::GLSLOutput out;
	gd::ShaderTranscompiler::Transcompile(shader, out);

	printf("[vertex]\n%s\n\n", out.Vertex.c_str());
	printf("[fragment]\n%s\n\n", out.Fragment.c_str());

	
	return 0;
}