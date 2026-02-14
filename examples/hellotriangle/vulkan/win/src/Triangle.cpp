#include "Triangle.h"
#include "Colors.h"
#include "Utils.h"
using namespace glm;

namespace Luna
{
    void Triangle::Init()
    {
        renderer = new Renderer();
        geometry = new Mesh("Triangle");
        BuildGeometry();
    }

    void Triangle::Update()
    {
        if(input->KeyDown(VK_ESCAPE))
            window->Close();
    }

    void Triangle::Display()
    {
        graphics->Clear();

        vkCmdBindPipeline(graphics->CommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->Pipeline());
        
        VkDeviceSize offset{};
        vkCmdBindVertexBuffers(graphics->CommandBuffer(), 0, 1, &geometry->vertexBuffer, &offset);

        vkCmdDraw(graphics->CommandBuffer(), geometry->vertexCount, 1, 0, 0);

        vkCmdEndRenderPass(graphics->CommandBuffer());
        vkEndCommandBuffer(graphics->CommandBuffer());

        graphics->Present();
    }

    void Triangle::Finalize()
    {
        SafeDelete(renderer);
        SafeDelete(geometry);
    }

    void Triangle::BuildGeometry()
    {
        constexpr Vertex vertices[]
        {
            { Position(0.0f, -0.5f, 0.0f), Color(Colors::Red) },
            { Position(-0.5f, 0.5f, 0.0f), Color(Colors::Orange) },
            { Position(0.5f, 0.5f, 0.0f), Color(Colors::Yellow) }
        };

        geometry->vertexCount = Countof(vertices);
        geometry->vertexBufferSize = sizeof(Vertex) * geometry->vertexCount;
        geometry->device = graphics->Device();
		
        graphics->Allocate(
            geometry->vertexBufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &geometry->vertexUploadBuffer.buffer,
            &geometry->vertexUploadBuffer.memory
        );

        graphics->Copy(vertices, geometry->vertexBufferSize, geometry->vertexUploadBuffer.memory);

        graphics->Allocate(
            geometry->vertexBufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            &geometry->vertexBuffer,
            &geometry->vertexBufferMemory
        );

        graphics->Copy(geometry->vertexBuffer, geometry->vertexUploadBuffer.buffer, geometry->vertexBufferSize);

        vkDestroyBuffer(graphics->Device(), geometry->vertexUploadBuffer.buffer, nullptr);
        vkFreeMemory(graphics->Device(), geometry->vertexUploadBuffer.memory, nullptr);
        geometry->vertexUploadBuffer.buffer = nullptr;
        geometry->vertexUploadBuffer.memory = nullptr;

        renderer->Initialize(graphics, geometry);
    }
}