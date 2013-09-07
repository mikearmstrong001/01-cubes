#include "actor.h"
#include "level.h"
#include "fpumath.h"
#include "kvstore.h"

void Actor::PreDeltaUpdate( float dt )
{
	unlinkFromMap( m_level->GetMap(), this, m_pos );

	m_onGround = false;
	m_vel[2] -= 1.f * dt;

	if ( m_vel[2] <= 0.f &&
		issolid( m_level->GetMap(), (int)(m_pos[0]), (int)(m_pos[1]), (int)(m_pos[2] + m_vel[2] - 0.1f) ) )
	{
		m_vel[2] = 0.f;
		m_onGround = true;
	}

	if ( m_onGround )
	{
		m_vel[0] = 0.f;
		m_vel[1] = 0.f;
	}

#if 0
	if ( attach.ent != -1 )
	{
		entitystate_s &attachee = entities[player->attach.ent];
		int index = -1;
		for ( unsigned int i=0; i<attachee.mesh.m_attachments.size(); i++ )
		{
			if ( attachee.mesh.m_attachments[i].name == player->attach.name )
			{
				index = i;
				break;
			}
		}
		if ( index != -1 )
		{
			AnimMeshAttach &attach = attachee.mesh.m_attachments[index];
			calcAttachMtx( player->wmtx, attachee.mesh, attach.idx, attach.ofs, attach.rot, attachee.wmtx );
			player->pos[0] = player->wmtx[12];
			player->pos[1] = player->wmtx[13];
			player->pos[2] = player->wmtx[14];
		}
	}
#endif
}

void Actor::PostDeltaUpdate( float dt )
{
	mtxRotateZ(m_wmtx, m_rotz );
	m_wmtx[12] = m_pos[0];
	m_wmtx[13] = m_pos[1];
	m_wmtx[14] = m_pos[2];

	linkToMap( m_level->GetMap(), this, m_pos );
}

void Actor::OnAddToLevel( Level *l )
{
	Super::OnAddToLevel( l );

	l->AddDelta( this );
	l->AddRender( this );
}

void Actor::OnRemoveFromLevel( Level *l )
{
	l->RemoveDelta( this );
	l->RemoveRender( this );

	Super::OnRemoveFromLevel( l );
}

void Actor::Spawn( KVStore const &kv )
{
	Super::Spawn( kv );
	kv.GetKeyValueFloatArray( m_pos, 3, "pos" );
	m_rotz = kv.GetKeyValueFloat( "rotz" );
}

bool Actor::AnimUpdate( const char *animName, float dt )
{
	float v = m_vel[0]*m_vel[0] + m_vel[1]*m_vel[1];
	m_moveSpeed = v;
	m_animTime += dt;

	const AnimMesh &mesh = *m_meshDef->GetMesh();

	int animIndex = 0;
	for ( unsigned int i=0; i<mesh.m_anims.size(); i++ )
	{
		if ( mesh.m_anims[i].name == animName )
		{
			animIndex = i;
			break;
		}
	}

	//if ( animIndex != -1 )
	{
		float *animStack = (float*)alloca( sizeof(float)*16*mesh.m_submeshs.size() );
		for ( unsigned int i=0; i<mesh.m_submeshs.size(); i++ )
		{
			const AnimSubMesh &am = mesh.m_submeshs[i];
			AnimSubMeshJoint &aj = m_meshInstance.m_submeshPose[i];
			for ( unsigned int j=0; j<3; j++ )
			{
				float val = animSample( mesh.m_anims[animIndex], i, j, m_animTime, mesh.m_submeshs[i].baseofs[j] );
				aj.animofs[j] = val;
			}
			for ( unsigned int j=0; j<3; j++ )
			{
				float val = animSample( mesh.m_anims[animIndex], i, 3+j, m_animTime, mesh.m_submeshs[i].baserot[j] );
				aj.animrot[j] = val;
			}
			float *mtx = &animStack[i*16];
			mtxRotateXYZ(mtx, aj.animrot[0], aj.animrot[1], aj.animrot[2] );
			mtx[12] = aj.animofs[0];
			mtx[13] = aj.animofs[1];
			mtx[14] = aj.animofs[2];
			memcpy( aj.animmtx, mtx, sizeof(float)*16 );
		}
		for ( unsigned int i=0; i<mesh.m_skeleton.size(); i++ )
		{
			const AnimSkeleton &as = mesh.m_skeleton[i];
			float *c = &animStack[as.child*16];
			float *f = m_meshInstance.m_submeshPose[as.child].animmtx;
			float *p = m_meshInstance.m_submeshPose[as.parent].animmtx;
			mtxMul( f, c, p );
		}
	}

	return !mesh.m_anims[animIndex].looping && m_animTime >= mesh.m_anims[animIndex].time;
}
