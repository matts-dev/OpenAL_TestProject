
#include <AL/al.h>
#include <AL/alc.h>
#include <AudioFile/AudioFile.h>
#include <iostream>

//OpenAL Error Check
#define OpenAL_ErrorCheck(MSG)\
{\
	ALenum error = alGetError();\
	if (error != AL_NO_ERROR)\
	{\
		std::cerr << "OpenAL error : " << error << "with call for " << #MSG << std::endl;\
	}\
}

#define alec(FUNCTION_CALL)\
FUNCTION_CALL;\
OpenAL_ErrorCheck(FUNCTION_CALL)

#define ENABLE_SAMPLE_CODE_MAIN 0
#if ENABLE_SAMPLE_CODE_MAIN
int main()
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//find out the default audio device
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const ALCchar* defaultDeviceString = alcGetString(/*device*/NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
	ALCdevice* device = alcOpenDevice(defaultDeviceString);
	if (!device)
	{
		std::cerr << "failed to find device" << std::endl;
		return -1;
	}

	std::cout << "OpenAL device: " << alcGetString(device, ALC_DEVICE_SPECIFIER) << std::endl;
	OpenAL_ErrorCheck(device);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//create an OpenAL audio context from the device; i'd imagine there could be multiple contexts for different players
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ALCcontext* context = alcCreateContext(device, /*attrlist*/nullptr);
	OpenAL_ErrorCheck(context);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//active this context so that OpenAL state modifications are applied to the context
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (!alcMakeContextCurrent(context))
	{
		std::cerr << "failed to make context active" << std::endl;
		return -1;
	}
	OpenAL_ErrorCheck("Make Context Active");
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//create a listener in 3d space (ie the player!)
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	alec(alListener3f(AL_POSITION, 0.f, 0.f, 0.f));
	alec(alListener3f(AL_VELOCITY, 0, 0, 0));
	ALfloat frontAndUpVectors[] = {
		/* front*/1.f, 0.f, 0.f,
		/* up */ 0.f, 1.f, 0.f
	};
	alec(alListenerfv(AL_ORIENTATION, frontAndUpVectors));

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//create buffers that hold our sound data; these are shared between contexts as buffers are defined on the device level
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	AudioFile<float> monoSoundFile;
	if (!monoSoundFile.load("sounds/TestSound_Mono.wav"))
	{
		std::cerr << "failed to load test mono sound" << std::endl;
	}
	std::vector<uint8_t> monoPCMData;
	monoSoundFile.writePCMToBuffer(monoPCMData);
	auto getALSoundFormat = [](AudioFile<float>& audioFile) 
	{
		int bitDepth = audioFile.getBitDepth();
		if (bitDepth == 16)
			return audioFile.isStereo() ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
		else if (bitDepth == 8)
			return audioFile.isStereo() ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
		else
			return -1;
	};
	ALuint monoSoundBuffer;
	alec(alGenBuffers(1, &monoSoundBuffer));
	alec(alBufferData(monoSoundBuffer, getALSoundFormat(monoSoundFile), monoPCMData.data(), monoPCMData.size(), monoSoundFile.getSampleRate()));

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//load a stereo sound into a buffer
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	AudioFile<float> stereoFile;
	if (!stereoFile.load("sounds/TestSound.wav"))
	{
		std::cerr << "failed to load test stereo sound" << std::endl;
	}
	std::vector<uint8_t> stereoPCMBytes;
	stereoFile.writePCMToBuffer(stereoPCMBytes);
	ALuint stereoSoundBuffer;
	alec(alGenBuffers(1, &stereoSoundBuffer));
	alec(alBufferData(stereoSoundBuffer, getALSoundFormat(stereoFile), stereoPCMBytes.data(), stereoPCMBytes.size(), stereoFile.getSampleRate()));
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//create a sound source for a mono sound
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ALuint monoSource;
	alec(alGenSources(1, &monoSource));
	alec(alSource3f(monoSource, AL_POSITION, 1.f, 0.f, 0.f)); //move this around to hear it come out of different ears
	alec(alSource3f(monoSource, AL_VELOCITY, 0.f, 0.f, 0.f));
	alec(alSourcef(monoSource, AL_PITCH, 1.f));
	alec(alSourcef(monoSource, AL_GAIN, 1.f));
	alec(alSourcei(monoSource, AL_LOOPING, AL_FALSE));
	alec(alSourcei(monoSource, AL_BUFFER, monoSoundBuffer));

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//create a sound source for a stereo sound
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ALuint stereoSource;
	alec(alGenSources(1, &stereoSource));
	alec(alSource3f(stereoSource, AL_POSITION, 0.f, 0.f, 1.f)); //this is irrelevant for stereo, because stereo isn't 3d virtualized; stereo is for music.
	alec(alSource3f(stereoSource, AL_VELOCITY, 0.f, 0.f, 0.f));
	alec(alSourcef(stereoSource, AL_PITCH, 1.f));
	alec(alSourcef(stereoSource, AL_GAIN, 1.f));
	alec(alSourcei(stereoSource, AL_LOOPING, AL_FALSE));
	alec(alSourcei(stereoSource, AL_BUFFER, stereoSoundBuffer));
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//test playing the mono sound
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	alec(alSourcePlay(monoSource));
	ALint sourceState;
	alec(alGetSourcei(monoSource, AL_SOURCE_STATE, &sourceState))
	while (sourceState == AL_PLAYING)
	{
		alec(alGetSourcei(monoSource, AL_SOURCE_STATE, &sourceState))
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//test playing the stereo sound
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	alec(alSourcePlay(stereoSource));
	alec(alGetSourcei(stereoSource, AL_SOURCE_STATE, &sourceState))
	while (sourceState == AL_PLAYING)
	{
		alec(alGetSourcei(stereoSource, AL_SOURCE_STATE, &sourceState))
	}

	alec(alDeleteSources(1, &stereoSource))
	alec(alDeleteSources(1, &monoSource))
	alec(alDeleteBuffers(1, &monoSoundBuffer));
	alec(alDeleteBuffers(1, &stereoSoundBuffer));
	alec(alcMakeContextCurrent(nullptr));
	alec(alcDestroyContext(context));
	alec(alcCloseDevice(device));
}
#endif