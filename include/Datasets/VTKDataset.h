#ifndef  VTKDATASET_INC
#define  VTKDATASET_INC

#include <string>
#include <memory>
#include <vector>
#include <thread>
#include "VTKParser.h"
#include "Dataset.h"

namespace sereno
{
    /** \brief  Represent a VTK dataset information to take account of */
    class VTKDataset : public Dataset
    {
        public:
            /**
             * \brief  Constructor. Initialize this VTKDataset with a set of VTKFieldValue to take account of
             * \param parser the parser
             * \param ptFieldValues   the point field values to take account of. Must be part of the parser parameter.
             * \param cellFieldValues the cell  field values to take account of. Must be part of the parser parameter.
             */
            VTKDataset(std::shared_ptr<VTKParser>& parser, const std::vector<const VTKFieldValue*>& ptFieldValues, 
                                     const std::vector<const VTKFieldValue*>& cellFieldValues);

            ~VTKDataset();

            /* \brief  Get the Parser containing the dataset data
             * \return  the VTKParser */
            const std::shared_ptr<VTKParser> getParser() const {return m_parser;}

            /**
             * \brief  Get the point field values
             * \return   the VTKFieldValue* objects
             */
            const std::vector<const VTKFieldValue*>& getPtFieldValues() const {return m_ptFieldValues;}

            /* \brief  Get the indice of the point field value <value> in the Dataset (VTKParser) array
             * \param value the value to look at
             * \return  the indice of the value. -1 if not found */
            int32_t getPtFieldValueIndice(const VTKFieldValue* value) const 
            {
                std::vector<const VTKFieldValue*> values = m_parser->getPointFieldValueDescriptors();
                for(size_t i = 0; i < values.size(); i++)
                    if(values[i] == value)
                        return i;
                return -1;
            }

            /* \brief  Get the indice of the cell field value <value> in the Dataset (VTKParser) array
             * \param value the value to look at
             * \return  the indice of the value. -1 if not found */
            int32_t getCellFieldValueIndice(const VTKFieldValue* value) const 
            {
                std::vector<const VTKFieldValue*> values = m_parser->getCellFieldValueDescriptors();
                for(size_t i = 0; i < values.size(); i++)
                    if(values[i] == value)
                        return i;
                return -1;
            }

            virtual void removeSubDataset(SubDataset* sd) 
            {
                Dataset::removeSubDataset(sd);
            }

            virtual void loadValues(LoadCallback clbk, void* data);

            virtual bool create1DHistogram(uint32_t* output, uint32_t width, uint32_t ptFieldXID) const;

            virtual bool create2DHistogram(uint32_t* output, uint32_t width, uint32_t height, uint32_t ptFieldXID, uint32_t ptFieldYID) const;

            /** \brief  Has this dataset a mask?
             * \return   true if yes, false otherwise */
            bool hasMaskComputed() const {return m_mask != NULL;}

            /** \brief  Get the mask to apply at indice "ind"
             * \param ind the indice to look at
             * \return  true if yes, false otherwise */
            bool getMask(uint32_t ind) const
            {
                if(m_mask == NULL)
                    return true;
                return m_mask[ind/8]&(1 << (ind%8));
            }

        protected:
            virtual DatasetGradient* computeGradient(const std::vector<uint32_t>& indices);

        private:

            /** \brief  Compute the multi-dimensional "gradient magnitude". Call it AFTER loading the data
             * This function generate the L2 norm of delta = (Df)^T . Df, with 
             *
             *      |dF1/dx   dF1/dy   dF1/dz|
             * Df = |......   ......   ......|
             *      |dFn/dx   dFn/dy   dFn/dz|
             *
             * See Joe Kniss, Gordon Kindlmann, and Charles Hansen. 2002. Multidimensional Transfer Functions for Interactive Volume Rendering. IEEE Transactions on Visualization and Computer Graphics 8, 3 (July 2002)
             *
             */
            void computeMultiDGradient();

            std::vector<const VTKFieldValue*> m_ptFieldValues;     /*!< The point field values*/
            std::vector<const VTKFieldValue*> m_cellFieldValues;   /*!< The cell  field values*/
            std::shared_ptr<VTKParser>        m_parser;            /*!< The VTK parser*/
            uint8_t*                          m_mask = NULL;       /*!< The mask values to apply. Here, 1 bit == 1 value*/
            std::thread                       m_readThread;        /*!< The reading thread*/
            bool                              m_readThreadRunning = false; /*!< Is the reading thread running?*/
    };
}

#endif
