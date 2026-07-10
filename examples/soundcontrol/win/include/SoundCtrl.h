#include "All.h"

namespace Luna
{
    class SoundCtrl final : public Game
    {
    private:
        Audio * audio;
        float volume;
        float frequency;

    public:
        void Init() override;
        void Update() override;
        void Finalize() override;
        void Display() override;
    };
}