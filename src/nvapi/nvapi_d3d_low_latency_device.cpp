#include "nvapi_d3d_low_latency_device.h"
#include "../util/com_pointer.h"
#include "../util/util_string.h"
#include "../util/util_log.h"

namespace dxvk {
    std::unordered_map<IUnknown*, std::shared_ptr<NvapiD3dLowLatencyDevice>> NvapiD3dLowLatencyDevice::m_nvapiDeviceMap = {};
    std::mutex NvapiD3dLowLatencyDevice::m_mutex = {};

    void NvapiD3dLowLatencyDevice::Reset() {
        std::scoped_lock lock{m_mutex};
        m_nvapiDeviceMap.clear();
    }

    static Com<ID3DLowLatencyDevice> GetD3DLowLatencyDevice(IUnknown* device) {
        Com<ID3DLowLatencyDevice> d3dLowLatencyDevice;

        if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&d3dLowLatencyDevice))))
            return d3dLowLatencyDevice;

        if (Com<ID3D11DeviceChild> d3d11DeviceChild; SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&d3d11DeviceChild)))) {
            Com<ID3D11Device> d3d11Device;
            d3d11DeviceChild->GetDevice(&d3d11Device);
            if (SUCCEEDED(d3d11Device->QueryInterface(IID_PPV_ARGS(&d3dLowLatencyDevice))))
                return d3dLowLatencyDevice;
        }

        if (Com<ID3D12DeviceChild> d3d12DeviceChild; SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&d3d12DeviceChild)))) {
            if (SUCCEEDED(d3d12DeviceChild->GetDevice(IID_PPV_ARGS(&d3dLowLatencyDevice))))
                return d3dLowLatencyDevice;
        }

        return nullptr;
    }

    NvapiD3dLowLatencyDevice* NvapiD3dLowLatencyDevice::GetOrCreate(IUnknown* device) {
        std::scoped_lock lock{m_mutex};

        if (auto lowLatencyDevice = Get(device))
            return lowLatencyDevice;

        auto d3dLowLatencyDevice = GetD3DLowLatencyDevice(device);
        if (d3dLowLatencyDevice == nullptr)
            return nullptr;

        // Look for a cache entry where NvapiD3dLowLatencyDevice's m_d3dLowLatencyDevice matches the one we found
        auto itF = std::find_if(m_nvapiDeviceMap.begin(), m_nvapiDeviceMap.end(),
            [&d3dLowLatencyDevice](auto& item) { return item.second->m_d3dLowLatencyDevice == d3dLowLatencyDevice.ptr(); });

        auto [itI, inserted] = itF == m_nvapiDeviceMap.end()
            ? m_nvapiDeviceMap.emplace(device, std::make_shared<NvapiD3dLowLatencyDevice>(d3dLowLatencyDevice.ptr()))
            : m_nvapiDeviceMap.emplace(device, itF->second);

        if (!inserted)
            return nullptr;

        return itI->second.get();
    }

    NvapiD3dLowLatencyDevice* NvapiD3dLowLatencyDevice::Get(IUnknown* device) {
        auto it = m_nvapiDeviceMap.find(device);
        return it == m_nvapiDeviceMap.end() ? nullptr : it->second.get();
    }

    std::optional<uint32_t> NvapiD3dLowLatencyDevice::ToMarkerType(NV_LATENCY_MARKER_TYPE markerType) {
        static_assert(static_cast<int>(SIMULATION_START) == static_cast<int>(VK_LATENCY_MARKER_SIMULATION_START_NV));
        static_assert(static_cast<int>(SIMULATION_END) == static_cast<int>(VK_LATENCY_MARKER_SIMULATION_END_NV));
        static_assert(static_cast<int>(RENDERSUBMIT_START) == static_cast<int>(VK_LATENCY_MARKER_RENDERSUBMIT_START_NV));
        static_assert(static_cast<int>(RENDERSUBMIT_END) == static_cast<int>(VK_LATENCY_MARKER_RENDERSUBMIT_END_NV));
        static_assert(static_cast<int>(PRESENT_START) == static_cast<int>(VK_LATENCY_MARKER_PRESENT_START_NV));
        static_assert(static_cast<int>(PRESENT_END) == static_cast<int>(VK_LATENCY_MARKER_PRESENT_END_NV));
        static_assert(static_cast<int>(INPUT_SAMPLE) == static_cast<int>(VK_LATENCY_MARKER_INPUT_SAMPLE_NV));
        static_assert(static_cast<int>(TRIGGER_FLASH) == static_cast<int>(VK_LATENCY_MARKER_TRIGGER_FLASH_NV));
        static_assert(static_cast<int>(OUT_OF_BAND_RENDERSUBMIT_START) - 1 == static_cast<int>(VK_LATENCY_MARKER_OUT_OF_BAND_RENDERSUBMIT_START_NV));
        static_assert(static_cast<int>(OUT_OF_BAND_RENDERSUBMIT_END) - 1 == static_cast<int>(VK_LATENCY_MARKER_OUT_OF_BAND_RENDERSUBMIT_END_NV));
        static_assert(static_cast<int>(OUT_OF_BAND_PRESENT_START) - 1 == static_cast<int>(VK_LATENCY_MARKER_OUT_OF_BAND_PRESENT_START_NV));
        static_assert(static_cast<int>(OUT_OF_BAND_PRESENT_END) - 1 == static_cast<int>(VK_LATENCY_MARKER_OUT_OF_BAND_PRESENT_END_NV));

        switch (markerType) {
            case SIMULATION_START:
            case SIMULATION_END:
            case RENDERSUBMIT_START:
            case RENDERSUBMIT_END:
            case PRESENT_START:
            case PRESENT_END:
            case INPUT_SAMPLE:
            case TRIGGER_FLASH:
                return markerType;
            // VkLatencyMarkerNV misses PC_LATENCY_PING and all following enum values are offset
            // See https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#VUID-VkSetLatencyMarkerInfoNV-marker-parameter
            case PC_LATENCY_PING:
                return {};
            case OUT_OF_BAND_RENDERSUBMIT_START:
            case OUT_OF_BAND_RENDERSUBMIT_END:
            case OUT_OF_BAND_PRESENT_START:
            case OUT_OF_BAND_PRESENT_END:
                return markerType - 1;
        }

        log::info(str::format("Unknown NV_LATENCY_MARKER_TYPE: ", markerType));
        return {};
    }

    NvapiD3dLowLatencyDevice::NvapiD3dLowLatencyDevice(ID3DLowLatencyDevice* d3dLowLatencyDevice)
        : m_d3dLowLatencyDevice(d3dLowLatencyDevice) {}

    bool NvapiD3dLowLatencyDevice::SupportsLowLatency() const {
        return m_d3dLowLatencyDevice->SupportsLowLatency();
    }

    HRESULT NvapiD3dLowLatencyDevice::LatencySleep() const {
        return m_d3dLowLatencyDevice->LatencySleep();
    }

    HRESULT NvapiD3dLowLatencyDevice::SetLatencySleepMode(bool lowLatencyMode, bool lowLatencyBoost, uint32_t minimumIntervalUs) {
        auto result = m_d3dLowLatencyDevice->SetLatencySleepMode(lowLatencyMode, lowLatencyBoost, minimumIntervalUs);
        if (SUCCEEDED(result))
            m_lowLatencyMode = lowLatencyMode;

        return result;
    }

    HRESULT NvapiD3dLowLatencyDevice::GetLatencyInfo(D3D_LATENCY_RESULTS* latencyResults) {
        auto result = m_d3dLowLatencyDevice->GetLatencyInfo(latencyResults);
        if (FAILED(result))
            return result;

        for (auto& frameReport : latencyResults->frame_reports) {
            frameReport.frameID = m_frameIdGenerator.GetApplicationFrameId(frameReport.frameID);
            if (!frameReport.frameID) {
                memset(latencyResults->frame_reports, 0, sizeof(latencyResults->frame_reports));
                break;
            }
        }

        return result;
    }

    HRESULT NvapiD3dLowLatencyDevice::SetLatencyMarker(uint64_t frameID, uint32_t markerType) {
        if (m_frameIdGenerator.IsRepeatedFrame(frameID, markerType))
            return S_OK; // Silently drop repeated frame IDs

        return m_d3dLowLatencyDevice->SetLatencyMarker(
            m_frameIdGenerator.GetLowLatencyDeviceFrameId(frameID), markerType);
    }

    bool NvapiD3dLowLatencyDevice::GetLowLatencyMode() const {
        return m_lowLatencyMode;
    }
}
