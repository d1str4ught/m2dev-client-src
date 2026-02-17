#pragma once

class CGrannyMotion {
public:
  CGrannyMotion();
  virtual ~CGrannyMotion();

  bool IsEmpty();

  void Destroy();
  bool BindGrannyAnimation(granny_animation *pgrnAni);

  granny_animation *GetGrannyAnimationPointer() const;

  const char *GetName() const;
  float GetDuration() const;
  void GetTextTrack(const char *c_szTextTrackName, int *pCount,
                    float *pArray) const;

protected:
  void Initialize();

protected:
  granny_animation *m_pgrnAni;
};
