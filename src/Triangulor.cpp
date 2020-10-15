#include "Triangulor.h"

namespace sereno
{
    std::vector<int> triangulate(const std::vector<glm::vec2>& points)
    {
        std::vector<int> indices;

        int n = points.size();
        if (n < 3)
            return indices;

        int* V = new int[n];
        if(area(points) > 0)
        {
            for (int v = 0; v < n; v++)
                V[v] = v;
        }
        else
        {
            for (int v = 0; v < n; v++)
                V[v] = (n - 1) - v;
        }

        int nv = n;
        int count = 2 * nv;
        for (int v = nv - 1; nv > 2;)
        {
            if ((count--) <= 0)
                goto endTriangular;

            int u = v;
            if (nv <= u)
                u = 0;
            v = u + 1;
            if (nv <= v)
                v = 0;
            int w = v + 1;
            if (nv <= w)
                w = 0;

            if (snip(points, u, v, w, nv, V))
            {
                int a, b, c, s, t;
                a = V[u];
                b = V[v];
                c = V[w];
                indices.push_back(a);
                indices.push_back(b);
                indices.push_back(c);
                for (s = v, t = v + 1; t < nv; s++, t++)
                    V[s] = V[t];
                nv--;
                count = 2 * nv;
            }
        }

        std::reverse(indices.begin(), indices.end());

endTriangular:
        delete[] V;
        return indices;
    }

    float area(const std::vector<glm::vec2>& points)
    {
        int n = points.size();
        float A = 0.0f;
        for (int p = n - 1, q = 0; q < n; p = q++)
        {
            glm::vec2 pval = points[p];
            glm::vec2 qval = points[q];
            A += pval.x * qval.y - qval.x * pval.y;
        }
        return (A * 0.5f);
    }

    bool snip(const std::vector<glm::vec2>& points, int u, int v, int w, int n, int* V)
    {
        const float EPSILON = 0.0000001f;

        int p;
        glm::vec2 A = points[V[u]];
        glm::vec2 B = points[V[v]];
        glm::vec2 C = points[V[w]];
        if(EPSILON > (((B.x - A.x) * (C.y - A.y)) - ((B.y - A.y) * (C.x - A.x))))
            return false;
        for (p = 0; p < n; p++)
        {
            if ((p == u) || (p == v) || (p == w))
                continue;
            glm::vec2 P = points[V[p]];
            if (insideTriangle(A, B, C, P))
                return false;
        }
        return true;
    }

    bool insideTriangle(const glm::vec2& A, const glm::vec2& B, const glm::vec2& C, const glm::vec2& P)
    {
        float ax, ay, bx, by, cx, cy, apx, apy, bpx, bpy, cpx, cpy;
        float cCROSSap, bCROSScp, aCROSSbp;

        ax = C.x - B.x; ay = C.y - B.y;
        bx = A.x - C.x; by = A.y - C.y;
        cx = B.x - A.x; cy = B.y - A.y;
        apx = P.x - A.x; apy = P.y - A.y;
        bpx = P.x - B.x; bpy = P.y - B.y;
        cpx = P.x - C.x; cpy = P.y - C.y;

        aCROSSbp = ax * bpy - ay * bpx;
        cCROSSap = cx * apy - cy * apx;
        bCROSScp = bx * cpy - by * cpx;

        return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
    }
}
