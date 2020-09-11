#include "nvapi_output.h"

namespace dxvk {
    NvapiOutput::NvapiOutput(uintptr_t parent) {
        m_parent = parent;
    }

    NvapiOutput::~NvapiOutput() { }

    void NvapiOutput::Initialize(Com<IDXGIOutput> dxgiOutput) {
        DXGI_OUTPUT_DESC desc;
        dxgiOutput->GetDesc(&desc);

        m_deviceName = FromWideString(desc.DeviceName);
        std::cerr << "NvAPI Output: " << m_deviceName << std::endl;
    }

    uintptr_t NvapiOutput::GetParent() {
        return m_parent;
    }

    std::string NvapiOutput::GetDeviceName() {
        return m_deviceName;
    }

    std::string NvapiOutput::FromWideString(const WCHAR *ws) {
        size_t len = ::WideCharToMultiByte(CP_UTF8, 0, ws, -1, nullptr, 0, nullptr, nullptr);

        if (len <= 1)
            return "";

        len -= 1;

        std::string result;
        result.resize(len);
        ::WideCharToMultiByte(CP_UTF8, 0, ws, -1, &result.at(0), len, nullptr, nullptr);
        return result;
    }
}
