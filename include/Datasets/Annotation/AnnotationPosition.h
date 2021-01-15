#ifndef  ANNOTATIONPOSITION_INC
#define  ANNOTATIONPOSITION_INC

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include "Datasets/Annotation/AnnotationLog.h"


namespace sereno
{
    /** \brief  Represent a view from AnnotationLog to read positions. The positions CANNOT BE MODIFIED as this represents only a view*/
    class AnnotationPosition
    {
        public:
            class AnnotationPositionIterator
            {
                public:
                    //Iterator specifications
                    typedef std::random_access_iterator_tag  iterator_category;
                    typedef ptrdiff_t                        difference_type;
                    typedef glm::vec3                       value_type;
                    typedef glm::vec3*                      pointer;
                    typedef glm::vec3&                      reference;

                    AnnotationPositionIterator(AnnotationLog* ann, const glm::ivec3& xyzInd, int32_t pos=0) : m_ann(ann), m_xyzInd(xyzInd), m_readPos(pos){readVal();}

                    AnnotationPositionIterator& operator++()     {m_readPos++; readVal(); return *this;}
                    AnnotationPositionIterator  operator++(int)  {AnnotationPositionIterator tmp(*this); ++(*this); return tmp;}
                    AnnotationPositionIterator& operator--()     {m_readPos--; readVal(); return *this;}
                    AnnotationPositionIterator  operator--(int)  {AnnotationPositionIterator tmp(*this); --(*this); return tmp;}
                    AnnotationPositionIterator  operator+(int i) {return AnnotationPositionIterator(m_ann, m_xyzInd, m_readPos+i);}
                    AnnotationPositionIterator  operator-(int i) {return AnnotationPositionIterator(m_ann, m_xyzInd, m_readPos-i);}

                    AnnotationPositionIterator  operator+=(int i) {m_readPos+=i; readVal(); return *this;}
                    AnnotationPositionIterator  operator-=(int i) {m_readPos-=i; readVal(); return *this;}

                    const glm::vec3& operator*()   const {return m_pos;}
                    const glm::vec3* operator->()  const {return &m_pos;}

                    bool operator==(const AnnotationPositionIterator& it)
                    {
                        return (m_ann == it.m_ann && m_readPos == it.m_readPos);
                    }

                    bool operator!=(const AnnotationPositionIterator& it) {return !((*this)==it);}
                private:
                    void readVal() 
                    {
                        if(m_readPos >= 0 && m_readPos < (int32_t)m_ann->size())
                            m_pos = glm::vec3((m_xyzInd[0] > -1 ? std::stof((*m_ann)[m_readPos][m_xyzInd[0]]) : 0),
                                              (m_xyzInd[1] > -1 ? std::stof((*m_ann)[m_readPos][m_xyzInd[1]]) : 0),
                                              (m_xyzInd[2] > -1 ? std::stof((*m_ann)[m_readPos][m_xyzInd[2]]) : 0));
                        if(m_readPos >= (int32_t)m_ann->size() || m_readPos < 0)
                            m_readPos = -1;
                    }

                    AnnotationLog* m_ann;
                    glm::ivec3     m_xyzInd;
                    int32_t        m_readPos;
                    glm::vec3      m_pos;
            };
        public:
            AnnotationPosition(AnnotationLog* ann) : m_ann(ann) {}

            void setXYZIndices(int32_t x, int32_t y, int32_t z) {m_xInd = x; m_yInd = y, m_zInd = z;}
            void setXYZHeaders(const std::string& x, const std::string& y, const std::string& z) {m_xInd = m_ann->indiceFromHeader(x);
                                                                                                  m_yInd = m_ann->indiceFromHeader(y);
                                                                                                  m_zInd = m_ann->indiceFromHeader(z);}

            AnnotationPositionIterator begin() const {return AnnotationPositionIterator(m_ann, glm::ivec3(m_xInd, m_yInd, m_zInd));}
            AnnotationPositionIterator end()   const {return AnnotationPositionIterator(m_ann, glm::ivec3(m_xInd, m_yInd, m_zInd), -1);}
        private:
            AnnotationLog* m_ann;
            int32_t m_xInd = -1;
            int32_t m_yInd = -1;
            int32_t m_zInd = -1;
    };
}

#endif
