#include "Utils.hh"
#include "../../Core/Platform.hh"
#include "../../Debug/Log.hh"
#include <AL/al.h>
#include <FLAC/stream_decoder.h>
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

// ------------------------------------------------------------------------------------------------
/// Wav header structure (ish).
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

void boyd::PrintOpenALError(const char *file, int line)
{
    ALenum error = alGetError();
    if(error != AL_NO_ERROR)
        BOYD_LOG(Warn, "In {}:{} -> {}", alGetString(error));
}

void boyd::PrintOpenALCError(ALCdevice *device, const char *file, int line)
{
    ALCenum error = alcGetError(device);
    if(error != ALC_NO_ERROR)
        BOYD_LOG(Error, "{}", alcGetString(device, error));
}

ALenum GetFormat(int bitsPerSample, int channels)
{

    if(channels < 1 || channels >= 2)
    {
        BOYD_LOG(Warn, "OpenAL expected 1 or 2 channels, got {}", channels);
        return -1;
    }
    if(bitsPerSample != 8 && bitsPerSample != 16)
    {
        BOYD_LOG(Warn, "OpenAL expected a bps rate of 8 or 16, got {}", bitsPerSample);
        return -1;
    }

    if(bitsPerSample == 8 && channels == 1)
        return AL_FORMAT_MONO8;
    if(bitsPerSample == 16 && channels == 1)
        return AL_FORMAT_MONO16;
    if(bitsPerSample == 8 && channels == 2)
        return AL_FORMAT_STEREO8;
    if(bitsPerSample == 16 && channels == 2)
        return AL_FORMAT_STEREO16;

    // Unreached
    return -1;
}

/// Generate an OpenAL source from its buffer.
void GenSourceFromBuffer(AudioSource &audioSource)
{
    alGenSources(1, &audioSource.alSource);
    BOYD_OPENAL_ERROR();
    alSourcei(audioSource.alSource, AL_BUFFER, audioSource.alBuffer);
    BOYD_OPENAL_ERROR();

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
    BOYD_OPENAL_ERROR();
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

    /// Check the WAV format has the right number of channels and bits per sample
    format = GetFormat(header.BitsPerSample, header.NumChannels);

    if(format == -1)
        return;

    unique_ptr<uint8_t> data{(uint8_t *)malloc(header.Subchunk2Size)};
    reader.read((char *)data.get(), header.Subchunk2Size);

    alGenBuffers(1, &audioSource.alBuffer);
    BOYD_OPENAL_ERROR();
    alBufferData(audioSource.alBuffer, format, data.get(),
                 header.Subchunk2Size, header.SampleRate);
    BOYD_OPENAL_ERROR();

    GenSourceFromBuffer(audioSource);
}

// ------------------------------------------------------------------------------------------------

/// Copy the buffer from the decoded FLAC stream to an OpenAL buffer
/// `decoder` - a FLAC decoder
/// `frame` - a FLAC frame
/// `buffer` - an uncompressed PCM buffer
/// `client_data` - a data structure that acts as a sink
FLAC__StreamDecoderWriteStatus writeCallback(const FLAC__StreamDecoder *decoder,
                                             const FLAC__Frame *frame,
                                             const FLAC__int32 *const buffer[],
                                             void *client_data);

void errorCallback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
    (void)decoder, (void)client_data;

    BOYD_LOG(Error, "Got error callback: {}", FLAC__StreamDecoderErrorStatusString[status]);
}

void boyd::LoadFlac(const path &path, AudioSource &audioSource)
{
    FLAC__StreamDecoder *decoder = nullptr;

    if(!(decoder = FLAC__stream_decoder_new()))
    {
        BOYD_LOG(Error, "Could not allocate a FLAC decoder");
        return;
    }

    (void)FLAC__stream_decoder_set_md5_checking(decoder, true);

    auto init_status = FLAC__stream_decoder_init_file(decoder, path.c_str(), writeCallback, nullptr, errorCallback, &audioSource);

    if(init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK)
    {
        BOYD_LOG(Error, "Failed to initialize an audio stream: {}", FLAC__StreamDecoderInitStatusString[init_status]);
        return;
    }
    bool ok = FLAC__stream_decoder_process_until_end_of_stream(decoder);
    BOYD_LOG(Debug, "Decoding {}: {}", ok ? "succeeded" : "FAILED");
    BOYD_LOG(Debug, "   state: {}", FLAC__StreamDecoderStateString[FLAC__stream_decoder_get_state(decoder)]);

    FLAC__stream_decoder_delete(decoder);

    GenSourceFromBuffer(audioSource);
}

FLAC__StreamDecoderWriteStatus writeCallback(const FLAC__StreamDecoder *decoder,
                                             const FLAC__Frame *frame,
                                             const FLAC__int32 *const buffer[],
                                             void *client_data)
{
    AudioSource *audioSource = (AudioSource *)client_data;

    if(!frame)
    {
        BOYD_LOG(Error, "Cannot read the header of this flac file.");
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }

    int channels = frame->header.channels;
    int bitsPerSample = frame->header.bits_per_sample;
    int totalSamples = frame->header.number.sample_number;

    ALenum format = GetFormat(bitsPerSample, channels);

    const FLAC__uint32 total_size = (FLAC__uint32)(totalSamples * channels * (bitsPerSample / 8));
    const size_t bytesPerSampleMono = bitsPerSample / 8;

    bool ok = true;
    if(format == -1)
    {
        BOYD_LOG(Error, "Unrecognized format, aborting");
        ok = false;
    }
    else if(!buffer[0])
    {
        BOYD_LOG(Error, "Null buffer[0] detected");
        ok = false;
    }
    else if(channels == 2 && !buffer[1])
    {
        BOYD_LOG(Error, "Null buffer[1] detected");
        ok = false;
    }

    if(!ok)
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

    unique_ptr<uint8_t> data{new uint8_t[total_size]};

    // Iterate over all frames
    for(size_t i = 0; i < frame->header.blocksize; i++)
    {
        for(size_t channel = 0; channel < channels; channel++)
            memcpy(data.get() + i * bytesPerSampleMono * (channel + 1), &buffer[channel][i], bytesPerSampleMono);
    }

    alGenBuffers(1, &audioSource->alBuffer);
    BOYD_OPENAL_ERROR();
    alBufferData(audioSource->alBuffer, format, data.get(),
                 total_size, frame->header.sample_rate);
    BOYD_OPENAL_ERROR();

    GenSourceFromBuffer(*audioSource);

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}