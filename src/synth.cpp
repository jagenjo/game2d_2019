#include "synth.h"
#include "framework.h"
#include <math.h>

Synth::Sample::~Sample()
{
	SDL_free(buffer);
}

Synth::Synth()
{
	volume = 0.2;
	noise_volume = 0;

	memset(&samples_playback, 0, sizeof(SamplePlayback)*MAX_PLAYBACK_SAMPLES);
}

Synth::~Synth()
{
	//iterate samples
	for (auto it = samples.begin(); it != samples.end(); it++)
	{
		Sample* s = it->second;
		delete s;
	}
	samples.clear();
}


void Synth::generateAudio( float* buffer, int len, SDL_AudioSpec& spec )
{
	//synth
	generateOscillator(osc1, spec);
	generateOscillator(osc2, spec);
	generateOscillator(osc3, spec);

	applyFilter(osc1, spec);
	applyFilter(osc2, spec);
	applyFilter(osc3, spec);

	//samples
	updateSamplesBuffer(spec);

	//mix
	for (int i = 0; i < AUDIO_BUFFER_LENGTH; ++i)
	{
		float s = 0.0;
		
		s = osc1.buffer[i];
		s += osc2.buffer[i];
		s += osc3.buffer[i];
		s += noise_volume * ((rand() % 255) / 255.0);
		
		s += samples_buffer[i];
		buffer[i] = volume * s;
	}
}

void Synth::generateOscillator(Oscillator& osc, SDL_AudioSpec& spec)
{
	double wave_length = osc.freq / (spec.freq);
	double pos = osc._phase;
	//int tmp;
	float amplitude = osc.amplitude;
	if (amplitude == 0)
	{
		memset(osc.buffer, 0, AUDIO_BUFFER_LENGTH * sizeof(float));
		return;
	}

	switch (osc.wave)
	{
		case SIN:
			for (int i = 0; i < AUDIO_BUFFER_LENGTH; ++i)
			{
				osc.buffer[i] = sin(pos * (2.0 * PI)) * amplitude;
				pos += wave_length;
			}
			break;
		case SAW:
			for (int i = 0; i < AUDIO_BUFFER_LENGTH; ++i)
			{
				osc.buffer[i] = (pos - (int)pos) * amplitude;
				pos += wave_length;
			}
			break;
		case SQR:
			for (int i = 0; i < AUDIO_BUFFER_LENGTH; ++i)
			{
				osc.buffer[i] = (pos - (int)pos) > osc.pw ? 0.0 : amplitude;
				pos += wave_length;
			}
			break;
		case TRI:
			for (int i = 0; i < AUDIO_BUFFER_LENGTH; ++i)
			{
				float f = (pos - (int)pos);
				osc.buffer[i] = (f < 0.5 ? f * 2.0 : 1.0 - f * 2.0 ) * amplitude;
				pos += wave_length;
			}
			break;
		default:
			break;
	}
	osc._phase = pos;
}

void Synth::applyFilter( Oscillator& osc, SDL_AudioSpec& spec )
{
	if (osc.LPF >= 1)
		return;

	float filter = clamp(osc.LPF,0.0,1.0);

	float current = 0;
	float last = osc._last;

	for (int i = 0; i < AUDIO_BUFFER_LENGTH; ++i)
	{
		current = osc.buffer[i];
		last = osc.buffer[i] = last - filter * (last - current);
	}

	osc._last = last;
}

Synth::Sample* Synth::loadSample(std::string filename)
{
	std::map<std::string, Sample*>::iterator it = samples.find(filename);
	if (it != samples.end())
		return it->second;

	Sample* sample = new Sample();
	memset(sample, 0, sizeof(Sample));

	Uint32 wav_length;
	Uint8 *wav_buffer;

	//loads a wav file, but it can come in any format
	if (SDL_LoadWAV(filename.c_str(), &sample->spec, &wav_buffer, &wav_length) == NULL)
	{
		std::cerr << "Could not open wav: " << filename << std::endl;
		delete sample;
		return NULL;
	}

	//so we need to convert them
	SDL_AudioCVT cvt;
	SDL_BuildAudioCVT( &cvt, sample->spec.format, sample->spec.channels, sample->spec.freq, AUDIO_F32, 1, 48000 );
	if (cvt.needed)
	{
		cvt.len = wav_length;
		cvt.buf = (Uint8 *)SDL_malloc(cvt.len * cvt.len_mult); //already computes the final size
		memcpy(cvt.buf, wav_buffer, wav_length );
		if (SDL_ConvertAudio(&cvt) != 0 )
		{
			std::cout << "Error during WAV conversion: " << filename << std::endl;
			SDL_FreeWAV(wav_buffer);
			return NULL;
		}
		sample->length = cvt.len_cvt / sizeof(float);
		sample->buffer = (float*)cvt.buf;
		sample->spec.format = cvt.dst_format;
		sample->spec.channels = 1;
		sample->spec.freq = 48000;
	}
	else
	{
		sample->length = wav_length / sizeof(float);
		sample->buffer = (float*) SDL_malloc(wav_length);
		memcpy( (Uint8*)sample->buffer, (Uint8*)wav_buffer, wav_length );
	}

	samples[filename] = sample;
	SDL_FreeWAV(wav_buffer);
	return sample;
}

Synth::SamplePlayback* Synth::playSample( Synth::Sample* sample, float volume, bool loop )
{
	//find free sample
	int i = 0;
	for (; i < MAX_PLAYBACK_SAMPLES; ++i)
	{
		SamplePlayback& sp = samples_playback[i];
		if (!sp.in_use)
			break;
	}

	if (i == MAX_PLAYBACK_SAMPLES)
		return NULL;

	SamplePlayback& sp = samples_playback[i];
	sp.in_use = true;
	sp.sample = sample;
	sp.start_time = time;
	sp.offset = 0;
	sp.volume = volume;
	sp.loop = loop ? 1 : 0;
	return &sp;
}

Synth::SamplePlayback* Synth::playSample( std::string filename, float volume, bool loop)
{
	Sample* sample = loadSample(filename);
	if(sample)
		return playSample(sample, volume, loop);
	return NULL;
}


bool Synth::updateSamplesBuffer(SDL_AudioSpec& spec)
{
	bool playing = false;
	for (int j = 0; j < MAX_PLAYBACK_SAMPLES; ++j)
	{
		SamplePlayback& sp = samples_playback[j];
		if (!sp.in_use)
			continue;
		playing = true;
		break;
	}

	memset( samples_buffer, 0, sizeof(float) * AUDIO_BUFFER_LENGTH );

	if (!playing)
		return false;

	for (int j = 0; j < MAX_PLAYBACK_SAMPLES; ++j)
	{
		SamplePlayback& sp = samples_playback[j];
		if (!sp.in_use)
			continue;

		unsigned int size = AUDIO_BUFFER_LENGTH;
		unsigned int offset = sp.offset;
		if (sp.offset + size > sp.sample->length)
		{
			size = sp.sample->length - sp.offset;
			if (sp.loop)
				sp.offset = 0;
			else
				sp.in_use = false;
		}
		else
			sp.offset += size;

		SDL_MixAudio((Uint8*)samples_buffer, (Uint8*)(sp.sample->buffer + offset), size * sizeof(float), sp.volume * 128 );
	}
	return true;
}
