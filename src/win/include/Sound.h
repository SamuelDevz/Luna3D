#pragma once

#include "Types.h"
#include <XAudio2.h>

namespace Luna
{
    class Sound
    {
    private:
        WAVEFORMATEXTENSIBLE format;
        XAUDIO2_BUFFER       buffer;
        float                volume;
        float                frequency;

        IXAudio2SourceVoice  ** voices;
        uint32               tracks;
        uint32               index;

        HRESULT FindChunk(HANDLE hFile,
            DWORD fourcc,
            DWORD & dwChunkSize,
            DWORD & dwChunkDataPosition) noexcept;

        HRESULT ReadChunkData(HANDLE hFile,
            void * buffer,
            DWORD buffersize,
            DWORD bufferoffset) noexcept;

        friend class Audio;

    public:
        Sound(const string_view fileName, const uint32 nTracks) noexcept;
        ~Sound() noexcept;
    };
}