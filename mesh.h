#pragma once

#include <vector>
#include <bgfx.h>

#include "anim.h"
#include "misc.h"


struct MeshSubSet
{
	const bgfx::Memory* memVerts;
	const bgfx::Memory* memIndices;
	bgfx::VertexBufferHandle vbh;
	bgfx::IndexBufferHandle ibh;
};

struct Mesh
{
	std::vector<MeshSubSet> m_subsets;
};

#define MAPXDIM 64
#define MAPYDIM 64
#define MAPZDIM 32
struct MapMesh
{
	bool dirty[512/MAPXDIM][512/MAPYDIM][64/MAPZDIM];
	Mesh meshes[512/MAPXDIM][512/MAPYDIM][64/MAPZDIM];
};

struct AnimMeshAttach
{
	std::string name;
	int   idx;
	float rot[3];
	float ofs[3];
};

struct AnimSubMesh
{
	float baserot[3];
	float baseofs[3];
	Mesh *mesh;
	std::string name;
};

struct AnimSkeleton
{
	int child, parent;
};

struct AnimMesh
{
	float globalScale;
	std::vector<AnimSubMesh> m_submeshs;
	std::vector<Anim> m_anims;
	std::vector<AnimMeshAttach> m_attachments;
	std::vector<AnimSkeleton> m_skeleton;
};

struct AnimSubMeshJoint
{
	float animmtx[16];
	float animrot[3];
	float animofs[3];
};

struct AnimMeshInstance
{
	std::vector<AnimSubMeshJoint> m_submeshPose;
};

void initMeshDecl();


void makeMesh( Mesh *mesh, struct map_s *map, float scale, int *from=NULL, int *to=NULL );
void renderMesh( Mesh &mesh );


void initMapMesh( MapMesh &mapMesh );
void makeMapMesh( MapMesh &mapMesh, struct map_s *map, float scale );
void renderMesh( MapMesh &mapMesh );
void clearMap( map_s *m, MapMesh &mapMesh, int f[3], int t[3] );

int findmesh( const AnimMesh &mesh, const char *name );
int findmeshattach( const AnimMesh &mesh, const char *name );
void addMesh( AnimMesh &amesh, Mesh *mesh, const char *name, float ox, float oy, float oz, float rx, float ry, float rz );
void init_animmeshinstance( AnimMeshInstance &inst, const AnimMesh &amesh );
void renderMesh( const AnimMesh &amesh, const AnimMeshInstance &inst, const float pos[3], float rotz );
void renderMesh( const AnimMesh &amesh, const AnimMeshInstance &inst, const float wmtx[16] );

void calcAttachMtx( float fmtx[16], const AnimMeshInstance &inst, int index, const float ofs[3], const float rot[3], const float pos[3], float rotz );
void calcAttachMtx( float fmtx[16], const AnimMeshInstance &inst, int index, const float ofs[3], const float rot[3], const float wmtx[16] );
void calcAttachPos( float fpos[3], const AnimMeshInstance &inst, int index, const float ofs[3], const float rot[3], const float pos[3], float rotz );

void load_kv6( Mesh &mesh, const char *filename, float scale );
void load_cub( Mesh &mesh, const char *filename, float px, float py, float pz, float scale );
void load_animmesh( AnimMesh &mesh, const char *filename, float globalScale );


class AnimMeshDef
{
	struct ReloadCallback
	{
		CoreClass ptr;
		void *user;
	};
	AnimMesh m_mesh;
	std::string m_filename;
	std::vector< ReloadCallback > m_reload;
public:

	virtual bool Load( const char *filename );
	virtual void Reload();
	virtual void Clear();

	//virtual void Register( CoreClass *p, void *user );
	//virtual void Unregister( CoreClass *p );

	const AnimMesh * GetMesh() const { return &m_mesh; }
};

class AnimMeshDefMgr : public TypedDefMgr<AnimMeshDef>
{
public:
};

AnimMeshDefMgr *AnimMeshDefManager();
