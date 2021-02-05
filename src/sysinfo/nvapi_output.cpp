#include "nvapi_output.h"

namespace dxvk {
    NvapiOutput::NvapiOutput(const uintptr_t parent) {
        m_parent = parent;
    }

    NvapiOutput::~NvapiOutput() = default;

    void NvapiOutput::Initialize(const Com<IDXGIOutput> dxgiOutput) {
        DXGI_OUTPUT_DESC desc;
        dxgiOutput->GetDesc(&desc);

        m_deviceName = str::fromws(desc.DeviceName);
        std::cerr << str::format("NvAPI Output:", " ", m_deviceName) << std::endl;

        MONITORINFO info;
        info.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(desc.Monitor, &info);

        m_isPrimary = (info.dwFlags & MONITORINFOF_PRIMARY);
    }

    uintptr_t NvapiOutput::GetParent() {
        return m_parent;
    }

    std::string NvapiOutput::GetDeviceName() {
        return m_deviceName;
    }

    bool NvapiOutput::IsPrimary() {
        return m_isPrimary;
    }
}
