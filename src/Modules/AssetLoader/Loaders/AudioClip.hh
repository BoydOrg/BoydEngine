#pragma once

#include "../../../Components/AudioClip.hh"
#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <memory>

#ifdef BOYD_PLATFORM_WIN32
#    include <malloc.h>
#    include <winsock2.h>
#else
#    include <alloca.h>
#    include <netinet/in.h>
#endif

namespace boyd
{

// ------------------------------------------------------------------------------------------------
/// Wav header structure (ish).
/// See http://soundfile.sapp.org/doc/WaveFormat/
/// However, this does not include the LIST chunk, which makes things spicier
/// So we basically ignore the subchunk1size parameter and always skip 2 + multiples of 4
/// until we get "data"
struct WavHeader
{
    uint32_t ChunkID;
    uint32_t ChunkSize;
    uint32_t Format;
    uint32_t Subchunk1ID;
    uint32_t Subchunk1Size;
    uint16_t AudioFormat;
    uint16_t NumChannels;
    uint32_t SampleRate;
    uint32_t ByteRate;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;
    uint32_t Subchunk2ID;
    uint32_t Subchunk2Size;
};

template <>
struct Loader<comp::AudioClip>
{
    static std::unique_ptr<LoadedAssetBase> Load(std::string filepath)
    {
        WavHeader header;

        Wave result;
        std::ifstream reader{filepath, std::ios_base::binary | std::ios_base::in};

        if(!reader)
        {
            BOYD_LOG(Warn, "Could not load the asset {}", filepath);
            return nullptr;
        }

        // Parse the header...
        reader.read((char *)&header, sizeof(WavHeader));

        assert(header.ChunkID == htonl(0x52494646));     // "RIFF"
        assert(header.Format == htonl(0x57415645));      // "WAVE"
        assert(header.Subchunk1ID == htonl(0x666d7420)); // "fmt "

        /// ... skip the extra parameters ...
        reader.seekg(2, reader.cur);
        bool skipped = false;
        while(header.Subchunk2ID != htonl(0x64617461)) // "data"
        {
            skipped = true;
            reader.read((char *)&header.Subchunk2ID, 4);
        }
        assert(header.Subchunk2ID == htonl(0x64617461)); // "data"
        if(skipped)
            reader.read((char *)&header.Subchunk2Size, 4); // The data size

        /// ... and fill the buffer
        result.data.reset(new uint8_t[header.Subchunk2Size]);
        reader.read((char *)result.data.get(), header.Subchunk2Size);

        result.channels = header.NumChannels;
        result.sampleRate = header.SampleRate;
        result.sampleSize = header.BitsPerSample;
        result.sampleCount = header.Subchunk2Size / (header.NumChannels * header.BitsPerSample / 8);

        return std::make_unique<LoadedAsset<comp::AudioClip>>(std::move(result));
    }
};
} // namespace boyd