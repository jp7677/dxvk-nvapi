#include "nvapi_output.h"
#include "../util/util_string.h"
#include "../util/util_log.h"

namespace dxvk {
    NvapiOutput::NvapiOutput(const uintptr_t parent, const uint32_t adapterIndex, const uint32_t outputIndex) {
        m_parent = parent;
        m_id = ((adapterIndex + 1) << 16) + (outputIndex + 1);
    }

    NvapiOutput::~NvapiOutput() = default;

    void NvapiOutput::Initialize(Com<IDXGIOutput>& dxgiOutput) {
        DXGI_OUTPUT_DESC desc;
        dxgiOutput->GetDesc(&desc);

        m_deviceName = str::fromws(desc.DeviceName);
        log::write(str::format("NvAPI Output: ", m_deviceName));

        MONITORINFO info;
        info.cbSize = sizeof(MONITORINFO);
        ::GetMonitorInfo(desc.Monitor, &info);

        m_isPrimary = (info.dwFlags & MONITORINFOF_PRIMARY);
    }

    uintptr_t NvapiOutput::GetParent() const {
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
}
