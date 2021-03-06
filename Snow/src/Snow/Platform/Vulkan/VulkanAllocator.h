#pragma once

#include <string>
#include "Snow/Platform/Vulkan/VulkanCommon.h"
#include "Snow/Platform/Vulkan/VulkanDevice.h"

#include "Snow/Core/Ref.h"


namespace Snow {
    class VulkanAllocator {
    public:
        VulkanAllocator() = default;
        VulkanAllocator(const std::string& tag);
        VulkanAllocator(Ref<VulkanDevice>& device, const std::string& tag = "");

        void Allocate(VkMemoryRequirements reqs, VkDeviceMemory* dest, VkMemoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    private:
        Ref<VulkanDevice> m_Device;
        std::string m_Tag;
    };
}