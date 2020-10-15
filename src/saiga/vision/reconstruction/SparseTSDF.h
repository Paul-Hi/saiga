﻿/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#pragma once
#include "saiga/core/geometry/all.h"
#include "saiga/core/image/all.h"
#include "saiga/core/util/BinaryFile.h"
#include "saiga/core/util/ProgressBar.h"
#include "saiga/core/util/Thread/SpinLock.h"
#include "saiga/core/util/Thread/omp.h"

#include "MarchingCubes.h"

namespace Saiga
{
// A block sparse truncated signed distance field.
// Generated by integrating (fusing) aligned depth maps.
// Each block consists of VOXEL_BLOCK_SIZE^3 voxels.
// A typical value for VOXEL_BLOCK_SIZE is 8.
//
// The size in meters is given in the constructor.
//
// The voxel blocks are stored sparse using a hashmap. For each hashbucket,
// we store a linked-list with all blocks inside this bucket.
struct SAIGA_VISION_API SparseTSDF
{
    static constexpr int VOXEL_BLOCK_SIZE = 8;
    using VoxelBlockIndex                 = ivec3;

    // Each TSDF voxel consists of the (signed) distance to the surface and a confidence weight.
    // If desired, you can add a RGB member to estimate the color during integration.
    struct Voxel
    {
        float distance = 0;
        float weight   = 0;
    };


    // A voxel block is a 3 dimensional array of voxels.
    // Given a VOXEL_BLOCK_SIZE of 8 a voxel blocks consists of 8*8*8=512 voxels.
    //
    // Due to the sparse storage, each voxel block has to known it's own index.
    // The next_index points to the next voxel block in the same hash bucket.
    struct VoxelBlock
    {
        //        Voxel data[VOXEL_BLOCK_SIZE][VOXEL_BLOCK_SIZE][VOXEL_BLOCK_SIZE];
        std::array<std::array<std::array<Voxel, 8>, 8>, 8> data;
        VoxelBlockIndex index = VoxelBlockIndex(-973454, -973454, -973454);
        int next_index        = -1;
    };


    SparseTSDF(float voxel_size = 0.01, int reserve_blocks = 100000, int hash_size = 100000)
        : voxel_size(voxel_size),
          voxel_size_inv(1.0 / voxel_size),
          hash_size(hash_size),
          blocks(reserve_blocks),
          first_hashed_block(hash_size, -1),
          hash_locks(hash_size)

    {
        static_assert(sizeof(VoxelBlock::data) == 8 * 8 * 8 * 2 * sizeof(float), "Incorrect Voxel Size");
        block_size_inv = 1.0 / (voxel_size * VOXEL_BLOCK_SIZE);

//        std::cout << "SparseTSDF created. Allocated Memory: " << Memory() / (1000 * 1000) << " MB." << std::endl;
    }

    SparseTSDF(const std::string& file) { Load(file); }

    SAIGA_VISION_API friend std::ostream& operator<<(std::ostream& os, const SparseTSDF& tsdf);



    size_t Memory()
    {
        size_t mem_blocks = blocks.capacity() * sizeof(VoxelBlock);
        size_t mem_hash   = first_hashed_block.capacity() * sizeof(int);
        return mem_blocks + mem_hash + sizeof(*this);
    }

    // Returns the voxel block or 0 if it doesn't exist.
    VoxelBlock* GetBlock(const VoxelBlockIndex& i) { return GetBlock(i, H(i)); }


    // Insert a new block into the TSDF and returns a pointer to it.
    // If the block already exists, nothing is inserted.
    VoxelBlock* InsertBlock(const VoxelBlockIndex& i)
    {
        int h      = H(i);
        auto block = GetBlock(i, h);

        if (block)
        {
            // block already exists
            return block;
        }

        // Create block and insert as the first element.
        int new_index = current_blocks.fetch_add(1);

        if (new_index >= blocks.size())
        {
            blocks.resize(blocks.size() * 2);
        }

        int hash                 = H(i);
        auto* new_block          = &blocks[new_index];
        new_block->index         = i;
        new_block->next_index    = first_hashed_block[hash];
        first_hashed_block[hash] = new_index;
        return new_block;
    }

    VoxelBlock* InsertBlockLock(const VoxelBlockIndex& i)
    {
        int h = H(i);

        std::unique_lock lock(hash_locks[h]);

        auto block = GetBlock(i, h);

        if (block)
        {
            // block already exists
            return block;
        }

        // Create block and insert as the first element.
        int new_index = current_blocks.fetch_add(1);

        if (new_index >= blocks.size())
        {
            SAIGA_EXIT_ERROR("Resizing not allowed during parallel insertion!");
        }

        int hash                 = H(i);
        auto* new_block          = &blocks[new_index];
        new_block->index         = i;
        new_block->next_index    = first_hashed_block[hash];
        first_hashed_block[hash] = new_index;
        return new_block;
    }

    VoxelBlockIndex GetBlockIndex(const vec3& position)
    {
        vec3 normalized_position = (position - make_vec3(0.5f * voxel_size)) * block_size_inv;
        return normalized_position.array().floor().cast<int>();
    }

    vec3 BlockCenter(const VoxelBlockIndex& i)
    {
        int half_size = VOXEL_BLOCK_SIZE / 2;
        return GlobalPosition(i, half_size, half_size, half_size);
    }

    // The bottom right corner of this voxelblock
    vec3 GlobalBlockOffset(const VoxelBlockIndex& i) { return i.cast<float>() * voxel_size * VOXEL_BLOCK_SIZE; }

    vec3 GlobalPosition(const VoxelBlockIndex& i, int z, int y, int x)
    {
        return vec3(x, y, z) * voxel_size + GlobalBlockOffset(i);
    }


    using Triangle = std::array<vec3, 3>;

    // Triangle surface extraction on the sparse TSDF.
    // Returns for each block a list of triangles
    std::vector<std::vector<Triangle>> ExtractSurface(double iso, int threads = OMP::getMaxThreads());

    // Create a triangle mesh from the list of triangles
    TriangleMesh<VertexNC, uint32_t> CreateMesh(const std::vector<std::vector<Triangle>>& triangles);

    void Compact(){
        blocks.resize(current_blocks);
    }

   public:
    float voxel_size;
    float voxel_size_inv;

    float block_size_inv;


    unsigned int hash_size;
    std::atomic_int current_blocks = 0;
    std::vector<VoxelBlock> blocks;
    std::vector<int> first_hashed_block;
    std::vector<SpinLock> hash_locks;

    void Save(const std::string& file);

    void Load(const std::string& file);

    bool operator==(const SparseTSDF& other) const;

    int H(const VoxelBlockIndex& i)
    {
        unsigned int u = i.x() + i.y() * 1000 + i.z() * 1000 * 1000;
        int result     = u % hash_size;
        SAIGA_ASSERT(result >= 0 && result < hash_size);
        return result;
    }


    int GetBlockId(const VoxelBlockIndex& i) { return GetBlockId(i, H(i)); }

    // Returns the actual (memory) block id
    // returns -1 if it does not exist
    int GetBlockId(const VoxelBlockIndex& i, int hash)
    {
        int block_id = first_hashed_block[hash];

        while (block_id != -1)
        {
            auto* block = &blocks[block_id];
            if (block->index == i)
            {
                break;
            }

            block_id = block->next_index;
        }
        return block_id;
    }

    VoxelBlock* GetBlock(const VoxelBlockIndex& i, int hash)
    {
        auto id = GetBlockId(i, hash);
        if (id >= 0)
        {
            return &blocks[id];
        }
        else
        {
            return nullptr;
        }
    }

    // Returns the voxel block or 0 if it doesn't exist.
    //    VoxelBlock* GetBlock(const VoxelBlockIndex& i, int hash)
    //    {
    //        int block_id = first_hashed_block[hash];

    //        while (block_id != -1)
    //        {
    //            auto* block = &blocks[block_id];
    //            if (block->index == i)
    //            {
    //                return block;
    //            }

    //            block_id = block->next_index;
    //        }
    //        return nullptr;
    //    }
};



}  // namespace Saiga