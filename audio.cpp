#include <malloc.h>
#include <Windows.h>
#include <stdio.h>

#include <AL\al.h>
#include <AL\alc.h>


static void list_audio_devices(const ALCchar *devices)
{
	const ALCchar *device = devices, *next = devices + 1;
	size_t len = 0;

	fprintf(stdout, "Devices list:\n");
	fprintf(stdout, "----------\n");
	while (device && *device != '\0' && next && *next != '\0') {
		fprintf(stdout, "%s\n", device);
		len = strlen(device);
		device += (len + 1);
		next += (len + 2);
	}
	fprintf(stdout, "----------\n");
}

typedef const ALCchar* (ALC_APIENTRY *alcGetString_t)(ALCdevice *device, ALCenum param);
typedef ALC_API ALCdevice* (ALC_APIENTRY *alcOpenDevice_t)(const ALCchar *devicename);
typedef ALC_API ALCcontext* (ALC_APIENTRY *alcCreateContext_t)(ALCdevice *device, const ALCint* attrlist);
typedef ALC_API ALCboolean  (ALC_APIENTRY *alcMakeContextCurrent_t)(ALCcontext *context);

typedef AL_API void (AL_APIENTRY *alListenerfv_t)(ALenum param, const ALfloat *values);
typedef AL_API void (AL_APIENTRY *alListener3f_t)(ALenum param, ALfloat value1, ALfloat value2, ALfloat value3);
typedef AL_API void (AL_APIENTRY *alListeneriv_t)(ALenum param, const ALint *values);

typedef AL_API void (AL_APIENTRY *alGenSources_t)(ALsizei n, ALuint *sources);
typedef AL_API void (AL_APIENTRY *alSourcef_t)(ALuint source, ALenum param, ALfloat value);
typedef AL_API void (AL_APIENTRY *alSource3f_t)(ALuint source, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3);
typedef AL_API void (AL_APIENTRY *alSourcei_t)(ALuint source, ALenum param, ALint value);
typedef AL_API void (AL_APIENTRY *alSourcePlay_t)(ALuint source);

typedef AL_API void (AL_APIENTRY *alGetSourcef_t)(ALuint source, ALenum param, ALfloat *value);
typedef AL_API void (AL_APIENTRY *alGetSource3f_t)(ALuint source, ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3);
typedef AL_API void (AL_APIENTRY *alGetSourcefv_t)(ALuint source, ALenum param, ALfloat *values);
typedef AL_API void (AL_APIENTRY *alGetSourcei_t)(ALuint source,  ALenum param, ALint *value);
typedef AL_API void (AL_APIENTRY *alGetSource3i_t)(ALuint source, ALenum param, ALint *value1, ALint *value2, ALint *value3);
typedef AL_API void (AL_APIENTRY *alGetSourceiv_t)(ALuint source,  ALenum param, ALint *values);

typedef AL_API void (AL_APIENTRY *alGenBuffers_t)(ALsizei n, ALuint *buffers);
typedef AL_API void (AL_APIENTRY *alBufferData_t)(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq);


alcGetString_t _alcGetString = NULL;
alcOpenDevice_t _alcOpenDevice = NULL;
alcCreateContext_t _alcCreateContext = NULL;
alcMakeContextCurrent_t _alcMakeContextCurrent = NULL;

alListenerfv_t _alListenerfv = NULL;
alListener3f_t _alListener3f = NULL;
alListeneriv_t _alListeneriv = NULL;

alGenSources_t _alGenSources = NULL;
alSourcef_t _alSourcef = NULL;
alSource3f_t _alSource3f = NULL;
alSourcei_t _alSourcei = NULL;
alSourcePlay_t _alSourcePlay = NULL;
alGetSourcef_t _alGetSourcef = NULL;
alGetSource3f_t _alGetSource3f = NULL;
alGetSourcefv_t _alGetSourcefv = NULL;
alGetSourcei_t _alGetSourcei = NULL;
alGetSource3i_t _alGetSource3i = NULL;
alGetSourceiv_t _alGetSourceiv = NULL;

alGenBuffers_t _alGenBuffers = NULL;
alBufferData_t _alBufferData = NULL;

void InitOpenAl()
{
	HMODULE hOpenAl = LoadLibrary( "soft_oal32.dll" );
	_alcGetString = (alcGetString_t)GetProcAddress( hOpenAl, "alcGetString" );
	_alcOpenDevice = (alcOpenDevice_t)GetProcAddress( hOpenAl, "alcOpenDevice" );
	_alcCreateContext = (alcCreateContext_t)GetProcAddress( hOpenAl, "alcCreateContext" );
	_alcMakeContextCurrent = (alcMakeContextCurrent_t)GetProcAddress( hOpenAl, "alcMakeContextCurrent" );

	_alListenerfv = (alListenerfv_t)GetProcAddress( hOpenAl, "alListenerfv" );
	_alListener3f = (alListener3f_t)GetProcAddress( hOpenAl, "alListener3f" );
	_alListeneriv = (alListeneriv_t)GetProcAddress( hOpenAl, "alListeneriv" );

	_alGenSources = (alGenSources_t)GetProcAddress( hOpenAl, "alGenSources" );
	_alSourcef = (alSourcef_t)GetProcAddress( hOpenAl, "alSourcef" );
	_alSource3f = (alSource3f_t)GetProcAddress( hOpenAl, "alSource3f" );
	_alSourcei = (alSourcei_t)GetProcAddress( hOpenAl, "alSourcei" );
	_alSourcePlay = (alSourcePlay_t)GetProcAddress( hOpenAl, "alSourcePlay" );
	_alGetSourcef = (alGetSourcef_t)GetProcAddress( hOpenAl, "alGetSourcef" );
	_alGetSource3f = (alGetSource3f_t)GetProcAddress( hOpenAl, "alGetSource3f" );
	_alGetSourcefv = (alGetSourcefv_t)GetProcAddress( hOpenAl, "alGetSourcefv" );
	_alGetSourcei = (alGetSourcei_t)GetProcAddress( hOpenAl, "alGetSourcei" );
	_alGetSource3i = (alGetSource3i_t)GetProcAddress( hOpenAl, "alGetSource3i" );
	_alGetSourceiv = (alGetSourceiv_t)GetProcAddress( hOpenAl, "alGetSourceiv" );

	_alGenBuffers = (alGenBuffers_t)GetProcAddress( hOpenAl, "alGenBuffers" );
	_alBufferData = (alBufferData_t)GetProcAddress( hOpenAl, "alBufferData" );
}

#pragma pack(1)
typedef struct __WAVEDESCR
{
    BYTE riff[4];
    DWORD size;
    BYTE wave[4];
 
} _WAVEDESCR, *_LPWAVEDESCR;
 
typedef struct __WAVEFORMAT
{
    BYTE id[4];
    DWORD size;
    SHORT format;
    SHORT channels;
    DWORD sampleRate;
    DWORD byteRate;
    SHORT blockAlign;
    SHORT bitsPerSample;
 
} _WAVEFORMAT, *_LPWAVEFORMAT;
#pragma pack()
 
void* LoadWav(const char *lpszFilePath, int &channels, int &sampleRate, int &bitsPerSample, int &wavsize )
{
    bool bResult = false;
    LPBYTE lpData = NULL;
	wavsize = 0;
    // Load .WAV file
     _WAVEDESCR descriptor;
     _WAVEFORMAT format;
    FILE* file = fopen(lpszFilePath, ("rb"));
    if (file != NULL)
    {
        // Read .WAV descriptor
        fread(&descriptor, sizeof(_WAVEDESCR), 1, file);

        // Check for valid .WAV file
        if (strncmp((LPCSTR)descriptor.wave, "WAVE", 4) == 0)
        {
            // Read .WAV format
            fread(&format, sizeof(_WAVEFORMAT), 1, file);

            // Check for valid .WAV file
            if ((strncmp((LPCSTR)format.id, "fmt", 3) == 0) && (format.format == 1))
            {
				channels = format.channels;
				sampleRate = format.sampleRate;
				bitsPerSample = format.bitsPerSample;

                // Read next chunk
                BYTE id[4];
                DWORD size;
                fread(id, sizeof(BYTE), 4, file);
                fread(&size, sizeof(DWORD), 1, file);
                DWORD offset = ftell(file);

                // Read .WAV data
                while (offset < descriptor.size)
                {
                    // Check for .WAV data chunk
                    if (strncmp((LPCSTR)id, "data", 4) == 0)
                    {
                        lpData = (LPBYTE)realloc(lpData, (wavsize+size)*sizeof(BYTE));
                        fread(lpData+wavsize, sizeof(BYTE), size, file);
                        wavsize += size;
                    }
                    else
					{
						fseek( file, size, SEEK_CUR );
					}

                    // Read next chunk
                    fread(id, sizeof(BYTE), 4, file);
                    fread(&size, sizeof(DWORD), 1, file);
                    offset = ftell(file);
                }
            }
        }

        // Close .WAV file
        fclose(file);
    }

    return lpData;
}


void AudioTest()
{
	int ch, sr, bps, wavsize;
	void *wavdata = LoadWav( "blowraspberry.wav", ch, sr, bps, wavsize );

	list_audio_devices(_alcGetString(NULL, ALC_DEVICE_SPECIFIER));

	const ALCchar *defaultDeviceName = _alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
	ALCdevice *device = _alcOpenDevice(defaultDeviceName);

	ALCcontext *context = _alcCreateContext(device, NULL);
	_alcMakeContextCurrent(context);

	_alListener3f(AL_POSITION, 0, 0, 1.0f);
   	_alListener3f(AL_VELOCITY, 0, 0, 0);
	
	ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
	ALuint buffer, source;
	
	_alListenerfv(AL_ORIENTATION, listenerOri);
	_alGenSources((ALuint)1, &source);
	_alSourcef(source, AL_PITCH, 1);
	_alSourcef(source, AL_GAIN, 1);
	_alSource3f(source, AL_POSITION, 0, 0, 0);
	_alSource3f(source, AL_VELOCITY, 0, 0, 0);
	_alSourcei(source, AL_LOOPING, AL_FALSE);
	_alGenBuffers(1, &buffer);
	_alBufferData(buffer, AL_FORMAT_MONO8, wavdata, wavsize, sr);

	ALint source_state;
	_alSourcei(source, AL_BUFFER, buffer);
	_alSourcePlay(source);
	_alGetSourcei(source, AL_SOURCE_STATE, &source_state);
	while (source_state == AL_PLAYING) {
		_alGetSourcei(source, AL_SOURCE_STATE, &source_state);
	}
}



