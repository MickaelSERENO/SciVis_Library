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

            const std::list<SubDataset*>& getSubDatasets() {return m_subDatasets;}
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
        STACK_END,
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

            /** \brief  Get the "original" SubDataset
             * \return  a pointer to the original SubDataset */
            SubDataset* getBase() {return m_base;}

            /** \brief  Get the "original" SubDataset
             * \return  a pointer to the original SubDataset */
            const SubDataset* getBase() const {return m_base;}
        protected:
            SubDataset* m_base; /*!< The SubDataset serving as models for the others*/
    };

    class SubDatasetSubjectiveStackedGroup : public SubDatasetSubjectiveGroup
    {
        public:
            /** \brief  Constructor
             * \param base the original SubDataset acting as a "base" to the others */
            SubDatasetSubjectiveStackedGroup(SubDataset* base);

            /** \brief  Pure virtual constructor to make the class abstract */
            virtual ~SubDatasetSubjectiveStackedGroup() = 0;

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
        protected:
            StackingEnum           m_stack          = STACK_VERTICAL; /*!< The stacking method to apply*/
            bool                   m_mergeSubjViews = true;           /*!< Are, when applied, stacked SubDatasets merged?*/
            float                  m_gap            = 0.10f;          /*!< The distance (world-space) between stacked SubDatasets.*/
    };

    /** \brief  A subjective views group where each subjective view is composed of one stacked Subdataset and one linked subdataset */
    class SubDatasetSubjectiveStackedLinkedGroup : public SubDatasetSubjectiveStackedGroup
    {
        public:
            SubDatasetSubjectiveStackedLinkedGroup(SubDataset* base);

            virtual void updateSubDatasets();

            virtual bool removeSubDataset(SubDataset* sd);

            /** \brief  Add a subjective view to stack with the base SubDataset, and another one to link with this stacked subjective view
             * \param sdStacked The SubDataset to stack with the base. sdStacked can be null. In this case, this class only relies on sdLinked which should be link to getBase()
             * \param sdLinked the SubDataset to link with sdStacked or getBase(). If nullptr, no linked SubDataset is added
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

            const std::list<std::pair<SubDataset*, SubDataset*>>& getLinkedSubDatasets() const {return m_subjViews;}

            /** \brief Is the subdataset spatially modifiable?
             * \param sd the subdataset to consider
             * \return true if sd == getBase() or is a "sdLinked", false otherwise*/
            virtual bool isSpatiallyModifiable(SubDataset* sd);
        protected:
            std::list<std::pair<SubDataset*, SubDataset*>> m_subjViews; /*!< List of linked subdatasets. pair.first == sdStacked, pair.second == sdLinked.*/
    };
}

#endif
