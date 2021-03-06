/*
 * Copyright 2011-2013 Branimir Karadzic. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#include "common.h"

#include <bgfx.h>
#include <bx/timer.h>
#include "entry.h"
#include "dbg.h"
#include "fpumath.h"
#include "processevents.h"
#include "voxel.h"
#include "gui.h"

#include <stdio.h>
#include <string.h>
#include <vector>
#include <map>
#include <string>

#include "misc.h"
#include "mesh.h"
#include "anim.h"
#include "kv6.h"
#include "cub.h"
#include "vxl.h"
#include "particle.h"
#include <imgui\imgui.h>
#include "entitydef.h"
#include "craft.h"
#include "inventory.h"
#include "world.h"
#include "player.h"
#include "graphics.h"

static const char* s_shaderPath = NULL;

static void shaderFilePath(char* _out, const char* _name)
{
	strcpy(_out, s_shaderPath);
	strcat(_out, _name);
	strcat(_out, ".bin");
}

static const bgfx::Memory* load(const char* _filePath)
{
	FILE* file = fopen(_filePath, "rb");
	if (NULL != file)
	{
		uint32_t size = (uint32_t)fsize(file);
		const bgfx::Memory* mem = bgfx::alloc(size+1);
		size_t ignore = fread(mem->data, 1, size, file);
		BX_UNUSED(ignore);
		fclose(file);
		mem->data[mem->size-1] = '\0';
		return mem;
	}

	return NULL;
}

static const bgfx::Memory* loadShader(const char* _name)
{
	char filePath[512];
	shaderFilePath(filePath, _name);
	return load(filePath);
}


bgfx::UniformHandle u_flash;
bgfx::TextureHandle g_whiteTexture;
bgfx::UniformHandle u_tex;

#include <Windows.h>
int _main_(int /*_argc*/, char** /*_argv*/)
{
	uint32_t width = 1280;
	uint32_t height = 720;
	uint32_t debug = BGFX_DEBUG_TEXT;
	uint32_t reset = BGFX_RESET_VSYNC;

	CoreType::InitClassTypes();


	bgfx::init();
	InitGraphics();

	bgfx::reset(width, height, reset);

	// Enable debug text.
	bgfx::setDebug(debug);

	// Set view 0 clear state.
	bgfx::setViewClear(0
		, BGFX_CLEAR_COLOR_BIT|BGFX_CLEAR_DEPTH_BIT
		, 0x7f99b2ff
		, 1.0f
		, 0
		);

	EntityDefManager()->LoadGroup( "entities.template" );
	CraftRecipeDefManager()->LoadGroup( "craft.rec" );
	InventoryItemDefManager()->LoadGroup( "inventory.items" );
	EntityDefManager()->Get( "level1.def" );
	ShaderUniformDefManager()->LoadGroup( "shader_uniforms.sun" );
	ShaderProgramDefManager()->LoadGroup( "shaders.shdr" );
	MaterialDefManager()->LoadGroup( "materials.mtr" );

	const bgfx::Memory *mapMem = load( "data/stbmap.vxl" );
	map_s map;
	load_vxl_memory( &map, mapMem->data );

	// Setup root path for binary shaders. Shader binaries are different 
	// for each renderer.
	switch (bgfx::getRendererType() )
	{
	default:
	case bgfx::RendererType::Direct3D9:
		s_shaderPath = "shaders/dx9/";
		break;

	case bgfx::RendererType::Direct3D11:
		s_shaderPath = "shaders/dx11/";
		break;

	case bgfx::RendererType::OpenGL:
		s_shaderPath = "shaders/glsl/";
		break;

	case bgfx::RendererType::OpenGLES2:
	case bgfx::RendererType::OpenGLES3:
		s_shaderPath = "shaders/gles/";
		break;
	}

	const bgfx::Memory* whiteTexMem = bgfx::alloc(4 * 4 * 4);
	memset( whiteTexMem->data, 0xff, whiteTexMem->size );
	g_whiteTexture = bgfx::createTexture2D(4, 4, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_NONE, whiteTexMem);

	initParticleDecl();
	initMeshDecl();
	guiInit();

	float playerScale = 1.f/8.f;
	float playerOffsetZ = 20.f * playerScale;

	const bgfx::Memory* mem;

	// Load vertex shader.
	//mem = loadShader("vs_cubes");
	//bgfx::VertexShaderHandle vsh = bgfx::createVertexShader(mem);

	// Load fragment shader.
	//mem = loadShader("fs_cubes");
	//u_flash = bgfx::createUniform("u_flash",     bgfx::UniformType::Uniform1f);
	//u_tex = bgfx::createUniform("u_tex", bgfx::UniformType::Uniform1i);

	//bgfx::FragmentShaderHandle fsh = bgfx::createFragmentShader(mem);

	// Create program from shaders.
	//bgfx::ProgramHandle program = bgfx::createProgram(vsh, fsh);

	// We can destroy vertex and fragment shader here since
	// their reference is kept inside bgfx after calling createProgram.
	// Vertex and fragment shader will be destroyed once program is
	// destroyed.
	//bgfx::destroyVertexShader(vsh);
	//bgfx::destroyFragmentShader(fsh);


	FILE* file = fopen("font/droidsans.ttf", "rb");
	uint32_t size = (uint32_t)fsize(file);
	void* data = malloc(size);
	size_t ignore = fread(data, 1, size, file);
	BX_UNUSED(ignore);
	fclose(file);

	imguiCreate(data, size);

	free(data);

	World world;
	world.Load( "world1.world" );

	float at[3] = { 0.0f, 0.0f, 0.0f };
	float eye[3] = { 52.0f, 52.0f, 32.0f };
	float up[3] = { 0.f, 0.f, 1.f };

	int64_t timeOffset = bx::getHPCounter();

	particleSystem_s testps;
	testps.def = NULL;

	float error = 0.f;
	MouseState mouseState;
	while (!processEvents(width, height, debug, reset, &mouseState) )
	{		
		int64_t now = bx::getHPCounter();
		static int64_t last = now;
		const int64_t frameTime = now - last;
		last = now;
		const double freq = double(bx::getHPFrequency() );
		const double toMs = 1000.0/freq;

		float time = (float)( (now-timeOffset)/double(bx::getHPFrequency() ) );

		error += time;

		if ( error >= (1.f/30.f) )
		{
	// update
			imguiBeginFrame( mouseState.m_mx
				, mouseState.m_my
				, (mouseState.m_buttons[entry::MouseButton::Left  ] ? IMGUI_MBUT_LEFT  : 0)
				| (mouseState.m_buttons[entry::MouseButton::Right ] ? IMGUI_MBUT_RIGHT : 0)
				, 0
				, width
				, height
				);

			float deltaDT = min( time, 6.f/60.f) ;
			world.UpdateBegin();
			world.UpdateDelta( deltaDT );
			world.UpdateEnd();

			error -= time;
		
			imguiEndFrame();

	// render
			float view[16];
			float proj[16];
			Player *player = (Player*)world.FindEntityInCurrentLevel( "player0" );
			const float *pos = player->GetPos();
			const float rotz = player->GetRotZ();
			eye[0] = (eye[0]) * 0.97f + (pos[0]-10.f*sinf(rotz)) * 0.03f;
			eye[1] = (eye[1]) * 0.97f + (pos[1]-10.f*cosf(rotz)) * 0.03f;
			eye[2] = (eye[2]) * 0.95f + (pos[2]+2.f) * 0.05f;
			at[0] = (at[0]) * 0.9f + (pos[0]) * 0.1f;
			at[1] = (at[1]) * 0.9f + (pos[1]) * 0.1f;
			at[2] = (at[2]) * 0.9f + (pos[2]) * 0.1f;
		
			mtxLookAtUp(view, eye, at, up);
			mtxProj(proj, 60.0f, 16.0f/9.0f, 0.1f, 1000.0f);

			// Set view and projection matrix for view 0.
			bgfx::setViewTransform(0, view, proj);
			// Set view 0 default viewport.
			bgfx::setViewRect(0, 0, 0, (uint16_t)width, (uint16_t)height);

			// This dummy draw call is here to make sure that view 0 is cleared
			// if no other draw calls are submitted to view 0.
			bgfx::submit(0);

			bgfx::dbgTextPrintf(0, 3, 0x0f, "Frame: % 7.3f[ms]", double(frameTime)*toMs);

			float mtx[16];
			mtxIdentity(mtx);
			// Set model matrix for rendering.
			bgfx::setTransform(mtx);
			// Set vertex and fragment shaders.
			//bgfx::setProgram(program);

			world.Render();

			// Set view and projection matrix for view 1.
			mtxOrtho(proj, 0.f, 1280.f, 720.f, 0.f, 0.0f, 1000.f);
			bgfx::setViewTransform(1, NULL, proj);
			bgfx::setTransform(mtx);
			bgfx::setViewRect(1, 0, 0, (uint16_t)width, (uint16_t)height);
			bgfx::submit(1);
			world.RenderGUI();

			bgfx::frame();
		}
	}

	// Cleanup.
	//bgfx::destroyProgram(program);

	// Shutdown bgfx.
	bgfx::shutdown();

	return 0;
}
