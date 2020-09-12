#pragma once

#include "../nvapi_private.h"
#include "../dxvk/dxvk_interfaces.h"
#include "../util/com_pointer.h"

#include "nvapi_output.h"

namespace dxvk {

    class NvapiAdapter {

    public:

        NvapiAdapter();
        ~NvapiAdapter();

        bool Initialize(Com<IDXGIAdapter> dxgiAdapter, std::vector<NvapiOutput*>* outputs);

        std::string GetDeviceName();
        u_int GetDriverVersion();
        u_int GetDeviceId();
        u_int GetGpuType();

    private:

        VkPhysicalDevice m_vkDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties m_deviceProperties;
        u_int m_vkDriverVersion;

    };
}
