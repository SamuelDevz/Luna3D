#pragma once

#include "All.h"
#include "Mesh.h"
#include "Renderer.h"

namespace Luna
{
    class Triangle final : public Game
    {
    private:
        Renderer* renderer;
        Mesh* geometry;

    public:
        void Init() override;
        void Update() override;
        void Finalize() override;
        void Display() override;

        void BuildGeometry();
    };
}