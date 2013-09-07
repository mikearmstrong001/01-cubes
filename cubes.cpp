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
#include "world.h"
#include "player.h"

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


struct entityattach_s
{
	int ent;
	std::string name;
};

struct entitystate_s
{
	entityattach_s attach;

	AnimMesh mesh;
	float pos[3];
	float rotz;
	float wmtx[16];

	float vel[3];
	
	float animTime;

	int weaponIndex;

	float animDir;
	bool onGround;

	int stateIndex;
	float stateTime;
	float noProgress;
	float moveSpeed;

	unsigned int collisionMask;
	int collisionHeight;
	entitystate_s *voxelNext;

	void (*fixedupdate)( entitystate_s *self, map_s *map, float dt );
	void (*deltaupdate)( entitystate_s *self, map_s *map, float dt );
	void (*guiupdate)( entitystate_s *self, map_s *map );
	void (*render)( entitystate_s *self, map_s *map );
};


void grenadeThink( entitystate_s *self, map_s *map, float dt )
{
#if 0
	self->vel[2] -= 9.8f * dt;
	HitInfo_s hi;
	if ( IntersectRay( hi, self->pos, self->vel, map, (unsigned int)(~(0x1|0x4)) ) )
	{
		int f[3] = { (int)(self->pos[0] - 3.f), (int)(self->pos[1] - 3.f), (int)(self->pos[2] - 2.f) };
		int t[3] = { (int)(self->pos[0] + 3.f), (int)(self->pos[1] + 3.f), (int)(self->pos[2] + 2.f) };
		clearMap( map, mapMesh, f, t );
		self->fixedupdate = NULL;
		self->deltaupdate = NULL;
	}
	else
	{
		self->pos[0] += self->vel[0];
		self->pos[1] += self->vel[1];
		self->pos[2] += self->vel[2];
	}
#endif
}

void zombieThink( entitystate_s *self, map_s *map, float dt )
{
#if 0
	// wander
	if ( self->moveSpeed < 0.001f && self->onGround )
	{
		self->noProgress -= 1.f;
	}
	if ( self->noProgress < 0.f )
	{
		// quick check for player
		self->rotz += 3.14f;
		self->noProgress = 5 * 60.f;
		self->stateIndex = 0;
	}
	float moveSpeed = 1.f;
	if ( self->stateIndex == 0 )
	{
		self->stateTime -= 1.f;
		if ( self->stateTime < 0.f )
		{
			self->rotz += ((rand() % 3) - 1) * 0.2f;
			self->stateTime = (float)(rand() % 100);

			entitystate_s *playerOne = &entities[0];

			float delta[3];
			vec3Sub( delta, playerOne->pos, self->pos );
			float distSq = vec3LenSq( delta );
			if ( distSq < (16.f*16.f) )
			{
				float lookDir[3] = {sinf(self->rotz), cosf(self->rotz), 0.f};
				float dp = vec3Dot( lookDir, delta );
				if ( dp > 0.f )
				{
					self->rotz = atan2f( delta[0], delta[1] );
					self->stateIndex = 1;
				}
			}
		}
	}
	else
	if ( self->stateIndex == 1 )
	{
		float delta[3];
		entitystate_s *playerOne = &entities[0];
		vec3Sub( delta, playerOne->pos, self->pos );
		float distSq = vec3LenSq( delta );
		if ( distSq < (32.f*32.f) )
		{
			float lookDir[3] = {sinf(self->rotz), cosf(self->rotz), 0.f};
			float dp = vec3Dot( lookDir, delta );
			if ( dp > 0.f )
			{
				self->rotz = atan2f( delta[0], delta[1] );
				moveSpeed *= 3.f;
				if ( distSq < 1.f )
				{
					self->stateIndex = 2;
					self->stateTime = 50.f;
				}
			}
			else
			{
				self->stateIndex = 0;
			}
		}
		else
		{
			self->stateIndex = 0;
		}
	}
	else
	if ( self->stateIndex == 2 )
	{
		moveSpeed = 0.f;
		self->stateTime -= 1.f;
		if ( self->stateTime < 0.f )
		{
			int f[3] = { (int)(self->pos[0] - 4.f), (int)(self->pos[1] - 4.f), (int)(self->pos[2] - 2.f) };
			int t[3] = { (int)(self->pos[0] + 4.f), (int)(self->pos[1] + 4.f), (int)(self->pos[2] + 2.f) };
			clearMap( map, mapMesh, f, t );
			self->stateIndex = 3;
			self->fixedupdate = NULL;
			self->deltaupdate = NULL;
		}
	}
	else
	if ( self->stateIndex == 3 )
	{
		moveSpeed = 0.f;
	}

	self->animDir = 1.f;
	if ( self->onGround )
	{
		self->vel[0] = moveSpeed*sinf(self->rotz)*dt;
		self->vel[1] = moveSpeed*cosf(self->rotz)*dt;
	}

	animUpdate( self, "walk" );

	move( self->pos, self->vel, map );
#endif
}

void nothingThink( entitystate_s * /*self*/, map_s * /*map*/, float /*dt*/ )
{
}

void avoidThink( entitystate_s *self, map_s *map, float dt )
{
#if 0
	// wander
	if ( self->moveSpeed < 0.001f && self->onGround )
	{
		self->noProgress -= 1.f;
	}
	if ( self->noProgress < 0.f )
	{
		// quick check for player
		self->rotz += 3.14f;
		self->noProgress = 5 * 60.f;
		self->stateIndex = 0;
	}
	float moveSpeed = 1.f;
	if ( self->stateIndex == 0 )
	{
		self->stateTime -= 1.f;
		if ( self->stateTime < 0.f )
		{
			self->rotz += ((rand() % 3) - 1) * 0.2f;
			self->stateTime = (float)(rand() % 100);

			float delta[3];
			entitystate_s *playerOne = &entities[0];
			vec3Sub( delta, playerOne->pos, self->pos );
			float distSq = vec3LenSq( delta );
			if ( distSq < (16.f*16.f) )
			{
				float lookDir[3] = {sinf(self->rotz), cosf(self->rotz), 0.f};
				float dp = vec3Dot( lookDir, delta );
				if ( dp > 0.f )
				{
					self->rotz = atan2f( delta[1], delta[0] );
					self->stateIndex = 1;
					self->stateTime = 200;
				}
			}
		}
	}
	else
	if ( self->stateIndex == 1 )
	{
		moveSpeed *= 3.f;
		self->stateTime -= 1.f;
		if ( self->stateTime < 0.f )
		{
			self->stateIndex = 0;
		}
	}

	self->animDir = 1.f;
	if ( self->onGround )
	{
		self->vel[0] = moveSpeed*sinf(self->rotz)*dt;
		self->vel[1] = moveSpeed*cosf(self->rotz)*dt;
	}

	animUpdate( self, "walk" );

	move( self->pos, self->vel, map );
#endif
}

#include <Windows.h>
int _main_(int /*_argc*/, char** /*_argv*/)
{
	uint32_t width = 1280;
	uint32_t height = 720;
	uint32_t debug = BGFX_DEBUG_TEXT;
	uint32_t reset = BGFX_RESET_VSYNC;

	bgfx::init();
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

	EntityDefManager()->Get( "level1.def" );

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

	initParticleDecl();
	initMeshDecl();

	float playerScale = 1.f/8.f;
	float playerOffsetZ = 20.f * playerScale;
#if 0
	entities[0].animTime = 0.f;
	entities[0].rotz = 0.f;
	entities[0].weaponIndex = 0;
	entities[0].pos[0] = 128.f;
	entities[0].pos[1] = 256.f;
	entities[0].pos[2] = 0.f;
	entities[0].vel[0] = 0.f;
	entities[0].vel[1] = 0.f;
	entities[0].vel[2] = 0.f;
	entities[0].deltaupdate = playerThink;
	entities[0].voxelNext = NULL;
	entities[0].collisionHeight = 5;
	entities[0].collisionMask = 0x1;
	entities[0].attach.ent = -1;
	load_animmesh( entities[0].mesh, "dwarf.am", playerScale );

	entities[1].animTime = 0.f;
	entities[1].rotz = 0.f;
	entities[1].weaponIndex = 0;
	entities[1].pos[0] = 128.f;
	entities[1].pos[1] = 256.f;
	entities[1].pos[2] = 0.f;
	entities[1].vel[0] = 0.f;
	entities[1].vel[1] = 0.f;
	entities[1].vel[2] = 0.f;
	entities[1].deltaupdate = nothingThink;
	entities[1].voxelNext = NULL;
	entities[1].collisionHeight = 5;
	entities[1].collisionMask = 0x1;
	entities[1].attach.ent = 0;
	entities[1].attach.name = "hand";
	load_animmesh( entities[1].mesh, "sword.am", playerScale );


	for (int i=2; i<50; i++)
	{
		entities[i].animTime = 0.f;
		entities[i].rotz = (rand() % 100) / 7.f;
		entities[i].weaponIndex = 0;
		entities[i].pos[0] = (float)(rand() % 512);
		entities[i].pos[1] = (float)(rand() % 512);
		entities[i].pos[2] = 64.f;
		entities[i].vel[0] = 0.f;
		entities[i].vel[1] = 0.f;
		entities[i].vel[2] = 0.f;
		entities[i].deltaupdate = (rand() & 1) ? zombieThink : avoidThink;
		entities[i].stateTime = 0.f;
		entities[i].noProgress = 100.f;
		entities[i].voxelNext = NULL;
		entities[i].collisionHeight = 5;
		entities[i].collisionMask = 0x2;
		entities[i].attach.ent = -1;
		if ( entities[i].deltaupdate == avoidThink )
		{
			load_animmesh( entities[i].mesh, rand() & 1 ? "sheep.am" :"wolf.am", playerScale );
		}
		else
		{
			load_animmesh( entities[i].mesh, "zombie.am", playerScale );
		}
	}
#endif

	const bgfx::Memory* mem;

	// Load vertex shader.
	mem = loadShader("vs_cubes");
	bgfx::VertexShaderHandle vsh = bgfx::createVertexShader(mem);

	// Load fragment shader.
	mem = loadShader("fs_cubes");
	bgfx::FragmentShaderHandle fsh = bgfx::createFragmentShader(mem);

	// Create program from shaders.
	bgfx::ProgramHandle program = bgfx::createProgram(vsh, fsh);

	// We can destroy vertex and fragment shader here since
	// their reference is kept inside bgfx after calling createProgram.
	// Vertex and fragment shader will be destroyed once program is
	// destroyed.
	bgfx::destroyVertexShader(vsh);
	bgfx::destroyFragmentShader(fsh);


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
			world.UpdateDelta( deltaDT );

			error -= time;
		
			if ( GetAsyncKeyState( 'Y' ) & 1 )
				initParticleSystem( testps, "explode.psys" );

			updateParticleSystem( testps, 1.f/60.f );

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
			bgfx::setProgram(program);

			world.Render();

			mtxIdentity(mtx);
			// Set model matrix for rendering.
			//bgfx::setTransform(entities[0].wmtx);
			//renderParticleSystem( testps );
			// Advance to next frame. Rendering thread will be kicked to 
			// process submitted rendering primitives.
			bgfx::frame();
		}
	}

	// Cleanup.
	bgfx::destroyProgram(program);

	// Shutdown bgfx.
	bgfx::shutdown();

	return 0;
}
