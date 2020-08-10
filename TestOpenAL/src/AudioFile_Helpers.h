#pragma once
#include <vector>
#include <AudioFile/AudioFile.h>

namespace AudioDataHelpers
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// declarations
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template<typename FloatOrDouble>
	void getPCMData(const AudioFile<FloatOrDouble>& loadedFile, std::vector<char>& outPCM);


	////////////////////////////////////////////////////////
	// this is taken from AUDIO file writing; just as a test for now but perhaps should make these public
	////////////////////////////////////////////////////////
	template <class T>
	int16_t sampleToSixteenBitInt(T sample)
	{
		sample = clamp(sample, -1., 1.);
		return static_cast<int16_t> (sample * 32767.);
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template<typename FloatOrDouble>
	void getPCMData(const AudioFile<FloatOrDouble>& loadedFile, std::vector<char>& outPCM)
	{
		outPCM.clear();

		// samples is a vector of a vector
		// if there are two channels (stereo), there outermost vector will have size 2. one for each ear.
		if (loadedFile.samples.size() > 0)
		{
			//making assumption that all channels should be the same length.
			size_t rawChannelSize_bytes = loadedFile.samples[0].size() * sizeof(FloatOrDouble);


			//if the file has two channels, we need to interweave the samples for PCM to be correct.
			//branching outside of loop since this is probably going to be a large file and probably not 
			//great to have a branch every loop iteration. but maybe; I did not profile.
			if (loadedFile.isStereo() && loadedFile.samples.size() >= 2)
			{
				const FloatOrDouble* fpData_ch0 = loadedFile.samples[0].data();
				const FloatOrDouble* fpData_ch1 = loadedFile.samples[1].data();

				//#TODO this is totally the slow way, we should do a resize(size) then do memcpy on char* ptr
				const char* rawBytes_ch0 = reinterpret_cast<const char*>(fpData_ch0);
				const char* rawBytes_ch1 = reinterpret_cast<const char*>(fpData_ch1);
				for (size_t byte = 0; byte < rawChannelSize_bytes; ++byte)
				{
					outPCM.push_back(rawBytes_ch0[byte]);
					outPCM.push_back(rawBytes_ch1[byte]);
				}
			}
			else //when file is mono, we just need to just the samples; no interweaving
			{
				const FloatOrDouble* fpData = loadedFile.samples[0].data();

				//#TODO this is totally the slow way, we should do a resize(size) then do memcpy on char* ptr
				const char* rawBytes = reinterpret_cast<const char*>(fpData);
				for (size_t byte = 0; byte < rawChannelSize_bytes; ++byte)
				{
					outPCM.push_back(rawBytes[byte]);
				}
			}
			
		}
	}
}

