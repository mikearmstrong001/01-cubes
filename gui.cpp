#include "gui.h"
#include "bgfx.h"
#include "misc.h"
#include "vs_gui_texture.bin.h"
#include "fs_gui_texture.bin.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "../../../3rdparty/stb_truetype/stb_truetype.h"
#include "fpumath.h"
#include <bx\bx.h>
#include <bx/uint32_t.h>

#define MAX_TEMP_COORDS 100
#define NUM_CIRCLE_VERTS (8 * 4)

struct PosColorUvVertex
{
	float m_x;
	float m_y;
	float m_u;
	float m_v;
	uint32_t m_abgr;
};

static bgfx::VertexDecl m_decl;
static stbtt_bakedchar m_cdata[96]; // ASCII 32..126 is 95 glyphs

static uint16_t m_textureWidth = 512;
static uint16_t m_textureHeight = 512;
static float m_invTextureWidth = 1.f / m_textureWidth;
static float m_invTextureHeight = 1.f / m_textureHeight;
static float m_halfTexel = 0.f;

static float m_tempCoords[MAX_TEMP_COORDS * 2];
static float m_tempNormals[MAX_TEMP_COORDS * 2];
static float m_circleVerts[NUM_CIRCLE_VERTS * 2];
static bgfx::TextureHandle m_fontTexture;
static bgfx::TextureHandle m_whiteTexture;
static bgfx::ProgramHandle m_textureProgram;
static bgfx::UniformHandle u_texColor;

static const float s_tabStops[4] = {150, 210, 270, 330};


void guiInit()
{
	for (int32_t ii = 0; ii < NUM_CIRCLE_VERTS; ++ii)
	{
		float a = (float)ii / (float)NUM_CIRCLE_VERTS * (float)(M_PI * 2.0);
		m_circleVerts[ii * 2 + 0] = cosf(a);
		m_circleVerts[ii * 2 + 1] = sinf(a);
	}

	m_decl.begin();
	m_decl.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float);
	m_decl.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);
	m_decl.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true);
	m_decl.end();

	u_texColor  = bgfx::createUniform("u_texColor", bgfx::UniformType::Uniform1i);

	bgfx::VertexShaderHandle vsh;
	bgfx::FragmentShaderHandle fsh;

	const bgfx::Memory* vs_gui_texture;
	const bgfx::Memory* fs_gui_texture;

	switch (bgfx::getRendererType() )
	{
	case bgfx::RendererType::Direct3D9:
		vs_gui_texture = bgfx::makeRef(vs_gui_texture_dx9, sizeof(vs_gui_texture_dx9) );
		fs_gui_texture = bgfx::makeRef(fs_gui_texture_dx9, sizeof(fs_gui_texture_dx9) );
		m_halfTexel = 0.5f;
		break;

	case bgfx::RendererType::Direct3D11:
		vs_gui_texture = bgfx::makeRef(vs_gui_texture_dx11, sizeof(vs_gui_texture_dx11) );
		fs_gui_texture = bgfx::makeRef(fs_gui_texture_dx11, sizeof(fs_gui_texture_dx11) );
		break;

	default:
		vs_gui_texture = bgfx::makeRef(vs_gui_texture_glsl, sizeof(vs_gui_texture_glsl) );
		fs_gui_texture = bgfx::makeRef(fs_gui_texture_glsl, sizeof(fs_gui_texture_glsl) );
		break;
	}

	vsh = bgfx::createVertexShader(vs_gui_texture);
	fsh = bgfx::createFragmentShader(fs_gui_texture);
	m_textureProgram = bgfx::createProgram(vsh, fsh);
	bgfx::destroyVertexShader(vsh);
	bgfx::destroyFragmentShader(fsh);

	const bgfx::Memory* mem = bgfx::alloc(m_textureWidth * m_textureHeight);
	void* _data = fload( "font/droidsans.ttf" );
	stbtt_BakeFontBitmap( (uint8_t*)_data, 0, 15.0f, mem->data, m_textureWidth, m_textureHeight, 32, 96, m_cdata);
	m_fontTexture = bgfx::createTexture2D(m_textureWidth, m_textureHeight, 1, bgfx::TextureFormat::L8, BGFX_TEXTURE_NONE, mem);


	const bgfx::Memory* whiteTexMem = bgfx::alloc(4 * 4);
	memset( whiteTexMem->data, 0xff, whiteTexMem->size );
	m_whiteTexture = bgfx::createTexture2D(4, 4, 1, bgfx::TextureFormat::L8, BGFX_TEXTURE_NONE, whiteTexMem);
}

void guiDrawPolygon(const float* _coords, uint32_t _numCoords, float _r, uint32_t _abgr)
{
	_numCoords = bx::uint32_min(_numCoords, MAX_TEMP_COORDS);

	for (uint32_t ii = 0, jj = _numCoords - 1; ii < _numCoords; jj = ii++)
	{
		const float* v0 = &_coords[jj * 2];
		const float* v1 = &_coords[ii * 2];
		float dx = v1[0] - v0[0];
		float dy = v1[1] - v0[1];
		float d = sqrtf(dx * dx + dy * dy);
		if (d > 0)
		{
			d = 1.0f / d;
			dx *= d;
			dy *= d;
		}

		m_tempNormals[jj * 2 + 0] = dy;
		m_tempNormals[jj * 2 + 1] = -dx;
	}

	for (uint32_t ii = 0, jj = _numCoords - 1; ii < _numCoords; jj = ii++)
	{
		float dlx0 = m_tempNormals[jj * 2 + 0];
		float dly0 = m_tempNormals[jj * 2 + 1];
		float dlx1 = m_tempNormals[ii * 2 + 0];
		float dly1 = m_tempNormals[ii * 2 + 1];
		float dmx = (dlx0 + dlx1) * 0.5f;
		float dmy = (dly0 + dly1) * 0.5f;
		float dmr2 = dmx * dmx + dmy * dmy;
		if (dmr2 > 0.000001f)
		{
			float scale = 1.0f / dmr2;
			if (scale > 10.0f)
			{
				scale = 10.0f;
			}

			dmx *= scale;
			dmy *= scale;
		}

		m_tempCoords[ii * 2 + 0] = _coords[ii * 2 + 0] + dmx * _r;
		m_tempCoords[ii * 2 + 1] = _coords[ii * 2 + 1] + dmy * _r;
	}

	uint32_t numVertices = _numCoords*6 + (_numCoords-2)*3;
	if (bgfx::checkAvailTransientVertexBuffer(numVertices, m_decl) )
	{
		bgfx::TransientVertexBuffer tvb;
		bgfx::allocTransientVertexBuffer(&tvb, numVertices, m_decl);
		uint32_t trans = _abgr&0xffffff;

		PosColorUvVertex* vertex = (PosColorUvVertex*)tvb.data;
		for (uint32_t ii = 0, jj = _numCoords-1; ii < _numCoords; jj = ii++)
		{
			vertex->m_x = _coords[ii*2+0];
			vertex->m_y = _coords[ii*2+1];
			vertex->m_u = 0.f;
			vertex->m_v = 0.f;
			vertex->m_abgr = _abgr;
			++vertex;

			vertex->m_x = _coords[jj*2+0];
			vertex->m_y = _coords[jj*2+1];
			vertex->m_u = 0.f;
			vertex->m_v = 0.f;
			vertex->m_abgr = _abgr;
			++vertex;

			vertex->m_x = m_tempCoords[jj*2+0];
			vertex->m_y = m_tempCoords[jj*2+1];
			vertex->m_u = 0.f;
			vertex->m_v = 0.f;
			vertex->m_abgr = trans;
			++vertex;

			vertex->m_x = m_tempCoords[jj*2+0];
			vertex->m_y = m_tempCoords[jj*2+1];
			vertex->m_u = 0.f;
			vertex->m_v = 0.f;
			vertex->m_abgr = trans;
			++vertex;

			vertex->m_x = m_tempCoords[ii*2+0];
			vertex->m_y = m_tempCoords[ii*2+1];
			vertex->m_u = 0.f;
			vertex->m_v = 0.f;
			vertex->m_abgr = trans;
			++vertex;

			vertex->m_x = _coords[ii*2+0];
			vertex->m_y = _coords[ii*2+1];
			vertex->m_u = 0.f;
			vertex->m_v = 0.f;
			vertex->m_abgr = _abgr;
			++vertex;
		}

		for (uint32_t ii = 2; ii < _numCoords; ++ii)
		{
			vertex->m_x = _coords[0];
			vertex->m_y = _coords[1];
			vertex->m_u = 0.f;
			vertex->m_v = 0.f;
			vertex->m_abgr = _abgr;
			++vertex;

			vertex->m_x = _coords[(ii-1)*2+0];
			vertex->m_y = _coords[(ii-1)*2+1];
			vertex->m_u = 0.f;
			vertex->m_v = 0.f;
			vertex->m_abgr = _abgr;
			++vertex;

			vertex->m_x = _coords[ii*2+0];
			vertex->m_y = _coords[ii*2+1];
			vertex->m_u = 0.f;
			vertex->m_v = 0.f;
			vertex->m_abgr = _abgr;
			++vertex;
		}

		bgfx::setTexture(0, u_texColor, m_whiteTexture);
		bgfx::setVertexBuffer(&tvb);
		bgfx::setState(0
			| BGFX_STATE_RGB_WRITE
			| BGFX_STATE_ALPHA_WRITE
			| BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
			);
		bgfx::setProgram(m_textureProgram);
		bgfx::submit(1);
	}
}


void guiDrawRect(float _x, float _y, float w, float h, uint32_t _argb, float _fth)
{
	float verts[4 * 2] =
	{
		_x + 0.5f,     _y + 0.5f,
		_x + w - 0.5f, _y + 0.5f,
		_x + w - 0.5f, _y + h - 0.5f,
		_x + 0.5f,     _y + h - 0.5f,
	};

	guiDrawPolygon(verts, 4, _fth, _argb);
}

void guiDrawRoundedRect(float _x, float _y, float w, float h, float r, unsigned int _argb, float _fth)
{
	const uint32_t num = NUM_CIRCLE_VERTS / 4;
	const float* cverts = m_circleVerts;
	float verts[(num + 1) * 4 * 2];
	float* vv = verts;

	for (uint32_t ii = 0; ii <= num; ++ii)
	{
		*vv++ = _x + w - r + cverts[ii * 2] * r;
		*vv++ = _y + h - r + cverts[ii * 2 + 1] * r;
	}

	for (uint32_t ii = num; ii <= num * 2; ++ii)
	{
		*vv++ = _x + r + cverts[ii * 2] * r;
		*vv++ = _y + h - r + cverts[ii * 2 + 1] * r;
	}

	for (uint32_t ii = num * 2; ii <= num * 3; ++ii)
	{
		*vv++ = _x + r + cverts[ii * 2] * r;
		*vv++ = _y + r + cverts[ii * 2 + 1] * r;
	}

	for (uint32_t ii = num * 3; ii < num * 4; ++ii)
	{
		*vv++ = _x + w - r + cverts[ii * 2] * r;
		*vv++ = _y + r + cverts[ii * 2 + 1] * r;
	}

	*vv++ = _x + w - r + cverts[0] * r;
	*vv++ = _y + r + cverts[1] * r;

	guiDrawPolygon(verts, (num + 1) * 4, _fth, _argb);
}

void guiDrawLine(float _x0, float _y0, float _x1, float _y1, float _r, unsigned int _abgr, float _fth)
{
	float dx = _x1 - _x0;
	float dy = _y1 - _y0;
	float d = sqrtf(dx * dx + dy * dy);
	if (d > 0.0001f)
	{
		d = 1.0f / d;
		dx *= d;
		dy *= d;
	}

	float nx = dy;
	float ny = -dx;
	float verts[4 * 2];
	_r -= _fth;
	_r *= 0.5f;
	if (_r < 0.01f)
	{
		_r = 0.01f;
	}

	dx *= _r;
	dy *= _r;
	nx *= _r;
	ny *= _r;

	verts[0] = _x0 - dx - nx;
	verts[1] = _y0 - dy - ny;

	verts[2] = _x0 - dx + nx;
	verts[3] = _y0 - dy + ny;

	verts[4] = _x1 + dx + nx;
	verts[5] = _y1 + dy + ny;

	verts[6] = _x1 + dx - nx;
	verts[7] = _y1 + dy - ny;

	guiDrawPolygon(verts, 4, _fth, _abgr);
}

void guiDrawTriangle(float _x, float _y, float _width, float _height, int _flags, unsigned int _abgr)
{
	if (1 == _flags)
	{
		const float verts[3 * 2] =
		{
			(float)_x + 0.5f,                        (float)_y + 0.5f,
			(float)_x + 0.5f + (float)_width * 1.0f, (float)_y + 0.5f + (float)_height / 2.0f - 0.5f,
			(float)_x + 0.5f,                        (float)_y + 0.5f + (float)_height - 1.0f,
		};

		guiDrawPolygon(verts, 3, 1.0f, _abgr);
	}
	else
	{
		const float verts[3 * 2] =
		{
			(float)_x + 0.5f,                               (float)_y + 0.5f + (float)_height - 1.0f,
			(float)_x + 0.5f + (float)_width / 2.0f - 0.5f, (float)_y + 0.5f,
			(float)_x + 0.5f + (float)_width - 1.0f,        (float)_y + 0.5f + (float)_height - 1.0f,
		};

		guiDrawPolygon(verts, 3, 1.0f, _abgr);
	}
}

static void getBakedQuad(stbtt_bakedchar* _chardata, int32_t char_index, float* _xpos, float* _ypos, stbtt_aligned_quad* _quad)
{
	stbtt_bakedchar* b = _chardata + char_index;
	int32_t round_x = STBTT_ifloor(*_xpos + b->xoff);
	int32_t round_y = STBTT_ifloor(*_ypos + b->yoff);

	_quad->x0 = (float)round_x;
	_quad->y0 = (float)round_y;
	_quad->x1 = (float)round_x + b->x1 - b->x0;
	_quad->y1 = (float)round_y + b->y1 - b->y0;

	_quad->s0 = (b->x0 + m_halfTexel) * m_invTextureWidth;
	_quad->t0 = (b->y0 + m_halfTexel) * m_invTextureWidth;
	_quad->s1 = (b->x1 + m_halfTexel) * m_invTextureHeight;
	_quad->t1 = (b->y1 + m_halfTexel) * m_invTextureHeight;

	*_xpos += b->xadvance;
}

static float getTextHeight(stbtt_bakedchar* _chardata)
{
#if 1
	return 15.f;
#else
	stbtt_bakedchar* b = _chardata + 'W' - ' ';
	return b->y1 - b->y0;
#endif
}

static float getTextLength(stbtt_bakedchar* _chardata, const char* _text, uint32_t& _numVertices)
{
	float xpos = 0;
	float len = 0;
	uint32_t numVertices = 0;

	while (*_text)
	{
		int32_t ch = (uint8_t)*_text;
		if (ch == '\t')
		{
			for (int32_t ii = 0; ii < 4; ++ii)
			{
				if (xpos < s_tabStops[ii])
				{
					xpos = s_tabStops[ii];
					break;
				}
			}
		}
		else if (ch >= ' '
				&&  ch < 128)
		{
			stbtt_bakedchar* b = _chardata + ch - ' ';
			int32_t round_x = STBTT_ifloor( (xpos + b->xoff) + 0.5);
			len = round_x + b->x1 - b->x0 + 0.5f;
			xpos += b->xadvance;
			numVertices += 6;
		}

		++_text;
	}

	_numVertices = numVertices;

	return len;
}

void guiDrawText(float _x, float _y, const char* _text, guiTextAlignX _alignX, guiTextAlignY _alignY, unsigned int _abgr)
{
	if (!_text)
	{
		return;
	}

	uint32_t numVertices = 0;
	if (_alignX == GUI_ALIGNX_CENTER)
	{
		_x -= getTextLength(m_cdata, _text, numVertices) / 2;
	}
	else if (_alignX == GUI_ALIGNX_RIGHT)
	{
		_x -= getTextLength(m_cdata, _text, numVertices);
	}
	else // just count vertices
	{
		getTextLength(m_cdata, _text, numVertices);
	}

	if (_alignY == GUI_ALIGNY_CENTER)
	{
		_y += getTextHeight(m_cdata) / 2;
	}
	else if (_alignY == GUI_ALIGNY_TOP)
	{
		_y += getTextHeight(m_cdata);
	}

	if (bgfx::checkAvailTransientVertexBuffer(numVertices, m_decl) )
	{
		bgfx::TransientVertexBuffer tvb;
		bgfx::allocTransientVertexBuffer(&tvb, numVertices, m_decl);

		PosColorUvVertex* vertex = (PosColorUvVertex*)tvb.data;

		const float ox = _x;

		while (*_text)
		{
			int32_t ch = (uint8_t)*_text;
			if (ch == '\t')
			{
				for (int32_t i = 0; i < 4; ++i)
				{
					if (_x < s_tabStops[i] + ox)
					{
						_x = s_tabStops[i] + ox;
						break;
					}
				}
			}
			else if (ch >= ' '
					&&  ch < 128)
			{
				stbtt_aligned_quad quad;
				getBakedQuad(m_cdata, ch - 32, &_x, &_y, &quad);

				vertex->m_x = quad.x0;
				vertex->m_y = quad.y0;
				vertex->m_u = quad.s0;
				vertex->m_v = quad.t0;
				vertex->m_abgr = _abgr;
				++vertex;

				vertex->m_x = quad.x1;
				vertex->m_y = quad.y1;
				vertex->m_u = quad.s1;
				vertex->m_v = quad.t1;
				vertex->m_abgr = _abgr;
				++vertex;

				vertex->m_x = quad.x1;
				vertex->m_y = quad.y0;
				vertex->m_u = quad.s1;
				vertex->m_v = quad.t0;
				vertex->m_abgr = _abgr;
				++vertex;

				vertex->m_x = quad.x0;
				vertex->m_y = quad.y0;
				vertex->m_u = quad.s0;
				vertex->m_v = quad.t0;
				vertex->m_abgr = _abgr;
				++vertex;

				vertex->m_x = quad.x0;
				vertex->m_y = quad.y1;
				vertex->m_u = quad.s0;
				vertex->m_v = quad.t1;
				vertex->m_abgr = _abgr;
				++vertex;

				vertex->m_x = quad.x1;
				vertex->m_y = quad.y1;
				vertex->m_u = quad.s1;
				vertex->m_v = quad.t1;
				vertex->m_abgr = _abgr;
				++vertex;
			}

			++_text;
		}

		bgfx::setTexture(0, u_texColor, m_fontTexture);
		bgfx::setVertexBuffer(&tvb);
		bgfx::setState(0
			| BGFX_STATE_RGB_WRITE
			| BGFX_STATE_ALPHA_WRITE
			| BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
			);
		bgfx::setProgram(m_textureProgram);
		bgfx::submit(1);
	}
}

static void getBakedQuad3d(stbtt_bakedchar* _chardata, int32_t char_index, float* _xpos, float* _ypos, float scale, stbtt_aligned_quad* _quad)
{
	stbtt_bakedchar* b = _chardata + char_index;
	float round_x = *_xpos + b->xoff * scale;
	float round_y = *_ypos - (b->yoff) * scale;

	_quad->x0 = (float)round_x;
	_quad->y0 = (float)round_y;
	_quad->x1 = (float)round_x + (b->x1 - b->x0) * scale;
	_quad->y1 = (float)round_y + ((b->y1 - b->y0)) * scale;

	_quad->s0 = (b->x0 + m_halfTexel) * m_invTextureWidth;
	_quad->t0 = (b->y0 + m_halfTexel) * m_invTextureWidth;
	_quad->s1 = (b->x1 + m_halfTexel) * m_invTextureHeight;
	_quad->t1 = (b->y1 + m_halfTexel) * m_invTextureHeight;

	*_xpos += b->xadvance * scale;
}

static float getTextLength3d(stbtt_bakedchar* _chardata, const char* _text, float scale, uint32_t& _numVertices)
{
	float xpos = 0;
	float len = 0;
	uint32_t numVertices = 0;

	while (*_text)
	{
		int32_t ch = (uint8_t)*_text;
		if (ch == '\t')
		{
			for (int32_t ii = 0; ii < 4; ++ii)
			{
				if (xpos < s_tabStops[ii] * scale)
				{
					xpos = s_tabStops[ii] * scale;
					break;
				}
			}
		}
		else if (ch >= ' '
				&&  ch < 128)
		{
			stbtt_bakedchar* b = _chardata + ch - ' ';
			float round_x = xpos + b->xoff * scale;
			len = round_x + (b->x1 - b->x0)*scale;
			xpos += b->xadvance * scale;
			numVertices += 6;
		}

		++_text;
	}

	_numVertices = numVertices;

	return len;
}

static float getTextHeight3d(stbtt_bakedchar* _chardata,float scale)
{
#if 1
	return 15.f * scale;
#else
	stbtt_bakedchar* b = _chardata + 'W' - ' ';
	return b->y1 - b->y0;
#endif
}


extern bgfx::UniformHandle u_flash;
extern bgfx::TextureHandle g_whiteTexture;
extern bgfx::UniformHandle u_tex;

inline void calcpos( PosColorVertex *vertex, float x, float y, float pos[3], float xaxis[3], float zaxis[3] )
{
	vertex->m_x = pos[0] + x * xaxis[0] - y * zaxis[0];
	vertex->m_y = pos[1] + x * xaxis[1] - y * zaxis[1];
	vertex->m_z = pos[2] + x * xaxis[2] - y * zaxis[2];
}

extern bgfx::VertexDecl s_PosColorDecl;
void guiDrawText3d(float pos[3], float xaxis[3], float zaxis[3], const char* _text, guiTextAlignX _alignX, guiTextAlignY _alignY, unsigned int _abgr)
{
	if (!_text)
	{
		return;
	}

	float _x = 0.f;
	float _z = 0.f;
	uint32_t numVertices = 0;
	if (_alignX == GUI_ALIGNX_CENTER)
	{
		_x -= getTextLength(m_cdata, _text, numVertices) / 2;
	}
	else if (_alignX == GUI_ALIGNX_RIGHT)
	{
		_x -= getTextLength(m_cdata, _text, numVertices);
	}
	else // just count vertices
	{
		getTextLength(m_cdata, _text, numVertices);
	}

	if (_alignY == GUI_ALIGNY_CENTER)
	{
		_z += getTextHeight(m_cdata) / 2;
	}
	else if (_alignY == GUI_ALIGNY_TOP)
	{
		_z += getTextHeight(m_cdata);
	}

	if (bgfx::checkAvailTransientVertexBuffer(numVertices, m_decl) )
	{
		bgfx::TransientVertexBuffer tvb;
		bgfx::allocTransientVertexBuffer(&tvb, numVertices, s_PosColorDecl);

		PosColorVertex* vertex = (PosColorVertex*)tvb.data;

		const float ox = _x;

		while (*_text)
		{
			int32_t ch = (uint8_t)*_text;
			if (ch == '\t')
			{
				for (int32_t i = 0; i < 4; ++i)
				{
					if (_x < (s_tabStops[i]) + ox)
					{
						_x = (s_tabStops[i]) + ox;
						break;
					}
				}
			}
			else if (ch >= ' '
					&&  ch < 128)
			{
				stbtt_aligned_quad quad;
				getBakedQuad(m_cdata, ch - 32, &_x, &_z, &quad);

				calcpos( vertex, quad.x0, quad.y0, pos, xaxis, zaxis );
				vertex->m_u = quad.s0;
				vertex->m_v = quad.t0;
				vertex->m_abgr = _abgr;
				++vertex;

				calcpos( vertex, quad.x1, quad.y1, pos, xaxis, zaxis );
				vertex->m_u = quad.s1;
				vertex->m_v = quad.t1;
				vertex->m_abgr = _abgr;
				++vertex;

				calcpos( vertex, quad.x1, quad.y0, pos, xaxis, zaxis );
				vertex->m_u = quad.s1;
				vertex->m_v = quad.t0;
				vertex->m_abgr = _abgr;
				++vertex;

				calcpos( vertex, quad.x0, quad.y0, pos, xaxis, zaxis );
				vertex->m_u = quad.s0;
				vertex->m_v = quad.t0;
				vertex->m_abgr = _abgr;
				++vertex;

				calcpos( vertex, quad.x0, quad.y1, pos, xaxis, zaxis );
				vertex->m_u = quad.s0;
				vertex->m_v = quad.t1;
				vertex->m_abgr = _abgr;
				++vertex;

				calcpos( vertex, quad.x1, quad.y1, pos, xaxis, zaxis );
				vertex->m_u = quad.s1;
				vertex->m_v = quad.t1;
				vertex->m_abgr = _abgr;
				++vertex;
			}

			++_text;
		}

		bgfx::setTexture(0, u_tex, m_fontTexture);
		bgfx::setVertexBuffer(&tvb);
		bgfx::setState(0
			| BGFX_STATE_RGB_WRITE
			| BGFX_STATE_DEPTH_TEST_LESS
			//| BGFX_STATE_ALPHA_WRITE
			| BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
			);
		//bgfx::setProgram(m_textureProgram);
		bgfx::submit(0);
	}
}
