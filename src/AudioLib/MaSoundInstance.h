#pragma once
#define MA_NO_WASAPI
#define MA_ENABLE_DSOUND
#define MA_ENABLE_WINMM
#include <miniaudio.h>

inline constexpr float CS_CLIENT_FPS = 61.0f;

class MaSoundInstance {
public:
  bool InitFromBuffer(ma_engine &engine, const std::vector<uint8_t> &buffer,
                      const std::string &identity);

  bool InitFromFile(ma_engine &engine, const std::string &filePathOnDisk);

  void Destroy();

  bool IsInitialized() const;

  bool IsPlaying() const;

  bool Play();

  bool Resume();

  bool Stop();

  void Loop();

  float GetVolume() const;

  void SetVolume(float volume);

  void SetPitch(float pitch);

  void SetPosition(float x, float y, float z);

  const std::string &GetIdentity() const;

  void Config3D(bool toggle, float minDist = 100.0f /*1m*/,
                float maxDist = 4000.0f /*40m*/);

  void Fade(float toVolume, float secDurationFromMinMax);

  void StopFading();

  bool IsFading() const;

  void Update();

private:
  std::string m_Identity;
  ma_sound m_Sound{};
  ma_decoder m_Decoder{};
  bool m_Initialized{};
  float m_FadeTargetVolume{};
  float m_FadeRatePerFrame{};
};
