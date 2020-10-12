#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif

#include "VolumetricSelection.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <iostream>
#include <algorithm>
#include "Datasets/CloudPointDataset.h"

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

namespace sereno
{
    bool rayTriangleIntersection(const glm::vec3& pos, const glm::vec3& rayDir, const glm::vec3* triangle, float* t = NULL)
    {
        return false;
    }

    /**
     * \brief  Apply the volumetric selection
     *
     * @tparam T the type of spatialPosAt
     *
     * \param mesh the mesh data to consider
     * \param sd the subdataset containing the data
     * \param spatialPosAt function to call to get the 3D position of a cell/object at k
     */
    template <typename T>
    void applyVolumetricSelection(const VolumetricMesh& mesh, SubDataset* sd, const T& spatialPosAt)
    {
        const int CUBE_SIZE_X = 16;
        const int CUBE_SIZE_Y = 16;
        const int CUBE_SIZE_Z = 16;

        const int CUBE_SIZE[] = { CUBE_SIZE_X, CUBE_SIZE_Y, CUBE_SIZE_Z };

        if (mesh.triangles.size() % 3 != 0)
        {
            std::cerr << "The number of triangles is not a multiple of 3. Abort";
            return;
        }

        if(sd->isVolumetricMaskReset())
            sd->resetVolumetricMask(false, true);
        
        //First, transform every point using the provided matrix
        glm::vec3* points = new glm::vec3[mesh.points.size()];
        glm::mat4  mat    = sd->getModelWorldMatrix();

#if defined(_OPENMP)
        #pragma omp parallel for
#endif
        for(uint32_t i = 0; i < mesh.points.size(); i++)
            points[i] = mat * glm::vec4(mesh.points[i], 1.0f);

        //Second, initialize our raster 3D space. Each cell contains the list of triangles it contains. Indice(x, y, z) = z*CUBE_SIZE_X*CUBE_SIZE_Y + y*CUBE_SIZE_X + x
        RasteredCube* rasteredSpace = new RasteredCube[CUBE_SIZE_X*CUBE_SIZE_Y*CUBE_SIZE_Z];
        //Go through all the triangles. For each triangle, determine in which cells it is
        for(size_t i = 0; i < mesh.triangles.size()/3; i++)
        {
            //Get the bounding box of the triangle
            glm::vec3 minPos;
            glm::vec3 maxPos;
            for(int j = 0; j < 3; j++)
            {
                minPos[j] = MIN(points[mesh.triangles[3*i+0]][j], MIN(points[mesh.triangles[3*i+1]][j], points[mesh.triangles[3*i+2]][j]));
                maxPos[j] = MAX(points[mesh.triangles[3*i+0]][j], MAX(points[mesh.triangles[3*i+1]][j], points[mesh.triangles[3*i+2]][j]));

                //Set them as indices in our 3D restered space
                minPos[j] = CUBE_SIZE[j] * ((minPos[j] - sd->getParent()->getMinPos()[j]) / (sd->getParent()->getMaxPos()[j] - sd->getParent()->getMinPos()[j]));
                maxPos[j] = CUBE_SIZE[j] * ((maxPos[j] - sd->getParent()->getMinPos()[j]) / (sd->getParent()->getMaxPos()[j] - sd->getParent()->getMinPos()[j]));

                minPos[j] = MAX(0.0f,  minPos[j]);
                maxPos[j] = MAX(-1.0f, maxPos[j]);
            }

            //Cross the bounding box and our 3D rastered space
            for(int kk = (int)minPos[2]; kk <= (int)maxPos[2] && kk < CUBE_SIZE_Z; kk++)
                for(int jj = (int)minPos[1]; jj <= (int)maxPos[1] && jj < CUBE_SIZE_Y; jj++)
                    for(int ii = (int)minPos[0]; ii <= (int)maxPos[0] && ii < CUBE_SIZE_X; ii++)
                        rasteredSpace[ii + CUBE_SIZE_X*jj + CUBE_SIZE_Y*CUBE_SIZE_X*kk].triangleIDs.push_back(i);
        }

        //Update the variable "MaxNbTriangles", useful to know the most efficient way to cast a ray
        for(int k = 0; k < CUBE_SIZE_Z; k++)
        {
            for(int j = 0; j < CUBE_SIZE_Y; j++)
            {
                int nbTriangles = 0;
                for(int i = 0; i < CUBE_SIZE_X; i++)
                {
                    nbTriangles += rasteredSpace[i + CUBE_SIZE_X*j + CUBE_SIZE_Y*CUBE_SIZE_X*k].triangleIDs.size();
                    rasteredSpace[i + CUBE_SIZE_X*j + CUBE_SIZE_Y*CUBE_SIZE_X*k].maxNbTriangle = nbTriangles;
                }
            }
        }

        //Go through all the points of the dataset and check if it is inside or outside the Mesh

#if defined(_OPENMP)
        #pragma omp parallel for
#endif
        for(uint32_t k=0; k < sd->getParent()->getNbSpatialData(); k++)
        {
            glm::vec3 pos = spatialPosAt(k);
            glm::vec3 rayDir(1.0f, 0.0f, 0.0f);
            int nbIntersection = 0;
            std::vector<int> triangleIDAlready;

            //The particule X position in the rastered space
            int particuleX = (int)MIN(CUBE_SIZE_X-1, CUBE_SIZE_X * ((pos[0] - sd->getParent()->getMinPos()[0]) / (sd->getParent()->getMaxPos()[0] - sd->getParent()->getMinPos()[0])));
            int particuleY = (int)MIN(CUBE_SIZE_Y-1, CUBE_SIZE_Y * ((pos[1] - sd->getParent()->getMinPos()[1]) / (sd->getParent()->getMaxPos()[1] - sd->getParent()->getMinPos()[1])));
            int particuleZ = (int)MIN(CUBE_SIZE_Z-1, CUBE_SIZE_Z * ((pos[2] - sd->getParent()->getMinPos()[2]) / (sd->getParent()->getMaxPos()[2] - sd->getParent()->getMinPos()[2])));

            particuleX = MAX(particuleX, 0);
            particuleY = MAX(particuleY, 0);
            particuleZ = MAX(particuleZ, 0);

            //Go through all the cubes
            glm::vec3 triangle[3];

            auto rayCastAction = [&](int j)
            {
                RasteredCube& cube = rasteredSpace[j + particuleY*CUBE_SIZE_X + particuleZ*CUBE_SIZE_X*CUBE_SIZE_Y];

                for(int triangleID : cube.triangleIDs)
                {
                    for (int i = 0; i < 3; i++)
                        triangle[i] = points[mesh.triangles[3*triangleID + i]];

                    //Ray -- triangle intersection along the positive x axis
                    float t;
                    if(rayTriangleIntersection(pos, rayDir, triangle, &t))
                    {
                        //Do not check multiple times the same triangle
                        if (std::find(triangleIDAlready.begin(), triangleIDAlready.end(), triangleID) != triangleIDAlready.end())
                            continue;
                        nbIntersection++;
                        triangleIDAlready.push_back(triangleID);
                    }
                }
            };

            //Select the most efficient direction (i.e., less test)
            if(2*rasteredSpace[particuleX    + particuleY*CUBE_SIZE_Y + particuleZ*CUBE_SIZE_X*CUBE_SIZE_Y].maxNbTriangle - 
                 rasteredSpace[CUBE_SIZE_X-1 + particuleY*CUBE_SIZE_Y + particuleZ*CUBE_SIZE_X*CUBE_SIZE_Y].maxNbTriangle > 0)
            { 
                for(int j = particuleX; j < CUBE_SIZE_X; j++)
                    rayCastAction(j);
            }
            else
            {
                rayDir.x *= -1.0f;
                for(int j = particuleX; j >= 0; j--)
                    rayCastAction(j);
            }

            //Apply the boolean operation
            switch (mesh.op)
            {
                case SELECTION_OP_UNION:
                    sd->setVolumetricMaskAt(k, sd->getVolumetricMaskAt(k) || (nbIntersection % 2 == 1));
                    break;
                case SELECTION_OP_INTER:
                    sd->setVolumetricMaskAt(k, sd->getVolumetricMaskAt(k) && (nbIntersection % 2 == 1));
                    break;
                case SELECTION_OP_MINUS:
                    sd->setVolumetricMaskAt(k, sd->getVolumetricMaskAt(k) && !(nbIntersection % 2 == 1));
                    break;
                default:
                    break;
            }
        }

        delete[] points;
        delete[] rasteredSpace;
    }

    void applyVolumetricSelection_cloudPoint(const VolumetricMesh& mesh, SubDataset* sd)
    {
        const float* pos = ((CloudPointDataset*)sd->getParent())->getPointPositions();
        applyVolumetricSelection(mesh, sd, [pos](uint32_t k) {return glm::vec3(pos[3*k + 0], pos[3*k + 1], pos[3*k + 2]);});
    }
}
