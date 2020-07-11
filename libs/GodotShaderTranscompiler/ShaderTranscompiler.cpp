#include "ShaderTranscompiler.h"
#include "Godot/shader_language.h"

#include "Godot/shader_compiler_gles3.h"
#include "Godot/misc.h"

namespace gd
{
	VisualServer::ShaderMode ShaderTranscompiler::GetShaderType(const std::string& code)
	{
		std::string mode_string = ShaderLanguage::get_shader_type(code);

		if (mode_string == "canvas_item")
			return VisualServer::SHADER_CANVAS_ITEM;
		else if (mode_string == "particles")
			return VisualServer::SHADER_PARTICLES;

		return VisualServer::SHADER_SPATIAL;
	}

	void ShaderTranscompiler::Transcompile(const std::string& in, GLSLOutput& out)
	{
		ShaderCompilerGLES3::GeneratedCode genCode;
		VisualServer::ShaderMode shaderType = gd::ShaderTranscompiler::GetShaderType(in);

		ShaderCompilerGLES3::IdentifierActions actions_canvas;
		ShaderCompilerGLES3::IdentifierActions* actions = nullptr;

		bool usesColor = false;
		out.SCREEN_TEXTURE = false;
		out.SCREEN_UV = false;
		out.TIME = false;
		out.Error = false;
		out.LightMode = Shader::CanvasItem::LIGHT_MODE_UNSHADED;
		out.BlendMode = Shader::CanvasItem::BLEND_MODE_MIX;
		out.SkipVertexTransform = false;
		out.Uniforms.clear();

		if (shaderType == VisualServer::SHADER_CANVAS_ITEM) {
			actions_canvas.render_mode_values["blend_add"] = std::pair<int*, int>(&out.BlendMode, Shader::CanvasItem::BLEND_MODE_ADD);
			actions_canvas.render_mode_values["blend_mix"] = std::pair<int*, int>(&out.BlendMode, Shader::CanvasItem::BLEND_MODE_MIX);
			actions_canvas.render_mode_values["blend_sub"] = std::pair<int*, int>(&out.BlendMode, Shader::CanvasItem::BLEND_MODE_SUB);
			actions_canvas.render_mode_values["blend_mul"] = std::pair<int*, int>(&out.BlendMode, Shader::CanvasItem::BLEND_MODE_MUL);
			actions_canvas.render_mode_values["blend_premul_alpha"] = std::pair<int*, int>(&out.BlendMode, Shader::CanvasItem::BLEND_MODE_PREMULT_ALPHA);
			actions_canvas.render_mode_values["blend_disabled"] = std::pair<int*, int>(&out.BlendMode, Shader::CanvasItem::BLEND_MODE_DISABLED);

			actions_canvas.render_mode_values["unshaded"] = std::pair<int*, int>(&out.LightMode, Shader::CanvasItem::LIGHT_MODE_UNSHADED);
			actions_canvas.render_mode_values["light_only"] = std::pair<int*, int>(&out.LightMode, Shader::CanvasItem::LIGHT_MODE_LIGHT_ONLY);

			actions_canvas.render_mode_values["skip_vertex_transform"] = std::pair<int*, int>((int*)&out.SkipVertexTransform, 1);

			actions_canvas.usage_flag_pointers["SCREEN_UV"] = &out.SCREEN_UV;
			actions_canvas.usage_flag_pointers["SCREEN_PIXEL_SIZE"] = &out.SCREEN_UV;
			actions_canvas.usage_flag_pointers["SCREEN_TEXTURE"] = &out.SCREEN_TEXTURE;
			actions_canvas.usage_flag_pointers["TIME"] = &out.TIME;
			actions_canvas.usage_flag_pointers["COLOR"] = &usesColor;

			actions = &actions_canvas;
			actions->uniforms = &out.Uniforms;
		} // TODO: spatial

		std::string errMsg = "";
		int errLine = 0;


		ShaderCompilerGLES3 compiler;
		Error res = compiler.compile(VisualServer::SHADER_CANVAS_ITEM, in, actions, "shader.gds", genCode, errMsg, errLine);
		
		if (res != OK) {
			out.Error = true;
			out.ErrorMessage = errMsg;
			out.ErrorLine = errLine;
		}

		out.HasFragmentShader = genCode.fragment.size() > 0;
		out.HasVertexShader = genCode.vertex.size() > 0;



		out.Fragment = "#version 330\n";

		// inputs
		out.Fragment += "in vec2 uv_interp;\n";
		out.Fragment += "in vec4 color_interp;\n\n";

		// output
		out.Fragment += "layout(location = 0) out vec4 frag_color;\n\n";

		// user uniforms
		for (const auto& key : out.Uniforms)
			if (!ShaderLanguage::is_sampler_type(key.second.type))
				out.Fragment += "uniform " + ShaderLanguage::get_datatype_name(key.second.type) + " " + /*" m_" + */key.first + ";\n";

		// textures
		out.Fragment += "\nuniform sampler2D TEXTURE; // texunit:0";
		if (out.SCREEN_TEXTURE)
			out.Fragment += "\nuniform sampler2D SCREEN_TEXTURE; // texunit:1";
		out.Fragment += "\n\n";

		// system uniforms
		if (out.SCREEN_UV)
			out.Fragment += "uniform vec2 SCREEN_PIXEL_SIZE;\n"; // 1/w, 1/h
		if (genCode.uses_fragment_time)
			out.Fragment += "uniform float TIME;\n"; // 1/w, 1/h

		// fragment globals
		out.Fragment += genCode.fragment_global + "\n";

		// main()
		out.Fragment += "void main()\n{\n";
		out.Fragment += "\tvec4 COLOR = color_interp;\n";
		if (usesColor && !out.SCREEN_UV) out.Fragment += "#line 650001\n";
		out.Fragment += "\tvec2 UV = uv_interp;\n";
		if (!usesColor) {
			if (!out.SCREEN_UV) out.Fragment += "#line 650001\n";
			out.Fragment += "\nCOLOR *= texture(TEXTURE, UV);\n";
		}
		if (out.SCREEN_UV) {
			out.Fragment += "#line 650001\n";
			out.Fragment += "\tvec2 SCREEN_UV = gl_FragCoord.xy * SCREEN_PIXEL_SIZE;\n";
		}
		out.Fragment += genCode.fragment;
		out.Fragment += "#line 700001\n";
		out.Fragment += "\tfrag_color = COLOR;\n";
		out.Fragment += "}\n";




		out.Vertex = "#version 330\n";

		// inputs
		out.Vertex += "layout(location = 0) in vec3 pos_attrib;\n";
		out.Vertex += "layout(location = 1) in vec2 uv_attrib;\n\n";
		out.Vertex += "layout(location = 2) in vec4 color_attrib;\n";

		// output
		out.Vertex += "out vec2 uv_interp;\n";
		out.Vertex += "out vec4 color_interp;\n\n";

		// user uniforms
		for (const auto& key : out.Uniforms)
			if (!ShaderLanguage::is_sampler_type(key.second.type))
				out.Vertex += "uniform " + ShaderLanguage::get_datatype_name(key.second.type) + " " + /*" m_" + */key.first + ";\n";

		// system uniforms
		if (genCode.uses_vertex_time)
			out.Vertex += "uniform float TIME;\n";
		out.Vertex += "uniform mat4 PROJECTION_MATRIX;\n";
		out.Vertex += "uniform mat4 WORLD_MATRIX;\n\n";

		// vertex globals
		out.Vertex += genCode.vertex_global + "\n";

		// main()
		out.Vertex += "void main()\n{\n";
		out.Vertex += "\tvec4 COLOR = color_attrib;\n";
		out.Vertex += "\tvec4 outvec = WORLD_MATRIX * vec4(pos_attrib.xy, 0.0, 1.0);\n";
		out.Vertex += "\tvec2 VERTEX = outvec.xy;\n";
		out.Vertex += "\tfloat POINT_SIZE = 1.0;\n";
		out.Vertex += "#line 650001\n";
		out.Vertex += "\tvec2 UV = uv_attrib;\n";
		out.Vertex += genCode.vertex;
		out.Vertex += "#line 700001\n";
		out.Vertex += "\tgl_PointSize = POINT_SIZE;\n";
		out.Vertex += "\tuv_interp = UV;\n";
		out.Vertex += "\tcolor_interp = COLOR;\n";
		out.Vertex += "\toutvec.xy = VERTEX;\n";
		out.Vertex += "\tgl_Position = PROJECTION_MATRIX * outvec;\n";
		out.Vertex += "}\n";
	}
}