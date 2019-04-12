#ifndef SYNTH_H
#define SYNTH_H

#include "includes.h"
#include <string>
#include <map>
#include <vector>

#define AUDIO_BUFFER_LENGTH 1024
#define MAX_PLAYBACK_SAMPLES 32

class Synth {

	public:

		enum {
			SIN = 1, //sinousidal
			SAW, //saw wave
			TRI, //triangular
			SQR //square
		};

		//this class encapsulates an oscillator
		struct Oscillator
		{
			char wave; //wave shape: SIN,SAW,SQR
			float freq; //frequency in Hz
			float amplitude; //0 to 1
			float pw;  //pulse width (only in SQR),
			float LPF; //low-pass filter: 1 no filter 0 all filtered

			float buffer[AUDIO_BUFFER_LENGTH];
			double _phase;
			float _last; //last sample (used in filter)

			Oscillator() {
				freq = 440; 
				amplitude = 0; 
				wave = SIN; 
				pw = 0.5;
				_phase = 0;
				_last = 0;
				LPF = 1;
			}

			void setNote( int note ) { freq = 440.0 * pow(2.0, (note - 69) / 12.0); }
		};

		//master volume
		float volume;

		//oscilators
		Oscillator osc1;
		Oscillator osc2;
		Oscillator osc3;
		float noise_volume;

		float buffer[AUDIO_BUFFER_LENGTH];
		float time;

		Synth();
		~Synth();

		void generateAudio( float* buffer, int len, SDL_AudioSpec& spec );
		void generateOscillator(Oscillator& osc, SDL_AudioSpec& spec);
		void applyFilter(Oscillator& osc, SDL_AudioSpec& spec);

		//samples

		struct Sample {
			Uint32 length;
			float* buffer;
			SDL_AudioSpec spec;
			~Sample();
		};

		//object with info about a sample being played
		struct SamplePlayback {
			Sample* sample;
			unsigned int offset;
			float volume;
			char in_use;
			float start_time;
			char loop;
			void stop() { in_use = false;  }
			void resume() { in_use = true; }
		};

		float samples_buffer[AUDIO_BUFFER_LENGTH];
		SamplePlayback samples_playback[MAX_PLAYBACK_SAMPLES];
		std::map<std::string, Sample*> samples;

		Sample* loadSample(std::string filename);
		SamplePlayback* playSample( Sample* sample, float volume = 0.2, bool loop = false);
		SamplePlayback* playSample( std::string filename, float volume = 0.2, bool loop = false);
		bool updateSamplesBuffer(SDL_AudioSpec& spec );

		static float getNoteFreq(int note) { return 440 * pow(2.0, (note - 69) / 12.0); }
};

#endif