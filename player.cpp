#include "player.h"
#include "voxel.h"
#include "level.h"
#include "imgui\imgui.h"
#include "misc.h"
#include "pickup.h"
#include "fpumath.h"
#include "gui.h"
#include "world.h"

static ClassCreator<Player> s_PlayerCreator( "Player" );
CoreType Player::s_Type( &Player::Super::s_Type );


#include "menu.h"
#include <Windows.h>

class Entity;

class PlayerInventoryMenu : public Menu
{
	Player *m_player;
	int m_cursor;
public:

	PlayerInventoryMenu( Player *p );

	void Update();
	void Render();
};

PlayerInventoryMenu::PlayerInventoryMenu( Player *p ) : m_player(p), m_cursor(0)
{
	p->SetInput( false );
}

void PlayerInventoryMenu::Update()
{
	Inventory &inv = m_player->GetInventory();
	if ( GetAsyncKeyState( VK_LEFT ) & 1 )
	{
		m_cursor = (m_cursor - 1 + inv.GetNumItems()) % inv.GetNumItems();
	}
	else if ( GetAsyncKeyState( VK_RIGHT ) & 1 )
	{
		m_cursor = (m_cursor + 1 + inv.GetNumItems()) % inv.GetNumItems();
	}
	else if ( GetAsyncKeyState( VK_DELETE ) & 1 )
	{
		m_player->SetInput( true );
		m_player->GetWorld()->PopMenu();
	}
}

void PlayerInventoryMenu::Render()
{
	int num = 5;
	float size = 80.f; 
	float border = 5.f;
	float spacing = size+border+border;
	Inventory &inv = m_player->GetInventory();

	guiDrawRoundedRect( (1280.f/2) - (num/2.f) * spacing - 10.f, 720 - 10.f - spacing - 10.f, num * spacing + 20.f, spacing + 20.f, 10.f, guiColourRGBA( 64, 64, 64, 255 ) );
	for (int i=0; i<num; i++)
	{
		int offset = i - (num-1)/2;
		int index = m_cursor + i - (num-1)/2;
		if ( index < 0 )
			continue;
		if ( index >= inv.GetNumItems() )
			continue;
		float x = ((1280.f-spacing)/2) + offset * spacing + border;
		float y = 720 - 10.f - spacing + border;
		guiDrawRoundedRect( x, y, size, size, 5.f, guiColourRGBA( 16, 16, 16, 255 ) );

		const InventoryItem &item = inv.GetItem( index );
		const char *gui = item.def->GetKeyValueString( "gui" );
		guiDrawText( x+size/2.f, y+size/2.f, gui, GUI_ALIGNX_CENTER, GUI_ALIGNY_CENTER, guiColourRGBA( 200, 200, 200, 255 ) );	
	}
}


void Player::OnAddToLevel( Level *l )
{
	Super::OnAddToLevel(l);

	m_level->AddGui( this );
}

void Player::OnRemoveFromLevel( Level *l )
{
	Super::OnRemoveFromLevel(l);
}

void Player::Spawn( KVStore const &kv )
{
	Super::Spawn(kv);
	m_input = true;
}

void Player::UpdateDelta( float dt )
{
	Super::PreDeltaUpdate(dt);
	Super::UpdateDelta(dt);

	m_animDir = 0.f;
	if ( m_input )
	{
		if ( GetAsyncKeyState( VK_LEFT ) < 0 )
			m_rotz += 1.f * dt;
		if ( GetAsyncKeyState( VK_RIGHT ) < 0 )
			m_rotz -= 1.f * dt;
		if ( GetAsyncKeyState( 'W' ) < 0 )
		{
			m_animDir = 1.f;
			m_vel[0] = sinf(m_rotz) * dt;
			m_vel[1] = cosf(m_rotz) * dt;
		}
		if ( GetAsyncKeyState( 'S' ) < 0 )
		{
			m_animDir = -1.f;
			m_vel[0] = -sinf(m_rotz) * dt;
			m_vel[1] = -cosf(m_rotz) * dt;
		}
		if ( GetAsyncKeyState( 'A' ) < 0 )
		{
			m_vel[0] =  cosf(m_rotz) * dt;
			m_vel[1] = -sinf(m_rotz) * dt;
		}
		if ( GetAsyncKeyState( 'D' ) < 0 )
		{
			m_vel[0] = -cosf(m_rotz) * dt;
			m_vel[1] =  sinf(m_rotz) * dt;
		}
		if ( GetAsyncKeyState( 'O' ) < 0 )
		{
			char posstring[32];
			sprintf( posstring, "%f %f %f", m_pos[0], m_pos[1], m_pos[2] + 1.f );
			KVStore spawner;
			spawner.AddKeyValueString( "template", "Hitpoint" );
			spawner.AddKeyValueString( "pos", posstring );
			spawner.AddKeyValueString( "gui", "test" );
			m_level->Spawn( spawner );
		}
	
		if ( m_stateIndex == 0 && GetAsyncKeyState( VK_SPACE ) & 1 )
		{
			m_animTime = 0.f;
			m_stateIndex = 1;
		}

		if ( m_stateIndex == 0 && GetAsyncKeyState( 'E' ) & 1 )
		{
			std::vector<Entity*> usables;
			m_level->FindEntities( usables, m_pos, 1.f, Entity::s_Type );
			for (unsigned int i=0; i<usables.size(); i++)
			{
				if ( usables[i]->Use( this ) )
				{
					break;
				}
			}
		}

		if ( GetAsyncKeyState( VK_LSHIFT ) < 0 && m_onGround )
		{
			m_vel[2] = 0.6f;
		}

		if ( m_stateIndex == 0 && GetAsyncKeyState( 'I' ) & 1 )
		{
			m_world->PushMenu( new PlayerInventoryMenu( this ) );
		}
	}

#if 0
	{
		static float mesh = 0.f;
		static float anim = 0.f;
		//static float rot[3] = {0.f,0.f,0.f};
		//static float pos[3] = {0.f,0.f,0.f};
		if ( GetAsyncKeyState( 'T' ) & 1 )
		{
			m_stateIndex = (m_stateIndex==999) ? 0 : 999;
			mesh = 0.f;
			anim = 0.f;
			//rot[0] = self->mesh[
		}
		if ( m_stateIndex >= 999 )
		{
			int width = 1280;
			int height = 720;
			static int scrollArea = 0;
			imguiBeginScrollArea("Anim", width - width / 5 - 10, 10, width / 5, height / 2, &scrollArea);

			imguiSlider("mesh", &mesh, 0.0f, m_mesh.m_submeshs.size()-1, 1.f);
			imguiValue( m_mesh.m_submeshs[(int)floorf(mesh)].name.c_str() );
			imguiSeparator();
			imguiSlider("rotx", &m_mesh.m_anims[0].channels[(int)floorf(mesh)].keys[AKI_ROTX][0].value, -6.5f, 6.5f, 0.01f);
			imguiSlider("roty", &m_mesh.m_anims[0].channels[(int)floorf(mesh)].keys[AKI_ROTY][0].value, -6.5f, 6.5f, 0.01f);
			imguiSlider("rotz", &m_mesh.m_anims[0].channels[(int)floorf(mesh)].keys[AKI_ROTZ][0].value, -6.5f, 6.5f, 0.01f);
			imguiSeparator();
			imguiSeparator();
			imguiSlider("posx", &m_mesh.m_anims[0].channels[(int)floorf(mesh)].keys[AKI_OFSX][0].value, -10.0f, 10.f, 0.01f);
			imguiSlider("posy", &m_mesh.m_anims[0].channels[(int)floorf(mesh)].keys[AKI_OFSY][0].value, -10.0f, 10.f, 0.01f);
			imguiSlider("posz", &m_mesh.m_anims[0].channels[(int)floorf(mesh)].keys[AKI_OFSZ][0].value, -10.0f, 10.f, 0.01f);
			imguiSeparator();
			imguiSeparator();
			imguiSlider("anim", &anim, 0.0f, m_mesh.m_anims.size()-1, 1.f);
			if ( imguiButton("play") )
			{
				m_animTime = 0.f;
				m_stateIndex = 999 + 1 + (int)floorf(anim);
			}
			if ( imguiButton("stop") )
			{
				m_stateIndex = 999;
			}
			imguiSeparator();

			imguiEndScrollArea();
		}
	}
#endif

	if ( m_stateIndex == 1 )
	{
		int tags;
		if ( AnimUpdate( tags, "swipe", dt ) )
			m_stateIndex = 0;

		if ( tags & 1 )
		{
			float face[2];
			face[0] = sinf(m_rotz);
			face[1] = cosf(m_rotz);
			std::vector<Entity*> hits;
			m_level->FindEntities( hits, m_pos, 2.f, Entity::s_Type );
			for (unsigned int i=0; i<hits.size(); i++)
			{
				if ( hits[i] == this )
					continue;
				const float *pos = hits[i]->GetPos();
				float diff[3];
				vec3Sub( diff, pos, m_pos );
				float dir[3];
				vec3Norm( dir, diff );
				float dp = dir[0] * face[0] + dir[1] * face[1];
				if ( dp > 0.2f )
				{
					hits[i]->Attack( this );
				}
			}
		}
	}
#if 0
	else
	if ( m_stateIndex == 999 )
	{
		m_animTime = 0.f;
		AnimUpdate( "_", dt );
	}
	else
	if ( m_stateIndex > 999 )
	{
		if ( AnimUpdate( m_mesh.m_anims[m_stateIndex-999-1].name.c_str(), dt ) )
			m_stateIndex = 999;
	}
#endif
	else
	{
		int tags;
		AnimUpdate( tags, m_animDir != 0.f ? "walk" : "idle", dt );
	}

	move( m_pos, m_vel, m_level->GetMap() );

	Super::PostDeltaUpdate(dt);
}

void Player::RenderGUI()
{
	float healthWidth = 100.f;
	float healthRouding = 4.f;
	float ypos;
	
	ypos = 10.f;
	guiDrawRoundedRect( 10-3, ypos-3, healthWidth+6, 15+6, healthRouding, guiColourRGBA( 20, 20, 20, 255 ) );
	guiDrawRoundedRect( 10, ypos, healthWidth, 15, healthRouding, guiColourRGBA( 200, 0, 0, 255 ) );
	guiDrawText( 10+(100/2), ypos+(15/2), "Health", GUI_ALIGNX_CENTER, GUI_ALIGNY_CENTER, guiColourRGBA( 0, 0, 255, 255 ) );

	ypos += 20.f;
	guiDrawRoundedRect( 10-3, ypos-3, healthWidth+6, 15+6, healthRouding, guiColourRGBA( 20, 20, 20, 255 ) );
	guiDrawRoundedRect( 10, ypos, healthWidth, 15, healthRouding, guiColourRGBA( 200, 0, 0, 255 ) );
	guiDrawText( 10+(100/2), ypos+(15/2), "Hunger", GUI_ALIGNX_CENTER, GUI_ALIGNY_CENTER, guiColourRGBA( 0, 0, 255, 255 ) );
}
