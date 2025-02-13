#include "nvapi_d3d_low_latency_device.h"
#include "../util/util_string.h"
#include "../util/util_log.h"

namespace dxvk {
    LowLatencyFrameIdGenerator::LowLatencyFrameIdGenerator()
        : m_nextLowLatencyDeviceFrameId(1),
          m_applicationIdList({0}) {
        m_lastFrameId.fill(std::numeric_limits<uint64_t>::max());
    }

    LowLatencyFrameIdGenerator::~LowLatencyFrameIdGenerator() = default;

    uint64_t LowLatencyFrameIdGenerator::GetLowLatencyDeviceFrameId(uint64_t applicationFrameId) {
        std::scoped_lock lock(m_frameIdGeneratorMutex);

        auto it = m_applicationIdToDeviceId.find(applicationFrameId);
        if (it != m_applicationIdToDeviceId.end())
            return it->second;

        uint64_t lowLatencyDeviceFrameId = m_nextLowLatencyDeviceFrameId++;
        uint64_t frameIdIndex = (lowLatencyDeviceFrameId - 1) % applicationIdListSize;

        if ((lowLatencyDeviceFrameId - 1) >= applicationIdListSize)
            m_applicationIdToDeviceId.erase(m_applicationIdList[frameIdIndex]);

        m_applicationIdToDeviceId[applicationFrameId] = lowLatencyDeviceFrameId;
        m_applicationIdList[frameIdIndex] = applicationFrameId;

        return lowLatencyDeviceFrameId;
    }

    bool LowLatencyFrameIdGenerator::LowLatencyDeviceFrameIdInWindow(uint64_t lowLatencyDeviceFrameId) const {
        return ((lowLatencyDeviceFrameId < m_nextLowLatencyDeviceFrameId)
            && ((m_nextLowLatencyDeviceFrameId - lowLatencyDeviceFrameId) < applicationIdListSize));
    }

    uint64_t LowLatencyFrameIdGenerator::GetApplicationFrameId(uint64_t lowLatencyDeviceFrameId) {
        std::scoped_lock lock(m_frameIdGeneratorMutex);

        if (!lowLatencyDeviceFrameId || !LowLatencyDeviceFrameIdInWindow(lowLatencyDeviceFrameId))
            return 0;

        return m_applicationIdList[((lowLatencyDeviceFrameId - 1) % applicationIdListSize)];
    }

    bool LowLatencyFrameIdGenerator::IsRepeatedFrame(uint64_t frameID, uint32_t markerType) {
        // Should always be within bounds since we drop unsupported marker types in the entrypoints
        return std::exchange(m_lastFrameId[markerType], frameID) == frameID;
    }

    bool NvapiD3dLowLatencyDevice::SupportsLowLatency(IUnknown* device) {
        auto d3dLowLatencyDevice = GetLowLatencyDevice(device);
        if (d3dLowLatencyDevice == nullptr)
            return false;

        return d3dLowLatencyDevice->SupportsLowLatency();
    }

    bool NvapiD3dLowLatencyDevice::SupportsLowLatency(ID3D12CommandQueue* commandQueue) {
        auto d3dLowLatencyDevice = GetLowLatencyDevice(commandQueue);
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

        auto success = SUCCEEDED(d3dLowLatencyDevice->SetLatencySleepMode(lowLatencyMode, lowLatencyBoost, minimumIntervalUs));
        if (success) {
            std::scoped_lock lock(m_lowLatencyModeMutex);
            m_lowLatencyModeMap[device] = lowLatencyMode;
        }

        return success;
    }

    bool NvapiD3dLowLatencyDevice::GetLatencyInfo(IUnknown* device, D3D_LATENCY_RESULTS* latencyResults) {
        auto d3dLowLatencyDevice = GetLowLatencyDevice(device);
        if (d3dLowLatencyDevice == nullptr)
            return false;

        if (FAILED(d3dLowLatencyDevice->GetLatencyInfo(latencyResults)))
            return false;

        auto frameIdGenerator = GetFrameIdGenerator(d3dLowLatencyDevice.ptr());
        for (auto& frameReport : latencyResults->frame_reports) {
            frameReport.frameID = frameIdGenerator->GetApplicationFrameId(frameReport.frameID);
            if (!frameReport.frameID) {
                memset(latencyResults->frame_reports, 0, sizeof(latencyResults->frame_reports));
                break;
            }
        }

        return true;
    }

    bool NvapiD3dLowLatencyDevice::SetLatencyMarker(IUnknown* device, uint64_t frameID, uint32_t markerType) {
        auto d3dLowLatencyDevice = GetLowLatencyDevice(device);
        if (d3dLowLatencyDevice == nullptr)
            return false;

        auto frameIdGenerator = GetFrameIdGenerator(d3dLowLatencyDevice.ptr());
        if (frameIdGenerator->IsRepeatedFrame(frameID, markerType))
            return true; // Silently drop repeated frame IDs

        return SUCCEEDED(d3dLowLatencyDevice->SetLatencyMarker(
            frameIdGenerator->GetLowLatencyDeviceFrameId(frameID), markerType));
    }

    bool NvapiD3dLowLatencyDevice::SetLatencyMarker(ID3D12CommandQueue* commandQueue, uint64_t frameID, uint32_t markerType) {
        auto d3dLowLatencyDevice = GetLowLatencyDevice(commandQueue);
        if (d3dLowLatencyDevice == nullptr)
            return false;

        auto frameIdGenerator = GetFrameIdGenerator(d3dLowLatencyDevice.ptr());
        if (frameIdGenerator->IsRepeatedFrame(frameID, markerType))
            return true; // Silently drop repeated frame IDs

        return SUCCEEDED(d3dLowLatencyDevice->SetLatencyMarker(
            frameIdGenerator->GetLowLatencyDeviceFrameId(frameID), markerType));
    }

    bool NvapiD3dLowLatencyDevice::GetLowLatencyMode(IUnknown* unknown) {
        std::scoped_lock lock(m_lowLatencyModeMutex);
        auto it = m_lowLatencyModeMap.find(unknown);
        if (it != m_lowLatencyModeMap.end())
            return it->second;

        return false;
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

    void NvapiD3dLowLatencyDevice::Reset() {
        std::scoped_lock lock(m_lowLatencyDeviceMutex, m_lowLatencyFrameIdGeneratorMutex);

        m_lowLatencyDeviceMap.clear();
        m_frameIdGeneratorMap.clear();
    }

    Com<ID3DLowLatencyDevice> NvapiD3dLowLatencyDevice::GetLowLatencyDevice(IUnknown* device) {
        if (!device)
            return nullptr;

        std::scoped_lock lock(m_lowLatencyDeviceMutex);
        auto it = m_lowLatencyDeviceMap.find(device);
        if (it != m_lowLatencyDeviceMap.end())
            return it->second;

        Com<ID3DLowLatencyDevice> d3dLowLatencyDevice;
        if (FAILED(device->QueryInterface(IID_PPV_ARGS(&d3dLowLatencyDevice))))
            return nullptr;

        m_lowLatencyDeviceMap.emplace(device, d3dLowLatencyDevice.ptr());

        return d3dLowLatencyDevice;
    }

    Com<ID3DLowLatencyDevice> NvapiD3dLowLatencyDevice::GetLowLatencyDevice(ID3D12CommandQueue* commandQueue) {
        if (!commandQueue)
            return nullptr;

        auto unknown = static_cast<IUnknown*>(commandQueue);

        std::scoped_lock lock(m_lowLatencyDeviceMutex);
        auto it = m_lowLatencyDeviceMap.find(unknown);
        if (it != m_lowLatencyDeviceMap.end())
            return it->second;

        Com<ID3DLowLatencyDevice> d3dLowLatencyDevice;

        // some games like The Cycle: Frontier (868270) pass ID3D11DeviceContext to NvAPI_D3D12_SetAsyncFrameMarker,
        // so let's not trust the caller and handle this somewhat gracefully instead of just crashing
        if (Com<ID3D11DeviceChild> d3d11DeviceChild; SUCCEEDED(commandQueue->QueryInterface(IID_PPV_ARGS(&d3d11DeviceChild)))) {
            Com<ID3D11Device> d3d11Device;
            d3d11DeviceChild->GetDevice(&d3d11Device);
            if (FAILED(d3d11Device->QueryInterface(IID_PPV_ARGS(&d3dLowLatencyDevice))))
                return nullptr;
        } else if (FAILED(commandQueue->GetDevice(IID_PPV_ARGS(&d3dLowLatencyDevice))))
            return nullptr;

        m_lowLatencyDeviceMap.emplace(unknown, d3dLowLatencyDevice.ptr());

        return d3dLowLatencyDevice;
    }

    LowLatencyFrameIdGenerator* NvapiD3dLowLatencyDevice::GetFrameIdGenerator(ID3DLowLatencyDevice* device) {
        std::scoped_lock lock(m_lowLatencyFrameIdGeneratorMutex);
        auto it = m_frameIdGeneratorMap.find(device);
        if (it != m_frameIdGeneratorMap.end())
            return it->second.get();

        return m_frameIdGeneratorMap.emplace(device, std::make_unique<LowLatencyFrameIdGenerator>()).first->second.get();
    }
}
