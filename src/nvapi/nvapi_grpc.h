#pragma once
#ifdef DXVK_NVAPI_GRPC

#include "../nvapi_private.h"
#include "nvapi.pb.h"
#include "nvapi.grpc.pb.h"

namespace dxvk {
    const std::string GrpcSocketPath();

    class NvapiService final : public DxvkNvapi::Service {
        grpc::Status SetDwordSetting(grpc::ServerContext* context, const SetDwordSettingRequest* request, NvapiReply* response) override;
    };
}

#endif