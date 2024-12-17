#include <algorithm>
#include <cinttypes>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "vkroots.h"

#define LOG_CHANNEL "vkreflex_layer"
#define LOG_LEVEL_ENV "DXVK_NVAPI_VKREFLEX_LAYER_LOG_LEVEL"
#include "log.h"

using namespace std::literals;

struct ReflexMarker {
    uint64_t id;
    bool ongoing;
};

struct ReflexDeviceContextData {
    VkSwapchainKHR swapchain;
    VkLatencySleepModeInfoNV latencySleepModeInfo;
    std::unordered_set<VkQueue> queues;
    struct
    {
        ReflexMarker simulation;
        ReflexMarker renderSubmit;
        ReflexMarker present;
        ReflexMarker outOfBandRenderSubmit;
        ReflexMarker outOfBandPresent;
    } markers;
};

static std::optional<bool> GetFlag(char const* name) {
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

static bool exposeExtension = false;
static bool injectSubmitFrameIDs = true;
static bool injectPresentFrameIDs = true;
static bool injectFrameIDs = injectSubmitFrameIDs || injectPresentFrameIDs;
static bool allowFallbackToOutOfBandFrameID = true;
static bool allowFallbackToPresentFrameID = true;
static bool allowFallbackToSimulationFrameID = true;

[[gnu::constructor]]
static void Init() {
    ::InitLogger();

#define READ_FLAG(var, env) var = ::GetFlag(env).value_or(var)
    READ_FLAG(exposeExtension, "DXVK_NVAPI_VKREFLEX_EXPOSE_EXTENSION");
    READ_FLAG(injectSubmitFrameIDs, "DXVK_NVAPI_VKREFLEX_INJECT_SUBMIT_FRAME_IDS");
    READ_FLAG(injectPresentFrameIDs, "DXVK_NVAPI_VKREFLEX_INJECT_PRESENT_FRAME_IDS");
    injectFrameIDs = injectSubmitFrameIDs || injectPresentFrameIDs;
    READ_FLAG(allowFallbackToOutOfBandFrameID, "DXVK_NVAPI_VKREFLEX_ALLOW_FALLBACK_TO_OOB_FRAME_ID");
    READ_FLAG(allowFallbackToPresentFrameID, "DXVK_NVAPI_VKREFLEX_ALLOW_FALLBACK_TO_PRESENT_FRAME_ID");
    READ_FLAG(allowFallbackToSimulationFrameID, "DXVK_NVAPI_VKREFLEX_ALLOW_FALLBACK_TO_SIMULATION_FRAME_ID");
#undef READ_FLAG
}

static uint64_t GetFrameId(ReflexDeviceContextData const* deviceContext, bool present, bool outOfBand) {
#define TRY_MARKER(marker)       \
    do {                         \
        if ((marker)->ongoing)   \
            return (marker)->id; \
    } while (0)
    if (!present) {
        if (!outOfBand)
            TRY_MARKER(&deviceContext->markers.renderSubmit);

        if (outOfBand || allowFallbackToOutOfBandFrameID)
            TRY_MARKER(&deviceContext->markers.outOfBandRenderSubmit);

        if (allowFallbackToSimulationFrameID)
            TRY_MARKER(&deviceContext->markers.simulation);
    }
    if (present || allowFallbackToPresentFrameID) {
        if (!outOfBand)
            TRY_MARKER(&deviceContext->markers.present);

        if (outOfBand || allowFallbackToOutOfBandFrameID)
            TRY_MARKER(&deviceContext->markers.outOfBandPresent);
    }
#undef TRY_MARKER

    return 0;
}

struct ReflexQueueContextData {
    VkDevice device;
    bool outOfBandRenderSubmit;
    bool outOfBandPresent;
};

using ReflexInstanceContextData = uint32_t;

VKROOTS_DEFINE_SYNCHRONIZED_MAP_TYPE(ReflexInstanceContext, VkInstance);
VKROOTS_DEFINE_SYNCHRONIZED_MAP_TYPE(ReflexDeviceContext, VkDevice);
VKROOTS_DEFINE_SYNCHRONIZED_MAP_TYPE(ReflexQueueContext, VkQueue);

VKROOTS_IMPLEMENT_SYNCHRONIZED_MAP_TYPE(ReflexInstanceContext);
VKROOTS_IMPLEMENT_SYNCHRONIZED_MAP_TYPE(ReflexDeviceContext);
VKROOTS_IMPLEMENT_SYNCHRONIZED_MAP_TYPE(ReflexQueueContext);

static inline auto GetContext(VkDevice device) {
    return ReflexDeviceContext::get(device);
}

static inline auto GetContexts(VkQueue queue) {
    auto queueContext = ReflexQueueContext::get(queue);

    if (queueContext) {
        auto deviceContext = ReflexDeviceContext::get(queueContext->device);

        return std::make_pair(std::move(queueContext), std::move(deviceContext));
    } else {
        return std::make_pair(ReflexQueueContext{nullptr}, ReflexDeviceContext{nullptr});
    }
}

static bool PhysicalDeviceSupportsExtension(PFN_vkEnumerateDeviceExtensionProperties pvkEnumerateDeviceExtensionProperties, VkPhysicalDevice physicalDevice, char const* extensionName, uint32_t specVersion) {
    uint32_t count;
    auto vr = pvkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);

    if (vr != VK_SUCCESS)
        return false;

    std::vector<VkExtensionProperties> properties{count};
    vr = pvkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, properties.data());

    if (vr != VK_SUCCESS && vr != VK_INCOMPLETE)
        return false;

    auto extName = std::string_view{extensionName};

    for (auto property = properties.data(); property < properties.data() + count; ++property) {
        if (property->extensionName == extName && property->specVersion >= specVersion)
            return true;
    }

    return false;
}

template <typename Type, typename AnyStruct>
static inline void AddToChain(AnyStruct* pParent, Type* pType) {
    pType->pNext = const_cast<void*>(std::exchange(pParent->pNext, pType));
}

static inline void ProcessDeviceQueue(VkDevice device, VkQueue* pQueue) {
    if (!injectFrameIDs)
        return;

    if (auto context = ::GetContext(device); context && pQueue) {
        if (auto queue = *pQueue) {
            if (context->queues.insert(queue).second)
                ReflexQueueContext::create(queue, {.device = device});
        }
    }
}

static VkResult WaitSemaphores(
    PFN_vkWaitSemaphores pvkWaitSemaphores,
    VkDevice device,
    VkSemaphoreWaitInfo const* pWaitInfo,
    uint64_t timeout) {
    if (logLevel < LogLevel_Debug)
        return pvkWaitSemaphores(device, pWaitInfo, timeout);

    if (pWaitInfo) {
        switch (pWaitInfo->semaphoreCount) {
            case 0:
                DBG("(%p, %p { %" PRIu32 ", %" PRIu32 ", [], [] }, %" PRIu64 ")",
                    device, pWaitInfo, pWaitInfo->flags, pWaitInfo->semaphoreCount, timeout);
                break;
            case 1:
                DBG("(%p, %p { %" PRIu32 ", %" PRIu32 ", [%p], [%" PRIu64 "] }, %" PRIu64 ")",
                    device, pWaitInfo, pWaitInfo->flags, pWaitInfo->semaphoreCount, pWaitInfo->pSemaphores[0], pWaitInfo->pValues[0], timeout);
                break;
            default:
                DBG("(%p, %p { %" PRIu32 ", %" PRIu32 ", [%p, ...], [%" PRIu64 ", ...] }, %" PRIu64 ")",
                    device, pWaitInfo, pWaitInfo->flags, pWaitInfo->semaphoreCount, pWaitInfo->pSemaphores[0], pWaitInfo->pValues[0], timeout);
                break;
        }
    } else {
        DBG("(%p, %p, %" PRIu64 ")", device, pWaitInfo, timeout);
    }

    auto begin = ::GetTimestamp();

    auto result = pvkWaitSemaphores(device, pWaitInfo, timeout);

    auto end = ::GetTimestamp();

    auto diff = (end.seconds - begin.seconds) * 1000 + (end.milliseconds - begin.milliseconds);

    DBG("waited for %" PRIi32 " ms", diff);

    return result;
}

static VkResult QueueSubmit2(
    PFN_vkQueueSubmit2 pvkQueueSubmit2,
    VkQueue queue,
    uint32_t submitCount,
    VkSubmitInfo2 const* pSubmits,
    VkFence fence) {
    if (!injectSubmitFrameIDs)
        return pvkQueueSubmit2(queue, submitCount, pSubmits, fence);

    auto [queueContext, deviceContext] = ::GetContexts(queue);

    if (deviceContext && deviceContext->latencySleepModeInfo.lowLatencyMode && pSubmits && submitCount) {
        uint64_t id = ::GetFrameId(deviceContext.get(), false, queueContext->outOfBandRenderSubmit);

        DBG("(%p, %" PRIu32 ", %p, %p) frameID = %" PRIu64 ", oob = %d",
            queue, submitCount, pSubmits, fence, id, queueContext->outOfBandRenderSubmit);

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

            return pvkQueueSubmit2(queue, submitCount, submitInfos.data(), fence);
        }
    }

    return pvkQueueSubmit2(queue, submitCount, pSubmits, fence);
}

static constexpr auto gpdp2 = std::string_view{VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};
static constexpr auto ll = std::string_view{VK_NV_LOW_LATENCY_EXTENSION_NAME};
static constexpr auto ll2 = std::string_view{VK_NV_LOW_LATENCY_2_EXTENSION_NAME};
static constexpr auto ts = std::string_view{VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME};
static constexpr auto pid = std::string_view{VK_KHR_PRESENT_ID_EXTENSION_NAME};

struct VkInstanceOverrides {
    static VkResult CreateInstance(
        PFN_vkCreateInstance pfnCreateInstanceProc,
        VkInstanceCreateInfo const* pCreateInfo,
        VkAllocationCallbacks const* pAllocator,
        VkInstance* pInstance) {
        if (!pCreateInfo)
            return pfnCreateInstanceProc(pCreateInfo, pAllocator, pInstance);

        auto info = *pCreateInfo;
        auto apiVersion = info.pApplicationInfo ? info.pApplicationInfo->apiVersion : VK_VERSION_1_0;
        std::vector<char const*> extensions;

        if (apiVersion < VK_VERSION_1_1) {
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
            ReflexInstanceContext::create(*pInstance, apiVersion);

        return vr;
    }

    static VkResult EnumerateDeviceExtensionProperties(
        vkroots::VkInstanceDispatch const* pDispatch,
        VkPhysicalDevice physicalDevice,
        char const* pLayerName,
        uint32_t* pPropertyCount,
        VkExtensionProperties* pProperties) {
        if (!exposeExtension)
            return pDispatch->EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);

        static constexpr auto exts = std::array<VkExtensionProperties, 1>{{
            {
                .extensionName = VK_NV_LOW_LATENCY_EXTENSION_NAME,
                .specVersion = VK_NV_LOW_LATENCY_SPEC_VERSION,
            },
        }};

        if (pLayerName) {
            if (pLayerName == "VK_LAYER_DXVK_NVAPI_reflex"sv) {
                if (PhysicalDeviceSupportsExtension(pDispatch->EnumerateDeviceExtensionProperties, physicalDevice, ll2.data(), 2))
                    return vkroots::helpers::array(exts, pPropertyCount, pProperties);
                else
                    return *pPropertyCount = 0, VK_SUCCESS;
            } else {
                return pDispatch->EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
            }
        }

        if (PhysicalDeviceSupportsExtension(pDispatch->EnumerateDeviceExtensionProperties, physicalDevice, ll2.data(), 2))
            return vkroots::helpers::append(pDispatch->EnumerateDeviceExtensionProperties, exts, pPropertyCount, pProperties, physicalDevice, pLayerName);
        else
            return pDispatch->EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
    }

    static VkResult CreateDevice(
        vkroots::VkInstanceDispatch const* pDispatch,
        VkPhysicalDevice physicalDevice,
        VkDeviceCreateInfo const* pCreateInfo,
        VkAllocationCallbacks const* pAllocator,
        VkDevice* pDevice) {
        if (!pCreateInfo)
            return pDispatch->CreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);

        auto info = *pCreateInfo;

        for (auto ext = info.ppEnabledExtensionNames; ext && ext < info.ppEnabledExtensionNames + info.enabledExtensionCount; ++ext) {
            if (*ext == ll2) {
                INFO("%s already requested by the application, skipping setup of compatibility layer", ll2.data());
                return pDispatch->CreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
            }
        }

        auto extensions = info.ppEnabledExtensionNames && info.enabledExtensionCount
            ? std::vector<char const*>{
                  info.ppEnabledExtensionNames,
                  info.ppEnabledExtensionNames + info.enabledExtensionCount}
            : injectFrameIDs ? std::vector<char const*>{{pid.data(), ts.data(), ll2.data()}}
                             : std::vector<char const*>{{ts.data(), ll2.data()}};

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

        if (auto apiver = ReflexInstanceContext::get(pDispatch->Instance); apiver && *apiver.get() < VK_VERSION_1_2) {
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

        auto vr = pDispatch->CreateDevice(physicalDevice, &info, pAllocator, pDevice);

        if (vr == VK_SUCCESS)
            ReflexDeviceContext::create(*pDevice, {});

        return vr;
    }
};

struct VkDeviceOverrides {
    static void DestroyDevice(
        vkroots::VkDeviceDispatch const* pDispatch,
        VkDevice device,
        VkAllocationCallbacks const* pAllocator) {
        auto node = ReflexDeviceContext::get(device);

        if (node) {
            auto context = node.get();

            for (auto queue : context->queues)
                ReflexQueueContext::remove(queue);

            node.remove(device);
        }

        pDispatch->DestroyDevice(device, pAllocator);
    }

    static VkResult CreateSwapchainKHR(
        vkroots::VkDeviceDispatch const* pDispatch,
        VkDevice device,
        VkSwapchainCreateInfoKHR const* pCreateInfo,
        VkAllocationCallbacks const* pAllocator,
        VkSwapchainKHR* pSwapchain) {
        auto context = ::GetContext(device);

        if (!context)
            return pDispatch->CreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);

        auto info = *pCreateInfo;

        auto swapchainLatencyCreateInfo = VkSwapchainLatencyCreateInfoNV{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_LATENCY_CREATE_INFO_NV,
            .pNext = info.pNext,
            .latencyModeEnable = VK_TRUE,
        };

        info.pNext = &swapchainLatencyCreateInfo;

        auto vr = pDispatch->CreateSwapchainKHR(device, &info, pAllocator, pSwapchain);

        if (vr == VK_SUCCESS) {
            if (context->latencySleepModeInfo.sType == VK_STRUCTURE_TYPE_LATENCY_SLEEP_MODE_INFO_NV) {
                vr = pDispatch->SetLatencySleepModeNV(device, *pSwapchain, &context->latencySleepModeInfo);

                if (vr != VK_SUCCESS) {
                    pDispatch->DestroySwapchainKHR(device, *pSwapchain, pAllocator);

                    return vr;
                }
            }

            context->swapchain = *pSwapchain;
        }

        return vr;
    }

    static void DestroySwapchainKHR(
        vkroots::VkDeviceDispatch const* pDispatch,
        VkDevice device,
        VkSwapchainKHR swapchain,
        VkAllocationCallbacks const* pAllocator) {
        auto context = ::GetContext(device);

        pDispatch->DestroySwapchainKHR(device, swapchain, pAllocator);

        if (context && context->swapchain == swapchain)
            context->swapchain = VK_NULL_HANDLE;
    }

    static void GetDeviceQueue(
        vkroots::VkDeviceDispatch const* pDispatch,
        VkDevice device,
        uint32_t queueFamilyIndex,
        uint32_t queueIndex,
        VkQueue* pQueue) {
        pDispatch->GetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);

        ::ProcessDeviceQueue(device, pQueue);
    }

    static void GetDeviceQueue2(
        vkroots::VkDeviceDispatch const* pDispatch,
        VkDevice device,
        VkDeviceQueueInfo2 const* pQueueInfo,
        VkQueue* pQueue) {
        pDispatch->GetDeviceQueue2(device, pQueueInfo, pQueue);

        ::ProcessDeviceQueue(device, pQueue);
    }

    static VkResult WaitSemaphores(
        vkroots::VkDeviceDispatch const* pDispatch,
        VkDevice device,
        VkSemaphoreWaitInfo const* pWaitInfo,
        uint64_t timeout) {
        return ::WaitSemaphores(pDispatch->WaitSemaphores, device, pWaitInfo, timeout);
    }

    static VkResult WaitSemaphoresKHR(
        vkroots::VkDeviceDispatch const* pDispatch,
        VkDevice device,
        VkSemaphoreWaitInfoKHR const* pWaitInfo,
        uint64_t timeout) {
        return ::WaitSemaphores(pDispatch->WaitSemaphoresKHR, device, pWaitInfo, timeout);
    }

    static VkResult QueueSubmit(
        vkroots::VkDeviceDispatch const* pDispatch,
        VkQueue queue,
        uint32_t submitCount,
        VkSubmitInfo const* pSubmits,
        VkFence fence) {
        if (!injectSubmitFrameIDs)
            return pDispatch->QueueSubmit(queue, submitCount, pSubmits, fence);

        auto [queueContext, deviceContext] = ::GetContexts(queue);

        if (deviceContext && deviceContext->latencySleepModeInfo.lowLatencyMode && pSubmits && submitCount) {
            uint64_t id = ::GetFrameId(deviceContext.get(), false, queueContext->outOfBandRenderSubmit);

            DBG("(%p, %" PRIu32 ", %p, %p) frameID = %" PRIu64 ", oob = %d",
                queue, submitCount, pSubmits, fence, id, queueContext->outOfBandRenderSubmit);

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

                return pDispatch->QueueSubmit(queue, submitCount, submitInfos.data(), fence);
            }
        }

        return pDispatch->QueueSubmit(queue, submitCount, pSubmits, fence);
    }

    static VkResult QueueSubmit2(
        vkroots::VkDeviceDispatch const* pDispatch,
        VkQueue queue,
        uint32_t submitCount,
        VkSubmitInfo2 const* pSubmits,
        VkFence fence) {
        return ::QueueSubmit2(pDispatch->QueueSubmit2, queue, submitCount, pSubmits, fence);
    }

    static VkResult QueueSubmit2KHR(
        vkroots::VkDeviceDispatch const* pDispatch,
        VkQueue queue,
        uint32_t submitCount,
        VkSubmitInfo2KHR const* pSubmits,
        VkFence fence) {
        return ::QueueSubmit2(pDispatch->QueueSubmit2KHR, queue, submitCount, pSubmits, fence);
    }

    static VkResult QueuePresentKHR(
        vkroots::VkDeviceDispatch const* pDispatch,
        VkQueue queue,
        VkPresentInfoKHR const* pPresentInfo) {
        if (!injectPresentFrameIDs)
            return pDispatch->QueuePresentKHR(queue, pPresentInfo);

        auto [queueContext, deviceContext] = ::GetContexts(queue);

        if (deviceContext && deviceContext->latencySleepModeInfo.lowLatencyMode && pPresentInfo && pPresentInfo->pSwapchains && pPresentInfo->swapchainCount) {
            uint32_t i;
            uint64_t id = ::GetFrameId(deviceContext.get(), true, queueContext->outOfBandPresent);

            DBG("(%p, %p) frameID = %" PRIu64 ", oob = %d",
                queue, pPresentInfo, id, queueContext->outOfBandPresent);

            if (!id)
                goto end;

            for (i = 0; i < pPresentInfo->swapchainCount; ++i) {
                if (pPresentInfo->pSwapchains[i] == deviceContext->swapchain)
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

                return pDispatch->QueuePresentKHR(queue, &info);
            }
        }

    end:
        return pDispatch->QueuePresentKHR(queue, pPresentInfo);
    }

    static VkResult SetLatencySleepModeNV(
        vkroots::VkDeviceDispatch const* pDispatch,
        VkDevice device,
        VkSwapchainKHR swapchain,
        VkLatencySleepModeInfoNV const* pSleepModeInfo) {
        auto context = ::GetContext(device);

        if (!context)
            return pDispatch->SetLatencySleepModeNV(device, swapchain, pSleepModeInfo);

        if (pSleepModeInfo)
            TRACE("(%p, %p = %p, %p { %" PRIu32 ", %" PRIu32 ", %" PRIu32 " })",
                device, swapchain, context->swapchain, pSleepModeInfo, pSleepModeInfo->lowLatencyMode, pSleepModeInfo->lowLatencyBoost, pSleepModeInfo->minimumIntervalUs);
        else
            TRACE("(%p, %p = %p, %p)",
                device, swapchain, context->swapchain, pSleepModeInfo);

        swapchain = context->swapchain;

        auto vr = VK_SUCCESS;

        if (swapchain)
            vr = pDispatch->SetLatencySleepModeNV(device, swapchain, pSleepModeInfo);

        if (vr == VK_SUCCESS) {
            if (pSleepModeInfo) {
                context->latencySleepModeInfo = *pSleepModeInfo;
                context->latencySleepModeInfo.pNext = nullptr;
            } else {
                context->latencySleepModeInfo = VkLatencySleepModeInfoNV{};
            }
        }

        return vr;
    }

    static VkResult LatencySleepNV(
        vkroots::VkDeviceDispatch const* pDispatch,
        VkDevice device,
        VkSwapchainKHR swapchain,
        VkLatencySleepInfoNV const* pSleepInfo) {
        if (!pSleepInfo)
            return VK_ERROR_UNKNOWN;

        auto context = ::GetContext(device);

        if (!context)
            return pDispatch->LatencySleepNV(device, swapchain, pSleepInfo);

        TRACE("(%p, %p = %p, %p { %p, %" PRIu64 " })",
            device, swapchain, context->swapchain, pSleepInfo, pSleepInfo->signalSemaphore, pSleepInfo->value);

        swapchain = context->swapchain;

        if (swapchain)
            return pDispatch->LatencySleepNV(device, swapchain, pSleepInfo);

        auto semaphoreSignalInfo = VkSemaphoreSignalInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
            .semaphore = pSleepInfo->signalSemaphore,
            .value = pSleepInfo->value,
        };

        return pDispatch->SignalSemaphore(device, &semaphoreSignalInfo);
    }

    static void SetLatencyMarkerNV(
        vkroots::VkDeviceDispatch const* pDispatch,
        VkDevice device,
        VkSwapchainKHR swapchain,
        VkSetLatencyMarkerInfoNV const* pLatencyMarkerInfo) {
        if (!pLatencyMarkerInfo)
            return;

        auto context = ::GetContext(device);

        if (!context) {
            pDispatch->SetLatencyMarkerNV(device, swapchain, pLatencyMarkerInfo);
            return;
        }

        TRACE("(%p, %p = %p, %p { %" PRIu64 ", %s })",
            device, swapchain, context->swapchain, pLatencyMarkerInfo, pLatencyMarkerInfo->presentID, vkroots::helpers::enumString(pLatencyMarkerInfo->marker));

        swapchain = context->swapchain;

        if (swapchain)
            pDispatch->SetLatencyMarkerNV(device, swapchain, pLatencyMarkerInfo);

        if (!injectFrameIDs)
            return;

        switch (pLatencyMarkerInfo->marker) {
            case VK_LATENCY_MARKER_SIMULATION_START_NV:
                context->markers.simulation.id = pLatencyMarkerInfo->presentID;
                context->markers.simulation.ongoing = true;
                break;
            case VK_LATENCY_MARKER_SIMULATION_END_NV:
                context->markers.simulation.ongoing = false;
                break;
            case VK_LATENCY_MARKER_RENDERSUBMIT_START_NV:
                context->markers.renderSubmit.id = pLatencyMarkerInfo->presentID;
                context->markers.renderSubmit.ongoing = true;
                break;
            case VK_LATENCY_MARKER_RENDERSUBMIT_END_NV:
                context->markers.renderSubmit.ongoing = false;
                break;
            case VK_LATENCY_MARKER_PRESENT_START_NV:
                context->markers.present.id = pLatencyMarkerInfo->presentID;
                context->markers.present.ongoing = true;
                break;
            case VK_LATENCY_MARKER_PRESENT_END_NV:
                context->markers.present.ongoing = false;
                break;
            case VK_LATENCY_MARKER_INPUT_SAMPLE_NV:
                break;
            case VK_LATENCY_MARKER_TRIGGER_FLASH_NV:
                break;
            case VK_LATENCY_MARKER_OUT_OF_BAND_RENDERSUBMIT_START_NV:
                context->markers.outOfBandRenderSubmit.id = pLatencyMarkerInfo->presentID;
                context->markers.outOfBandRenderSubmit.ongoing = true;
                break;
            case VK_LATENCY_MARKER_OUT_OF_BAND_RENDERSUBMIT_END_NV:
                context->markers.outOfBandRenderSubmit.ongoing = false;
                break;
            case VK_LATENCY_MARKER_OUT_OF_BAND_PRESENT_START_NV:
                context->markers.outOfBandPresent.id = pLatencyMarkerInfo->presentID;
                context->markers.outOfBandPresent.ongoing = true;
                break;
            case VK_LATENCY_MARKER_OUT_OF_BAND_PRESENT_END_NV:
                context->markers.outOfBandPresent.ongoing = false;
                break;
            default:
                break;
        }
    }

    static void GetLatencyTimingsNV(
        vkroots::VkDeviceDispatch const* pDispatch,
        VkDevice device,
        VkSwapchainKHR swapchain,
        VkGetLatencyMarkerInfoNV* pLatencyMarkerInfo) {
        if (!pLatencyMarkerInfo)
            return;

        auto context = ::GetContext(device);

        if (!context) {
            pDispatch->GetLatencyTimingsNV(device, swapchain, pLatencyMarkerInfo);
            return;
        }

        TRACE("(%p, %p = %p, %p { %" PRIu32 ", %p })",
            device, swapchain, context->swapchain, pLatencyMarkerInfo, pLatencyMarkerInfo->timingCount, pLatencyMarkerInfo->pTimings);

        swapchain = context->swapchain;

        if (swapchain)
            pDispatch->GetLatencyTimingsNV(device, swapchain, pLatencyMarkerInfo);
        else
            pLatencyMarkerInfo->timingCount = 0;
    }

    static void QueueNotifyOutOfBandNV(
        vkroots::VkDeviceDispatch const* pDispatch,
        VkQueue queue,
        VkOutOfBandQueueTypeInfoNV const* pQueueTypeInfo) {
        if (!pQueueTypeInfo)
            return;

        TRACE("(%p, %p { %s })",
            queue, pQueueTypeInfo, vkroots::helpers::enumString(pQueueTypeInfo->queueType));

        pDispatch->QueueNotifyOutOfBandNV(queue, pQueueTypeInfo);

        if (!injectFrameIDs)
            return;

        if (auto [queueContext, deviceContext] = ::GetContexts(queue); queueContext) {
            switch (pQueueTypeInfo->queueType) {
                case VK_OUT_OF_BAND_QUEUE_TYPE_RENDER_NV:
                    queueContext->outOfBandRenderSubmit = true;
                    break;
                case VK_OUT_OF_BAND_QUEUE_TYPE_PRESENT_NV:
                    queueContext->outOfBandPresent = true;
                    break;
                default:
                    break;
            }
        }
    }
};

VKROOTS_DEFINE_LAYER_INTERFACES(VkInstanceOverrides, vkroots::NoOverrides, VkDeviceOverrides);
