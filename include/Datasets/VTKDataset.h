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
    /** \brief  A VTKTimestep structure containing meta data for any timestep */
    struct VTKTimestep
    {
        std::shared_ptr<VTKParser>        parser;          /*!< The VTKParser containing the timestep data*/
        std::vector<const VTKFieldValue*> ptFieldValues;   /*!< The point field values*/
        std::vector<const VTKFieldValue*> cellFieldValues; /*!< The cell  field values*/
    };

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

            /** \brief  Add another timestep in this VTKDataset object
             * \param parser the VTKParser containing the new timestep data
             * \return  true on success, false on failure. The ptFieldValues and cellFieldValues to take into account must be the same, as should be the spatial resolution of the dataset. The loading of the values should not have started yet */
            bool addTimestep(std::shared_ptr<VTKParser>& parser);

            /* \brief  Get the Parser containing the dataset data
             * \param t the timestep to look at
             * \return  the VTKParser */
            const std::shared_ptr<VTKParser> getParser(uint32_t t=0) const {return m_timesteps[t].parser;}

            /**
             * \brief  Get the point field values
             * \param t the timestep to look at
             * \return   the VTKFieldValue* objects */
            const std::vector<const VTKFieldValue*>& getPtFieldValues(uint32_t t=0) const {return m_timesteps[t].ptFieldValues;}

            /** \brief  Get the timestep data at t
             * \param t the time to look at. Must be inferior at getNbTimesteps or the behavior is undefined.
             * \return  a const reference to the timestep at t */
            const VTKTimestep& getTimestep(uint32_t t) const {return m_timesteps[t];}

            /** \brief  Get the number of registered timesteps
             * \return   the number of registered timesteps */
            uint32_t getNbTimesteps() const {return m_timesteps.size();}

            /* \brief  Get the indice of the point field value <value> in the Dataset (VTKParser) array of the first VTKParser (useful for sharing data)
             * \param value the value to look at
             * \return  the indice of the value. -1 if not found */
            int32_t getPtFieldValueIndice(const VTKFieldValue* value) const 
            {
                std::vector<const VTKFieldValue*> values = getParser()->getPointFieldValueDescriptors();
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
                std::vector<const VTKFieldValue*> values = getParser()->getCellFieldValueDescriptors();
                for(size_t i = 0; i < values.size(); i++)
                    if(values[i] == value)
                        return i;
                return -1;
            }

            virtual void removeSubDataset(SubDataset* sd) 
            {
                Dataset::removeSubDataset(sd);
            }

            virtual std::thread* loadValues(LoadCallback clbk, void* data);

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

            std::vector<VTKTimestep> m_timesteps;         /*!< The timestep this data contain*/
            uint8_t*                 m_mask = NULL;       /*!< The mask values to apply. Here, 1 bit == 1 value*/
            std::thread              m_readThread;        /*!< The reading thread*/
            bool                     m_readThreadRunning = false; /*!< Is the reading thread running?*/
    };
}

#endif
