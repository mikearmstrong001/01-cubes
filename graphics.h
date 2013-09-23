#pragma once

#include "misc.h"
#include <bgfx.h>
#include <map>
#include <vector>

enum ParamOps
{
	POPS_ZERO = 0,
	POPS_CONST,
	POPS_BINDPARAM
};

struct UniformParam
{
	ParamOps type;
	union
	{
		int i;
		float f;
	};
};

struct ShaderUniformDef
{
	bgfx::UniformHandle handle;

	bool Load( const char *filename );
	void Parse( const char *name, const char *&cursor );
	void Reload();
	void Clear();
};

class ShaderUniformDefMgr : public TypedDefMgr<ShaderUniformDef, true>
{

public:

};

ShaderUniformDefMgr *ShaderUniformDefManager();

typedef void (*CreateTexture)( struct TextureDef *def );
struct TextureDef
{
	bgfx::TextureHandle handle;

	CreateTexture create_f;
	std::string filename;

	bool Load( const char *filename );
	void Parse( const char *name, const char *&cursor );
	void Reload();
	void Clear();
};

class TextureDefMgr : public TypedDefMgr<TextureDef>
{

public:

	void AddFunctionTexture( const char *name, CreateTexture texfn );
};

TextureDefMgr *TextureDefManager();

struct ShaderProgramDef
{
	bgfx::VertexShaderHandle vertexHandle;
	bgfx::FragmentShaderHandle fragmentHandle;
	bgfx::ProgramHandle programHandle;

	bool Load( const char *filename );
	void Parse( const char *name, const char * &cursor );
	void Reload();
	void Clear();
};

class ShaderProgramDefMgr : public TypedDefMgr<ShaderProgramDef,true>
{

public:

};

ShaderProgramDefMgr *ShaderProgramDefManager();

struct MaterialDef
{
	struct Texture
	{
		Guid id;
		int index;
		bgfx::UniformHandle handle;
		const TextureDef *texture;
	};

	struct Uniform
	{
		Guid id;
		const ShaderUniformDef *handle;
		UniformParam v[4];
	};

	struct Pass
	{
		std::vector<Texture> m_textures;
		std::vector<Uniform> m_uniforms;
		const ShaderProgramDef *m_shaderProgram;
	};

	std::vector<Pass> m_passes;

//public:

	bool Load( const char *filename );
	void Parse( const char *name, const char *&cursor );
	void Reload();
	void Clear();

	void Bind() const;
	int  GetNumPasses() const;
	void BindPass( int i, const float *binds ) const;
};

class MaterialDefMgr : public TypedDefMgr<MaterialDef,true>
{

public:

};

MaterialDefMgr *MaterialDefManager();

void InitGraphics();