#ifndef  SUBDATASETGROUP_INC
#define  SUBDATASETGROUP_INC

#include "Datasets/SubDataset.h"

namespace sereno
{
    /** \brief  Basic class for grouping subdatasets together. */
    class SubDatasetGroup
    {
        public:
            /** \brief  General constructor. */
            SubDatasetGroup() {}
            /** \brief  Disable the copy constructor
             * \param copy the parameter to copy */
            SubDatasetGroup(const SubDatasetGroup& copy) = delete;

            /** \brief  Destructor */
            virtual ~SubDatasetGroup();

            /** \brief  Remove a subdataset from this group
             * \param sd the subdataset to remove 
             * \return true on success, false of failure*/
            virtual bool removeSubDataset(SubDataset* sd);
            
            /** \brief Update the properties of all registered SubDatasets based on the group property */
            virtual void updateSubDatasets() {}

            /** \brief Is the SubDataset "sd" modifiable in this group?
             * \param sd The SubDataset to consider 
             * \return true if yes, false otherwise */
            virtual bool isSpatiallyModifiable(SubDataset* sd) {return true;}
        protected:
            /** \brief  Register a new SubDataset. This method is protected because it shall depends on the upper-class classification and data storage method
             * \param sd The SubDataset to register
             * \return true on success, false on failure  */
            bool addSubDataset(SubDataset* sd); 

            std::list<SubDataset*> m_subDatasets; /*!< The registered subdatasets*/

        friend class SubDataset;
    };

    /** \brief  The different stacking method */
    enum StackingEnum
    {
        STACK_VERTICAL   = 0, /*!< Vertical */
        STACK_HORIZONTAL = 1, /*!< Horizontal */
    };

    /** \brief  Base class for Subjective Views */
    class SubDatasetSubjectiveGroup : public SubDatasetGroup
    {
        public:
            /** \brief  Constructor
             * \param base the original SubDataset acting as a "base" to the others */
            SubDatasetSubjectiveGroup(SubDataset* base);

            /** \brief  Pure virtual constructor to make the class abstract */
            virtual ~SubDatasetSubjectiveGroup() = 0;

            /** \brief  Set the gap between subjective views (when stacked)
             * \param gap the gap distance (world-space distance) */
            void setGap(float gap) {m_gap = gap;}

            /** \brief  Get the gap between subjective views (when stacked)
             * \return the gap distance (world-space distance) */
            float getGap() const {return m_gap;}

            /** \brief Set whether or not stacked subjective views are spatially merged?
             * \param merge true if yes, false otherwise */
            void setMerge(bool merge) {m_mergeSubjViews = merge;}

            /** \brief get whether or not stacked subjective views are spatially merged?
             * \return true if yes, false otherwise */
            bool getMerge() const {return m_mergeSubjViews;}

            /** \brief  Set the stacking method of subjective views (when applied)
             * \param stack the new stacking method to apply */
            void setStackingMethod(StackingEnum stack) { m_stack = stack; }

            /** \brief  Get the stacking method of subjective views (when applied)
             * \return the stacking method to apply */
            StackingEnum getStackingMethod() const {return m_stack;}

            SubDataset* getBase() {return m_base;}

            const SubDataset* getBase() const {return m_base;}
        protected:
            SubDataset*            m_base;                            /*!< The SubDataset serving as models for the others*/
            StackingEnum           m_stack          = STACK_VERTICAL; /*!< The stacking method to apply*/
            bool                   m_mergeSubjViews = false;          /*!< Are, when applied, stacked SubDatasets merged?*/
            float                  m_gap            = 0.20f;          /*!< The distance (world-space) between stacked SubDatasets.*/
    };

    /** \brief  Group caracterizing Stacked Subjective views. */
    class SubDatasetSubjectiveStackedGroup : public SubDatasetSubjectiveGroup
    {
        public:
            /** \brief  Constructor
             * \param base the Original SubDataset serving as a base for the others */
            SubDatasetSubjectiveStackedGroup(SubDataset* base);

            virtual void updateSubDatasets();

            /** \brief  Remove a registered subdataset.
             * \param sd the SubDataset to remove. It sd is the base SubDataset, all subdatasets are removed from this group
             * \return   true on success (the subdataset is found), false otherwise */
            virtual bool removeSubDataset(SubDataset* sd);

            /** \brief  Add a subjective view to stack with the base SubDataset
             * \param sd The SubDataset to stack with the base.
             * \return true on success (the SubDataset was not registered before), false otherwise */
            bool addSubjectiveSubDataset(SubDataset* sd);

            /** \brief Is the subdataset spatially modifiable?
             * \param sd the subdataset to consider
             * \return true if sd == getBase(), false otherwise*/
            virtual bool isSpatiallyModifiable(SubDataset* sd);
        protected:
            std::list<SubDataset*> m_subjViews; /*!< The subjective views being stacked */
    };

    class SubDatasetSubjectiveLinkedGroup : public SubDatasetSubjectiveGroup
    {
        public:
            SubDatasetSubjectiveLinkedGroup(SubDataset* base);

            virtual void updateSubDatasets();

            virtual bool removeSubDataset(SubDataset* sd);

            /** \brief  Add a subjective view to stack with the base SubDataset, and another one to link with this stacked subjective view
             * \param sdStacked The SubDataset to stack with the base.
             * \param sdLinked the SubDataset to link with sdStacked.
             * \return true on success (the SubDatasets were not registered before), false otherwise */
            bool addSubjectiveSubDataset(SubDataset* sdStacked, SubDataset* sdLinked);

            /** \brief  Get the pair of subdatasets linked together
             * \param sd the subdataset to evaluate.
             * \return   The pair containing sd and its linked subdataset. pair.first == sdStacked, pair.second == sdLinked.*/
            std::pair<SubDataset*, SubDataset*> getLinkedSubDataset(SubDataset* sd);

            /** \brief  Get the pair of subdatasets linked together
             * \param sd the subdataset to evaluate.
             * \return   The pair containing sd and its linked subdataset. pair.first == sdStacked, pair.second == sdLinked.*/
            std::pair<const SubDataset*, const SubDataset*> getLinkedSubDataset(SubDataset* sd) const;

            /** \brief Is the subdataset spatially modifiable?
             * \param sd the subdataset to consider
             * \return true if sd == getBase() or is a "sdLinked", false otherwise*/
            virtual bool isSpatiallyModifiable(SubDataset* sd);
        protected:
            std::list<std::pair<SubDataset*, SubDataset*>> m_subjViews; /*!< List of linked subdatasets. pair.first == sdStacked, pair.second == sdLinked.*/
    };
}

#endif
