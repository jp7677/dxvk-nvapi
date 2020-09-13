#include "nvapi_output.h"

namespace dxvk {
    NvapiOutput::NvapiOutput(uintptr_t parent) {
        m_parent = parent;
    }

    NvapiOutput::~NvapiOutput() { }

    void NvapiOutput::Initialize(Com<IDXGIOutput> dxgiOutput) {
        DXGI_OUTPUT_DESC desc;
        dxgiOutput->GetDesc(&desc);

        m_deviceName = str::fromws(desc.DeviceName);
        std::cerr << str::format("NvAPI Output: ", m_deviceName) << std::endl;
    }

    uintptr_t NvapiOutput::GetParent() {
        return m_parent;
    }

    std::string NvapiOutput::GetDeviceName() {
        return m_deviceName;
    }
}
