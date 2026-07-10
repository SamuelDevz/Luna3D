#include "Audio.h"
#include "Sound.h"
#include <memory>

namespace Luna
{
    Audio::Audio() noexcept
        : audioEngine{nullptr},
        masterVoice{nullptr}
    {
        if (SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
            if (SUCCEEDED(XAudio2Create(&audioEngine, 0, XAUDIO2_DEFAULT_PROCESSOR)))
                audioEngine->CreateMasteringVoice(&masterVoice);
    }

    Audio::~Audio() noexcept
    {
        for (const auto & [id, sound] : soundTable)
        {
            for (size_t k = 0; k < sound->tracks; ++k)
                sound->voices[k]->DestroyVoice();
            delete sound;
        }

        if (masterVoice) masterVoice->DestroyVoice();
        if (audioEngine) audioEngine->Release();

        CoUninitialize();
    }

    void Audio::Add(const uint32 id, const string_view filename, const uint32 nVoices) noexcept
    {
        Sound * sound = new Sound(filename, nVoices);

        for (size_t i = 0; i < nVoices; ++i)
            audioEngine->CreateSourceVoice(&sound->voices[i], reinterpret_cast<WAVEFORMATEX*>(&sound->format));

        soundTable[id] = sound;
    }

    void Audio::Play(const uint32 id, const bool repeat) noexcept
    {
        Sound * selected = soundTable[id];

        selected->buffer.LoopCount = (repeat) ? XAUDIO2_LOOP_INFINITE : 0;

        selected->voices[selected->index]->Stop();
        selected->voices[selected->index]->FlushSourceBuffers();
        selected->voices[selected->index]->SetVolume(selected->volume);
        selected->voices[selected->index]->SubmitSourceBuffer(&selected->buffer);
        selected->voices[selected->index]->Start();

        selected->index = (selected->index + 1) % selected->tracks;
    }

    void Audio::Stop(const uint32 id) noexcept
    {
        Sound * selected = soundTable[id];

        for (size_t i = 0; i < selected->tracks; ++i)
            selected->voices[i]->Stop();
    }

    void Audio::Volume(const uint32 id, float level) noexcept
    {
        if (level < 0)
            level = 0;

        if (level > XAUDIO2_MAX_VOLUME_LEVEL)
            level = XAUDIO2_MAX_VOLUME_LEVEL;

        Sound * selected = soundTable[id];

        selected->volume = level;
        selected->voices[selected->index]->SetVolume(level);
    }

    void Audio::Frequency(const uint32 id, float level) noexcept
    {
        if (level < XAUDIO2_MIN_FREQ_RATIO)
            level = XAUDIO2_MIN_FREQ_RATIO;

        Sound * selected = soundTable[id];

        selected->frequency = level;
        selected->voices[selected->index]->SetFrequencyRatio(level);
    }
}