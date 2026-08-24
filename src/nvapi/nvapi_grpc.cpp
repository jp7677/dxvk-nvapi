#ifdef DXVK_NVAPI_GRPC

#include "nvapi_grpc.h"
#include "../util/util_env.h"
#include "../util/util_log.h"
#include "../util/util_string.h"
#include <winsock2.h>
#include <afunix.h>

namespace dxvk {
    static std::unordered_map<std::string_view, NvU32> parseports(const std::string& str) {
        std::unordered_map<std::string_view, NvU32> result;
        if (str.empty())
            return result;

        auto entries = str::split<std::vector<std::string_view>>(str, std::regex(","));

        for (auto entry : entries) {
            auto eq = entry.find('=');

            if (eq == entry.npos || eq == 0 || eq == entry.size() - 1)
                continue;

            NvU32 value;
            if (str::parsedword(entry.substr(eq + 1), value))
                result[std::string_view(entry.begin(), entry.begin() + eq)] = value;
        }

        return result;
    }

    const std::unordered_map<std::string_view, NvU32>& GrpcExecutableToPortMap() {
        static const auto nvapiGrpcPortsEnvName = "DXVK_NVAPI_GRPC_PORTS";
        static const auto nvapiGrpcPortsString = env::getEnvVariable(nvapiGrpcPortsEnvName);
        static const auto nvapiGrpcPorts = parseports(nvapiGrpcPortsString);

        return nvapiGrpcPorts;
    }

    const std::string GrpcSocketPath() {
        std::string path(UNIX_PATH_MAX, 0);

        auto pos = path.data();
        auto len1 = GetTempPathA(UNIX_PATH_MAX, pos);

        if (len1 == 0) {
            log::info(str::format("GetTempPathA failed with error: ", GetLastError()));
            return {};
        }

        if (len1 >= UNIX_PATH_MAX) {
            log::info(str::format("GetTempPathA output truncated: ", len1, " >= ", UNIX_PATH_MAX));
            return {};
        }

        if (pos[len1 - 1] != '\\')
            pos[++len1] = '\\';

        auto remaining = UNIX_PATH_MAX - len1;
        auto len2 = std::snprintf(pos + len1, remaining, "dxvk-nvapi-%lu.sock", GetCurrentProcessId());

        if (len2 < 0) {
            log::info(str::format("snprintf failed with error: ", GetLastError()));
            return {};
        }

        if ((DWORD)len2 >= remaining) {
            log::info(str::format("snprintf output truncated: ", len2, " >= ", remaining));
            return {};
        }

        path.resize(len1 + len2);
        return path;
    }

    grpc::Status NvapiService::SetDwordSetting(grpc::ServerContext* context, const SetDwordSettingRequest* request, NvapiReply* response) {
        static auto [map, mutex] = DrsDwords();

        auto settingId = request->id();
        auto settingValue = request->value();

        std::scoped_lock lock(*mutex);

        map->insert_or_assign(settingId, settingValue);
        response->set_status(NvapiStatus::SUCCESS);

        log::info(std::format("SetDwordSetting: set {} to {}", settingId, settingValue));

        return grpc::Status::OK;
    }
}

#endif
