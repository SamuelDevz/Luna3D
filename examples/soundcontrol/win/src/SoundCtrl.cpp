#include "SoundCtrl.h"

namespace Luna
{
    enum Sounds { INTRO, PULSE, EXPLOSION };

    void SoundCtrl::Init()
    {
        audio = nullptr;
        volume = 1.0f;
        frequency = 1.0f;

        audio = new Audio();
        audio->Add(INTRO, "resources/Intro.wav");
        audio->Add(PULSE, "resources/Pulse.wav", 3);
        audio->Add(EXPLOSION, "resources/Explosion.wav", 3);

        audio->Play(INTRO, true);
    }

    void SoundCtrl::Update()
    {
        if (input->KeyDown(VK_ESCAPE))
            window->Close();

        if(input->KeyPress(VK_SPACE))
            audio->Play(PULSE);

        if(input->KeyPress(VK_RETURN))
            audio->Play(EXPLOSION);

        if (input->KeyDown(VK_UP))
        {
            volume += 0.5f * frameTime;
            if (volume > 2)
                volume = 2;
        }

        if (input->KeyDown(VK_DOWN))
        {
            volume -= 0.5f * frameTime;
            if (volume < 0)
                volume = 0;
        }

        if (input->KeyDown(VK_RIGHT))
        {
            frequency += 0.5f * frameTime;
            if (frequency > 2)
                frequency = 2;
        }

        if (input->KeyDown(VK_LEFT))
        {
            frequency -= 0.5f * frameTime;
            if (frequency < 0)
                frequency = 0;
        }

        if (input->KeyDown(VK_R))
        {
            volume = 1.0f;
            frequency = 1.0f;
        }

        audio->Frequency(INTRO, frequency);
        audio->Volume(INTRO, volume);
    }

    void SoundCtrl::Display()
    {
    }

    void SoundCtrl::Finalize()
    {
        delete audio;
    }
}