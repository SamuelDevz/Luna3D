#include "All.h"

namespace Luna
{
    class WinGame final : public Game
    {
    public:
        void Init() override;
        void Update() override;
        void Finalize() override;
        void Display() override;
    };
}