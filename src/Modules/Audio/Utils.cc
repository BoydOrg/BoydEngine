#include "Utils.hh"
#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"
#include <AL/al.h>
#include <fstream>
#include <memory>

using namespace boyd;
using namespace boyd::comp;
using namespace std::filesystem;
using namespace std;

#if BOYD_PLATFORM_WIN32
#    include <malloc.h>
#    include <winsock2.h>
#else
#    include <alloca.h>
#    include <netinet/in.h>
#endif

/// See http://soundfile.sapp.org/doc/WaveFormat/
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

void boyd::PrintOpenALError()
{
    ALenum error = alGetError();
    if(error != AL_NO_ERROR)
        BOYD_LOG(Warn, "{}", alGetString(error));
}

void boyd::PrintOpenALCError(ALCdevice *device)
{
    ALCenum error = alcGetError(device);
    if(error != ALC_NO_ERROR)
        BOYD_LOG(Error, "{}", alcGetString(device, error));
}

void boyd::LoadWav(const path &path, AudioSource &audioSource)
{
    ifstream reader{path, ios_base::binary | ios_base::in};

    WavHeader header;

    if(!reader)
    {
        BOYD_LOG(Warn, "Could not load the asset {}", path.string());
    }

    reader.read((char *)&header, sizeof(WavHeader));

    assert(header.ChunkID == htonl(0x52494646));     // "RIFF"
    assert(header.Format == htonl(0x57415645));      // "WAVE"
    assert(header.Subchunk1ID == htonl(0x666d7420)); // "fmt "

    /// Skip the extra parameters
    reader.seekg(2, reader.cur);

    while(header.Subchunk2ID != htonl(0x64617461)) // "data"
    {
        reader.read((char *)&header.Subchunk2ID, 4);
    }

    assert(header.Subchunk2ID == htonl(0x64617461)); // "data"
    reader.read((char *)&header.Subchunk2Size, 4);   // The data size

    ALenum format, error;

    switch(header.BitsPerSample)
    {
    case 8:
        switch(header.NumChannels)
        {
        case 1:
            format = AL_FORMAT_MONO8;
            break;
        case 2:
            format = AL_FORMAT_MONO16;
            break;
        default:
            BOYD_LOG(Warn, "OpenAL does not support more than 2 channels!");
            return;
        }
        break;
    case 16:
        switch(header.NumChannels)
        {
        case 1:
            format = AL_FORMAT_STEREO8;
            break;
        case 2:
            format = AL_FORMAT_STEREO16;
            break;
        default:
            BOYD_LOG(Warn, "OpenAL does not support more than 2 channels!");
            return;
        }
        break;
    default:
        BOYD_LOG(Warn, "Bit sample rate {} not supported!", header.BitsPerSample);
        return;
    }

    unique_ptr<uint8_t> data{(uint8_t *)malloc(header.Subchunk2Size)};
    reader.read((char *)data.get(), header.Subchunk2Size);

    alGenBuffers(1, &audioSource.alBuffer);
    PrintOpenALError();
    alBufferData(audioSource.alBuffer, format, data.get(),
                 header.Subchunk2Size, header.SampleRate);
    PrintOpenALError();

    alGenSources(1, &audioSource.alSource);
    PrintOpenALError();
    alSourcei(audioSource.alSource, AL_BUFFER, audioSource.alBuffer);
    PrintOpenALError();

    switch(audioSource.soundType)
    {
    case AudioSource::SoundType::SFX:
        alSourcei(audioSource.alSource, AL_LOOPING, AL_FALSE);
        break;
    case AudioSource::SoundType::SFX_LOOPABLE:
    case AudioSource::SoundType::BGM:
        alSourcei(audioSource.alSource, AL_LOOPING, AL_TRUE);
        break;
    }

    alSourcePlay(audioSource.alSource);
    PrintOpenALError();
}

void boyd::LoadFlac(const path &path, AudioSource &audioSource)
{
    BOYD_LOG(Error, "FLAC loading not yet implemented");
}