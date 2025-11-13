#include "Triangle.h"
#include "Colors.h"
using namespace glm;

namespace Luna
{
    void Triangle::Init()
    {
        renderer = new Renderer();
        BuildGeometry();
    }

    void Triangle::Update()
    {
        if(input->KeyDown(VK_ESCAPE))
            window->Close();
    }

    void Triangle::Display()
    {
        graphics->BeginCommandRecording();

        renderer->BindDrawResources();

        vkCmdDraw(graphics->CommandBuffer(), 3, 1, 0, 0);

        graphics->EndCommandRecording();

        graphics->Present();
    }

    void Triangle::Finalize()
    {
        SafeDelete(renderer);
    }

    void Triangle::BuildGeometry()
    {
        Vertex vertices[]
        {
            { Position(0.0f, -0.5f, 0.0f), Color(Colors::Red) },
            { Position(-0.5f, 0.5f, 0.0f), Color(Colors::Orange) },
            { Position(0.5f, 0.5f, 0.0f), Color(Colors::Yellow) }
        };

        renderer->Initialize(graphics, vertices, 3);
    }
}