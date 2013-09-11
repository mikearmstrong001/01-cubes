#include "zombie.h"
#include "voxel.h"
#include "level.h"
#include "fpumath.h"

static ClassCreator<Zombie> s_ZombieCreator( "Zombie" );
CoreType Zombie::s_Type( &Zombie::Super::s_Type );

void Zombie::OnAddToLevel( Level *l )
{
	Super::OnAddToLevel(l);
}

void Zombie::OnRemoveFromLevel( Level *l )
{
	Super::OnRemoveFromLevel(l);
}

void Zombie::UpdateDelta( float dt )
{
	Super::PreDeltaUpdate(dt);
	Super::UpdateDelta(dt);

	if ( m_moveSpeed < 0.001f && m_onGround )
	{
		m_noProgress -= dt;
	}
	if ( m_noProgress < 0.f )
	{
		// quick check for player
		m_rotz += 3.14f;
		m_noProgress = 5 * 60.f;
		m_stateIndex = 0;
	}
	m_moveSpeed = 1.f;
	if ( m_stateIndex == 0 )
	{
		m_stateTime -= dt;
		if ( m_stateTime < 0.f )
		{
			m_rotz += ((rand() % 3) - 1) * 0.2f;
			m_stateTime = (float)(rand() % 100);

			Actor *playerOne = (Actor*)m_level->FindEntity( "player0" );

			float delta[3];
			vec3Sub( delta, playerOne->GetPos(), m_pos );
			float distSq = vec3LenSq( delta );
			if ( distSq < (16.f*16.f) )
			{
				float lookDir[3] = {sinf(m_rotz), cosf(m_rotz), 0.f};
				float dp = vec3Dot( lookDir, delta );
				if ( dp > 0.f )
				{
					m_rotz = atan2f( delta[0], delta[1] );
					m_stateIndex = 1;
				}
			}
		}
	}
	else
	if ( m_stateIndex == 1 )
	{
		float delta[3];
		Actor *playerOne = (Actor*)m_level->FindEntity( "player0" );
		vec3Sub( delta, playerOne->GetPos(), m_pos );
		float distSq = vec3LenSq( delta );
		if ( distSq < (32.f*32.f) )
		{
			float lookDir[3] = {sinf(m_rotz), cosf(m_rotz), 0.f};
			float dp = vec3Dot( lookDir, delta );
			if ( dp > 0.f )
			{
				m_rotz = atan2f( delta[0], delta[1] );
				m_moveSpeed *= 3.f;
				if ( distSq < 1.f )
				{
					m_stateIndex = 2;
					m_stateTime = 50.f;
				}
			}
			else
			{
				m_stateIndex = 0;
			}
		}
		else
		{
			m_stateIndex = 0;
		}
	}
	else
	if ( m_stateIndex == 2 )
	{
		m_moveSpeed = 0.f;
		m_stateTime -= dt;
		if ( m_stateTime < 0.f )
		{
		}
	}
	else
	if ( m_stateIndex == 3 )
	{
		m_moveSpeed = 0.f;
	}

	m_animDir = 1.f;
	if ( m_onGround )
	{
		m_vel[0] = m_moveSpeed*sinf(m_rotz)*dt;
		m_vel[1] = m_moveSpeed*cosf(m_rotz)*dt;
	}

	int tags;
	AnimUpdate( tags, "walk", dt );

	move( m_pos, m_vel, m_level->GetMap() );

	Super::PostDeltaUpdate(dt);
}
