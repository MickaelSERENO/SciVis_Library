#ifndef  CLMANAGER_INC
#define  CLMANAGER_INC

#ifdef COMPILE_OPENCL

#include <iostream>
#include <string>
#include <cstdlib>

//OpenCL Libraries
#include <CL/cl.h>
#include <CL/cl_gl.h>

#include "utils.h"

#ifndef CL_EXTENSION_CHAR_SIZE
#define CL_EXTENSION_CHAR_SIZE 1024
#endif

#ifndef CL_NB_PLATFORMS
#define CL_NB_PLATFORMS        8
#endif

#define CL_EXT_GETGLCONTEXTINFOKHR_BIT 0x01

namespace sereno
{
    /** Pointer function type for using a function pointer pointing to clGetGLContextInfoKHR*/
    typedef CL_API_ENTRY cl_int(CL_API_CALL *CLGETGLCONTEXTINFOKHR)(const cl_context_properties*, 
                                                                    cl_gl_context_info, 
                                                                    size_t, 
                                                                    void*, 
                                                                    size_t*);

    /** Structure containing information about the opencl extension functionalities*/
    typedef struct _CLExt
    {
        CLGETGLCONTEXTINFOKHR myClGetGLContextInfoKHR; /*!< Pointer to clGetGLContextInfoKHR*/
    }CLExt;

    /** \brief The OpenCL data structure. It will hold all the information the application needs to run opencl's kernels */
    class CLManager
    {
        public:
            /** \brief  Constructor. The OpenCL context is not yet created. Call "init" function to do so */
            CLManager();

            /** \brief  Destructor, clear the OpenCL context object */
            virtual ~CLManager();

            /** \brief  Initialize the OpenCL context
             * \param extFlags extensions flags to enable. See CL_EXT_*_BIT values
             * \param deviceType The device type to look at. This parameter is discarded if CL_EXT_GETGLCONTEXTINFOKHR_BIT flag is set (GPU will be used automatically)
             * \return  true on success, false on failure */
            bool init(uint64_t extFlags = 0x00, cl_device_type deviceType = CL_DEVICE_TYPE_GPU);

            /** \brief  Get the OpenCL command queue. Make sure that this object is first initialized
             * \return  The OpenCL command queue */
            cl_command_queue getQueue() {return m_queue;}

            /** \brief  Get the OpenCL context. Make sure that this object is first initialized
             * \return   The OpenCL context */
            cl_context getContext() {return m_context;}

            /** \brief  Is the OpenCL context object initialized? Use this function before using getQueue and getContext
             * \return    true if yes, false otherwise. */
            bool isInit() const {return m_isInit;}
        private:
            /** \brief  Clear the OpenCL context objects */
            void clear();

            cl_command_queue m_queue     = NULL;  /*!< The queue command */
            cl_context       m_context   = NULL;  /*!< The opencl context*/
            cl_device_id     m_device;            /*!< Information about the device selected*/
            cl_platform_id*  m_platforms = NULL;  /*!< Information about the platform*/
            uint8_t          m_idPlatform;        /*!< The platform ID used in our program*/
            CLExt            m_extData;           /*!< The OpenCL extension data*/
            bool             m_isInit    = false; /*!< Is the OpenCL initialized?*/
    };
}

#endif
#endif
