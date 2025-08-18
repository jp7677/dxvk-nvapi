#include <algorithm>
#include <cinttypes>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <mutex>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#define VK_NO_PROTOTYPES

#include <vulkan/vulkan_core.h>

// hijack VKAPI_ATTR before vk_layer.h is included to make sure that vkNegotiateLoaderLayerInterfaceVersion
// has appropriate visibility attribute in its initial declaration / prototype, otherwise MSVC will complain

#undef VKAPI_ATTR

#if defined(_WIN32)
#define VKAPI_ATTR __declspec(dllexport)
#elif defined(__GNUC__)
#define VKAPI_ATTR [[gnu::visibility("default")]]
#else
#error No known visibility attribute
#endif

#include <vulkan/vk_layer.h>

#undef VKAPI_ATTR
// hopefully nobody is going to build this for 32-bit ARM Android
#define VKAPI_ATTR

#include "vkroots.h"

#define LOG_CHANNEL "vkreflex_layer"
#include "log.h"
#include "config.h"
#include "version.h"

using namespace std::literals;

static std::optional<bool> GetFlag(const char* name) {
    auto value = std::getenv(name);

    if (!value)
        return std::nullopt;

    switch (*value) {
        case '0':
            return false;
        case '1':
            return true;
        default:
            return std::nullopt;
    }
}

static bool injectSubmitFrameIDs = false;
static bool injectPresentFrameIDs = false;
static bool injectFrameIDs = injectSubmitFrameIDs || injectPresentFrameIDs;
static bool allowFallbackToOutOfBandFrameID = true;
static bool allowFallbackToPresentFrameID = true;
static bool allowFallbackToSimulationFrameID = true;

static void Init() {
    ::InitLogger("DXVK_NVAPI_VKREFLEX_LAYER_LOG_LEVEL");

    INFO("dxvk-nvapi-vkreflex-layer %s (%s %s, %s, %s)",
        DXVK_NVAPI_VERSION,
        DXVK_NVAPI_BUILD_COMPILER,
        DXVK_NVAPI_BUILD_COMPILER_VERSION,
        DXVK_NVAPI_BUILD_TARGET,
        DXVK_NVAPI_BUILD_TYPE);

#define READ_FLAG(var, env) var = ::GetFlag("DXVK_NVAPI_VKREFLEX_" env).value_or(var)
    READ_FLAG(injectSubmitFrameIDs, "INJECT_SUBMIT_FRAME_IDS");
    READ_FLAG(injectPresentFrameIDs, "INJECT_PRESENT_FRAME_IDS");
    injectFrameIDs = injectSubmitFrameIDs || injectPresentFrameIDs;
    READ_FLAG(allowFallbackToOutOfBandFrameID, "ALLOW_FALLBACK_TO_OOB_FRAME_ID");
    READ_FLAG(allowFallbackToPresentFrameID, "ALLOW_FALLBACK_TO_PRESENT_FRAME_ID");
    READ_FLAG(allowFallbackToSimulationFrameID, "ALLOW_FALLBACK_TO_SIMULATION_FRAME_ID");
#undef READ_FLAG

#define LOG_FLAG(var) INFO("%s = %s", #var, var ? "1" : "0")
    LOG_FLAG(injectSubmitFrameIDs);
    LOG_FLAG(injectPresentFrameIDs);
    LOG_FLAG(allowFallbackToOutOfBandFrameID);
    LOG_FLAG(allowFallbackToPresentFrameID);
    LOG_FLAG(allowFallbackToSimulationFrameID);
#undef LOG_FLAG
}

struct ReflexInstanceContextData {
    const std::string applicationName;
    const std::string engineName;
    uint32_t apiVersion;
};

struct ReflexMarker {
    uint64_t id;
    bool ongoing;
};

struct ReflexDeviceContextData {
    VkSwapchainKHR swapchain;
    VkLatencySleepModeInfoNV latencySleepModeInfo;
    struct
    {
        ReflexMarker simulation;
        ReflexMarker renderSubmit;
        ReflexMarker present;
        ReflexMarker outOfBandRenderSubmit;
        ReflexMarker outOfBandPresent;
    } markers;
};

struct ReflexQueueContextData {
    bool outOfBandRenderSubmit;
    bool outOfBandPresent;
};

static uint64_t GetFrameId(const ReflexDeviceContextData& deviceContext, bool present, bool outOfBand) {
#define TRY_MARKER(marker)       \
    do {                         \
        if ((marker)->ongoing)   \
            return (marker)->id; \
    } while (0)
    if (!present) {
        if (!outOfBand)
            TRY_MARKER(&deviceContext.markers.renderSubmit);

        if (outOfBand || allowFallbackToOutOfBandFrameID)
            TRY_MARKER(&deviceContext.markers.outOfBandRenderSubmit);

        if (allowFallbackToSimulationFrameID)
            TRY_MARKER(&deviceContext.markers.simulation);
    }
    if (present || allowFallbackToPresentFrameID) {
        if (!outOfBand)
            TRY_MARKER(&deviceContext.markers.present);

        if (outOfBand || allowFallbackToOutOfBandFrameID)
            TRY_MARKER(&deviceContext.markers.outOfBandPresent);
    }
#undef TRY_MARKER

    return 0;
}

using PFN_VkDeviceDispatchWaitSemaphores = decltype(&vkroots::VkDeviceDispatch::WaitSemaphores);

static VkResult WaitSemaphores(
    PFN_VkDeviceDispatchWaitSemaphores waitSemaphores,
    const vkroots::VkDeviceDispatch& dispatch,
    VkDevice device,
    const VkSemaphoreWaitInfo* pWaitInfo,
    uint64_t timeout) {
    if (logLevel < LogLevel_Debug)
        return std::invoke(waitSemaphores, dispatch, device, pWaitInfo, timeout);

    if (pWaitInfo) {
        switch (pWaitInfo->semaphoreCount) {
            case 0:
                TRACE("(%p, %p { %" PRIu32 ", %" PRIu32 ", [], [] }, %" PRIu64 ")",
                    device, pWaitInfo, pWaitInfo->flags, pWaitInfo->semaphoreCount, timeout);
                break;
            case 1:
                TRACE("(%p, %p { %" PRIu32 ", %" PRIu32 ", [%p], [%" PRIu64 "] }, %" PRIu64 ")",
                    device, pWaitInfo, pWaitInfo->flags, pWaitInfo->semaphoreCount, pWaitInfo->pSemaphores[0], pWaitInfo->pValues[0], timeout);
                break;
            default:
                TRACE("(%p, %p { %" PRIu32 ", %" PRIu32 ", [%p, ...], [%" PRIu64 ", ...] }, %" PRIu64 ")",
                    device, pWaitInfo, pWaitInfo->flags, pWaitInfo->semaphoreCount, pWaitInfo->pSemaphores[0], pWaitInfo->pValues[0], timeout);
                break;
        }
    } else {
        TRACE("(%p, %p, %" PRIu64 ")", device, pWaitInfo, timeout);
    }

    auto begin = ::GetTimestamp();

    auto result = std::invoke(waitSemaphores, dispatch, device, pWaitInfo, timeout);

    auto end = ::GetTimestamp();

    auto diff = (end.seconds - begin.seconds) * 1000 + (end.milliseconds - begin.milliseconds);

    DBG("waited for %" PRIi32 " ms", diff);

    return result;
}

using PFN_VkQueueDispatchQueueSubmit2 = decltype(&vkroots::VkQueueDispatch::QueueSubmit2);

static VkResult QueueSubmit2(
    PFN_VkQueueDispatchQueueSubmit2 queueSubmit2,
    const vkroots::VkQueueDispatch& dispatch,
    VkQueue queue,
    uint32_t submitCount,
    const VkSubmitInfo2* pSubmits,
    VkFence fence) {
    if (!injectSubmitFrameIDs)
        return std::invoke(queueSubmit2, dispatch, queue, submitCount, pSubmits, fence);

    if (!dispatch.pDeviceDispatch->UserData)
        return std::invoke(queueSubmit2, dispatch, queue, submitCount, pSubmits, fence);

    auto& deviceContext = dispatch.pDeviceDispatch->UserData.cast<ReflexDeviceContextData>();
    auto outOfBandRenderSubmit = dispatch.UserData ? dispatch.UserData.cast<ReflexQueueContextData>().outOfBandRenderSubmit : false;

    if (deviceContext.latencySleepModeInfo.lowLatencyMode && pSubmits && submitCount) {
        uint64_t id = ::GetFrameId(deviceContext, false, outOfBandRenderSubmit);

        TRACE("(%p, %" PRIu32 ", %p, %p) frameID = %" PRIu64 ", oob = %d",
            queue, submitCount, pSubmits, fence, id, outOfBandRenderSubmit);

        if (id) {
            auto submitInfos = std::vector<VkSubmitInfo2>{
                pSubmits,
                pSubmits + submitCount,
            };

            auto latencySubmissionPresentIds = std::vector<VkLatencySubmissionPresentIdNV>{submitCount};

            for (auto i = 0u; i < submitCount; ++i) {
                auto info = &latencySubmissionPresentIds[i];

                info->sType = VK_STRUCTURE_TYPE_LATENCY_SUBMISSION_PRESENT_ID_NV;
                info->pNext = std::exchange(submitInfos[i].pNext, info);
                info->presentID = id;
            }

            return std::invoke(queueSubmit2, dispatch, queue, submitCount, submitInfos.data(), fence);
        }
    }

    return std::invoke(queueSubmit2, dispatch, queue, submitCount, pSubmits, fence);
}

static constexpr auto gpdp2 = std::string_view{VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};
static constexpr auto ll = std::string_view{VK_NV_LOW_LATENCY_EXTENSION_NAME};
static constexpr auto ll2 = std::string_view{VK_NV_LOW_LATENCY_2_EXTENSION_NAME};
static constexpr auto ts = std::string_view{VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME};
static constexpr auto pid = std::string_view{VK_KHR_PRESENT_ID_EXTENSION_NAME};

// Similar to vkroots::AddToChain but with const_cast
template <typename Type, typename AnyStruct>
static inline void AddToChain(AnyStruct* pParent, Type* pType) {
    pType->pNext = const_cast<void*>(std::exchange(pParent->pNext, pType));
}

struct VkInstanceOverrides {
    static VkResult CreateInstance(
        PFN_vkCreateInstance pfnCreateInstanceProc,
        const VkInstanceCreateInfo* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkInstance* pInstance) {
        static std::once_flag initialized{};
        std::call_once(initialized, ::Init);

        if (!pCreateInfo)
            return pfnCreateInstanceProc(pCreateInfo, pAllocator, pInstance);

        auto info = *pCreateInfo;
        auto applicationName = info.pApplicationInfo && info.pApplicationInfo->pApplicationName ? std::string(info.pApplicationInfo->pApplicationName) : std::string("Unknown");
        auto engineName = info.pApplicationInfo && info.pApplicationInfo->pEngineName ? std::string(info.pApplicationInfo->pEngineName) : std::string("Unknown");
        auto apiVersion = info.pApplicationInfo ? info.pApplicationInfo->apiVersion : VK_API_VERSION_1_0;
        std::vector<const char*> extensions;

        if (apiVersion < VK_API_VERSION_1_1) {
            auto end = info.ppEnabledExtensionNames + info.enabledExtensionCount;

            if (!info.ppEnabledExtensionNames || !info.enabledExtensionCount || std::find(info.ppEnabledExtensionNames, end, gpdp2) == end) {
                extensions.reserve(info.enabledExtensionCount + 1);

                if (info.ppEnabledExtensionNames && info.enabledExtensionCount)
                    extensions.assign(info.ppEnabledExtensionNames, end);

                extensions.push_back(gpdp2.data());

                info.ppEnabledExtensionNames = extensions.data();
                info.enabledExtensionCount = extensions.size();
            }
        }

        auto vr = pfnCreateInstanceProc(&info, pAllocator, pInstance);

        if (vr == VK_SUCCESS)
            vkroots::LookupDispatch(*pInstance)->UserData.emplace<ReflexInstanceContextData>(applicationName, engineName, apiVersion);

        return vr;
    }

    static VkResult CreateDevice(
        const vkroots::VkPhysicalDeviceDispatch& dispatch,
        VkPhysicalDevice physicalDevice,
        const VkDeviceCreateInfo* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDevice* pDevice) {
        if (!pCreateInfo)
            return dispatch.CreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);

        if (!dispatch.pInstanceDispatch->UserData.has())
            return dispatch.CreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);

        auto& context = dispatch.pInstanceDispatch->UserData.cast<ReflexInstanceContextData>();

        uint32_t count;
        auto vr = dispatch.EnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);

        if (vr != VK_SUCCESS)
            return dispatch.CreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);

        std::vector<VkExtensionProperties> properties{count};
        vr = dispatch.EnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, properties.data());

        if (vr != VK_SUCCESS)
            return dispatch.CreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);

        if (std::ranges::none_of(properties, [&](auto& prop) { return prop.extensionName == ll2 && prop.specVersion >= 2; })) {
            INFO("%s not supported by physical device, skipping setup of compatibility layer (%s/%s)", ll2.data(), context.applicationName.c_str(), context.engineName.c_str());
            return dispatch.CreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
        }

        auto info = *pCreateInfo;

        for (auto ext = info.ppEnabledExtensionNames; ext && ext < info.ppEnabledExtensionNames + info.enabledExtensionCount; ++ext) {
            if (*ext == ll2) {
                INFO("%s already requested by the application, skipping setup of compatibility layer (%s/%s)", ll2.data(), context.applicationName.c_str(), context.engineName.c_str());
                return dispatch.CreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
            }
        }

        auto extensions = info.ppEnabledExtensionNames && info.enabledExtensionCount
            ? std::vector<const char*>{
                  info.ppEnabledExtensionNames,
                  info.ppEnabledExtensionNames + info.enabledExtensionCount}
            : injectFrameIDs ? std::vector<const char*>{{ts.data(), ll2.data(), pid.data()}}
                             : std::vector<const char*>{{ts.data(), ll2.data()}};

        auto presentIdFeatures = VkPhysicalDevicePresentIdFeaturesKHR{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR,
            .presentId = VK_TRUE,
        };

        auto timelineSemaphoreFeatures = VkPhysicalDeviceTimelineSemaphoreFeatures{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES,
            .timelineSemaphore = VK_TRUE,
        };

        bool hasLL2 = false;

        for (auto& ext : extensions) {
            static constexpr auto llSize = ll.size();

            if (!strncmp(ext, ll.data(), llSize)) {
                auto c = ext[llSize];

                if (!c)
                    ext = ll2.data();
                else if (c != '2' || ext[llSize + 1])
                    continue;

                hasLL2 = true;
            }
        }

        if (!hasLL2)
            extensions.push_back(ll2.data());

        if (context.apiVersion < VK_API_VERSION_1_2) {
            if (std::ranges::find(extensions, ts) == extensions.end())
                extensions.push_back(ts.data());
        }

        auto pNext = const_cast<void*>(info.pNext);

        if (injectFrameIDs) {
            if (std::ranges::find(extensions, pid) == extensions.end())
                extensions.push_back(pid.data());

            if (auto f = vkroots::FindInChainMutable<VkPhysicalDevicePresentIdFeaturesKHR>(pNext))
                f->presentId = VK_TRUE;
            else
                ::AddToChain(&info, &presentIdFeatures);
        }

        if (auto f = vkroots::FindInChainMutable<VkPhysicalDeviceVulkan12Features>(pNext))
            f->timelineSemaphore = VK_TRUE;
        else if (auto f = vkroots::FindInChainMutable<VkPhysicalDeviceTimelineSemaphoreFeatures>(pNext))
            f->timelineSemaphore = VK_TRUE;
        else
            ::AddToChain(&info, &timelineSemaphoreFeatures);

        info.ppEnabledExtensionNames = extensions.data();
        info.enabledExtensionCount = extensions.size();

        vr = dispatch.CreateDevice(physicalDevice, &info, pAllocator, pDevice);

        if (vr != VK_SUCCESS)
            return vr;

        vkroots::LookupDispatch(*pDevice)->UserData.emplace<ReflexDeviceContextData>();

        INFO("Setup of compatibility layer succeeded (%s/%s)", context.applicationName.c_str(), context.engineName.c_str());
        return VK_SUCCESS;
    }
};

struct VkDeviceOverrides {
    static VkResult CreateSwapchainKHR(
        const vkroots::VkDeviceDispatch& dispatch,
        VkDevice device,
        const VkSwapchainCreateInfoKHR* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkSwapchainKHR* pSwapchain) {
        if (!dispatch.UserData)
            return dispatch.CreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);

        auto& context = dispatch.UserData.cast<ReflexDeviceContextData>();
        auto info = *pCreateInfo;

        auto swapchainLatencyCreateInfo = VkSwapchainLatencyCreateInfoNV{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_LATENCY_CREATE_INFO_NV,
            .pNext = info.pNext,
            .latencyModeEnable = VK_TRUE,
        };

        info.pNext = &swapchainLatencyCreateInfo;

        auto vr = dispatch.CreateSwapchainKHR(device, &info, pAllocator, pSwapchain);

        if (vr == VK_SUCCESS) {
            if (context.latencySleepModeInfo.sType == VK_STRUCTURE_TYPE_LATENCY_SLEEP_MODE_INFO_NV) {
                vr = dispatch.SetLatencySleepModeNV(device, *pSwapchain, &context.latencySleepModeInfo);

                if (vr != VK_SUCCESS) {
                    dispatch.DestroySwapchainKHR(device, *pSwapchain, pAllocator);

                    return vr;
                }
            }

            context.swapchain = *pSwapchain;
        }

        return vr;
    }

    static void DestroySwapchainKHR(
        const vkroots::VkDeviceDispatch& dispatch,
        VkDevice device,
        VkSwapchainKHR swapchain,
        const VkAllocationCallbacks* pAllocator) {
        dispatch.DestroySwapchainKHR(device, swapchain, pAllocator);

        if (dispatch.UserData) {
            if (auto& context = dispatch.UserData.cast<ReflexDeviceContextData>(); context.swapchain == swapchain)
                context.swapchain = VK_NULL_HANDLE;
        }
    }

    static VkResult WaitSemaphores(
        const vkroots::VkDeviceDispatch& dispatch,
        VkDevice device,
        const VkSemaphoreWaitInfo* pWaitInfo,
        uint64_t timeout) {
        return ::WaitSemaphores(&vkroots::VkDeviceDispatch::WaitSemaphores, dispatch, device, pWaitInfo, timeout);
    }

    static VkResult WaitSemaphoresKHR(
        const vkroots::VkDeviceDispatch& dispatch,
        VkDevice device,
        const VkSemaphoreWaitInfoKHR* pWaitInfo,
        uint64_t timeout) {
        return ::WaitSemaphores(&vkroots::VkDeviceDispatch::WaitSemaphoresKHR, dispatch, device, pWaitInfo, timeout);
    }

    static VkResult QueueSubmit(
        const vkroots::VkQueueDispatch& dispatch,
        VkQueue queue,
        uint32_t submitCount,
        const VkSubmitInfo* pSubmits,
        VkFence fence) {
        if (!injectSubmitFrameIDs)
            return dispatch.QueueSubmit(queue, submitCount, pSubmits, fence);

        if (!dispatch.pDeviceDispatch->UserData)
            return dispatch.QueueSubmit(queue, submitCount, pSubmits, fence);

        auto& deviceContext = dispatch.pDeviceDispatch->UserData.cast<ReflexDeviceContextData>();
        auto outOfBandRenderSubmit = dispatch.UserData ? dispatch.UserData.cast<ReflexQueueContextData>().outOfBandRenderSubmit : false;

        if (deviceContext.latencySleepModeInfo.lowLatencyMode && pSubmits && submitCount) {
            uint64_t id = ::GetFrameId(deviceContext, false, outOfBandRenderSubmit);

            TRACE("(%p, %" PRIu32 ", %p, %p) frameID = %" PRIu64 ", oob = %d",
                queue, submitCount, pSubmits, fence, id, outOfBandRenderSubmit);

            if (id) {
                auto submitInfos = std::vector<VkSubmitInfo>{
                    pSubmits,
                    pSubmits + submitCount,
                };

                auto latencySubmissionPresentIds = std::vector<VkLatencySubmissionPresentIdNV>{submitCount};

                for (auto i = 0u; i < submitCount; ++i) {
                    auto info = &latencySubmissionPresentIds[i];

                    info->sType = VK_STRUCTURE_TYPE_LATENCY_SUBMISSION_PRESENT_ID_NV;
                    info->pNext = std::exchange(submitInfos[i].pNext, info);
                    info->presentID = id;
                }

                return dispatch.QueueSubmit(queue, submitCount, submitInfos.data(), fence);
            }
        }

        return dispatch.QueueSubmit(queue, submitCount, pSubmits, fence);
    }

    static VkResult QueueSubmit2(
        const vkroots::VkQueueDispatch& dispatch,
        VkQueue queue,
        uint32_t submitCount,
        const VkSubmitInfo2* pSubmits,
        VkFence fence) {
        return ::QueueSubmit2(&vkroots::VkQueueDispatch::QueueSubmit2, dispatch, queue, submitCount, pSubmits, fence);
    }

    static VkResult QueueSubmit2KHR(
        const vkroots::VkQueueDispatch& dispatch,
        VkQueue queue,
        uint32_t submitCount,
        const VkSubmitInfo2KHR* pSubmits,
        VkFence fence) {
        return ::QueueSubmit2(&vkroots::VkQueueDispatch::QueueSubmit2KHR, dispatch, queue, submitCount, pSubmits, fence);
    }

    static VkResult QueuePresentKHR(
        const vkroots::VkQueueDispatch& dispatch,
        VkQueue queue,
        const VkPresentInfoKHR* pPresentInfo) {
        if (!injectPresentFrameIDs)
            return dispatch.QueuePresentKHR(queue, pPresentInfo);

        if (!dispatch.pDeviceDispatch->UserData)
            return dispatch.QueuePresentKHR(queue, pPresentInfo);

        auto& deviceContext = dispatch.pDeviceDispatch->UserData.cast<ReflexDeviceContextData>();
        auto outOfBandPresent = dispatch.UserData ? dispatch.UserData.cast<ReflexQueueContextData>().outOfBandPresent : false;

        if (deviceContext.latencySleepModeInfo.lowLatencyMode && pPresentInfo && pPresentInfo->pSwapchains && pPresentInfo->swapchainCount) {
            uint32_t i;
            uint64_t id = ::GetFrameId(deviceContext, true, outOfBandPresent);

            TRACE("(%p, %p) frameID = %" PRIu64 ", oob = %d",
                queue, pPresentInfo, id, outOfBandPresent);

            if (!id)
                goto end;

            for (i = 0; i < pPresentInfo->swapchainCount; ++i) {
                if (pPresentInfo->pSwapchains[i] == deviceContext.swapchain)
                    break;
            }

            if (i == pPresentInfo->swapchainCount)
                goto end;

            if (auto pid = vkroots::FindInChain<VkPresentIdKHR>(pPresentInfo->pNext)) {
                if (pid->swapchainCount <= i)
                    WARN("found VkPresentIdKHR with unexpected swapchain count (%" PRIu32 " <= %" PRIu32 ")", pid->swapchainCount, i);
                else if (!pid->pPresentIds)
                    WARN("found VkPresentIdKHR with NULL pPresentIds");
                else if (pid->pPresentIds[i] != id)
                    WARN("found VkPresentIdKHR (%" PRIu64 ") that does not match Reflex frame ID (%" PRIu64 ")", pid->pPresentIds[i], id);
            } else {
                auto presentIds = std::vector<uint64_t>{pPresentInfo->swapchainCount};

                presentIds[i] = id;

                auto presentId = VkPresentIdKHR{
                    .sType = VK_STRUCTURE_TYPE_PRESENT_ID_KHR,
                    .pNext = pPresentInfo->pNext,
                    .swapchainCount = pPresentInfo->swapchainCount,
                    .pPresentIds = presentIds.data(),
                };

                auto info = *pPresentInfo;
                info.pNext = &presentId;

                return dispatch.QueuePresentKHR(queue, &info);
            }
        }

    end:
        return dispatch.QueuePresentKHR(queue, pPresentInfo);
    }

    static VkResult SetLatencySleepModeNV(
        const vkroots::VkDeviceDispatch& dispatch,
        VkDevice device,
        VkSwapchainKHR swapchain,
        const VkLatencySleepModeInfoNV* pSleepModeInfo) {
        if (!dispatch.UserData)
            return dispatch.SetLatencySleepModeNV(device, swapchain, pSleepModeInfo);

        auto& context = dispatch.UserData.cast<ReflexDeviceContextData>();

        if (pSleepModeInfo)
            TRACE("(%p, %p = %p, %p { %" PRIu32 ", %" PRIu32 ", %" PRIu32 " })",
                device, swapchain, context.swapchain, pSleepModeInfo, pSleepModeInfo->lowLatencyMode, pSleepModeInfo->lowLatencyBoost, pSleepModeInfo->minimumIntervalUs);
        else
            TRACE("(%p, %p = %p, %p)",
                device, swapchain, context.swapchain, pSleepModeInfo);

        swapchain = context.swapchain;

        auto vr = VK_SUCCESS;

        if (swapchain)
            vr = dispatch.SetLatencySleepModeNV(device, swapchain, pSleepModeInfo);

        if (vr == VK_SUCCESS) {
            if (pSleepModeInfo) {
                context.latencySleepModeInfo = *pSleepModeInfo;
                context.latencySleepModeInfo.pNext = nullptr;
            } else {
                context.latencySleepModeInfo = VkLatencySleepModeInfoNV{};
            }
        }

        return vr;
    }

    static VkResult LatencySleepNV(
        const vkroots::VkDeviceDispatch& dispatch,
        VkDevice device,
        VkSwapchainKHR swapchain,
        const VkLatencySleepInfoNV* pSleepInfo) {
        if (!pSleepInfo)
            return VK_ERROR_UNKNOWN;

        if (!dispatch.UserData)
            return dispatch.LatencySleepNV(device, swapchain, pSleepInfo);

        auto& context = dispatch.UserData.cast<ReflexDeviceContextData>();

        TRACE("(%p, %p = %p, %p { %p, %" PRIu64 " })",
            device, swapchain, context.swapchain, pSleepInfo, pSleepInfo->signalSemaphore, pSleepInfo->value);

        swapchain = context.swapchain;

        if (swapchain)
            return dispatch.LatencySleepNV(device, swapchain, pSleepInfo);

        auto semaphoreSignalInfo = VkSemaphoreSignalInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
            .semaphore = pSleepInfo->signalSemaphore,
            .value = pSleepInfo->value,
        };

        return dispatch.SignalSemaphore(device, &semaphoreSignalInfo);
    }

    static void SetLatencyMarkerNV(
        const vkroots::VkDeviceDispatch& dispatch,
        VkDevice device,
        VkSwapchainKHR swapchain,
        const VkSetLatencyMarkerInfoNV* pLatencyMarkerInfo) {
        if (!pLatencyMarkerInfo)
            return;

        if (!dispatch.UserData) {
            dispatch.SetLatencyMarkerNV(device, swapchain, pLatencyMarkerInfo);
            return;
        }

        auto& context = dispatch.UserData.cast<ReflexDeviceContextData>();

        TRACE("(%p, %p = %p, %p { %" PRIu64 ", %s })",
            device, swapchain, context.swapchain, pLatencyMarkerInfo, pLatencyMarkerInfo->presentID, vkroots::helpers::enumString(pLatencyMarkerInfo->marker));

        swapchain = context.swapchain;

        if (swapchain)
            dispatch.SetLatencyMarkerNV(device, swapchain, pLatencyMarkerInfo);

        if (!injectFrameIDs)
            return;

        switch (pLatencyMarkerInfo->marker) {
            case VK_LATENCY_MARKER_SIMULATION_START_NV:
                context.markers.simulation.id = pLatencyMarkerInfo->presentID;
                context.markers.simulation.ongoing = true;
                break;
            case VK_LATENCY_MARKER_SIMULATION_END_NV:
                context.markers.simulation.ongoing = false;
                break;
            case VK_LATENCY_MARKER_RENDERSUBMIT_START_NV:
                context.markers.renderSubmit.id = pLatencyMarkerInfo->presentID;
                context.markers.renderSubmit.ongoing = true;
                break;
            case VK_LATENCY_MARKER_RENDERSUBMIT_END_NV:
                context.markers.renderSubmit.ongoing = false;
                break;
            case VK_LATENCY_MARKER_PRESENT_START_NV:
                context.markers.present.id = pLatencyMarkerInfo->presentID;
                context.markers.present.ongoing = true;
                break;
            case VK_LATENCY_MARKER_PRESENT_END_NV:
                context.markers.present.ongoing = false;
                break;
            case VK_LATENCY_MARKER_INPUT_SAMPLE_NV:
                break;
            case VK_LATENCY_MARKER_TRIGGER_FLASH_NV:
                break;
            case VK_LATENCY_MARKER_OUT_OF_BAND_RENDERSUBMIT_START_NV:
                context.markers.outOfBandRenderSubmit.id = pLatencyMarkerInfo->presentID;
                context.markers.outOfBandRenderSubmit.ongoing = true;
                break;
            case VK_LATENCY_MARKER_OUT_OF_BAND_RENDERSUBMIT_END_NV:
                context.markers.outOfBandRenderSubmit.ongoing = false;
                break;
            case VK_LATENCY_MARKER_OUT_OF_BAND_PRESENT_START_NV:
                context.markers.outOfBandPresent.id = pLatencyMarkerInfo->presentID;
                context.markers.outOfBandPresent.ongoing = true;
                break;
            case VK_LATENCY_MARKER_OUT_OF_BAND_PRESENT_END_NV:
                context.markers.outOfBandPresent.ongoing = false;
                break;
            default:
                break;
        }
    }

    static void GetLatencyTimingsNV(
        const vkroots::VkDeviceDispatch& dispatch,
        VkDevice device,
        VkSwapchainKHR swapchain,
        VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo) {
        if (!pLatencyMarkerInfo)
            return;

        if (!dispatch.UserData) {
            dispatch.GetLatencyTimingsNV(device, swapchain, pLatencyMarkerInfo);
            return;
        }

        auto& context = dispatch.UserData.cast<ReflexDeviceContextData>();

        TRACE("(%p, %p = %p, %p { %" PRIu32 ", %p })",
            device, swapchain, context.swapchain, pLatencyMarkerInfo, pLatencyMarkerInfo->timingCount, pLatencyMarkerInfo->pTimings);

        swapchain = context.swapchain;

        if (swapchain)
            dispatch.GetLatencyTimingsNV(device, swapchain, pLatencyMarkerInfo);
        else
            pLatencyMarkerInfo->timingCount = 0;
    }

    static void QueueNotifyOutOfBandNV(
        const vkroots::VkQueueDispatch& dispatch,
        VkQueue queue,
        const VkOutOfBandQueueTypeInfoNV* pQueueTypeInfo) {
        if (!pQueueTypeInfo)
            return;

        TRACE("(%p, %p { %s })",
            queue, pQueueTypeInfo, vkroots::helpers::enumString(pQueueTypeInfo->queueType));

        dispatch.QueueNotifyOutOfBandNV(queue, pQueueTypeInfo);

        if (!injectFrameIDs)
            return;

        if (!dispatch.UserData)
            dispatch.UserData.emplace<ReflexQueueContextData>();

        auto& context = dispatch.UserData.cast<ReflexQueueContextData>();

        switch (pQueueTypeInfo->queueType) {
            case VK_OUT_OF_BAND_QUEUE_TYPE_RENDER_NV:
                context.outOfBandRenderSubmit = true;
                break;
            case VK_OUT_OF_BAND_QUEUE_TYPE_PRESENT_NV:
                context.outOfBandPresent = true;
                break;
            default:
                break;
        }
    }
};

VKROOTS_DEFINE_LAYER_INTERFACES(VkInstanceOverrides, VkDeviceOverrides);
