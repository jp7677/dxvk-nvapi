#include "nvapi_private.h"
#include "util/util_statuscode.h"
#include "util/util_op_code.h"
#include "util/util_string.h"

extern "C" {
    using namespace dxvk;

    NvAPI_Status __cdecl NvAPI_D3D12_IsNvShaderExtnOpCodeSupported(ID3D12Device* pDevice, NvU32 opCode, bool* pSupported) {
        constexpr auto n = "NvAPI_D3D12_IsNvShaderExtnOpCodeSupported";

        if (pDevice == nullptr || pSupported == nullptr)
                return InvalidArgument(n);

        // VKD3D does not know any NVIDIA intrinsics
        *pSupported = false;

        return Ok(str::format(n, " ", opCode, " (", fromCode(opCode), ")"));
    }
}
