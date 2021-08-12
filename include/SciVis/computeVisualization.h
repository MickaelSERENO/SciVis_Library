#ifndef  COMPUTEVISUALIZATION_INC
#define  COMPUTEVISUALIZATION_INC

#include "Datasets/SubDataset.h"
#include "sciVisUtils.h"

namespace sereno
{
    /** \brief  Get the 3D color field of a subdataset being categorized as a VTK Structured Grid
     * \param sd the SubDataset to evaluate. It needs to be linked with a VTK Structured Grid (sd->getParent()) and have a valid transfer function
     * \param sizeOutput[out] array that shall contain the size of the 3D grid (width, height, depth). If nullptr, no value is stored in this array. Minimum size: 3
     * \return  the 3D color RGBA array. Size: width * height * depth * 4. Order: width, then height, then depth: 
     * x0y0z0
     * x1y0z0
     * x2y0z0
     * .
     * .
     * .
     * x0y1z0
     * x1y1z0
     * x2y1z0
     * .
     * .
     * .
     * x0y0z1
     * x1y0z1
     * x2y0z1
     * .
     * .
     * .
     * xN-1yN-1zN-1 */
    uint8_t* getVTKStructuredGridColorArray(SubDataset* sd, uint32_t* sizeOutput = nullptr);

    /** \brief  Save the 3D image of a Subdataset object being categorized as a VTK Structured Grid
     *
     * The format written is, in binary (big endian):
     *
     * width height depth (uint32_t per value)
     * The result of getVTKStructuredGridColor(SubDataset* sd) as uint8_t per component
     *
     * \param sd the SubDataset to evaluate. It needs to be linked with a VTK Structured Grid (sd->getParent()) and have a valid transfer function
     * \param path the path of the file on disk to write on
     * \return   true on success, false otherwise */
    bool saveVTKStructuredGridVisual(SubDataset* sd, const std::string& path);

    /** \brief  Save the cloud point (position + color) of a Subdataset object being categorized as a Cloud Point
     *
     * Format (binary, big endian):
     *
     * nbPoints (uint32_t)
     * Position array (Nb Values: nbPoints * 3 components (x, y, z). Each component is a float
     * Color array (Nb Values: nbPoints * 4 components (r, g, b, a). Each component is a uint8_t)
     *
     * \param sd the SubDataset to evaluate. It needs to be linked with a Cloud Point (sd->getParent()) and have a valid transfer function
     * \param path the path of the file on disk to write on
     * \return   true on success, false otherwise */
    bool saveCloudPointVisual(SubDataset* sd, const std::string& path);
}

#endif
