#ifndef  VOLUMETRICSELECTION_INC
#define  VOLUMETRICSELECTION_INC

#include "Datasets/SubDataset.h"
#include "Datasets/Dataset.h"
#include <vector>

namespace sereno
{
    /** \brief  The boolean operations available for volumetric selections */
    enum BooleanSelectionOp
    {
        SELECTION_OP_NONE  = -1, //No operation
        SELECTION_OP_UNION = 0,  //Perform a  Union
        SELECTION_OP_MINUS = 1,  //Perform a  difference
        SELECTION_OP_INTER = 2   //Perform an intersection
    };

    /** \brief  The volumetric mesh data to use */
    struct VolumetricMesh
    {
        std::vector<glm::vec3> points;    /*!< The list of points for this mesh*/
        std::vector<uint32_t>  triangles; /*!< The points IDs ordered to form triangles (each 3 values form a triangle)*/
        BooleanSelectionOp     op;        /*!< The boolean operation to do with this mesh*/

        VolumetricMesh(BooleanSelectionOp _op = SELECTION_OP_NONE) : op(_op){}
    };

    /** \brief  Our rastered cube space to fasten computation */
    struct RasteredCube
    {
        /** \brief The list of triangle being part of this Cube (in a rastered space)*/
        std::vector<int> triangleIDs;

        /** \brief   The maximum number of triangle a particular being on this cell might need to go through for the whole algorithm */
        int maxNbTriangle = 0;

        /** \brief  Operator to fasten the access of triangleIDs
         * \param x the indice of triangleIDs to get
         * \return   triangleIDs[x] */
        int operator[](int x)
        {
            return triangleIDs[x];
        }
    };

    /** \brief  Apply the volumetric selection on a CloudPoint SubDataset object
     *
     * \param mesh the volumetric mesh data
     * \param sd the subdataset object to modify */
    void applyVolumetricSelection_cloudPoint(const VolumetricMesh& mesh, SubDataset* sd);

    /** \brief  Apply the volumetric selection on a VTK SubDataset object
     *
     * \param mesh the volumetric mesh data
     * \param sd the subdataset object to modify */
    void applyVolumetricSelection_vtk(const VolumetricMesh& mesh, SubDataset* sd);
}

#endif
