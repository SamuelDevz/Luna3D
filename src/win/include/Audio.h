#pragma once

#include "Sound.h"
#include "Types.h"
#include <XAudio2.h>
#include <unordered_map>
using std::unordered_map;

namespace Luna
{
    class Audio
    {
    private:
        IXAudio2* audioEngine;
        IXAudio2MasteringVoice* masterVoice;
        unordered_map<uint32, Sound*> soundTable;

    public:
        Audio() noexcept;
        ~Audio() noexcept;

        void Add(const uint32 id, const string_view filename, const uint32 nVoices = 1) noexcept;
        void Play(const uint32 id, const bool repeat = false) noexcept;
        void Stop(const uint32 id) noexcept;
        void Volume(const uint32 id, float level) noexcept;
        void Frequency(const uint32 id, float level) noexcept;
    };
}