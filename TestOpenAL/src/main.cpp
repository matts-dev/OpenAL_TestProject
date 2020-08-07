
#include<iostream>
#include<AL/al.h>
#include <AL/alc.h>
#include<AudioFile/AudioFile.h>

//OpenAL error checking
#define OpenAL_ErrorCheck(message)\
{\
	ALenum error = alGetError();\
	if( error != AL_NO_ERROR)\
	{\
		std::cerr << "OpenAL Error: " << error << " with call for " << #message << std::endl;\
	}\
}

#define alec(FUNCTION_CALL)\
FUNCTION_CALL;\
OpenAL_ErrorCheck(FUNCTION_CALL)


int main()
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// find the default audio device
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const ALCchar* defaultDeviceString = alcGetString(/*device*/nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);
	ALCdevice* device = alcOpenDevice(defaultDeviceString);
	if (!device)
	{
		std::cerr << "failed to get the default device for OpenAL" << std::endl;
		return -1;
	}
	std::cout << "OpenAL Device: " << alcGetString(device, ALC_DEVICE_SPECIFIER) << std::endl;
	OpenAL_ErrorCheck(device);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create an OpenAL audio context from the device
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ALCcontext* context = alcCreateContext(device, /*attrlist*/ nullptr);
	OpenAL_ErrorCheck(context);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Activate this context so that OpenAL state modifications are applied to the context
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (!alcMakeContextCurrent(context))
	{
		std::cerr << "failed to make the OpenAL context the current context" << std::endl;
		return -1;
	}
	OpenAL_ErrorCheck("Make context current");

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create a listener in 3d space (ie the player); (there always exists as listener, you just configure data on it)
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	alec(alListener3f(AL_POSITION, 0.f, 0.f, 0.f));
	alec(alListener3f(AL_VELOCITY, 0.f, 0.f, 0.f));
	ALfloat forwardAndUpVectors[] = {
		/*forward = */ 1.f, 0.f, 0.f,
		/* up = */ 0.f, 1.f, 0.f
	};
	alec(alListenerfv(AL_ORIENTATION, forwardAndUpVectors));

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create buffers that hold our sound data; these are shared between contexts and ar defined at a device level
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	AudioFile<float> monoSoundFile;
	if (!monoSoundFile.load("sounds/TestSound_Mono.wav"))
	{
		std::cerr << "failed to load the test mono sound file" << std::endl;
		return -1;
	}
	std::vector<uint8_t> monoPCMDataBytes;
	monoSoundFile.writePCMToBuffer(monoPCMDataBytes); //remember, we added this function to the AudioFile library
	auto convertFileToOpenALFormat = [](const AudioFile<float>& audioFile) {
		int bitDepth = audioFile.getBitDepth();
		if (bitDepth == 16)
			return audioFile.isStereo() ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
		else if (bitDepth == 8)
			return audioFile.isStereo() ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
		else
			return -1; // this shouldn't happen!
	};
	ALuint monoSoundBuffer;
	alec(alGenBuffers(1, &monoSoundBuffer));
	alec(alBufferData(monoSoundBuffer, convertFileToOpenALFormat(monoSoundFile), monoPCMDataBytes.data(), monoPCMDataBytes.size(), monoSoundFile.getSampleRate()));

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// load a stereo file into a buffer
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	AudioFile<float> stereoSoundFile;
	if (!stereoSoundFile.load("sounds/TestSound.wav"))
	{
		std::cerr << "failed to load the test stereo sound file" << std::endl;
		return -1;
	}
	std::vector<uint8_t> stereoPCMDataBytes;
	stereoSoundFile.writePCMToBuffer(stereoPCMDataBytes); //remember, we added this function to the AudioFile library

	ALuint stereoSoundBuffer;
	alec(alGenBuffers(1, &stereoSoundBuffer));
	alec(alBufferData(stereoSoundBuffer, convertFileToOpenALFormat(stereoSoundFile), stereoPCMDataBytes.data(), stereoPCMDataBytes.size(), stereoSoundFile.getSampleRate()));

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// create a sound source that play's our mono sound (from the sound buffer)
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ALuint monoSource;
	alec(alGenSources(1, &monoSource));
	alec(alSource3f(monoSource, AL_POSITION, 1.f, 0.f, 0.f));
	alec(alSource3f(monoSource, AL_VELOCITY, 0.f, 0.f, 0.f));
	alec(alSourcef(monoSource, AL_PITCH, 1.f));
	alec(alSourcef(monoSource, AL_GAIN, 1.f));
	alec(alSourcei(monoSource, AL_LOOPING, AL_FALSE));
	alec(alSourcei(monoSource, AL_BUFFER, monoSoundBuffer));


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// create a sound source for our stereo sound; note 3d positioning doesn't work with stereo files because
	// stereo files are typically used for music. stereo files come out of both ears so it is hard to know
	// what the sound should be doing based on 3d position data.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	ALuint stereoSource;
	alec(alGenSources(1, &stereoSource));
	//alec(alSource3f(stereoSource, AL_POSITION, 0.f, 0.f, 1.f)); //NOTE: this does not work like mono sound positions!
	//alec(alSource3f(stereoSource, AL_VELOCITY, 0.f, 0.f, 0.f)); 
	alec(alSourcef(stereoSource, AL_PITCH, 1.f));
	alec(alSourcef(stereoSource, AL_GAIN, 1.f));
	alec(alSourcei(stereoSource, AL_LOOPING, AL_FALSE));
	alec(alSourcei(stereoSource, AL_BUFFER, stereoSoundBuffer));

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// play the mono sound source
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	alec(alSourcePlay(monoSource));
	ALint sourceState;
	alec(alGetSourcei(monoSource, AL_SOURCE_STATE, &sourceState));
	while (sourceState == AL_PLAYING)
	{
		//basically loop until we're done playing the mono sound source
		alec(alGetSourcei(monoSource, AL_SOURCE_STATE, &sourceState));
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// play the stereo sound source after the mono!
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	alec(alSourcePlay(stereoSource));
	alec(alGetSourcei(stereoSource, AL_SOURCE_STATE, &sourceState));
	while (sourceState == AL_PLAYING)
	{
		//basically loop until we're done playing the mono sound source
		alec(alGetSourcei(stereoSource, AL_SOURCE_STATE, &sourceState));
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// clean up our resources!
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	alec(alDeleteSources(1, &monoSource));
	alec(alDeleteSources(1, &stereoSource));
	alec(alDeleteBuffers(1, &monoSoundBuffer));
	alec(alDeleteBuffers(1, &stereoSoundBuffer));
	alec(alcMakeContextCurrent(nullptr));
	alec(alcDestroyContext(context));
	alec(alcCloseDevice(device));
	
}