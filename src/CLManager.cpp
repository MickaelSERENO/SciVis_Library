#include "CLManager.h"

#ifdef COMPILE_OPENCL

namespace sereno
{
    CLManager::CLManager()
    {}

    bool CLManager::init(uint64_t extFlags, cl_device_type deviceType)
    {
        INFO << "Initializing OpenCL data" << std::endl;

        //Get the opencl platform
        cl_uint nbPlatforms;
        m_platforms   = (cl_platform_id*)malloc(CL_NB_PLATFORMS*sizeof(cl_platform_id));
        cl_int result = clGetPlatformIDs(CL_NB_PLATFORMS, m_platforms, &nbPlatforms);    

        if(result != CL_SUCCESS || nbPlatforms == 0)
        {
            ERROR << "Could not fetch opencl's platform" << std::endl;
            goto clearCL;
        }

        uint8_t idPlatform       = 0;
        char    platformNotFound = 1;
        for(idPlatform = 0; idPlatform < nbPlatforms && platformNotFound; idPlatform++)
        {
            char platformName[1024];
            clGetPlatformInfo(m_platforms[idPlatform], CL_PLATFORM_NAME, 1024, platformName, NULL);
            INFO << "Checking OpenCL platform " << platformName << std::endl;

            /*----------------------------------------------------------------------------*/
            /*----------------Initialize opencl extension functionalities-----------------*/
            /*----------------------------------------------------------------------------*/

            //Start with GETGLCONTEXTINFOKHR
            if(extFlags & CL_EXT_GETGLCONTEXTINFOKHR_BIT)
            {
                m_extData.myClGetGLContextInfoKHR = 
                    (CLGETGLCONTEXTINFOKHR)clGetExtensionFunctionAddressForPlatform(m_platforms[idPlatform], "clGetGLContextInfoKHR");

                if(m_extData.myClGetGLContextInfoKHR == NULL)
                    continue;

                //The opencl properties
                const cl_context_properties properties[] = 
                {
                    CL_GL_CONTEXT_KHR,   (cl_context_properties)glXGetCurrentContext(),
                    CL_GLX_DISPLAY_KHR,  (cl_context_properties)glXGetCurrentDisplay(),
                    CL_CONTEXT_PLATFORM, (cl_context_properties)m_platforms[idPlatform],
                    0
                };

                //Get the opencl devices
                size_t deviceSize;
                m_device  = (cl_device_id*)malloc(sizeof(cl_device_id));
                if(m_device == NULL)
                {
                    ERROR << "Could not allocate memory for the opencl device structure" << std::endl;
                    goto clearCL;
                }

                result = m_extData.myClGetGLContextInfoKHR(properties, CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR, sizeof(cl_device_id), (void*)&m_device, &deviceSize);

                if(result != CL_SUCCESS || deviceSize == 0)
                    continue;
            }

            //If the device should not match the OpenGL one, find the most appropriate device based on the device type and the platform.
            else
            {
                cl_uint numDevices;
                result = clGetDeviceIDs(m_platforms[idPlatform], deviceType, 1, &m_device, &numDevices);

                if(result != CL_SUCCESS || numDevices == 0)
                    continue;
            }

            ////////////////////////////////////////////////////
            //Here we have found the correct platform
            //We stay here (in the for loop) for the properties
            //used. All error goto clearCL tag
            ////////////////////////////////////////////////////
            
            char deviceName[1024];
            clGetDeviceInfo  (m_device, CL_DEVICE_NAME, 1024, deviceName, NULL);
            INFO << "OpenCL Platform : " << platformName << " Device : " << deviceName << std::endl;
            platformNotFound = 0;
            m_idPlatform = idPlatform;

            //Get the opencl context
            m_context = clCreateContext(properties, 1, m_device, NULL, 0, &result);
            if(m_context == NULL)
            {
                ERROR << "Could not create opencl context" << std::endl;
                goto clearCL;
            }

            //Initialize the queue
            m_queue = clCreateCommandQueueWithProperties(m_context, m_device, 0, &result);

            if(result != CL_SUCCESS)
            {
                ERROR << "Could not initialize the opencl command queue" << std::endl;
                goto clearCL;
            }

            break;
        }

        if(idPlatform == nbPlatforms)
        {
            ERROR << "No Opencl platform found corresponds to our need." << std::endl;
            goto clearCL;
        }

        return true;

    //Clear the data if an error occured and return NULL
    clearCL:
        ERROR << "Error id : " << result << std::endl;
        clear();
        return false;
    }

    CLManager::~CLManager()
    {
        clear();
    }

    void CLManager::clear()
    {
        if(m_queue != NULL)
        {
            clFlush(m_queue);
            clFinish(m_queue);
            clReleaseCommandQueue(m_queue);

            m_queue = NULL;
        }

        if(m_platforms != NULL)
        {
            free(m_platforms);
            m_platforms = NULL;
        }

        if(m_context != NULL)
        {
            clReleaseContext(m_context);
            m_context = NULL;
        }

        m_isInit = false;
    }
}

#endif
