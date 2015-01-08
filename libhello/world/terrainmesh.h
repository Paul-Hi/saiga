#pragma once

#include "libhello/opengl/texture/texture.h"
#include "libhello/opengl/texture/cube_texture.h"
#include "libhello/opengl/indexedVertexBuffer.h"
#include "libhello/opengl/basic_shaders.h"
#include "libhello/util/perlinnoise.h"
#include "libhello/geometry/triangle_mesh_generator.h"


class TerrainMesh{
private:
    typedef TriangleMesh<Vertex,GLuint> mesh_t;
public:
    int n = 63;
     int m = (n+1)/4;


    TerrainMesh();



    std::shared_ptr<mesh_t> createMesh();
    std::shared_ptr<mesh_t> createMesh2();
     std::shared_ptr<TerrainMesh::mesh_t> createMeshFixUpV();
      std::shared_ptr<TerrainMesh::mesh_t> createMeshFixUpH();

      std::shared_ptr<TerrainMesh::mesh_t> createMeshTrimSW();
       std::shared_ptr<TerrainMesh::mesh_t> createMeshTrimSE();

       std::shared_ptr<TerrainMesh::mesh_t> createMeshTrimNW();
        std::shared_ptr<TerrainMesh::mesh_t> createMeshTrimNE();

       std::shared_ptr<TerrainMesh::mesh_t> createMeshCenter();

       std::shared_ptr<TerrainMesh::mesh_t> createMeshDegenerated();

      std::shared_ptr<mesh_t> createGridMesh(unsigned int w, unsigned int h, vec2 d, vec2 o);
};