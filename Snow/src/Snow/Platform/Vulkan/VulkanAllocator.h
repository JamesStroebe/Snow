#pragma once

#include <string>
#include <vulkan/vulkan.h>
#include "Snow/Platform/Vulkan/VulkanDevice.h"


namespace Snow {
    namespace Render {
        class VulkanAllocator {
        public:
            VulkanAllocator() = default;
            VulkanAllocator(const std::string& tag);
            VulkanAllocator(VulkanDevice* device, const std::string& tag = "");

            void Allocate(VkMemoryRequirements reqs, VkDeviceMemory* dest, VkMemoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        private:
            VulkanDevice* m_Device;
            std::string m_Tag;
        };
    }
}