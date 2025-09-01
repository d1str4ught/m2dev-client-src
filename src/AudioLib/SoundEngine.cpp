#include "stdafx.h"
#include "SoundEngine.h"

#include "EterBase/Random.h"
#include "EterBase/Timer.h"
#include "Eterpack/EterPackManager.h"

SoundEngine::SoundEngine()
{
}

SoundEngine::~SoundEngine()
{
	for (auto& [name, instance] : m_Sounds2D)
		instance.Destroy();

	for (auto& instance : m_Sounds3D)
		instance.Destroy();

	ma_engine_uninit(&m_Engine);
	m_Files.clear();
	m_Sounds2D.clear();
}

bool SoundEngine::Initialize()
{
	if (!MD_ASSERT(ma_engine_init(NULL, &m_Engine) == MA_SUCCESS))
	{
		TraceError("SoundEngine::Initialize: Failed to initialize engine.");
		return false;
	}

	SetListenerPosition(0.0f, 0.0f, 0.0f);
	SetListenerOrientation(0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);
	return true;
}

void SoundEngine::SetSoundVolume(float volume)
{
	m_SoundVolume = std::clamp<float>(volume, 0.0, 1.0);
}

bool SoundEngine::PlaySound2D(const std::string& name)
{
	if (!Internal_LoadSoundFromPack(name))
		return false;

	auto& instance = m_Sounds2D[name]; // 2d sounds are persistent, no need to destroy
	instance.InitFromBuffer(m_Engine, m_Files[name], name);
	instance.Config3D(false);
	instance.SetVolume(m_SoundVolume);
	return instance.Play();
}

MaSoundInstance* SoundEngine::PlaySound3D(const std::string& name, float fx, float fy, float fz)
{
	if (auto instance = Internal_GetInstance3D(name))
	{
		constexpr float minDist = 100.0f; // 1m
		constexpr float maxDist = 4000.0f; // 40m

		instance->SetPosition(fx, fy, fz);
		instance->Config3D(true, minDist, maxDist, 1.0f);
		instance->SetVolume(m_SoundVolume);
		instance->Play();
		return instance;
	}
	return nullptr;
}

MaSoundInstance* SoundEngine::PlayAmbienceSound3D(float fx, float fy, float fz, const std::string& name, int loopCount)
{
	auto vec3 = ma_engine_listener_get_position(&m_Engine, 0);
	float dx = fx - vec3.x;
	float dy = fy - vec3.y;
	float dz = fz - vec3.z;
	float distance = sqrtf(dx * dx + dy * dy + dz * dz);
	return PlaySound3D(name, fx, fy, fz);
}

void SoundEngine::StopAllSound3D()
{
	for (auto& instance : m_Sounds3D)
		instance.Stop();
}

void SoundEngine::UpdateSoundInstance(float fx, float fy, float fz, uint32_t dwcurFrame, const NSound::TSoundInstanceVector* c_pSoundInstanceVector, bool checkFrequency, bool isMain)
{
	for (uint32_t i = 0; i < c_pSoundInstanceVector->size(); ++i)
	{
		const NSound::TSoundInstance& c_rSoundInstance = c_pSoundInstanceVector->at(i);
		if (c_rSoundInstance.dwFrame == dwcurFrame)
		{
			if (checkFrequency)
			{
				float& lastPlay = m_PlaySoundHistoryMap[c_rSoundInstance.strSoundFileName];
				float diff = CTimer::Instance().GetCurrentSecond() - lastPlay;

				if (CTimer::Instance().GetCurrentSecond() - lastPlay < 0.3f)
					return;

				lastPlay = CTimer::Instance().GetCurrentSecond();
			}

			if (isMain) // Sounds coming from main instance will always be played in 2d
			{
				//	float volume = 0.9f + frandom(-0.1f, 0.1f);
				//	instance->SetPitch(pitch); // Should we play around with pitch a bit?
				PlaySound2D(c_rSoundInstance.strSoundFileName);
			}
			else
			{
				//	float volume = 0.9f + frandom(-0.1f, 0.1f);
				//	instance->SetPitch(pitch); // Should we play around with pitch a bit?
				PlaySound3D(c_rSoundInstance.strSoundFileName, fx, fy, fz);
			}
		}
	}
}

bool SoundEngine::FadeInMusic(const std::string& path, float targetVolume /* 1.0f by default */, float fadeInDurationSecondsFromMin)
{
	if (path.empty())
		return false;

	auto& fadeOutMusic = m_Music[m_CurrentMusicIndex];
	if (fadeOutMusic.IsPlaying() && path == fadeOutMusic.GetIdentity())
	{
		fadeOutMusic.Fade(targetVolume, fadeInDurationSecondsFromMin);
		return fadeOutMusic.Resume();
	}

	// We're basically just swapping
	FadeOutMusic(fadeOutMusic.GetIdentity());
	m_CurrentMusicIndex = int(!m_CurrentMusicIndex);

	auto& music = m_Music[m_CurrentMusicIndex];
	music.Destroy();
	music.InitFromFile(m_Engine, path);
	music.Config3D(false);
	music.Loop();
	music.SetVolume(0.0f);
	music.Fade(targetVolume, fadeInDurationSecondsFromMin);
	return music.Play();
}

void SoundEngine::FadeOutMusic(const std::string& name, float targetVolume, float fadeOutDurationSecondsFromMax)
{
	for (auto& music : m_Music)
	{
		if (music.GetIdentity() == name)
			music.Fade(targetVolume, fadeOutDurationSecondsFromMax);
	}
}

void SoundEngine::FadeOutAllMusic()
{
	for (auto& music : m_Music)
		FadeOutMusic(music.GetIdentity());
}

void SoundEngine::SetMusicVolume(float volume)
{
	m_MusicVolume = std::clamp<float>(volume, 0.0f, 1.0f);
	for (auto& music : m_Music)
	{
		if (music.IsInitialized() && !music.IsFading()) // fading music will update itself
			music.SetVolume(m_MusicVolume);
	}
}

void SoundEngine::SaveVolume(bool isMinimized)
{
	constexpr float ratePerSecond = 1.0f / CS_CLIENT_FPS;
	// 1.0 to 0 in 1s if minimized, 3s if just out of focus
	const float durationOnFullVolume = isMinimized ? 1.0 : 3.0f;

	float outOfFocusVolume = 0.35f;
	if (m_MasterVolume <= outOfFocusVolume)
		outOfFocusVolume = m_MasterVolume;

	m_MasterVolumeFadeTarget = isMinimized ? 0.0f : outOfFocusVolume;
	m_MasterVolumeFadeRatePerFrame = -ratePerSecond / durationOnFullVolume;
}

void SoundEngine::RestoreVolume()
{
	constexpr float ratePerSecond = 1.0f / CS_CLIENT_FPS;
	constexpr float durationToFullVolume = 4.0f; // 0 to 1.0 in 4s
	m_MasterVolumeFadeTarget = m_MasterVolume;
	m_MasterVolumeFadeRatePerFrame = ratePerSecond / durationToFullVolume;
}

void SoundEngine::SetMasterVolume(float volume)
{
	m_MasterVolume = volume;
	ma_engine_set_volume(&m_Engine, volume);
}

void SoundEngine::SetListenerPosition(float x, float y, float z)
{
	ma_engine_listener_set_position(&m_Engine, 0, x, y, z);
}

void SoundEngine::SetListenerOrientation(float forwardX, float forwardY, float forwardZ,
										 float upX, float upY, float upZ)
{
	ma_engine_listener_set_direction(&m_Engine, 0, forwardX, forwardY, forwardZ);
	ma_engine_listener_set_world_up(&m_Engine, 0, upX, upY, upZ);
}

void SoundEngine::SetListenerVelocity(float x, float y, float z)
{
	ma_engine_listener_set_velocity(&m_Engine, 0, x, y, z);
}

void SoundEngine::Update()
{
	for (auto& music : m_Music)
		music.Update(m_MusicVolume);

	if (m_MasterVolumeFadeRatePerFrame)
	{
		float volume = ma_engine_get_volume(&m_Engine) + m_MasterVolumeFadeRatePerFrame;
		if ((m_MasterVolumeFadeRatePerFrame > 0.0f && volume >= m_MasterVolumeFadeTarget) || (m_MasterVolumeFadeRatePerFrame < 0.0f && volume <= m_MasterVolumeFadeTarget))
		{
			volume = m_MasterVolumeFadeTarget;
			m_MasterVolumeFadeRatePerFrame = 0.0f;
		}
		ma_engine_set_volume(&m_Engine, volume);
	}
}

MaSoundInstance* SoundEngine::Internal_GetInstance3D(const std::string& name)
{
	if (Internal_LoadSoundFromPack(name))
	{
		for (auto& instance : m_Sounds3D)
		{
			if (!instance.IsPlaying())
			{
				instance.Destroy();
				instance.InitFromBuffer(m_Engine, m_Files[name], name);
				return &instance;
			}
		}
	}
	return nullptr;
}

bool SoundEngine::Internal_LoadSoundFromPack(const std::string& name)
{
	if (m_Files.find(name) == m_Files.end())
	{
		LPCVOID soundData;
		CMappedFile soundFile;
		if (!CEterPackManager::Instance().Get(soundFile, name.c_str(), &soundData))
		{
			TraceError("Internal_LoadSoundFromPack: SoundEngine: Failed to register file '%s' - not found.", name.c_str());
			return false;
		}

		auto& buffer = m_Files[name];
		buffer.resize(soundFile.Size());
		memcpy(buffer.data(), soundData, soundFile.Size());
	}
	return true;
}
