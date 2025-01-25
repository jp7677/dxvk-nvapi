#include "nvapi_output.h"
#include "../util/util_string.h"
#include "../util/util_log.h"

namespace dxvk {
    NvapiOutput::NvapiOutput(NvapiAdapter* parent, const uint32_t adapterIndex, const uint32_t outputIndex) {
        m_parent = parent;
        m_id = ((adapterIndex + 1) << 16) + (outputIndex + 1);
    }

    NvapiOutput::~NvapiOutput() = default;

    void NvapiOutput::Initialize(Com<IDXGIOutput>& dxgiOutput) {
        DXGI_OUTPUT_DESC desc{};
        dxgiOutput->GetDesc(&desc);

        m_deviceName = str::fromws(desc.DeviceName);
        log::info(str::format("NvAPI Output: ", m_deviceName));

        MONITORINFO info{};
        info.cbSize = sizeof(MONITORINFO);
        ::GetMonitorInfo(desc.Monitor, &info);

        m_isPrimary = info.dwFlags & MONITORINFOF_PRIMARY;

        Com<IDXGIOutput6> dxgiOutput6;
        if (SUCCEEDED(dxgiOutput->QueryInterface(IID_PPV_ARGS(&dxgiOutput6)))) {
            DXGI_OUTPUT_DESC1 desc1{};
            dxgiOutput6->GetDesc1(&desc1);
            constexpr auto m = 50000.0f;
            m_colorData.HasST2084Support = desc1.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
            switch (desc1.BitsPerColor) {
                case 6:
                    m_colorData.BitsPerColor = NV_BPC_6;
                    break;
                case 8:
                    m_colorData.BitsPerColor = NV_BPC_8;
                    break;
                default:
                case 10:
                    m_colorData.BitsPerColor = NV_BPC_10;
                    break;
                case 12:
                    m_colorData.BitsPerColor = NV_BPC_12;
                    break;
                case 16:
                    m_colorData.BitsPerColor = NV_BPC_16;
                    break;
            }
            m_colorData.RedPrimaryX = static_cast<uint16_t>(desc1.RedPrimary[0] * m);
            m_colorData.RedPrimaryY = static_cast<uint16_t>(desc1.RedPrimary[1] * m);
            m_colorData.GreenPrimaryX = static_cast<uint16_t>(desc1.GreenPrimary[0] * m);
            m_colorData.GreenPrimaryY = static_cast<uint16_t>(desc1.GreenPrimary[1] * m);
            m_colorData.BluePrimaryX = static_cast<uint16_t>(desc1.BluePrimary[0] * m);
            m_colorData.BluePrimaryY = static_cast<uint16_t>(desc1.BluePrimary[1] * m);
            m_colorData.WhitePointX = static_cast<uint16_t>(desc1.WhitePoint[0] * m);
            m_colorData.WhitePointY = static_cast<uint16_t>(desc1.WhitePoint[1] * m);
            m_colorData.MinLuminance = static_cast<uint16_t>(desc1.MinLuminance / 0.0001f);
            m_colorData.MaxLuminance = static_cast<uint16_t>(desc1.MaxLuminance);
            m_colorData.MaxFullFrameLuminance = static_cast<uint16_t>(desc1.MaxFullFrameLuminance);
        }
    }

    NvapiAdapter* NvapiOutput::GetParent() const {
        return m_parent;
    }

    uint32_t NvapiOutput::GetId() const {
        return m_id;
    }

    std::string NvapiOutput::GetDeviceName() const {
        return m_deviceName;
    }

    bool NvapiOutput::IsPrimary() const {
        return m_isPrimary;
    }

    const NvapiOutput::ColorData& NvapiOutput::GetColorData() const {
        return m_colorData;
    }
}
