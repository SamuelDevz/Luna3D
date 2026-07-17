#include "All.h"
#include "Renderer.h"

namespace Luna
{
    class Triangle final : public Game
    {
    private:
        Renderer * renderer;

    public:
        void Init() override;
        void Update() override;
        void Finalize() override;
        void Display() override;

        void BuildGeometry();
    };
}