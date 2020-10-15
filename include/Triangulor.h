#ifndef  TRIANGULOR_INC
#define  TRIANGULOR_INC

/** Triangulor helper functions. Create a mesh based on a 2D polygon list of points
    Those functions are adapted from from wiki.unity3d.com/index.php?title=Triangulator&_ga=2.105408325.796467802.1591785143-688777271.1584103287*/

#include <vector>
#include <algorithm>
#include <glm/glm.hpp>

namespace sereno
{
    /** \brief Compute the indice array which defines the triangles to create for the polygon passing in parameter at construction time
     * \param points the polygon points to triangulate
     * \return indice array of the polygon points*/
    std::vector<int> triangulate(const std::vector<glm::vec2>& points);

    /** \brief is point P inside the triangle ABC?
     * \return true if yes, false otherwise */
    bool insideTriangle(const glm::vec2& A, const glm::vec2& B, const glm::vec2& C, const glm::vec2& P);

    /** \brief  A helper function which I do not know what it serves (it comes from the wiki of unity3D)
     *
     * \param points
     * \param u
     * \param v
     * \param w
     * \param n
     * \param V
     *
     * \return   */
    bool snip(const std::vector<glm::vec2>& points, int u, int v, int w, int n, int* V);

    /** \brief  Compute the area of the polygon formed by points
     * \param points the polygon points
     * \return   the area */
    float area(const std::vector<glm::vec2>& points);
}

#endif
