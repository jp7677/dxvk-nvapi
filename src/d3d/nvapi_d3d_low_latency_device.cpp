#include "nvapi_d3d_low_latency_device.h"

namespace dxvk {
    bool NvapiD3dLowLatencyDevice::SupportsLowLatency(IUnknown* device) {
        auto d3dLowLatencyDevice = GetLowLatencyDevice(device);
        if (d3dLowLatencyDevice == nullptr)
            return false;

        return d3dLowLatencyDevice->SupportsLowLatency();
    }

    bool NvapiD3dLowLatencyDevice::LatencySleep(IUnknown* device) {
        auto d3dLowLatencyDevice = GetLowLatencyDevice(device);
        if (d3dLowLatencyDevice == nullptr)
            return false;

        return SUCCEEDED(d3dLowLatencyDevice->LatencySleep());
    }

    bool NvapiD3dLowLatencyDevice::SetLatencySleepMode(IUnknown* device, bool lowLatencyMode, bool lowLatencyBoost, uint32_t minimumIntervalUs) {
        auto d3dLowLatencyDevice = GetLowLatencyDevice(device);
        if (d3dLowLatencyDevice == nullptr)
            return false;

        return SUCCEEDED(d3dLowLatencyDevice->SetLatencySleepMode(lowLatencyMode, lowLatencyBoost, minimumIntervalUs));
    }

    bool NvapiD3dLowLatencyDevice::GetLatencyInfo(IUnknown* device, D3D_LATENCY_RESULTS* latencyResults) {
        auto d3dLowLatencyDevice = GetLowLatencyDevice(device);
        if (d3dLowLatencyDevice == nullptr)
            return false;

        return SUCCEEDED(d3dLowLatencyDevice->GetLatencyInfo(latencyResults));
    }

    bool NvapiD3dLowLatencyDevice::SetLatencyMarker(IUnknown* device, uint64_t frameID, uint32_t markerType) {
        auto d3dLowLatencyDevice = GetLowLatencyDevice(device);
        if (d3dLowLatencyDevice == nullptr)
            return false;

        return SUCCEEDED(d3dLowLatencyDevice->SetLatencyMarker(frameID, markerType));
    }

    void NvapiD3dLowLatencyDevice::ClearCacheMaps() {
        std::scoped_lock lock(m_LowLatencyDeviceMutex);

        m_lowLatencyDeviceMap.clear();
    }

    Com<ID3DLowLatencyDevice> NvapiD3dLowLatencyDevice::GetLowLatencyDevice(IUnknown* device) {
        if (device == nullptr)
            return nullptr;

        std::scoped_lock lock(m_LowLatencyDeviceMutex);
        auto it = m_lowLatencyDeviceMap.find(device);
        if (it != m_lowLatencyDeviceMap.end())
            return it->second;

        Com<ID3DLowLatencyDevice> d3dLowLatencyDevice;
        if (FAILED(device->QueryInterface(IID_PPV_ARGS(&d3dLowLatencyDevice))))
            return nullptr;

        m_lowLatencyDeviceMap.emplace(device, d3dLowLatencyDevice.ptr());

        return d3dLowLatencyDevice;
    }
}
