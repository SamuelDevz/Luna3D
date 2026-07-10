#include "Sound.h"

#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT  ' tmf'
#define fourccWAVE 'EVAW'

namespace Luna
{
    Sound::Sound(const string_view fileName, const uint32 nTracks) noexcept
        : format{},
        buffer{},
        volume{1.0f},
        frequency{1.0f},
        tracks{nTracks},
        index{}
    {
        voices = new IXAudio2SourceVoice*[tracks] { nullptr };

        HANDLE hFile = CreateFile(fileName.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);

        // search the block 'RIFF' in the sound file
        DWORD dwChunkSize{};
        DWORD dwChunkPosition{};
        FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);

        // check if the type of the file is WAVE
        DWORD filetype{};
        ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
        if (filetype != fourccWAVE)
            return;

        // read the block 'fmt' and copy your content for a struct WAVEFORMATEXTENSIBLE
        FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
        ReadChunkData(hFile, &format, dwChunkSize, dwChunkPosition);

        // find the block 'data' e read your content for the struct XAUDIO2_BUFFER
        FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
        BYTE * pDataBuffer = new BYTE[dwChunkSize];
        ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

        buffer.AudioBytes = dwChunkSize;
        buffer.pAudioData = pDataBuffer;
        buffer.Flags = XAUDIO2_END_OF_STREAM;

        CloseHandle(hFile);
    }

    Sound::~Sound() noexcept
    {
        delete[] buffer.pAudioData;
        delete[] voices;
    }

    HRESULT Sound::FindChunk(HANDLE hFile, DWORD fourcc, DWORD & dwChunkSize, DWORD & dwChunkDataPosition) noexcept
    {
        HRESULT hr = S_OK;
        if(SetFilePointer(hFile, 0, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
            return HRESULT_FROM_WIN32( GetLastError() );

        DWORD dwChunkType;
        DWORD dwChunkDataSize;
        DWORD dwRIFFDataSize = 0;
        DWORD dwFileType;
        DWORD bytesRead = 0;
        DWORD dwOffset = 0;

        while (hr == S_OK)
        {
            DWORD dwRead;
            if(ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, nullptr) == 0)
                hr = HRESULT_FROM_WIN32(GetLastError());

            if(ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, nullptr) == 0)
                hr = HRESULT_FROM_WIN32(GetLastError());

            switch (dwChunkType)
            {
            case fourccRIFF:
                dwRIFFDataSize = dwChunkDataSize;
                dwChunkDataSize = 4;
                if(ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, nullptr ) == 0)
                    hr = HRESULT_FROM_WIN32( GetLastError() );
                break;

            default:
                if(SetFilePointer(hFile, dwChunkDataSize, nullptr, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
                    return HRESULT_FROM_WIN32( GetLastError() );
            }

            dwOffset += sizeof(DWORD) * 2;

            if (dwChunkType == fourcc)
            {
                dwChunkSize = dwChunkDataSize;
                dwChunkDataPosition = dwOffset;
                return S_OK;
            }

            dwOffset += dwChunkDataSize;

            if (bytesRead >= dwRIFFDataSize)
                return S_FALSE;
        }

        return S_OK;
    }

    HRESULT Sound::ReadChunkData(HANDLE hFile, void * buffer, DWORD buffersize, DWORD bufferoffset) noexcept
    {
        if(SetFilePointer(hFile, bufferoffset, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
            return HRESULT_FROM_WIN32(GetLastError());

        DWORD dwRead{};
        if(ReadFile(hFile, buffer, buffersize, &dwRead, nullptr) == 0)
            return HRESULT_FROM_WIN32(GetLastError());

        return S_OK;
    }
}