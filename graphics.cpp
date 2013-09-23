#include "graphics.h"
#include "fpumath.h"
#include "misc.h"
#include <map>
#include <string>
#include <bgfx.h>

bgfx::VertexDecl g_PosColorDecl;

static const char* s_shaderPath = NULL;

static void shaderFilePath(char* _out, const char* _name)
{
	strcpy(_out, s_shaderPath);
	strcat(_out, _name);
	strcat(_out, ".bin");
}


void CreateWhiteTexture( TextureDef *def )
{
	const bgfx::Memory* whiteTexMem = bgfx::alloc(4 * 4 * 4);
	memset( whiteTexMem->data, 0xff, whiteTexMem->size );
	def->handle = bgfx::createTexture2D(4, 4, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_NONE, whiteTexMem);
}

void InitGraphics()
{
	// Create vertex stream declaration.
	g_PosColorDecl.begin();
	g_PosColorDecl.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
	g_PosColorDecl.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);
	g_PosColorDecl.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true);
	g_PosColorDecl.end();

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

	TextureDefManager()->AddFunctionTexture( "_white", CreateWhiteTexture );
}

bool ShaderUniformDef::Load( const char *filename )
{
	return false;
}

void ShaderUniformDef::Parse( const char *name, const char *&cursor )
{
	std::string token;
	while ( ParseToken( token, cursor ) && token != "}" )
	{
		if ( token == "type" )
		{
			ParseToken( token, cursor );
			if ( token == "float4" )
			{
				handle = bgfx::createUniform( name, bgfx::UniformType::Uniform4fv );
			}
			else if ( token == "texture" )
			{
				handle = bgfx::createUniform( name, bgfx::UniformType::Uniform1i );
			}
		}
	}
}

void ShaderUniformDef::Reload()
{
}

void ShaderUniformDef::Clear()
{
}


ShaderUniformDefMgr *ShaderUniformDefManager()
{
	static ShaderUniformDefMgr s_mgr;
	return &s_mgr;
}

bool ShaderProgramDef::Load( const char *filename )
{
	return false;
}


static const bgfx::Memory* load(const char* _filePath)
{
	FILE* file = fopen(_filePath, "rb");
	if (NULL != file)
	{
		uint32_t size = (uint32_t)fsize(file);
		const bgfx::Memory* mem = bgfx::alloc(size+1);
		size_t ignore = fread(mem->data, 1, size, file);
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

void ShaderProgramDef::Parse( const char *name, const char * &cursor )
{
	std::string token;
	while ( ParseToken( token, cursor ) && token != "}" )
	{
		if ( token == "fragment" )
		{
			ParseToken( token, cursor );
			const bgfx::Memory *mem = loadShader(token.c_str());
			fragmentHandle = bgfx::createFragmentShader(mem);
		}
		else if ( token == "vertex" )
		{
			ParseToken( token, cursor );
			const bgfx::Memory *mem = loadShader(token.c_str());
			vertexHandle = bgfx::createVertexShader(mem);
		}
	}
	programHandle = bgfx::createProgram( vertexHandle, fragmentHandle );
}

void ShaderProgramDef::Reload()
{
}

void ShaderProgramDef::Clear()
{
}

ShaderProgramDefMgr *ShaderProgramDefManager()
{
	static ShaderProgramDefMgr s_mgr;
	return &s_mgr;
}

static void ParseTextures( MaterialDef::Pass &pass, const char *&cursor )
{
	ParseExpect( "{", cursor );
	std::string token;
	while ( ParseToken( token, cursor ) && token != "}" )
	{
		pass.m_textures.push_back( MaterialDef::Texture() );
		MaterialDef::Texture &tex = pass.m_textures.back();
		GenerateGUID( tex.id, token.c_str() );

		tex.handle = bgfx::createUniform( token.c_str(), bgfx::UniformType::Uniform1i );

		ParseToken( token, cursor );
		tex.index = atoi( token.c_str() );

		ParseToken( token, cursor );
		tex.texture = TextureDefManager()->Get( token.c_str() );
	}
}

static void ParseVector4( UniformParam v[4], const char *&cursor )
{
	memset( v, 0, sizeof(v) );
	if ( ParseTest( "{", cursor ) )
	{
		std::string token;
		int index = 0;
		while ( index < 4 && ParseToken( token, cursor ) && token != "}" )
		{
			if ( token.substr( 0, 5 ) == "param" )
			{
				v[index].type = POPS_BINDPARAM;
				v[index].i = atoi( token.substr( 5 ).c_str() );
			}
			else
			{
				v[index].type = POPS_CONST;
				v[index].f = atof( token.c_str() );
			}
			index++;
		}
		while ( ParseToken( token, cursor ) && token != "}" )
		{
		}
	}
}

static void ParseUniforms( MaterialDef::Pass &pass, const char *&cursor )
{
	ParseExpect( "{", cursor );
	std::string token;
	while ( ParseToken( token, cursor ) && token != "}" )
	{
		pass.m_uniforms.push_back( MaterialDef::Uniform() );
		MaterialDef::Uniform &uniform = pass.m_uniforms.back();
		GenerateGUID( uniform.id, token.c_str() );

		uniform.handle = ShaderUniformDefManager()->Get( token.c_str() );

		ParseVector4( uniform.v, cursor );
	}
}

static void ParsePass( MaterialDef::Pass &pass, const char *&cursor )
{
	ParseExpect( "{", cursor );
	std::string token;
	while ( ParseToken( token, cursor ) && token != "}" )
	{
		if ( token == "textures" )
		{
			ParseTextures( pass, cursor );
		}
		else if ( token == "uniforms" )
		{
			ParseUniforms( pass, cursor );
		}
		else if ( token == "shader" )
		{
			ParseToken( token, cursor );
			pass.m_shaderProgram = ShaderProgramDefManager()->Get( token.c_str() );
		}
	}
}

bool MaterialDef::Load( const char * filename )
{
	return false;
}

void MaterialDef::Parse( const char *name, const char * &cursor )
{
	std::string token;
	while ( ParseToken( token, cursor ) )
	{
		if ( token == "pass" )
		{
			m_passes.push_back( Pass() );
			Pass &pass = m_passes.back();
			ParsePass( pass, cursor );
		}
	}
}

void MaterialDef::Reload()
{
}

void MaterialDef::Clear()
{
}

void MaterialDef::Bind() const
{
}

int  MaterialDef::GetNumPasses() const
{
	return (int)m_passes.size();
}

static void EvaluateUniform( float out[4], const UniformParam in[4], const float *binds )
{
	for (int i=0; i<4; i++)
	{
		switch (in[i].type)
		{
		case POPS_ZERO:
			out[i] = 0.f;
			break;
		case POPS_CONST:
			out[i] = in[i].f;
			break;
		case POPS_BINDPARAM:
			out[i] = binds[in[i].i];
			break;
		};
	}
}

void MaterialDef::BindPass( int i, const float *binds ) const
{
	Pass const &p = m_passes[i];
	for (unsigned int j=0; j<p.m_textures.size(); j++)
	{
		bgfx::setTexture( p.m_textures[j].index, p.m_textures[j].handle, p.m_textures[j].texture->handle );
	}
	for (unsigned int j=0; j<p.m_uniforms.size(); j++)
	{
		float uniforms[4];
		EvaluateUniform( uniforms, p.m_uniforms[j].v, binds );
		bgfx::setUniform( p.m_uniforms[j].handle->handle, uniforms );
	}
	bgfx::setProgram( p.m_shaderProgram->programHandle );
}


MaterialDefMgr *MaterialDefManager()
{
	static MaterialDefMgr s_mgr;
	return &s_mgr;
}


bool TextureDef::Load( const char *filename )
{
	const bgfx::Memory* mem = load(filename);
	handle = bgfx::createTexture(mem);
	return true;
}

void TextureDef::Reload()
{
}

void TextureDef::Clear()
{
}


void TextureDefMgr::AddFunctionTexture( const char *name, CreateTexture texfn )
{
	if ( Get( name, false ) )
		return;

	Guid g;
	GenerateGUID( g, name );

	TextureDef* def = new TextureDef;
	def->create_f = texfn;
	def->create_f( def );
	m_defs[g] = def;
}

TextureDefMgr *TextureDefManager()
{
	static TextureDefMgr s_mgr;
	return &s_mgr;
}
