#include "workbench_menu.h"
#include "gui.h"
#include "player.h"
#include "entitydef.h"
#include "inventory.h"
#include "world.h"
#include "craft.h"
#include <Windows.h>


WorkbenchMenu::WorkbenchMenu( Entity *i, Entity *u ) : m_item(i), m_user(u)
{
	Player *p = CoreCast<Player>(u);
	if ( p )
		p->SetInput( false );
	m_cursor = 0;
}

void WorkbenchMenu::Update()
{
	int numItems = 1;

	const EntityDef *edef = m_item->GetEntityDef();
	const KVStore *kv = edef->GetKeyValueKeyValue( "recipes" );
	if ( kv )
	{
		numItems = kv->GetNumItems();
	}

	if ( GetAsyncKeyState( VK_UP ) & 1 )
	{
		m_cursor = (m_cursor - 1 + numItems ) % numItems;
	} else
	if ( GetAsyncKeyState( VK_DOWN ) & 1 )
	{
		m_cursor = (m_cursor + 1 + numItems ) % numItems;
	} else
	if ( GetAsyncKeyState( VK_SPACE ) & 1 )
	{
		Player *p = CoreCast<Player>(m_user);

		const EntityDef *edef = m_item->GetEntityDef();
		const KVStore *kv = edef->GetKeyValueKeyValue( "recipes" );
		if ( kv )
		{
			const CraftRecipeDef *craft = CraftRecipeDefManager()->Get( kv->GetIndexValueString( m_cursor ) );
			if ( craft && p )
			{
				Inventory &inv = p->GetInventory();
				bool canCraft = true;
				const KVStore *kv = craft->GetKeyValueKeyValue( "items" );
				for (int j=0; j<kv->GetNumItems(); j++)
				{
					const KVStore *item = kv->GetIndexValueKeyValue( j );
					const InventoryItemDef *itemDef = InventoryItemDefManager()->Get( item->GetKeyValueString( "item", "unknown" ) );
					float amount = item->GetKeyValueFloat( "count", 1.f );
					if ( itemDef )
					{
						if ( !inv.CanTake( itemDef, amount ) )
						{
							canCraft = false;
							break;
						}
					}
				}
				if ( canCraft )
				{
					for (int j=0; j<kv->GetNumItems(); j++)
					{
						const KVStore *item = kv->GetIndexValueKeyValue( j );
						const InventoryItemDef *itemDef = InventoryItemDefManager()->Get( item->GetKeyValueString( "item", "unknown" ) );
						float amount = item->GetKeyValueFloat( "count", 1.f );
						if ( itemDef )
							inv.Take( itemDef, amount );
					}
					const EntityDef *pickupDef = EntityDefManager()->Get( craft->GetKeyValueString( "entity" ), false );
					if ( pickupDef )
					{
						const KVStore *kv = pickupDef->GetKeyValueKeyValue( "give" );
						for (int i=0; i<kv->GetNumItems(); i++)
						{
							const KVStore *item = kv->GetIndexValueKeyValue( i );
							const InventoryItemDef *itemDef = InventoryItemDefManager()->Get( item->GetKeyValueString( "item", "unknown" ) );
							float amount = item->GetKeyValueInt( "amount", 1.f );
							if ( itemDef )
								inv.Give( itemDef, amount );
						}
					}
				}
			}
		}
		if ( p )
			p->SetInput( true );
		m_item->FinishUsing( m_user );
		m_item->GetWorld()->PopMenu();
	} else
	if ( GetAsyncKeyState( VK_DELETE ) & 1 )
	{
		Player *p = CoreCast<Player>(m_user);
		if ( p )
			p->SetInput( true );
		m_item->FinishUsing( m_user );
		m_item->GetWorld()->PopMenu();
	}
}

void WorkbenchMenu::Render()
{
	Player *p = CoreCast<Player>(m_user);

	guiDrawRoundedRect( 10.f, 10.f, 1260.f, 700.f, 10.f, guiColourRGBA( 64, 64, 64, 255 ) );
	const EntityDef *edef = m_item->GetEntityDef();

	const KVStore *kv = edef->GetKeyValueKeyValue( "recipes" );
	if ( kv )
	{
		int numItems = kv->GetNumItems();
		for (int i=0; i<numItems; i++)
		{
			const CraftRecipeDef *craft = CraftRecipeDefManager()->Get( kv->GetIndexValueString( i ) );
			if ( craft )
			{
				bool canCraft = true;
				const KVStore *kv = craft->GetKeyValueKeyValue( "items" );
				for (int j=0; j<kv->GetNumItems(); j++)
				{
					const KVStore *item = kv->GetIndexValueKeyValue( j );
					const InventoryItemDef *itemDef = InventoryItemDefManager()->Get( item->GetKeyValueString( "item", "unknown" ) );
					float amount = item->GetKeyValueFloat( "count", 1.f );
					if ( itemDef )
					{
						if ( !p || !p->GetInventory().CanTake( itemDef, amount ) )
						{
							canCraft = false;
							break;
						}
					}
				}
				
				const char *gui = craft->GetKeyValueString( "gui_text", "unknown" );

				float ypos = 30.f + i*25.f;
				guiDrawRoundedRect( 30-3, ypos-3, 100+6, 15+6, 10, guiColourRGBA( 20, 20, 20, 255 ) );
				if ( i == m_cursor )
				{
					guiDrawRoundedRect( 30, ypos, 100, 15, 10, guiColourRGBA( 200, 0, 0, 255 ) );
				}
				guiDrawText( 30+(100/2), ypos+(15/2), gui, GUI_ALIGNX_CENTER, GUI_ALIGNY_CENTER, canCraft ? guiColourRGBA( 0, 0, 255, 255 ) : guiColourRGBA( 0, 0, 64, 255 ) );	

				if ( i == m_cursor )
				{
					for (int j=0; j<kv->GetNumItems(); j++)
					{
						const KVStore *item = kv->GetIndexValueKeyValue( j );
						const InventoryItemDef *itemDef = InventoryItemDefManager()->Get( item->GetKeyValueString( "item", "unknown" ) );
						if ( itemDef )
						{
							const char *gui = itemDef->GetKeyValueString( "gui" );
							float ypos2 = 30.f + j*25.f;
							guiDrawText( 500+(100/2), ypos2+(15/2), gui, GUI_ALIGNX_CENTER, GUI_ALIGNY_CENTER, guiColourRGBA( 0, 0, 255, 255 ) );
						}
					}
				}
			}
		}
	}
}

