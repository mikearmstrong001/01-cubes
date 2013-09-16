#include "workbench_menu.h"
#include "gui.h"


WorkbenchMenu::WorkbenchMenu( Entity *i, Entity *u ) : m_item(i), m_user(u)
{
}

void WorkbenchMenu::Render()
{
	guiDrawRoundedRect( 10.f, 10.f, 1260.f, 700.f, 10.f, guiColourRGBA( 64, 64, 64, 255 ) );
}

