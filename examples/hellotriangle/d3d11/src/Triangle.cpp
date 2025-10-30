#include "Triangle.h"
#include <DirectXColors.h>

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
        graphics->Clear();

        renderer->Draw();
        graphics->Context()->DrawInstanced(3, 1, 0, 0);

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
            { Position(0.0f, 0.5f, 0.0f), Color(Colors::Red) },
            { Position(0.5f, -0.5f, 0.0f), Color(Colors::Orange) },
            { Position(-0.5f, -0.5f, 0.0f), Color(Colors::Yellow) }
        };

        renderer->Initialize(graphics, vertices, Countof(vertices));
    }
}