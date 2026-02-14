#pragma once

#include "All.h"
#include "Mesh.h"
#include <glm/glm.hpp>

namespace Luna
{
    using Position = glm::vec3;
    using Color = glm::vec4;

    struct Vertex
    {
        Position position;
        Color color;
    };
    
    class Renderer final
    {
    private:
        Graphics                * graphics;
        Mesh                    * geometry;

        VkPipeline                pipeline;
        VkPipelineLayout          pipelineLayout;

    public:
        explicit Renderer() noexcept;
        ~Renderer();

        void Initialize(Graphics * graphics, Mesh * geometry);
        VkPipeline Pipeline() const noexcept;
    };

    inline VkPipeline Renderer::Pipeline() const noexcept
    { return pipeline; }
}