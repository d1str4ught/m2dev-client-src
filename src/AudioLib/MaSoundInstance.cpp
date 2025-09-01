#include "stdafx.h"
#include "MaSoundInstance.h"

#include <miniaudio.c>

bool MaSoundInstance::InitFromBuffer(ma_engine& engine, const std::vector<uint8_t>& buffer, const std::string& identity)
{
	if (!IsInitialized())
	{
		ma_decoder_config decoderConfig = ma_decoder_config_init_default();
		ma_result result = ma_decoder_init_memory(buffer.data(), buffer.size(),
												  &decoderConfig, &m_Decoder);
		if (!MD_ASSERT(result == MA_SUCCESS))
		{
			TraceError("Failed to initialize decoder memory.");
			return false;
		}

		ma_sound_config soundConfig = ma_sound_config_init();
		soundConfig.pDataSource = &m_Decoder;

		result = ma_sound_init_ex(&engine, &soundConfig, &m_Sound);
		if (!MD_ASSERT(result == MA_SUCCESS))
		{
			TraceError("Failed to initialize sound.");
			return false;
		}

		m_Identity = identity;
		m_Initialized = true;
	}
	return IsInitialized();
}

// Basically c&p
bool MaSoundInstance::InitFromFile(ma_engine& engine, const std::string& filePathOnDisk)
{
	if (!IsInitialized())
	{
		ma_decoder_config decoderConfig = ma_decoder_config_init_default();
		ma_result result = ma_decoder_init_file(filePathOnDisk.c_str(), &decoderConfig, &m_Decoder);
		if (!MD_ASSERT(result == MA_SUCCESS))
		{
			TraceError("Failed to initialize sound file decoder.");
			return false;
		}

		ma_sound_config soundConfig = ma_sound_config_init();
		soundConfig.pDataSource = &m_Decoder;

		result = ma_sound_init_ex(&engine, &soundConfig, &m_Sound);
		if (!MD_ASSERT(result == MA_SUCCESS))
		{
			TraceError("Failed to initialize sound.");
			return false;
		}

		m_Identity = filePathOnDisk;
		m_Initialized = true;
	}
	return IsInitialized();
}

void MaSoundInstance::Destroy()
{
	if (IsInitialized())
	{
		ma_sound_stop(&m_Sound);
		ma_sound_uninit(&m_Sound);
		ma_decoder_uninit(&m_Decoder);
	}

	m_Sound = {};
	m_Decoder = {};
	m_Identity.clear();
	m_Initialized = false;
	m_FadeTargetVolume = 0.0f;
	m_FadeRatePerFrame = 0.0f;
}

bool MaSoundInstance::IsInitialized() const
{
	return m_Initialized;
}

bool MaSoundInstance::IsPlaying() const
{
	if (!IsInitialized())
		return false;

	return ma_sound_is_playing(&m_Sound) == MA_TRUE;
}

bool MaSoundInstance::Play()
{
	if (!IsInitialized())
		return false;

	return ma_sound_seek_to_pcm_frame(&m_Sound, 0) == MA_SUCCESS && ma_sound_start(&m_Sound) == MA_SUCCESS;
}

bool MaSoundInstance::Resume()
{
	if (!IsInitialized())
		return false;

	return ma_sound_start(&m_Sound) == MA_SUCCESS;
}

bool MaSoundInstance::Stop()
{
	if (!IsInitialized())
		return false;

	return ma_sound_stop(&m_Sound) == MA_SUCCESS;
}

void MaSoundInstance::Loop()
{
	if (!IsInitialized())
		return;

	ma_sound_set_looping(&m_Sound, MA_TRUE);
}

float MaSoundInstance::GetVolume() const
{
	if (!IsInitialized())
		return 0.0f;

	return ma_sound_get_volume(&m_Sound);
}

void MaSoundInstance::SetVolume(float volume)
{
	if (!IsInitialized())
		return;

	ma_sound_set_volume(&m_Sound, volume);
}

void MaSoundInstance::SetPitch(float pitch)
{
	if (!IsInitialized())
		return;

	ma_sound_set_pitch(&m_Sound, pitch);
}

void MaSoundInstance::SetPosition(float x, float y, float z)
{
	if (!IsInitialized())
		return;

	ma_sound_set_position(&m_Sound, x, y, z);
}

const std::string& MaSoundInstance::GetIdentity() const
{
	return m_Identity;
}

void MaSoundInstance::Config3D(bool toggle, float minDist, float maxDist, float rolloff)
{
	if (!IsInitialized())
		return;

	ma_sound_set_spatialization_enabled(&m_Sound, toggle);
	ma_sound_set_rolloff(&m_Sound, rolloff);
	ma_sound_set_min_distance(&m_Sound, minDist);
	ma_sound_set_max_distance(&m_Sound, maxDist);
	ma_sound_set_attenuation_model(&m_Sound, ma_attenuation_model_linear);
}

void MaSoundInstance::Fade(float toVolume, float secDurationFromMinMax)
{
	if (!IsInitialized())
		return;

	toVolume = std::clamp<float>(toVolume, 0.0f, 1.0f);
	m_FadeTargetVolume = toVolume;

	float rate = 1.0f / 61.0f / secDurationFromMinMax;
	m_FadeRatePerFrame = GetVolume() > toVolume ? -rate : rate;
}

void MaSoundInstance::StopFading()
{
	m_FadeRatePerFrame = 0.0f;
}

bool MaSoundInstance::IsFading() const
{
	return m_FadeRatePerFrame != 0.0f;
}

void MaSoundInstance::Update(float volumeFactor) // volume factor is the user's volume
{
	if (!IsInitialized())
		return;

	if (m_FadeRatePerFrame != 0.0f)
	{
		float targetVolume = std::clamp<float>(m_FadeTargetVolume * volumeFactor, 0.0f, 1.0f);
		float volume = std::clamp<float>(GetVolume() + (m_FadeRatePerFrame * volumeFactor), 0.0f, 1.0f);
		if ((m_FadeRatePerFrame > 0.0f && volume >= targetVolume) || (m_FadeRatePerFrame < 0.0f && volume <= targetVolume))
		{
			volume = m_FadeTargetVolume * volumeFactor;
			m_FadeRatePerFrame = 0.0f;
			if (volume <= 0.0f)
				ma_sound_stop(&m_Sound);
		}
		ma_sound_set_volume(&m_Sound, volume);
	}
}
