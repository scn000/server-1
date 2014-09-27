/**
 * MaNGOS is a full featured server for World of Warcraft, supporting
 * the following clients: 1.12.x, 2.4.3, 3.3.5a, 4.3.4a and 5.4.8
 *
 * Copyright (C) 2005-2014  MaNGOS project <http://getmangos.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MODEL_H
#define MODEL_H

#include "loadlib/loadlib.h"
#include "vec3d.h"
//#include "mpq.h"
#include "modelheaders.h"
#include <vector>
#include "vmapexport.h"

class WMOInstance;
class MPQFile;

Vec3D fixCoordSystem(Vec3D v);

class Model
{
    public:
        ModelHeader header;
        uint32 offsBB_vertices, offsBB_indices;
        Vec3D* BB_vertices, *vertices;
        uint16* BB_indices, *indices;
        size_t nIndices;

        bool open(StringSet& failedPaths);
        bool ConvertToVMAPModel(const char* outfilename);

        bool ok;

        Model(std::string& filename);
        ~Model() {_unload();}

    private:
        void _unload()
        {
            delete[] vertices;
            delete[] indices;
            vertices = NULL;
            indices = NULL;
        }
        std::string filename;
        char outfilename;
};

class ModelInstance
{
    public:
        Model* model;

        uint32 id;
        uint16 scale;
        Vec3D pos, rot;
        float sc;

        ModelInstance() {}
        ModelInstance(MPQFile& f, const char* ModelInstName, uint32 mapID, uint32 tileX, uint32 tileY, FILE* pDirfile);

};

#endif
