#include "dear_imgui.h"

#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include <iostream>
#include "content.h"
#include "sprite.h"

using namespace Blah;

TextureRef BT::DearImgui::game;

namespace
{
	MeshRef mesh;
	MaterialRef material;
	ShaderRef shader;
	TextureRef tex;

	Batch batcher;

	std::unordered_map<ImTextureID, TextureRef> textures;

	const ShaderData opengl_shader_data = {
		"#version 330\n"
		"uniform mat4 u_matrix;"
		"layout(location=0) in vec2 a_position;\n"
		"layout(location=1) in vec2 a_tex;\n"
		"layout(location=2) in vec4 a_color;\n"
		"out vec2 v_tex;\n"
		"out vec4 v_color;\n"
		"void main() {\n"
		"	gl_Position = u_matrix * vec4(a_position.xy, 0, 1);\n"
		"	v_tex = a_tex;"
		"	v_color = a_color;\n"
		"}\n",

		"#version 330\n"
		"uniform sampler2D u_texture;\n"
		"in vec2 v_tex;\n"
		"in vec4 v_color;\n"
		"out vec4 frag_color;\n"
		"void main() {\n"
		"	frag_color = texture(u_texture, v_tex.st) * v_color;\n"
		"}\n"
	};

	const char* d3d11_shader = { ""
		"cbuffer constants : register(b0)\n"
		"{\n"
		"	row_major float4x4 u_matrix;\n"
		"}\n"

		"struct vs_in\n"
		"{\n"
		"	float2 position : POS;\n"
		"	float2 texcoord : TEX;\n"
		"	float4 color : COL;\n"
		"};\n"

		"struct vs_out\n"
		"{\n"
		"	float4 position : SV_POSITION;\n"
		"	float2 texcoord : TEX;\n"
		"	float4 color : COL;\n"
		"};\n"

		"Texture2D    u_texture : register(t0);\n"
		"SamplerState u_sampler : register(s0);\n"

		"vs_out vs_main(vs_in input)\n"
		"{\n"
		"	vs_out output;\n"

		"	output.position = mul(float4(input.position, 0.0f, 1.0f), u_matrix);\n"
		"	output.texcoord = input.texcoord;\n"
		"	output.color = input.color;\n"

		"	return output;\n"
		"}\n"

		"float4 ps_main(vs_out input) : SV_TARGET\n"
		"{\n"
		"	return u_texture.Sample(u_sampler, input.texcoord) * input.color;\n"
		"}\n" };

	const ShaderData d3d11_shader_data = {
		d3d11_shader,
		d3d11_shader,
		{
			{ "POS", 0 },
			{ "TEX", 0 },
			{ "COL", 0 },
		}
	};

	const VertexFormat format = VertexFormat({
		{ 0, VertexType::Float2, false },
		{ 1, VertexType::Float2, false },
		{ 2, VertexType::UByte4, true }
		});
}

void BT::DearImgui::startup()
{
	// Initializes ImGui
	ImGui::CreateContext();

	// Create ImGui Config
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.DisplaySize.x = (float)(App::draw_width());
	io.DisplaySize.y = (float)(App::draw_height());
	io.BackendFlags = 0;
	//io.ConfigFlags = ImGuiConfigFlags_DockingEnable;
	io.DisplayFramebufferScale = ImVec2(App::content_scale(), App::content_scale());

	// Key Mapping
	{
		io.KeyMap[ImGuiKey_Tab] = (int)Key::Tab;
		io.KeyMap[ImGuiKey_LeftArrow] = (int)Key::Left;
		io.KeyMap[ImGuiKey_RightArrow] = (int)Key::Right;
		io.KeyMap[ImGuiKey_UpArrow] = (int)Key::Up;
		io.KeyMap[ImGuiKey_DownArrow] = (int)Key::Down;
		io.KeyMap[ImGuiKey_PageUp] = (int)Key::PageUp;
		io.KeyMap[ImGuiKey_PageDown] = (int)Key::PageDown;
		io.KeyMap[ImGuiKey_Home] = (int)Key::Home;
		io.KeyMap[ImGuiKey_End] = (int)Key::End;
		io.KeyMap[ImGuiKey_Insert] = (int)Key::Insert;
		io.KeyMap[ImGuiKey_Delete] = (int)Key::Delete;
		io.KeyMap[ImGuiKey_Backspace] = (int)Key::Backspace;
		io.KeyMap[ImGuiKey_Space] = (int)Key::Space;
		io.KeyMap[ImGuiKey_Enter] = (int)Key::Enter;
		io.KeyMap[ImGuiKey_Escape] = (int)Key::Escape;
		io.KeyMap[ImGuiKey_KeyPadEnter] = (int)Key::KeypadEnter;
		io.KeyMap[ImGuiKey_A] = (int)Key::A;
		io.KeyMap[ImGuiKey_C] = (int)Key::C;
		io.KeyMap[ImGuiKey_V] = (int)Key::V;
		io.KeyMap[ImGuiKey_X] = (int)Key::X;
		io.KeyMap[ImGuiKey_Y] = (int)Key::Y;
		io.KeyMap[ImGuiKey_Z] = (int)Key::Z;
	}

	// Create Font Texture
	{
		io.Fonts->AddFontDefault();

		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
		tex = Texture::create(width, height, TextureFormat::RGBA, pixels);

		io.Fonts->SetTexID((ImTextureID)&tex);
	}

	{
		//help = Content::find_sprite("help")->get_animation("idle")->frames[0].image.texture;

		//maped[(ImTextureID)&help] = help;
		//textures[(ImTextureID)&game] = game;
		textures[(ImTextureID)&tex] = tex;
	}

	// Create Default Resources
	{
		if (App::renderer() == Renderer::OpenGL)
			shader = Shader::create(opengl_shader_data);
		else if (App::renderer() == Renderer::D3D11)
			shader = Shader::create(d3d11_shader_data);
		material = Material::create(shader);
		mesh = Mesh::create();
	}
}

void BT::DearImgui::update()
{
	// Setup ImGui Frame Data
	{
		auto draw_size = Point(App::draw_width(), App::draw_height());
		auto scale = App::content_scale();
		auto mouse = Input::mouse_draw();

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize.x = (float)(draw_size.x / scale);
		io.DisplaySize.y = (float)(draw_size.y / scale);
		io.MousePos = ImVec2(mouse.x / scale, mouse.y / scale);
		io.MouseDown[0] = Input::down(MouseButton::Left);
		io.MouseDown[1] = Input::down(MouseButton::Right);
		io.MouseDown[2] = Input::down(MouseButton::Middle);
		io.MouseWheel = (float)Input::mouse_wheel().y;
		io.MouseWheelH = (float)Input::mouse_wheel().x;
		io.KeyCtrl = Input::ctrl();
		io.KeyShift = Input::shift();
		io.KeyAlt = Input::alt();

		// toggle held keys
		for (int i = 0; i < 512; i++)
			io.KeysDown[i] = Input::down((Key)i);

		// add text strings
		io.AddInputCharactersUTF8(Input::text());

		ImGui::NewFrame();
	}
}

void BT::DearImgui::render()
{
	// render imgui
	ImGui::Render();
	ImDrawData* data = ImGui::GetDrawData();

	// set up render call
	RenderPass pass;
	pass.target = App::backbuffer;
	pass.mesh = mesh;
	pass.material = material;
	pass.has_scissor = true;
	pass.blend = BlendMode(BlendOp::Add, BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha);

	// size
	Vec2 size = Point(App::draw_width(), App::draw_height());

	// apply the ortho matrix
	Mat4x4 mat =
		Mat4x4::create_scale(data->FramebufferScale.x, data->FramebufferScale.y, 1.0f) *
		Mat4x4::create_ortho_offcenter(0, size.x, size.y, 0, 0.1f, 1000.0f);
	material->set_value("u_matrix", &mat.m11, 16);

	// draw imgui buffers to the screen
	for (int i = 0; i < data->CmdListsCount; i++)
	{
		ImDrawList* list = data->CmdLists[i];

		mesh->vertex_data(format, list->VtxBuffer.begin(), list->VtxBuffer.size());
		mesh->index_data(IndexFormat::UInt32, list->IdxBuffer.begin(), list->IdxBuffer.size());

		for (ImDrawCmd* cmd = list->CmdBuffer.begin(); cmd < list->CmdBuffer.end(); cmd++)
		{
			// Todo:
			// use cmd->TextureId to get the current texture we should draw

			if (auto texture = textures[cmd->TextureId])
				material->set_texture(0, texture);

			//material->set_texture(0, tex);

			material->set_sampler(0, TextureSampler(TextureFilter::Nearest));

			if (cmd->UserCallback != nullptr)
			{
				cmd->UserCallback(list, cmd);
			}
			else
			{
				pass.index_start = cmd->IdxOffset;
				pass.index_count = cmd->ElemCount;
				pass.scissor = Rect(
					cmd->ClipRect.x,
					cmd->ClipRect.y,
					(cmd->ClipRect.z - cmd->ClipRect.x),
					(cmd->ClipRect.w - cmd->ClipRect.y))
					.scale(data->FramebufferScale.x, data->FramebufferScale.y);

				pass.perform();
			}
		}
	}
}

void BT::DearImgui::display_texture(TextureRef texture, const Vec2 size, const Vec2 uv0, const Vec2 uv1, const Vec4& tint_col, const Vec4& border_col)
{
	textures[(ImTextureID)&texture] = texture;
	ImGui::Image((ImTextureID)&texture,
		ImVec2(size.x, size.y),
		ImVec2(uv0.x, uv0.y),
		ImVec2(uv1.x, uv1.y),
		ImVec4(tint_col.x, tint_col.y, tint_col.z, tint_col.w),
		ImVec4(border_col.x, border_col.y, border_col.z, border_col.w)
	);
}