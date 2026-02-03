#include "Renderer.h"
#include "VkError.h"
#include "Utils.h"
#include <fstream>
#include <vector>
#include <filesystem>

namespace Luna
{
    namespace fs = std::filesystem;

    Renderer::Renderer() noexcept
        : graphics{nullptr},
        pipeline{nullptr},
        pipelineLayout{nullptr}
    {
        geometry = new Mesh("Triangle");
    }

    Renderer::~Renderer()
    {
        graphics->ResetCommands();

        SafeDelete(geometry);
        
        vkDestroyPipelineLayout(graphics->Device(), pipelineLayout, nullptr);
        vkDestroyPipeline(graphics->Device(), pipeline, nullptr);
    }

    static fs::path GetShaderPath(const string_view filename)
    {
        char exePath[MAX_PATH];
        GetModuleFileNameA(nullptr, exePath, MAX_PATH);
        
        fs::path exeDir = fs::path(exePath).parent_path();
        fs::path shaderPath = exeDir / filename.data();
        
        return shaderPath;
    }

    static VkShaderModule CreateShaderModule(VkDevice device, const string_view filename)
    {
        fs::path shaderPath = GetShaderPath(filename);
        std::ifstream file(shaderPath, std::ios::ate | std::ios::binary);
        
        std::vector<char> fileBytes(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(fileBytes.data(), fileBytes.size());
        file.close();

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = fileBytes.size();
        createInfo.pCode = reinterpret_cast<uint32*>(fileBytes.data());

        VkShaderModule shaderModule;
        VkThrowIfFailed(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));

        return shaderModule;
    }

    void Renderer::BuildVertexBuffer(const Vertex* vertices, const uint32 count)
    {
        VkDeviceSize vertexBufferSize = sizeof(Vertex) * count;

        graphics->Allocate(
            vertexBufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &geometry->vertexBufferUpload,
            &geometry->vertexBufferUploadMemory
        );

        graphics->Copy(vertices, vertexBufferSize, geometry->vertexBufferUploadMemory);

        graphics->Allocate(
            vertexBufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            &geometry->vertexBuffer,
            &geometry->vertexBufferMemory
        );

        graphics->Copy(geometry->vertexBuffer, geometry->vertexBufferUpload, vertexBufferSize);
    }

    void Renderer::Initialize(Graphics* graphics, const Vertex * vertices, const uint32 verticesCount)
    {
        this->graphics = graphics;

        BuildVertexBuffer(vertices, verticesCount);

        // -----------------------------------------------------------
        // Pipeline layout
        // -----------------------------------------------------------

        // --------------------
        // ----- Shaders ------
        // --------------------

        VkShaderModule vertexShaderModule = CreateShaderModule(graphics->Device(), "Shaders/Vertex.spv");
        VkShaderModule fragmentShaderModule = CreateShaderModule(graphics->Device(), "Shaders/Fragment.spv");

        VkPipelineShaderStageCreateInfo vertexShaderCreateInfo{};
        vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderCreateInfo.module = vertexShaderModule;
        vertexShaderCreateInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo{};
        fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentShaderCreateInfo.module = fragmentShaderModule;
        fragmentShaderCreateInfo.pName = "main";

        const VkPipelineShaderStageCreateInfo shaderStages[]
        { vertexShaderCreateInfo, fragmentShaderCreateInfo };

        // --------------------
        // -- Input Assembly --
        // --------------------

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
        inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyCreateInfo.primitiveRestartEnable = false;

        // --------------------
        // --- Vertex Input ---
        // --------------------

        VkVertexInputBindingDescription vertexBindingDescription{};
        vertexBindingDescription.binding = 0;
        vertexBindingDescription.stride = sizeof(Vertex);
        vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription vertexInputAttributeDescription[2]{};
        // Position
        vertexInputAttributeDescription[0].binding = 0;
        vertexInputAttributeDescription[0].location = 0;
        vertexInputAttributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;

        // Color
        vertexInputAttributeDescription[1].binding = 0;
        vertexInputAttributeDescription[1].location = 1;
        vertexInputAttributeDescription[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        vertexInputAttributeDescription[1].offset = offsetof(Vertex, color);

        VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
        vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
        vertexInputCreateInfo.pVertexBindingDescriptions = &vertexBindingDescription;
        vertexInputCreateInfo.vertexAttributeDescriptionCount = 2;
        vertexInputCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescription;

        // --------------------
        // -- Viewport State --
        // --------------------

        VkPipelineViewportStateCreateInfo viewportCreateInfo{};
        viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportCreateInfo.viewportCount = 1;
        viewportCreateInfo.pViewports = &graphics->viewport;
        viewportCreateInfo.scissorCount = 1;
        viewportCreateInfo.pScissors = &graphics->scissorRect;

        // --------------------
        // ---- Rasterizer ----
        // --------------------

        VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo{};
        rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationCreateInfo.depthClampEnable = false;
        rasterizationCreateInfo.rasterizerDiscardEnable = false;
        rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizationCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizationCreateInfo.depthBiasEnable = false;
        rasterizationCreateInfo.lineWidth = 1.0f;

        // --------------------
        // --- Multi Sample ---
        // --------------------

        VkPipelineMultisampleStateCreateInfo multisampleCreateInfo{};
        multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleCreateInfo.sampleShadingEnable = false;

        // ---------------------
        // --- Color Blender ---
        // ---------------------

        VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
        colorBlendAttachmentState.blendEnable = false;
        colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT 
            | VK_COLOR_COMPONENT_G_BIT 
            | VK_COLOR_COMPONENT_B_BIT 
            | VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo{};
        colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendCreateInfo.logicOpEnable = false;
        colorBlendCreateInfo.attachmentCount = 1;
        colorBlendCreateInfo.pAttachments = &colorBlendAttachmentState;

        // ---------------------
        // -- Pipeline Layout --
        // ---------------------

        VkPipelineLayoutCreateInfo layoutCreateInfo{};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutCreateInfo.setLayoutCount = 0;
        layoutCreateInfo.pSetLayouts = nullptr;

        VkThrowIfFailed(vkCreatePipelineLayout(graphics->Device(), &layoutCreateInfo, nullptr, &pipelineLayout));

        // -------------------------
        // --- Graphics Pipeline ---
        // -------------------------

        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.stageCount = 2;
        pipelineCreateInfo.pStages = shaderStages;
        pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
        pipelineCreateInfo.pViewportState = &viewportCreateInfo;
        pipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
        pipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
        pipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
        pipelineCreateInfo.layout = pipelineLayout;
        pipelineCreateInfo.renderPass = graphics->RenderPass();
        pipelineCreateInfo.subpass = 0;

        VkThrowIfFailed(vkCreateGraphicsPipelines(graphics->Device(), nullptr, 1, &pipelineCreateInfo, nullptr, &pipeline));

        vkDestroyShaderModule(graphics->Device(), vertexShaderModule, nullptr);
        vkDestroyShaderModule(graphics->Device(), fragmentShaderModule, nullptr);
    }

    void Renderer::BindDrawResources() noexcept
    {
        vkCmdBindPipeline(graphics->CommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        
        VkDeviceSize offset{};
        vkCmdBindVertexBuffers(graphics->CommandBuffer(), 0, 1, &geometry->vertexBuffer, &offset);
    }
}