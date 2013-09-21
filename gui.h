#pragma once

enum guiTextAlignX
{
	GUI_ALIGNX_CENTER,
	GUI_ALIGNX_RIGHT,
	GUI_ALIGNX_LEFT,
};

enum guiTextAlignY
{
	GUI_ALIGNY_CENTER,
	GUI_ALIGNY_TOP,
	GUI_ALIGNY_BOTTOM,
};

inline unsigned int guiColourRGBA( unsigned char r, unsigned char g, unsigned char b, unsigned char a )
{
	return (((unsigned int)r)<<0) | 
		(((unsigned int)g)<<8) | 
		(((unsigned int)b)<<16) | 
		(((unsigned int)a)<<24);
}

void guiInit();
void guiDrawRect(float _x, float _y, float w, float h, unsigned int _argb, float _fth = 1.0f);
void guiDrawRoundedRect(float _x, float _y, float w, float h, float r, unsigned int _argb, float _fth = 1.0f);
void guiDrawLine(float _x0, float _y0, float _x1, float _y1, float _r, unsigned int _abgr, float _fth = 1.0f);
void guiDrawTriangle(float _x, float _y, float _width, float _height, int _flags, unsigned int _abgr);
void guiDrawText(float _x, float _y, const char* _text, guiTextAlignX _alignX, guiTextAlignY _alignY, unsigned int _abgr);
void guiDrawText3d(float pos[3], float xaxis[3], float zaxis[3], const char* _text, guiTextAlignX _alignX, guiTextAlignY _alignY, unsigned int _abgr);



