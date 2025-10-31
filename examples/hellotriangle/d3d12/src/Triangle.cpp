#include "Triangle.h"

#include <DirectXColors.h>

namespace Luna
{
    void Triangle::Init()
    {
        renderer = new Renderer();
        
        graphics->ResetCommands();
        BuildGeometry();
        graphics->SubmitCommands();
    }

    void Triangle::Update()
    {
        if(input->KeyDown(VK_ESCAPE))
            window->Close();
    }

    void Triangle::Display()
    {
        renderer->Clear();

        renderer->Draw();
        graphics->CommandList()->IASetVertexBuffers(0, 1, geometry->VertexBufferView());
        
        graphics->CommandList()->DrawInstanced(3, 1, 0, 0);

        graphics->Present();
    }

    void Triangle::Finalize()
    {
        SafeDelete(renderer);
        SafeDelete(geometry);
    }

    void Triangle::BuildGeometry()
    {
        Vertex vertices[]
        {
            { Position(0.0f, 0.5f, 0.0f), Color(Colors::Red) },
            { Position(0.5f, -0.5f, 0.0f), Color(Colors::Orange) },
            { Position(-0.5f, -0.5f, 0.0f), Color(Colors::Yellow) }
        };

        constexpr uint32 vbSize = 3 * sizeof(Vertex);

        geometry = new Mesh("Triangle");

        geometry->vertexByteStride = sizeof(Vertex);
        geometry->vertexBufferSize = vbSize;

        graphics->Allocate(vbSize, &geometry->vertexBufferCPU);
        graphics->Allocate(UPLOAD, vbSize, &geometry->vertexBufferUpload);
        graphics->Allocate(GPU, vbSize, &geometry->vertexBufferGPU);

        graphics->Copy(vertices, vbSize, geometry->vertexBufferCPU);

        graphics->Copy(vertices, vbSize, geometry->vertexBufferUpload, geometry->vertexBufferGPU);

        renderer->Initialize(graphics);
    }
}