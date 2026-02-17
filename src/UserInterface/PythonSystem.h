#pragma once

class CPythonSystem : public CSingleton<CPythonSystem> {
public:
  enum EWindow {
    WINDOW_STATUS,
    WINDOW_INVENTORY,
    WINDOW_ABILITY,
    WINDOW_SOCIETY,
    WINDOW_JOURNAL,
    WINDOW_COMMAND,

    WINDOW_QUICK,
    WINDOW_GAUGE,
    WINDOW_MINIMAP,
    WINDOW_CHAT,

    WINDOW_MAX_NUM,
  };

  enum { FREQUENCY_MAX_NUM = 30, RESOLUTION_MAX_NUM = 100 };

  typedef struct SResolution {
    DWORD width;
    DWORD height;
    DWORD bpp; // bits per pixel (high-color = 16bpp, true-color = 32bpp)

    DWORD frequency[20];
    BYTE frequency_count;
  } TResolution;

  typedef struct SWindowStatus {
    int isVisible;
    int isMinimized;

    int ixPosition;
    int iyPosition;
    int iHeight;
  } TWindowStatus;

  typedef struct SConfig {
    DWORD width;
    DWORD height;
    DWORD bpp;
    DWORD frequency;

    bool is_software_cursor;
    bool is_object_culling;
    int iDistance;
    int iShadowLevel;
    // MR-14: Fog update by Alaric
    int iFogLevel;
    // MR-14: -- END OF -- Fog update by Alaric

    FLOAT music_volume;
    FLOAT voice_volume;

    int gamma;

    int isSaveID;
    char SaveID[20];

    bool bWindowed;
    bool bDecompressDDS;
    bool bNoSoundCard;
    bool bUseDefaultIME;
    BYTE bSoftwareTiling;
    bool bViewChat;
    bool bAlwaysShowName;
    bool bShowDamage;
    bool bShowSalesText;
  } TConfig;

public:
  CPythonSystem();
  virtual ~CPythonSystem();

  void Clear();
  void SetInterfaceHandler(PyObject *poHandler);
  void DestroyInterfaceHandler();

  // Config
  void SetDefaultConfig();
  bool LoadConfig();
  bool SaveConfig();
  void ApplyConfig();
  void SetConfig(TConfig *set_config);
  TConfig *GetConfig();
  void ChangeSystem();

  // Interface
  bool LoadInterfaceStatus();
  void SaveInterfaceStatus();
  bool isInterfaceConfig();
  const TWindowStatus &GetWindowStatusReference(int iIndex);

  DWORD GetWidth();
  DWORD GetHeight();
  DWORD GetBPP();
  DWORD GetFrequency();
  bool IsSoftwareCursor();
  bool IsWindowed();
  bool IsViewChat();
  bool IsAlwaysShowName();
  bool IsShowDamage();
  bool IsShowSalesText();
  bool IsUseDefaultIME();
  bool IsNoSoundCard();
  bool IsAutoTiling();
  bool IsSoftwareTiling();
  void SetSoftwareTiling(bool isEnable);
  void SetViewChatFlag(int iFlag);
  void SetAlwaysShowNameFlag(int iFlag);
  void SetShowDamageFlag(int iFlag);
  void SetShowSalesTextFlag(int iFlag);

  // Window
  void SaveWindowStatus(int iIndex, int iVisible, int iMinimized, int ix,
                        int iy, int iHeight);

  // SaveID
  int IsSaveID();
  const char *GetSaveID();
  void SetSaveID(int iValue, const char *c_szSaveID);

  /// Display
  void GetDisplaySettings();

  int GetResolutionCount();
  int GetFrequencyCount(int index);
  bool GetResolution(int index, OUT DWORD *width, OUT DWORD *height,
                     OUT DWORD *bpp);
  bool GetFrequency(int index, int freq_index, OUT DWORD *frequncy);
  int GetResolutionIndex(DWORD width, DWORD height, DWORD bpp);
  int GetFrequencyIndex(int res_index, DWORD frequency);
  bool isViewCulling();

  // Sound
  float GetMusicVolume();
  float GetSoundVolume();
  void SetMusicVolume(float fVolume);
  void SetSoundVolume(float fVolume);

  int GetDistance();
  int GetShadowLevel();
  void SetShadowLevel(unsigned int level);

  // MR-14: Fog update by Alaric
  int GetFogLevel();
  void SetFogLevel(unsigned int level);
  // MR-14: -- END OF -- Fog update by Alaric

protected:
  TResolution m_ResolutionList[RESOLUTION_MAX_NUM];
  int m_ResolutionCount;

  TConfig m_Config;
  TConfig m_OldConfig;

  bool m_isInterfaceConfig;
  PyObject *m_poInterfaceHandler;
  TWindowStatus m_WindowStatus[WINDOW_MAX_NUM];
};