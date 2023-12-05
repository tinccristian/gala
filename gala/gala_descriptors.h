#pragma once

#include "gala_device.h"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace gala {

    class GalaDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(GalaDevice& galaDevice) : galaDevice{ galaDevice } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<GalaDescriptorSetLayout> build() const;

        private:
            GalaDevice& galaDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        GalaDescriptorSetLayout(
            GalaDevice& galaDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~GalaDescriptorSetLayout();
        GalaDescriptorSetLayout(const GalaDescriptorSetLayout&) = delete;
        GalaDescriptorSetLayout& operator=(const GalaDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        GalaDevice& galaDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class GalaDescriptorWriter;
    };

    class GalaDescriptorPool {
    public:
        class Builder {
        public:
            Builder(GalaDevice& galaDevice) : galaDevice{ galaDevice } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<GalaDescriptorPool> build() const;

        private:
            GalaDevice& galaDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        GalaDescriptorPool(
            GalaDevice& galaDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~GalaDescriptorPool();
        GalaDescriptorPool(const GalaDescriptorPool&) = delete;
        GalaDescriptorPool& operator=(const GalaDescriptorPool&) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        GalaDevice& galaDevice;
        VkDescriptorPool descriptorPool;

        friend class GalaDescriptorWriter;
    };

    class GalaDescriptorWriter {
    public:
        GalaDescriptorWriter(GalaDescriptorSetLayout& setLayout, GalaDescriptorPool& pool);

        GalaDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        GalaDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        GalaDescriptorSetLayout& setLayout;
        GalaDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}  // namespace gala