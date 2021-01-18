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
            /** \brief  The iterator corresponding to the AnnotationPosition view */
            class AnnotationPositionIterator
            {
                public:
                    //Iterator specifications
                    typedef std::random_access_iterator_tag iterator_category; //Access wherever you want
                    typedef int                             difference_type;
                    typedef glm::vec3                       value_type;
                    typedef glm::vec3*                      pointer;
                    typedef glm::vec3&                      reference;

                    AnnotationPositionIterator(const AnnotationLog* ann, const glm::ivec3& xyzInd, int32_t pos=0) : m_ann(ann), m_xyzInd(xyzInd), m_readPos(pos){readVal();}
                    AnnotationPositionIterator(const AnnotationPositionIterator& cpy) {*this = cpy;}

                    AnnotationPositionIterator& operator=(const AnnotationPositionIterator& cpy)
                    {
                        if(this != &cpy)
                        {
                            m_ann     = cpy.m_ann;
                            m_xyzInd  = cpy.m_xyzInd;
                            m_readPos = cpy.m_readPos;
                            readVal();
                        }
                        return *this;
                    }

                    glm::vec3 operator[](int rhs) const
                    {
                        int readPos = m_readPos + rhs;
                        if(readPos >= 0 && readPos < (int32_t)m_ann->size())
                            return glm::vec3((m_xyzInd[0] > -1 ? std::stof((*m_ann)[m_readPos][m_xyzInd[0]]) : 0),
                                             (m_xyzInd[1] > -1 ? std::stof((*m_ann)[m_readPos][m_xyzInd[1]]) : 0),
                                             (m_xyzInd[2] > -1 ? std::stof((*m_ann)[m_readPos][m_xyzInd[2]]) : 0));

                        return glm::vec3(-1, -1, -1);
                    }

                    AnnotationPositionIterator& operator++()      {m_readPos++; readVal(); return *this;}
                    AnnotationPositionIterator  operator++(int)   {AnnotationPositionIterator tmp(*this); ++(*this); return tmp;}
                    AnnotationPositionIterator& operator--()      {m_readPos--; readVal(); return *this;}
                    AnnotationPositionIterator  operator--(int)   {AnnotationPositionIterator tmp(*this); --(*this); return tmp;}

                    AnnotationPositionIterator  operator+(int i)  {return AnnotationPositionIterator(m_ann, m_xyzInd, m_readPos+i);}
                    AnnotationPositionIterator& operator+=(int i) {m_readPos+=i; readVal(); return *this;}
                    AnnotationPositionIterator  operator-(int i)  {return AnnotationPositionIterator(m_ann, m_xyzInd, m_readPos-i);}
                    AnnotationPositionIterator& operator-=(int i) {m_readPos-=i; readVal(); return *this;}

                    int                         operator- (const AnnotationPositionIterator& i)  {return m_readPos-i.m_readPos;}

                    const glm::vec3& operator*()   const {return m_pos;}
                    const glm::vec3* operator->()  const {return &m_pos;}

                    bool operator==(const AnnotationPositionIterator& it)
                    {
                        return (m_ann == it.m_ann && m_readPos == it.m_readPos);
                    }

                    bool operator!=(const AnnotationPositionIterator& it) {return !((*this)==it);}

                    inline bool operator>(const  AnnotationPositionIterator& rhs) {return m_readPos >  rhs.m_readPos;}
                    inline bool operator<(const  AnnotationPositionIterator& rhs) {return m_readPos <  rhs.m_readPos;}
                    inline bool operator>=(const AnnotationPositionIterator& rhs) {return m_readPos >= rhs.m_readPos;}
                    inline bool operator<=(const AnnotationPositionIterator& rhs) {return m_readPos <= rhs.m_readPos;}
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

                    const AnnotationLog* m_ann;
                    glm::ivec3     m_xyzInd;
                    int32_t        m_readPos;
                    glm::vec3      m_pos;
            };
        public:
            /** \brief  Constructor
             * \param ann the AnnotationLog to "view" on. The AnnotationPosition should not outlive ann.*/
            AnnotationPosition(const AnnotationLog* ann) : m_ann(ann) {}

            /** \brief  Set the x, y, and z column indices from the AnnotationLog to look upon. Negative values == we do not look at that component
             *
             * \param x the X values column indice
             * \param y the Y values column indice
             * \param z the Z values column indice */
            void setXYZIndices(int32_t x, int32_t y, int32_t z) {m_xInd = (x < (int32_t)m_ann->size() ? x : -1); 
                                                                 m_yInd = (y < (int32_t)m_ann->size() ? y : -1);
                                                                 m_zInd = (z < (int32_t)m_ann->size() ? z : -1);}

            /** \brief  Set the x, y, and z column headers from the AnnotationLog to look upon. Header not found == we do not look at that component. See AnnotationLog::indiceFromHeader for more information
             *
             * \param x the X values column header title
             * \param y the Y values column header title
             * \param z the Z values column header title */
            void setXYZHeaders(const std::string& x, const std::string& y, const std::string& z) {m_xInd = m_ann->indiceFromHeader(x);
                                                                                                  m_yInd = m_ann->indiceFromHeader(y);
                                                                                                  m_zInd = m_ann->indiceFromHeader(z);}

            /** \brief  Being iterator. The iterator values are constant. Modifying the XYZ indices while browsing the iterator has no effect. Values are given as glm::vec3 objects
             * \return  the first position to look at */
            AnnotationPositionIterator begin() const {return AnnotationPositionIterator(m_ann, glm::ivec3(m_xInd, m_yInd, m_zInd));}

            /** \brief  End iterator. The iterator values are constant
             * \return  An invalid iterator corresponding to "end"*/
            AnnotationPositionIterator end()   const {return AnnotationPositionIterator(m_ann, glm::ivec3(m_xInd, m_yInd, m_zInd), -1);}

            /** \brief Get the positions XYZ indices and stored them in the array "indices" 
             * \param indices the array to store the result. minimum size: 3
             *
             * \return  the parameter 'indices' */
            int32_t* getPosIndices(int32_t* indices) const {indices[0] = m_xInd; indices[1] = m_yInd; indices[2] = m_zInd; return indices;}

            /** \brief  Get the annotation log being read
             * \return the annotation log */
            const AnnotationLog* getAnnotationLog() const {return m_ann;}
        private:
            const AnnotationLog* m_ann;
            int32_t m_xInd = -1;
            int32_t m_yInd = -1;
            int32_t m_zInd = -1;
    };
}

#endif
