#include "StdAfx.h"
#include "IME.h"
#include "TextTag.h"
#include "EterBase/Utils.h"
#include "msctf.h"
#include <oleauto.h>
#include <algorithm>
#include <utf8.h>

#define COUNTOF(a) (sizeof(a) / sizeof((a)[0]))

int CIME::ms_compLen;
int CIME::ms_curpos;
int CIME::ms_lastpos;
wchar_t CIME::m_wText[IMESTR_MAXLEN];

#define MAKEIMEVERSION(major, minor)                                           \
  ((DWORD)(((BYTE)(major) << 24) | ((BYTE)(minor) << 16)))
#define IMEID_VER(dwId) ((dwId) & 0xffff0000)
#define IMEID_LANG(dwId) ((dwId) & 0x0000ffff)

#define GETLANG() LOWORD(CIME::ms_hklCurrent)
#define GETPRIMLANG() ((WORD)PRIMARYLANGID(GETLANG()))
#define GETSUBLANG() SUBLANGID(GETLANG())

#define LANG_CHT MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL)
#define LANG_CHS MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)

// Chinese Traditional
#define _CHT_HKL_DAYI ((HKL)0xE0060404)          // DaYi
#define _CHT_HKL_NEW_PHONETIC ((HKL)0xE0080404)  // New Phonetic
#define _CHT_HKL_NEW_CHANG_JIE ((HKL)0xE0090404) // New Chang Jie
#define _CHT_HKL_NEW_QUICK ((HKL)0xE00A0404)     // New Quick
#define _CHT_HKL_HK_CANTONESE ((HKL)0xE00B0404)  // Hong Kong Cantonese

#define CHT_IMEFILENAME1 L"TINTLGNT.IME" // New Phonetic
#define CHT_IMEFILENAME2 L"CINTLGNT.IME" // New Chang Jie
#define CHT_IMEFILENAME3 L"MSTCIPHA.IME" // Phonetic 5.1

#define IMEID_CHT_VER42                                                        \
  (LANG_CHT |                                                                  \
   MAKEIMEVERSION(4, 2)) // New(Phonetic/ChanJie)IME98  : 4.2.x.x // Win98
#define IMEID_CHT_VER43                                                        \
  (LANG_CHT |                                                                  \
   MAKEIMEVERSION(4, 3)) // New(Phonetic/ChanJie)IME98a : 4.3.x.x // Win2k
#define IMEID_CHT_VER44                                                        \
  (LANG_CHT |                                                                  \
   MAKEIMEVERSION(4, 4)) // New ChanJie IME98b          : 4.4.x.x // WinXP
#define IMEID_CHT_VER50                                                        \
  (LANG_CHT |                                                                  \
   MAKEIMEVERSION(5, 0)) // New(Phonetic/ChanJie)IME5.0 : 5.0.x.x // WinME
#define IMEID_CHT_VER51                                                        \
  (LANG_CHT |                                                                  \
   MAKEIMEVERSION(                                                             \
       5, 1)) // New(Phonetic/ChanJie)IME5.1 : 5.1.x.x // IME2002(w/OfficeXP)
#define IMEID_CHT_VER52                                                        \
  (LANG_CHT |                                                                  \
   MAKEIMEVERSION(                                                             \
       5, 2)) // New(Phonetic/ChanJie)IME5.2 : 5.2.x.x // IME2002a(w/Whistler)
#define IMEID_CHT_VER60                                                        \
  (LANG_CHT |                                                                  \
   MAKEIMEVERSION(                                                             \
       6, 0)) // New(Phonetic/ChanJie)IME6.0 : 6.0.x.x // IME XP(w/WinXP SP1)
#define IMEID_CHT_VER_VISTA                                                    \
  (LANG_CHT |                                                                  \
   MAKEIMEVERSION(7, 0)) // All TSF TIP under Cicero UI-less mode: a hack to
                         // make GetImeId() return non-zero value

// Chinese Simplized
#define _CHS_HKL ((HKL)0xE00E0804)              // MSPY
#define _CHS_HKL_QQPINYIN ((HKL)0xE0210804)     // QQ PinYin
#define _CHS_HKL_SOGOU ((HKL)0xE0220804)        // Sougou PinYin
#define _CHS_HKL_GOOGLEPINYIN ((HKL)0xE0230804) // Google PinYin

#define CHS_IMEFILENAME1 L"PINTLGNT.IME"                  // MSPY1.5/2/3
#define CHS_IMEFILENAME2 L"MSSCIPYA.IME"                  // MSPY3 for OfficeXP
#define CHS_IMEFILENAME_QQPINYIN "QQPINYIN.IME"           // QQ PinYin
#define CHS_IMEFILENAME_SOGOUPY "SOGOUPY.IME"             // Sougou PinYin
#define CHS_IMEFILENAME_GOOGLEPINYIN2 "GOOGLEPINYIN2.IME" // Google PinYin 2

#define IMEID_CHS_VER41                                                        \
  (LANG_CHS |                                                                  \
   MAKEIMEVERSION(                                                             \
       4, 1)) // MSPY1.5	// SCIME97 or MSPY1.5 (w/Win98, Office97)
#define IMEID_CHS_VER42                                                        \
  (LANG_CHS | MAKEIMEVERSION(4, 2)) // MSPY2	// Win2k/WinME
#define IMEID_CHS_VER53 (LANG_CHS | MAKEIMEVERSION(5, 3)) // MSPY3	// WinXP

enum {
  INDICATOR_NON_IME,
  INDICATOR_CHS,
  INDICATOR_CHT,
  INDICATOR_KOREAN,
  INDICATOR_JAPANESE
};
enum { IMEUI_STATE_OFF, IMEUI_STATE_ON, IMEUI_STATE_ENGLISH };

#define LCID_INVARIANT                                                         \
  MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)

wchar_t s_aszIndicator[5][3] = {
    L"En", L"\x7B80", L"\x7E41", L"\xAC00", L"\x3042",
};

INPUTCONTEXT *(WINAPI *CIME::_ImmLockIMC)(HIMC);
BOOL(WINAPI *CIME::_ImmUnlockIMC)(HIMC);
LPVOID(WINAPI *CIME::_ImmLockIMCC)(HIMCC);
BOOL(WINAPI *CIME::_ImmUnlockIMCC)(HIMCC);

UINT(WINAPI *CIME::_GetReadingString)(HIMC, UINT, LPWSTR, PINT, BOOL *, PUINT);
BOOL(WINAPI *CIME::_ShowReadingWindow)(HIMC, BOOL);

bool CIME::ms_bInitialized = false;
bool CIME::ms_bDisableIMECompletely = false;
bool CIME::ms_bImeEnabled = false;
bool CIME::ms_bUILessMode = false;
bool CIME::ms_bCaptureInput = false;
bool CIME::ms_bChineseIME = false;
bool CIME::ms_bUseIMMCandidate = false;

int CIME::ms_selbegin = 0;
int CIME::ms_selend = 0;
int CIME::ms_compCaret = 0;

std::vector<CIME::SUndoState> CIME::ms_undo;
std::vector<CIME::SUndoState> CIME::ms_redo;

HWND CIME::ms_hWnd;
HKL CIME::ms_hklCurrent;
wchar_t CIME::ms_szKeyboardLayout[KL_NAMELENGTH + 1];
OSVERSIONINFOW CIME::ms_stOSVI;

HINSTANCE CIME::ms_hImm32Dll;
HINSTANCE CIME::ms_hCurrentImeDll;
DWORD CIME::ms_dwImeState;

DWORD CIME::ms_adwId[2] = {0, 0};

// IME Level
DWORD CIME::ms_dwIMELevel;
DWORD CIME::ms_dwIMELevelSaved;

// Candidate List
bool CIME::ms_bCandidateList;
DWORD CIME::ms_dwCandidateCount;
bool CIME::ms_bVerticalCandidate;
int CIME::ms_iCandListIndexBase;
WCHAR CIME::ms_wszCandidate[CIME::MAX_CANDLIST][CIME::MAX_CANDIDATE_LENGTH];
DWORD CIME::ms_dwCandidateSelection;
DWORD CIME::ms_dwCandidatePageSize;

// Reading Window
bool CIME::ms_bReadingInformation;
int CIME::ms_iReadingError = 0;
bool CIME::ms_bHorizontalReading;
std::vector<wchar_t> CIME::ms_wstrReading;

// Indicator
wchar_t *CIME::ms_wszCurrentIndicator;

IIMEEventSink *CIME::ms_pEvent;

int CIME::ms_ulbegin;
int CIME::ms_ulend;

///////////////////////////////////////////////////////////////////////////////
//
//  CTsfUiLessMode
//      Handles IME events using Text Service Framework (TSF). Before Vista,
//      IMM (Input Method Manager) API has been used to handle IME events and
//      inqueries. Some IMM functions lose backward compatibility due to design
//      of TSF, so we have to use new TSF interfaces.
//
///////////////////////////////////////////////////////////////////////////////
class CTsfUiLessMode {
protected:
  // Sink receives event notifications
  class CUIElementSink : public ITfUIElementSink,
                         public ITfInputProcessorProfileActivationSink,
                         public ITfCompartmentEventSink {
  public:
    CUIElementSink();
    ~CUIElementSink();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ITfUIElementSink
    //   Notifications for Reading Window events. We could process candidate as
    //   well, but we'll use IMM for simplicity sake.
    STDMETHODIMP BeginUIElement(DWORD dwUIElementId, BOOL *pbShow);
    STDMETHODIMP UpdateUIElement(DWORD dwUIElementId);
    STDMETHODIMP EndUIElement(DWORD dwUIElementId);

    // ITfInputProcessorProfileActivationSink
    //   Notification for keyboard input locale change
    STDMETHODIMP OnActivated(DWORD dwProfileType, LANGID langid, REFCLSID clsid,
                             REFGUID catid, REFGUID guidProfile, HKL hkl,
                             DWORD dwFlags);

    // ITfCompartmentEventSink
    //    Notification for open mode (toggle state) change
    STDMETHODIMP OnChange(REFGUID rguid);

  private:
    LONG _cRef;
  };

  static void
  MakeReadingInformationString(ITfReadingInformationUIElement *preading);
  static void MakeCandidateStrings(ITfCandidateListUIElement *pcandidate);
  static ITfUIElement *GetUIElement(DWORD dwUIElementId);
  static BOOL GetCompartments(ITfCompartmentMgr **ppcm,
                              ITfCompartment **ppTfOpenMode,
                              ITfCompartment **ppTfConvMode);
  static BOOL SetupCompartmentSinks(BOOL bResetOnly = FALSE,
                                    ITfCompartment *pTfOpenMode = NULL,
                                    ITfCompartment *ppTfConvMode = NULL);

  static ITfThreadMgrEx *m_tm;
  static DWORD m_dwUIElementSinkCookie;
  static DWORD m_dwAlpnSinkCookie;
  static DWORD m_dwOpenModeSinkCookie;
  static DWORD m_dwConvModeSinkCookie;
  static CUIElementSink *m_TsfSink;
  static int
      m_nCandidateRefCount; // Some IME shows multiple candidate lists but the
                            // Library doesn't support multiple candidate list.
                            // So track open / close events to make sure the
                            // candidate list opened last is shown.
  CTsfUiLessMode() {}       // this class can't be instanciated

public:
  static BOOL SetupSinks();
  static void ReleaseSinks();
  static BOOL CurrentInputLocaleIsIme();
  static void UpdateImeState(BOOL bResetCompartmentEventSink = FALSE);
  static void EnableUiUpdates(bool bEnable);
};

ITfThreadMgrEx *CTsfUiLessMode::m_tm;
DWORD CTsfUiLessMode::m_dwUIElementSinkCookie = TF_INVALID_COOKIE;
DWORD CTsfUiLessMode::m_dwAlpnSinkCookie = TF_INVALID_COOKIE;
DWORD CTsfUiLessMode::m_dwOpenModeSinkCookie = TF_INVALID_COOKIE;
DWORD CTsfUiLessMode::m_dwConvModeSinkCookie = TF_INVALID_COOKIE;
CTsfUiLessMode::CUIElementSink *CTsfUiLessMode::m_TsfSink = NULL;
int CTsfUiLessMode::m_nCandidateRefCount = NULL;

// Class to disable Cicero in case ImmDisableTextFrameService() doesn't disable
// it completely
class CDisableCicero {
public:
  CDisableCicero() : m_ptim(NULL), m_bComInit(false) {}
  ~CDisableCicero() { Uninitialize(); }
  void Initialize() {
    if (m_bComInit) {
      return;
    }
    HRESULT hr;
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr)) {
      m_bComInit = true;
      hr = CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER,
                            __uuidof(ITfThreadMgr), (void **)&m_ptim);
    }
  }
  void Uninitialize() {
    if (m_ptim) {
      m_ptim->Release();
      m_ptim = NULL;
    }
    if (m_bComInit)
      CoUninitialize();
    m_bComInit = false;
  }

  void DisableCiceroOnThisWnd(HWND hwnd) {
    if (m_ptim == NULL)
      return;
    ITfDocumentMgr *pdimPrev; // the dim that is associated previously.
    // Associate NULL dim to the window.
    // When this window gets the focus, Cicero does not work and IMM32 IME
    // will be activated.
    if (SUCCEEDED(m_ptim->AssociateFocus(hwnd, NULL, &pdimPrev))) {
      if (pdimPrev)
        pdimPrev->Release();
    }
  }

private:
  ITfThreadMgr *m_ptim;
  bool m_bComInit;
};
static CDisableCicero g_disableCicero;

/*---------------------------------------------------------------------------*/ /* Public */
CIME::CIME() {
  ms_hWnd = NULL;

  ms_bCandidateList = false;
  ms_bReadingInformation = false;

  Clear();

  m_max = 0;
  m_userMax = 0;

  m_bOnlyNumberMode = FALSE;
  m_hOrgIMC = NULL;

  m_bEnablePaste = true;
  m_bUseDefaultIME = false;
}

CIME::~CIME() {
  SAFE_FREE_LIBRARY(ms_hCurrentImeDll);
  SAFE_FREE_LIBRARY(ms_hImm32Dll);
}

#pragma warning(disable : 4996)
bool CIME::Initialize(HWND hWnd) {
  if (ms_bInitialized)
    return true;
  ms_hWnd = hWnd;

  g_disableCicero.Initialize();

  ms_stOSVI.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
  GetVersionExW(&ms_stOSVI);

  bool bUnicodeImm = false;
  // IMM in NT or Win98 supports Unicode
  if (ms_stOSVI.dwPlatformId == VER_PLATFORM_WIN32_NT ||
      (ms_stOSVI.dwMajorVersion > 4) ||
      (ms_stOSVI.dwMajorVersion == 4) && (ms_stOSVI.dwMinorVersion > 0)) {
    bUnicodeImm = true;
  }

  // Load ImmLock/ImmUnlock Function Proc
  wchar_t szPath[MAX_PATH + 1];
  ms_bDisableIMECompletely = false;

  if (GetSystemDirectoryW(szPath, MAX_PATH + 1)) {
    wcscat_s(szPath, L"\\imm32.dll");
    ms_hImm32Dll = LoadLibraryW(szPath);
    if (ms_hImm32Dll) {
      _ImmLockIMC = (INPUTCONTEXT * (WINAPI *)(HIMC))
          GetProcAddress(ms_hImm32Dll, "ImmLockIMC");
      _ImmUnlockIMC =
          (BOOL(WINAPI *)(HIMC))GetProcAddress(ms_hImm32Dll, "ImmUnlockIMC");
      _ImmLockIMCC =
          (LPVOID(WINAPI *)(HIMCC))GetProcAddress(ms_hImm32Dll, "ImmLockIMCC");
      _ImmUnlockIMCC =
          (BOOL(WINAPI *)(HIMCC))GetProcAddress(ms_hImm32Dll, "ImmUnlockIMCC");
      BOOL(WINAPI * _ImmDisableTextFrameService)(DWORD) =
          (BOOL(WINAPI *)(DWORD))GetProcAddress(ms_hImm32Dll,
                                                "ImmDisableTextFrameService");
      if (_ImmDisableTextFrameService)
        _ImmDisableTextFrameService((DWORD)-1);
    } else {
      ms_bDisableIMECompletely = true;
    }
  }

  ms_bInitialized = true;

  m_hOrgIMC = ImmGetContext(ms_hWnd);
  ImmReleaseContext(ms_hWnd, m_hOrgIMC);

  CheckInputLocale();
  ChangeInputLanguageWorker();
  SetSupportLevel(2);

  ms_bUILessMode = CTsfUiLessMode::SetupSinks() != FALSE;
  CheckToggleState();
  if (ms_bUILessMode) {
    ms_bChineseIME = (GETPRIMLANG() == LANG_CHINESE) &&
                     CTsfUiLessMode::CurrentInputLocaleIsIme();
    CTsfUiLessMode::UpdateImeState();
  }

  return true;
}

void CIME::Uninitialize() {
  if (!ms_bInitialized)
    return;
  CTsfUiLessMode::ReleaseSinks();
  if (ms_hWnd)
    ImmAssociateContext(ms_hWnd, m_hOrgIMC);
  ms_hWnd = NULL;
  m_hOrgIMC = NULL;
  SAFE_FREE_LIBRARY(ms_hCurrentImeDll);
  SAFE_FREE_LIBRARY(ms_hImm32Dll);
  g_disableCicero.Uninitialize();
  ms_bInitialized = false;
}

void CIME::UseDefaultIME() { m_bUseDefaultIME = true; }

bool CIME::IsIMEEnabled() { return ms_bImeEnabled; }

void CIME::EnableIME(bool bEnable) {
  if (!ms_bInitialized || !ms_hWnd)
    return;
  if (ms_bDisableIMECompletely)
    bEnable = false;
  ImmAssociateContext(ms_hWnd, bEnable ? m_hOrgIMC : NULL);
  ms_bImeEnabled = bEnable;
  if (bEnable)
    CheckToggleState();
  CTsfUiLessMode::EnableUiUpdates(bEnable);
}

void CIME::DisableIME() { EnableIME(false); }

void CIME::EnableCaptureInput() { ms_bCaptureInput = true; }

void CIME::DisableCaptureInput() { ms_bCaptureInput = false; }

bool CIME::IsCaptureEnabled() { return ms_bCaptureInput; }

void CIME::Clear() {
  ms_lastpos = 0;
  ms_curpos = 0;

  ms_compLen = 0;
  ms_ulbegin = 0;
  ms_ulend = 0;

  ms_selbegin = 0;
  ms_selend = 0;
}

int CIME::GetReading(std::string &rstrText) {
  char reading[IMEREADING_MAXLEN];

  if (ms_wstrReading.size() == 0)
    return 0;

  int readingLen = WideCharToMultiByte(
      CP_UTF8, WC_ERR_INVALID_CHARS, &ms_wstrReading[0], ms_wstrReading.size(),
      reading, sizeof(reading), NULL, NULL);
  if (readingLen <= 0)
    readingLen = 0;

  rstrText.append(reading, reading + readingLen);

  return rstrText.size();
}

int CIME::GetReadingError() { return ms_iReadingError; }

void CIME::SetMax(int iMax) { m_max = iMax; }

void CIME::SetUserMax(int iMax) { m_userMax = iMax; }

void CIME::SetText(const char *szText, int len) {
  ms_compLen = 0;
  ms_ulbegin = 0;
  ms_ulend = 0;

  if (!szText)
    len = 0;

  if (len < 0 && szText)
    len = (int)strlen(szText);

  const int cap =
      (int)(sizeof(m_wText) / sizeof(m_wText[0])); // includes space for null
  if (cap <= 0)
    return;

  // Empty
  if (!szText || len == 0) {
    m_wText[0] = L'\0';
    ms_lastpos = 0;
    ms_curpos = 0;
    ms_selbegin = ms_selend = 0;
    return;
  }

  // Required wchar count (not including null)
  int required = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, szText, len,
                                     nullptr, 0);
  if (required <= 0) {
    // invalid UTF-8 -> clear
    m_wText[0] = L'\0';
    ms_lastpos = 0;
    ms_curpos = 0;
    ms_selbegin = ms_selend = 0;
    return;
  }

  // Clamp to buffer capacity - 1 (reserve null)
  int outChars = std::min(required, cap - 1);

  // Convert
  int written = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, szText, len,
                                    m_wText, outChars);
  if (written <= 0)
    written = 0;

  m_wText[written] = L'\0';

  ms_lastpos = written;
  ms_curpos = ms_lastpos;
  ms_selbegin = ms_selend = ms_curpos;
}

int CIME::GetText(std::string &rstrText) {
  std::wstring w;
  w.reserve((size_t)ms_lastpos + (size_t)ms_compLen + 8);

  // before cursor
  if (ms_curpos > 0)
    w.append(m_wText, m_wText + ms_curpos);

  // composition (uncommitted)
  if (ms_compLen > 0)
    w.append(m_wszComposition, m_wszComposition + ms_compLen);

  // after cursor
  if (ms_lastpos > ms_curpos)
    w.append(m_wText + ms_curpos, m_wText + ms_lastpos);

  std::string utf8 = WideToUtf8(w);
  rstrText.append(utf8);

  return (int)utf8.size();
}

int CIME::GetCandidatePageCount() { return ms_dwCandidatePageSize; }

int CIME::GetCandidateCount() { return ms_dwCandidateCount; }

int CIME::GetCandidate(DWORD index, std::string &rstrText) {
  if (index >= MAX_CANDLIST)
    return 0;

  LPCWSTR wszText = ms_wszCandidate[index];
  if (wszText == NULL)
    return 0;

  int wTextLen = wcslen(wszText);
  if (wTextLen == 0)
    return 0;

  char text[IMESTR_MAXLEN];
  rstrText.append(WideToUtf8(std::wstring(wszText, wszText + wTextLen)));

  return wTextLen;
}

int CIME::GetCandidateSelection() { return ms_dwCandidateSelection; }

void CIME::SetInputMode(DWORD dwMode) {
  HIMC hImc = ImmGetContext(ms_hWnd);

  ImmSetConversionStatus(hImc, dwMode, IME_SMODE_AUTOMATIC);

  ImmReleaseContext(ms_hWnd, hImc);
}

DWORD CIME::GetInputMode() {
  DWORD dwCMode, dwSMode;

  HIMC hImc = ImmGetContext(ms_hWnd);

  ImmGetConversionStatus(hImc, &dwCMode, &dwSMode);

  ImmReleaseContext(ms_hWnd, hImc);

  return dwCMode;
}

void CIME::SetNumberMode() { m_bOnlyNumberMode = TRUE; }

void CIME::SetStringMode() { m_bOnlyNumberMode = FALSE; }

void CIME::AddExceptKey(wchar_t key) { m_exceptKey.push_back(key); }

void CIME::ClearExceptKey() { m_exceptKey.clear(); }

bool CIME::__IsWritable(wchar_t key) {
  if (m_exceptKey.end() ==
      std::find(m_exceptKey.begin(), m_exceptKey.end(), key))
    return true;
  else
    return false;
}

void CIME::EnablePaste(bool bFlag) { m_bEnablePaste = bFlag; }

namespace {
inline bool HasSelection() { return CIME::ms_selbegin != CIME::ms_selend; }

inline void NormalizeSelection() {
  if (CIME::ms_selbegin > CIME::ms_selend)
    std::swap(CIME::ms_selbegin, CIME::ms_selend);
}
} // namespace

bool CIME::CanUndo() { return !ms_undo.empty(); }

bool CIME::CanRedo() { return !ms_redo.empty(); }

void CIME::ClearUndoRedo() {
  ms_undo.clear();
  ms_redo.clear();
}

void CIME::PushUndoState() {
  SUndoState st;
  st.text = m_wText;
  st.curpos = ms_curpos;
  st.lastpos = ms_lastpos;
  st.selbegin = ms_selbegin;
  st.selend = ms_selend;

  ms_undo.push_back(std::move(st));
  if (ms_undo.size() > MAX_UNDO)
    ms_undo.erase(ms_undo.begin());
  ms_redo.clear();
}

void CIME::RestoreState(const SUndoState &st) {
  const std::string utf8 = WideToUtf8(st.text);
  SetText(utf8.c_str(), (int)utf8.size());

  ms_curpos = std::max(0, std::min(st.curpos, ms_lastpos));
  ms_selbegin = std::max(0, std::min(st.selbegin, ms_lastpos));
  ms_selend = std::max(0, std::min(st.selend, ms_lastpos));

  if (ms_pEvent)
    ms_pEvent->OnUpdate();
}

void CIME::Undo() {
  if (ms_undo.empty())
    return;

  // push current into redo
  SUndoState cur;
  cur.text.assign(m_wText, m_wText + ms_lastpos);
  cur.curpos = ms_curpos;
  cur.lastpos = ms_lastpos;
  cur.selbegin = ms_selbegin;
  cur.selend = ms_selend;
  ms_redo.push_back(std::move(cur));
  if (ms_redo.size() > MAX_UNDO)
    ms_redo.erase(ms_redo.begin());

  // restore from undo
  SUndoState st = std::move(ms_undo.back());
  ms_undo.pop_back();

  RestoreState(st);

  if (ms_pEvent)
    ms_pEvent->OnUpdate();
}

void CIME::Redo() {
  if (ms_redo.empty())
    return;

  // push current into undo (WITHOUT clearing redo)
  SUndoState cur;
  cur.text.assign(m_wText, m_wText + ms_lastpos);
  cur.curpos = ms_curpos;
  cur.lastpos = ms_lastpos;
  cur.selbegin = ms_selbegin;
  cur.selend = ms_selend;

  ms_undo.push_back(std::move(cur));
  if (ms_undo.size() > MAX_UNDO)
    ms_undo.erase(ms_undo.begin());

  // restore from redo
  SUndoState st = std::move(ms_redo.back());
  ms_redo.pop_back();

  RestoreState(st);

  if (ms_pEvent)
    ms_pEvent->OnUpdate();
}

void CIME::SelectAll() {
  ms_selbegin = 0;
  ms_selend = ms_lastpos;
  ms_curpos = ms_lastpos;

  if (ms_pEvent)
    ms_pEvent->OnUpdate();
}

void CIME::DeleteSelection() {
  NormalizeSelection();
  if (!HasSelection())
    return;

  PushUndoState();

  const int a = ms_selbegin;
  const int b = ms_selend;
  const int selLen = b - a;
  if (selLen <= 0)
    return;

  const int tailCount = (ms_lastpos - b) + 1;
  memmove(m_wText + a, m_wText + b, (size_t)tailCount * sizeof(wchar_t));

  ms_lastpos -= selLen;
  ms_curpos = a;

  ms_selbegin = ms_selend = a;

  if (ms_pEvent)
    ms_pEvent->OnUpdate();
}

void CIME::CopySelectionToClipboard(HWND hWnd) {
  NormalizeSelection();
  if (!HasSelection())
    return;

  const int a = ms_selbegin;
  const int b = ms_selend;
  const int selLen = b - a;
  if (selLen <= 0)
    return;

  if (!OpenClipboard(hWnd))
    return;

  EmptyClipboard();

  const SIZE_T bytes = (SIZE_T)(selLen + 1) * sizeof(wchar_t);
  HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
  if (!hMem) {
    CloseClipboard();
    return;
  }

  wchar_t *dst = (wchar_t *)GlobalLock(hMem);
  if (!dst) {
    GlobalFree(hMem);
    CloseClipboard();
    return;
  }

  memcpy(dst, m_wText + a, (size_t)selLen * sizeof(wchar_t));
  dst[selLen] = L'\0';

  GlobalUnlock(hMem);

  if (!SetClipboardData(CF_UNICODETEXT, hMem))
    GlobalFree(hMem); // only free if SetClipboardData failed

  CloseClipboard();
}

void CIME::CutSelectionToClipboard(HWND hWnd) {
  CopySelectionToClipboard(hWnd);
  DeleteSelection();
}

void CIME::PasteTextFromClipBoard() {
  if (!m_bEnablePaste)
    return;

  if (!OpenClipboard(ms_hWnd))
    return;

  // 1) Prefer Unicode clipboard
  if (HANDLE hUni = GetClipboardData(CF_UNICODETEXT)) {
    if (wchar_t *wbuf = (wchar_t *)GlobalLock(hUni)) {
      InsertString(wbuf, (int)wcslen(wbuf));
      GlobalUnlock(hUni);
    }

    CloseClipboard();
    if (ms_pEvent)
      ms_pEvent->OnUpdate();
    return;
  }

  CloseClipboard();
  if (ms_pEvent)
    ms_pEvent->OnUpdate();
}

void CIME::FinalizeString(bool bSend) {
  HIMC himc;
  static bool s_bProcessing = false; // to avoid infinite recursion
  if (!ms_bInitialized || s_bProcessing ||
      NULL == (himc = ImmGetContext(ms_hWnd)))
    return;
  s_bProcessing = true;

  if (ms_dwIMELevel == 2 && bSend) {
    //// Send composition string to app.
    // LONG lRet = lstrlenW( m_wszComposition );
    ////assert( lRet >= 2);
    //// In case of CHT IME, don't send the trailing double byte space, if it
    /// exists.
    // if ( GETLANG() == LANG_CHT && (lRet >= 1)
    //	&& m_wszComposition[lRet - 1] == 0x3000 )
    //{
    //	lRet--;
    // }
    // SendCompString();
  }

  // InitCompStringData();
  //  clear composition string in IME
  ImmNotifyIME(himc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
  if (ms_bUILessMode) {
    // For some reason ImmNotifyIME doesn't work on DaYi and Array CHT IMEs.
    // Cancel composition string by setting zero-length string.
    ImmSetCompositionStringW(himc, SCS_SETSTR, NULL, 0, NULL, 0);
  }
  // the following line is necessary as Korean IME doesn't close cand list when
  // comp string is cancelled.
  ImmNotifyIME(himc, NI_CLOSECANDIDATE, 0, 0);
  ImmReleaseContext(ms_hWnd, himc);
  // Zooty2 RAID #4759: Sometimes application doesn't receive IMN_CLOSECANDIDATE
  // on Alt+Tab So the same code for IMN_CLOSECANDIDATE is replicated here.
  CloseCandidateList();
  s_bProcessing = false;
}

int CIME::GetCompLen() { return ms_compLen; }

int CIME::GetULBegin() { return ms_ulbegin; }

int CIME::GetULEnd() { return ms_ulend; }

int CIME::GetSelBegin() { return ms_selbegin; }

int CIME::GetSelEnd() { return ms_selend; }

void CIME::ClearSelection() { ms_selbegin = ms_selend = ms_curpos; }

void CIME::CloseCandidateList() {
  ms_bCandidateList = false;
  ms_dwCandidateCount = 0;
  memset(&ms_wszCandidate, 0, sizeof(ms_wszCandidate));
  if (ms_pEvent)
    ms_pEvent->OnCloseCandidateList();
}

void CIME::CloseReadingInformation() {
  CIME::ms_bReadingInformation = false;
  if (CIME::ms_pEvent)
    CIME::ms_pEvent->OnCloseReadingWnd();
}

void CIME::ChangeInputLanguage() {
  UINT uLanguage = (UINT)GETLANG();
  CheckToggleState();
  ChangeInputLanguageWorker();
  if (uLanguage != GETLANG()) {
    SetSupportLevel((GETPRIMLANG() == LANG_KOREAN) ? 3 : ms_dwIMELevelSaved);
  }
}

void CIME::ChangeInputLanguageWorker() {
  if (!ms_bUILessMode)
    ms_iCandListIndexBase = (ms_hklCurrent == _CHT_HKL_DAYI) ? 0 : 1;
  SetupImeApi();
}

void CIME::SetSupportLevel(DWORD dwImeLevel) {
  if (dwImeLevel < 2 || 3 < dwImeLevel)
    return;
  if (GETPRIMLANG() == LANG_KOREAN) {
    dwImeLevel = 3;
  }
  ms_dwIMELevel = dwImeLevel;
  // cancel current composition string.
  FinalizeString();
  // SetCompStringColor();
}

/*---------------------------------------------------------------------------*/ /* Protected */
void CIME::IncCurPos() {
  if (ms_curpos < ms_lastpos) {
    // Skip over tags (color tags, hyperlink tags, etc.)
    int tagLen = 0;
    std::wstring tagExtra;
    int tag = GetTextTag(&m_wText[ms_curpos], ms_lastpos - ms_curpos, tagLen,
                         tagExtra);

    if (tag != TEXT_TAG_PLAIN && tagLen > 0) {
      // We're at the start of a tag - skip the entire tag
      ms_curpos += tagLen;
    } else {
      // Normal character - move forward by 1
      ++ms_curpos;
    }
  }
}

void CIME::DecCurPos() {
  if (ms_curpos > 0) {
    // Move back one position
    --ms_curpos;

    // If we landed in the middle of a tag, skip backward to the tag start
    // Keep checking backward for tag starts until we find the beginning
    while (ms_curpos > 0) {
      int tagLen = 0;
      std::wstring tagExtra;

      // Check if current position is a tag start
      int tag = GetTextTag(&m_wText[ms_curpos], ms_lastpos - ms_curpos, tagLen,
                           tagExtra);

      if (tag != TEXT_TAG_PLAIN && tagLen > 0) {
        // We're at a tag start - this is good, stop here
        break;
      }

      // Check if the character BEFORE us starts a tag that extends over our
      // position
      if (ms_curpos > 0) {
        int prevTag =
            GetTextTag(&m_wText[ms_curpos - 1], ms_lastpos - (ms_curpos - 1),
                       tagLen, tagExtra);
        if (prevTag != TEXT_TAG_PLAIN && tagLen > 1) {
          // Previous position starts a multi-char tag - move to it
          --ms_curpos;
          continue;
        }
      }

      // Not in a tag, we're done
      break;
    }
  }
}

int CIME::GetCurPos() {
  int pos = GetTextTagOutputLen(m_wText, ms_curpos);
  return pos;
  // return ms_curpos;
}

void CIME::SetCurPos(int offset) {
  if (offset < 0 || offset > ms_lastpos) {
    ms_curpos = ms_lastpos;
    return;
  } else {
    ms_curpos = std::min(ms_lastpos, GetTextTagInternalPosFromRenderPos(
                                         m_wText, ms_lastpos, offset));
  }
}

void CIME::DelCurPos() {
  // If there is a selection, delete it first
  if (ms_selbegin != ms_selend) {
    DeleteSelection();
    return;
  }

  if (ms_curpos < ms_lastpos) {
    int eraseCount =
        FindColorTagEndPosition(m_wText + ms_curpos, ms_lastpos - ms_curpos) +
        1;
    size_t remainingChars =
        ms_lastpos - ms_curpos - eraseCount + 1; // +1 for null terminator
    wmemmove(m_wText + ms_curpos, m_wText + ms_curpos + eraseCount,
             remainingChars); // wcscpy > wmemmove to handle overlapping memory
    ms_lastpos -= eraseCount;
    ms_curpos = std::min(ms_lastpos, ms_curpos);
  }
}

void CIME::PasteString(const char *str) {
  const char *begin = str;
  const char *end = str + strlen(str);
  wchar_t m_wText[IMESTR_MAXLEN];
  int wstrLen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, begin,
                                    end - begin, m_wText, IMESTR_MAXLEN);

  // Check if pasting a hyperlink (contains |H tag)
  bool isHyperlink = false;
  for (int i = 0; i < wstrLen - 1; ++i) {
    if (m_wText[i] == L'|' && m_wText[i + 1] == L'H') {
      isHyperlink = true;
      break;
    }
  }

  // If pasting hyperlink, always append to end (avoid cursor position issues)
  if (isHyperlink) {
    ms_curpos = ms_lastpos; // Move cursor to end first
  }

  InsertString(m_wText, wstrLen);
  if (ms_pEvent)
    ms_pEvent->OnUpdate();
}

/*---------------------------------------------------------------------------*/ /* Private */
void CIME::InsertString(wchar_t *wString, int iSize) {
  PushUndoState();

  if (!wString || iSize <= 0)
    return;

  // Replace selection first
  if (ms_selbegin != ms_selend) {
    DeleteSelection(); // sets ms_curpos to ms_selbegin and clears selection
  }

  if (ms_lastpos < 0)
    ms_lastpos = 0;
  if (ms_curpos < 0)
    ms_curpos = 0;
  if (ms_curpos > ms_lastpos)
    ms_curpos = ms_lastpos;

  // Need room for NUL terminator
  if (ms_lastpos + iSize >= IMESTR_MAXLEN)
    return;

  if (IsMax(wString, iSize))
    return;

  // Move tail including the existing NUL
  if (ms_curpos < ms_lastpos) {
    size_t tailChars = (size_t)(ms_lastpos - ms_curpos) + 1; // +1 for NUL
    wmemmove(m_wText + ms_curpos + iSize, m_wText + ms_curpos, tailChars);
  }

  wmemcpy(m_wText + ms_curpos, wString, (size_t)iSize);

  ms_curpos += iSize;
  ms_lastpos += iSize;

  // Ensure termination
  m_wText[ms_lastpos] = L'\0';
}

void CIME::OnChar(wchar_t c) {
  if (m_bOnlyNumberMode)
    if (!iswdigit(c))
      return;

  if (c == 0x16)
    return;

  InsertString(&c, 1);
}

void CIME::CompositionProcess(HIMC hImc) {
  LONG bytes = ImmGetCompositionStringW(hImc, GCS_COMPSTR, m_wszComposition,
                                        sizeof(m_wszComposition));
  if (bytes <= 0) {
    ms_compLen = 0;
    m_wszComposition[0] = L'\0';
    return;
  }

  ms_compLen = (int)(bytes / (LONG)sizeof(wchar_t));

  if (IsMax(m_wszComposition, ms_compLen)) {
    ImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
    ms_compLen = 0;
    m_wszComposition[0] = L'\0';
  }
}

void CIME::CompositionProcessBuilding(HIMC hImc) {
  int textLen = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, m_wText,
                                    ms_lastpos, nullptr, 0, nullptr, nullptr);
  if (textLen <= 0)
    textLen = 0;

  if (textLen >= m_max) {
    ImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
    ms_compLen = 0;
    return;
  }

  ms_compLen = ImmGetCompositionStringW(hImc, GCS_COMPSTR, m_wszComposition,
                                        sizeof(m_wszComposition)) /
               sizeof(wchar_t);
}

void CIME::ResultProcess(HIMC hImc) {
  wchar_t temp[IMESTR_MAXLEN];

  LONG bytes =
      ImmGetCompositionStringW(hImc, GCS_RESULTSTR, temp, sizeof(temp));
  if (bytes <= 0)
    return;

  int len = (int)(bytes / (LONG)sizeof(wchar_t));
  if (len <= 0)
    return;

  InsertString(temp, len);
}

void CIME::AttributeProcess(HIMC hImc) {
  BYTE attribute[IMESTR_MAXLEN];
  LONG attributeLen = ImmGetCompositionStringW(hImc, GCS_COMPATTR, &attribute,
                                               sizeof(attribute)) /
                      sizeof(BYTE);

  int start, end;
  for (start = 0; start < attributeLen; ++start)
    if (attribute[start] == ATTR_TARGET_CONVERTED ||
        attribute[start] == ATTR_TARGET_NOTCONVERTED)
      break;
  for (end = start; end < attributeLen; ++end)
    if (attribute[end] != attribute[start])
      break;

  ms_ulbegin = start;
  ms_ulend = end;
}

static const wchar_t *CandidateStringAt(const CANDIDATELIST *cl, UINT idx) {
  if (!cl || idx >= cl->dwCount)
    return L"";
  const BYTE *base = reinterpret_cast<const BYTE *>(cl);
  return reinterpret_cast<const wchar_t *>(base + cl->dwOffset[idx]);
}

void CIME::CandidateProcess(HIMC hImc) {
  DWORD bytes = ImmGetCandidateListW(hImc, 0, nullptr, 0);
  if (bytes == 0)
    return;

  std::vector<BYTE> buf(bytes);

  auto *cl = reinterpret_cast<CANDIDATELIST *>(buf.data());
  DWORD got = ImmGetCandidateListW(hImc, 0, cl, bytes);
  if (got == 0 || cl->dwCount == 0)
    return;

  ms_bCandidateList = true;
  ms_dwCandidateCount = cl->dwCount;
  ms_dwCandidateSelection = cl->dwSelection;

  UINT pageStart = 0;

  if (ms_bUILessMode && cl->dwPageStart < cl->dwCount) {
    pageStart = cl->dwPageStart;
  } else {
    const UINT uiPage = (MAX_CANDLIST > 1) ? (MAX_CANDLIST - 1) : 1;
    if (ms_dwCandidateSelection < cl->dwCount)
      pageStart = (ms_dwCandidateSelection / uiPage) * uiPage;
  }

  UINT pageSize = 0;
  if (cl->dwPageSize > 0)
    pageSize = (UINT)std::min<DWORD>(cl->dwPageSize, (DWORD)MAX_CANDLIST);
  else
    pageSize =
        (UINT)std::min<DWORD>(cl->dwCount - pageStart, (DWORD)MAX_CANDLIST);

  ms_dwCandidatePageSize = pageSize;

  // selection relative to page (or none)
  if (ms_dwCandidateSelection < cl->dwCount &&
      ms_dwCandidateSelection >= pageStart)
    ms_dwCandidateSelection = ms_dwCandidateSelection - pageStart;
  else
    ms_dwCandidateSelection = (DWORD)-1;

  memset(ms_wszCandidate, 0, sizeof(ms_wszCandidate));

  for (UINT j = 0; j < pageSize; ++j) {
    UINT srcIdx = pageStart + j;
    const wchar_t *src = CandidateStringAt(cl, srcIdx);
    wcsncpy_s(ms_wszCandidate[j], CIME::MAX_CANDIDATE_LENGTH, src, _TRUNCATE);
  }

  // don't display selection in candidate list in case of Korean and old Chinese
  // IME.
  if (GETPRIMLANG() == LANG_KOREAN || (GETLANG() == LANG_CHT && !GetImeId()))
    ms_dwCandidateSelection = (DWORD)-1;
}

void CIME::ReadingProcess(HIMC hImc) {
  if (!ms_adwId[0]) {
    return;
  }

  DWORD dwErr = 0;

  if (_GetReadingString) {
    UINT uMaxUiLen;
    BOOL bVertical;
    // Obtain the reading string size
    int wstrLen =
        _GetReadingString(hImc, 0, NULL, (PINT)&dwErr, &bVertical, &uMaxUiLen);

    if (wstrLen == 0) {
      ms_wstrReading.resize(0);
    } else {
      wchar_t *wstr = (wchar_t *)alloca(sizeof(wchar_t) * wstrLen);
      _GetReadingString(hImc, wstrLen, wstr, (PINT)&dwErr, &bVertical,
                        &uMaxUiLen);
      ms_wstrReading.assign(wstr, wstr + wstrLen);
    }

    ms_bHorizontalReading = (bVertical == 0);

  } else {

    // IMEs that doesn't implement Reading String API
    wchar_t *temp = NULL;
    DWORD tempLen = 0;
    bool bUnicodeIme = false;
    INPUTCONTEXT *lpIC = _ImmLockIMC(hImc);

    if (lpIC == NULL) {
      temp = NULL;
      tempLen = 0;
    } else {
      LPBYTE p = 0;
      switch (ms_adwId[0]) {
      case IMEID_CHT_VER42: // New(Phonetic/ChanJie)IME98  : 4.2.x.x // Win98
      case IMEID_CHT_VER43: // New(Phonetic/ChanJie)IME98a : 4.3.x.x // WinMe,
                            // Win2k
      case IMEID_CHT_VER44: // New ChanJie IME98b          : 4.4.x.x // WinXP
        p = *(LPBYTE *)((LPBYTE)_ImmLockIMCC(lpIC->hPrivate) + 24);
        if (!p)
          break;
        tempLen = *(DWORD *)(p + 7 * 4 + 32 * 4);
        dwErr = *(DWORD *)(p + 8 * 4 + 32 * 4);
        temp = (wchar_t *)(p + 56);
        bUnicodeIme = true;
        break;

      case IMEID_CHT_VER50: // 5.0.x.x // WinME
        p = *(LPBYTE *)((LPBYTE)_ImmLockIMCC(lpIC->hPrivate) + 3 * 4);
        if (!p)
          break;
        p = *(LPBYTE *)((LPBYTE)p + 1 * 4 + 5 * 4 + 4 * 2);
        if (!p)
          break;
        tempLen = *(DWORD *)(p + 1 * 4 + (16 * 2 + 2 * 4) + 5 * 4 + 16);
        dwErr = *(DWORD *)(p + 1 * 4 + (16 * 2 + 2 * 4) + 5 * 4 + 16 + 1 * 4);
        temp = (wchar_t *)(p + 1 * 4 + (16 * 2 + 2 * 4) + 5 * 4);
        bUnicodeIme = false;
        break;

      case IMEID_CHT_VER51: // 5.1.x.x // IME2002(w/OfficeXP)
      case IMEID_CHT_VER52: // 5.2.x.x // (w/whistler)
      case IMEID_CHS_VER53: // 5.3.x.x // SCIME2k or MSPY3 (w/OfficeXP and
                            // Whistler)
        p = *(LPBYTE *)((LPBYTE)_ImmLockIMCC(lpIC->hPrivate) + 4);
        if (!p)
          break;
        p = *(LPBYTE *)((LPBYTE)p + 1 * 4 + 5 * 4);
        if (!p)
          break;
        tempLen = *(DWORD *)(p + 1 * 4 + (16 * 2 + 2 * 4) + 5 * 4 + 16 * 2);
        dwErr =
            *(DWORD *)(p + 1 * 4 + (16 * 2 + 2 * 4) + 5 * 4 + 16 * 2 + 1 * 4);
        temp = (wchar_t *)(p + 1 * 4 + (16 * 2 + 2 * 4) + 5 * 4);
        bUnicodeIme = true;
        break;

      // the code tested only with Win 98 SE (MSPY 1.5/ ver 4.1.0.21)
      case IMEID_CHS_VER41: {
        int nOffset;
        nOffset = (ms_adwId[1] >= 0x00000002) ? 8 : 7;

        p = *(LPBYTE *)((LPBYTE)_ImmLockIMCC(lpIC->hPrivate) + nOffset * 4);
        if (!p)
          break;
        tempLen = *(DWORD *)(p + 7 * 4 + 16 * 2 * 4);
        dwErr = *(DWORD *)(p + 8 * 4 + 16 * 2 * 4);
        dwErr = std::min(dwErr, tempLen);
        temp = (wchar_t *)(p + 6 * 4 + 16 * 2 * 1);
        bUnicodeIme = true;
      } break;

      case IMEID_CHS_VER42: // 4.2.x.x // SCIME98 or MSPY2 (w/Office2k, Win2k,
                            // WinME, etc)
      {
        OSVERSIONINFOW osi;
        osi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
        GetVersionExW(&osi);

        int nTcharSize = (osi.dwPlatformId == VER_PLATFORM_WIN32_NT)
                             ? sizeof(wchar_t)
                             : sizeof(char);
        p = *(LPBYTE *)((LPBYTE)_ImmLockIMCC(lpIC->hPrivate) + 1 * 4 + 1 * 4 +
                        6 * 4);
        if (!p)
          break;
        tempLen =
            *(DWORD *)(p + 1 * 4 + (16 * 2 + 2 * 4) + 5 * 4 + 16 * nTcharSize);
        dwErr = *(DWORD *)(p + 1 * 4 + (16 * 2 + 2 * 4) + 5 * 4 +
                           16 * nTcharSize + 1 * 4);
        temp = (wchar_t *)(p + 1 * 4 + (16 * 2 + 2 * 4) + 5 * 4);
        bUnicodeIme =
            (osi.dwPlatformId == VER_PLATFORM_WIN32_NT) ? true : false;
      } break;

      default:
        temp = NULL;
        tempLen = 0;
        break;
      }
    }

    if (tempLen == 0) {
      ms_wstrReading.resize(0);
    } else {
      if (bUnicodeIme) {
        ms_wstrReading.assign(temp, temp + tempLen);
      } else {
        int wstrLen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                          (char *)temp, tempLen, NULL, 0);
        wchar_t *wstr = (wchar_t *)alloca(sizeof(wchar_t) * wstrLen);
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, (char *)temp,
                            tempLen, wstr, wstrLen);
        ms_wstrReading.assign(wstr, wstr + wstrLen);
      }
    }

    _ImmUnlockIMCC(lpIC->hPrivate);
    _ImmUnlockIMC(hImc);

    ms_bHorizontalReading = GetReadingWindowOrientation();
  }

  if (ms_wstrReading.size()) {
    ms_bReadingInformation = true;
    if (ms_pEvent)
      ms_pEvent->OnOpenReadingWnd();
  } else {
    CloseReadingInformation();
  }
}

bool CIME::IsMax(const wchar_t *wInput, int len) {
  if (ms_lastpos + len >= IMESTR_MAXLEN) // keep room for NUL
    return true;

  int textLen = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, m_wText,
                                    ms_lastpos, nullptr, 0, nullptr, nullptr);
  int inputLen = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wInput, len,
                                     nullptr, 0, nullptr, nullptr);

  if (textLen <= 0)
    textLen = 0;

  if (inputLen <= 0)
    inputLen = 0;

  if (textLen + inputLen > m_max)
    return true;
  else if (m_userMax != 0 && m_max != m_userMax) {
    std::wstring str = GetTextTagOutputString(m_wText, ms_lastpos);
    std::wstring input = GetTextTagOutputString(wInput, len);

    int textLen =
        WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, str.c_str(),
                            str.length(), 0, 0, NULL, NULL);
    if (textLen <= 0)
      textLen = 0;

    int inputLen =
        WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, input.c_str(),
                            input.length(), 0, 0, NULL, NULL);
    if (inputLen <= 0)
      inputLen = 0;

    return textLen + inputLen > m_userMax;
  }

  return false;
}

DWORD CIME::GetImeId(UINT uIndex) {
  static HKL hklPrev = 0;
  wchar_t szTmp[1024];

  if (uIndex >= COUNTOF(ms_adwId))
    return 0;
  HKL hkl = ms_hklCurrent;
  if (hklPrev == hkl)
    return ms_adwId[uIndex];
  hklPrev = hkl;

  DWORD dwLang = ((DWORD)hkl & 0xffff);

  if (ms_bUILessMode && GETLANG() == LANG_CHT) {
    // In case of Vista, artifitial value is returned so that it's not
    // considered as older IME.
    ms_adwId[0] = IMEID_CHT_VER_VISTA;
    ms_adwId[1] = 0;
    return ms_adwId[0];
  }

  if (!((ms_hklCurrent == _CHT_HKL_NEW_PHONETIC) ||
        (ms_hklCurrent == _CHT_HKL_NEW_CHANG_JIE) ||
        (ms_hklCurrent == _CHT_HKL_NEW_QUICK) ||
        (ms_hklCurrent == _CHT_HKL_HK_CANTONESE) ||
        (ms_hklCurrent == _CHS_HKL))) {
    ms_adwId[0] = ms_adwId[1] = 0;
    return 0;
  }

  // Buffer size parameter
  if (!ImmGetIMEFileNameW(ms_hklCurrent, szTmp, _countof(szTmp))) {
    ms_adwId[0] = ms_adwId[1] = 0;
    return 0;
  }

  if (!_GetReadingString) {
    if ((CompareStringW(LCID_INVARIANT, NORM_IGNORECASE, szTmp, -1,
                        CHT_IMEFILENAME1, -1) != CSTR_EQUAL) &&
        (CompareStringW(LCID_INVARIANT, NORM_IGNORECASE, szTmp, -1,
                        CHT_IMEFILENAME2, -1) != CSTR_EQUAL) &&
        (CompareStringW(LCID_INVARIANT, NORM_IGNORECASE, szTmp, -1,
                        CHT_IMEFILENAME3, -1) != CSTR_EQUAL) &&
        (CompareStringW(LCID_INVARIANT, NORM_IGNORECASE, szTmp, -1,
                        CHS_IMEFILENAME1, -1) != CSTR_EQUAL) &&
        (CompareStringW(LCID_INVARIANT, NORM_IGNORECASE, szTmp, -1,
                        CHS_IMEFILENAME2, -1) != CSTR_EQUAL)) {
      ms_adwId[0] = ms_adwId[1] = 0;
      return 0;
    }
  }

  DWORD dwVerHandle;
  DWORD dwVerSize = GetFileVersionInfoSizeW(szTmp, &dwVerHandle);
  LANGID langId = LOWORD(ms_hklCurrent);

  if (dwVerSize) {
    LPVOID lpVerBuffer = alloca(dwVerSize);

    if (GetFileVersionInfoW(szTmp, dwVerHandle, dwVerSize, lpVerBuffer)) {
      LPVOID lpVerData;
      UINT cbVerData;

      if (VerQueryValueW(lpVerBuffer, L"\\", &lpVerData, &cbVerData)) {
        DWORD dwVer = ((VS_FIXEDFILEINFO *)lpVerData)->dwFileVersionMS;
        dwVer = (dwVer & 0x00ff0000) << 8 | (dwVer & 0x000000ff) << 16;

        if (_GetReadingString ||
            (langId == LANG_CHT &&
             (dwVer == MAKEIMEVERSION(4, 2) || dwVer == MAKEIMEVERSION(4, 3) ||
              dwVer == MAKEIMEVERSION(4, 4) || dwVer == MAKEIMEVERSION(5, 0) ||
              dwVer == MAKEIMEVERSION(5, 1) || dwVer == MAKEIMEVERSION(5, 2) ||
              dwVer == MAKEIMEVERSION(6, 0))) ||
            (langId == LANG_CHS &&
             (dwVer == MAKEIMEVERSION(4, 1) || dwVer == MAKEIMEVERSION(4, 2) ||
              dwVer == MAKEIMEVERSION(5, 3)))) {
          ms_adwId[0] = dwVer | langId;
          ms_adwId[1] = ((VS_FIXEDFILEINFO *)lpVerData)->dwFileVersionLS;
          return ms_adwId[uIndex];
        }
      }
    }
  }
  ms_adwId[0] = ms_adwId[1] = 0;
  return ms_adwId[0];
}

bool CIME::GetReadingWindowOrientation() {
  bool bHorizontalReading = (ms_hklCurrent == _CHS_HKL) ||
                            (ms_hklCurrent == _CHT_HKL_NEW_CHANG_JIE) ||
                            (ms_adwId[0] == 0);
  if (!bHorizontalReading && (GETLANG() == LANG_CHT)) {
    wchar_t wszRegPath[MAX_PATH];
    HKEY hKey;
    DWORD dwVer = ms_adwId[0] & 0xFFFF0000;
    wcscpy_s(wszRegPath, L"software\\microsoft\\windows\\currentversion\\");
    wcscpy_s(wszRegPath,
             (dwVer >= MAKEIMEVERSION(5, 1)) ? L"MSTCIPH" : L"TINTLGNT");
    LONG lRc = RegOpenKeyExW(HKEY_CURRENT_USER, wszRegPath, 0, KEY_READ, &hKey);
    if (lRc == ERROR_SUCCESS) {
      DWORD dwSize = sizeof(DWORD), dwMapping, dwType;
      lRc = RegQueryValueExW(hKey, L"Keyboard Mapping", NULL, &dwType,
                             (PBYTE)&dwMapping, &dwSize);
      if (lRc == ERROR_SUCCESS) {
        if ((dwVer <= MAKEIMEVERSION(5, 0) &&
             ((BYTE)dwMapping == 0x22 || (BYTE)dwMapping == 0x23)) ||
            ((dwVer == MAKEIMEVERSION(5, 1) || dwVer == MAKEIMEVERSION(5, 2)) &&
             (BYTE)dwMapping >= 0x22 && (BYTE)dwMapping <= 0x24)) {
          bHorizontalReading = true;
        }
      }
      RegCloseKey(hKey);
    }
  }

  return bHorizontalReading;
}

void CIME::SetupImeApi() {
  wchar_t szImeFile[MAX_PATH + 1];

  _GetReadingString = NULL;
  _ShowReadingWindow = NULL;
  ms_bUseIMMCandidate = false;

  if (ImmGetIMEFileNameW(ms_hklCurrent, szImeFile, COUNTOF(szImeFile) - 1) == 0)
    return;

  std::string imeUtf8 = WideToUtf8(szImeFile);

  if (_stricmp(imeUtf8.c_str(), CHS_IMEFILENAME_QQPINYIN) == 0 ||
      _stricmp(imeUtf8.c_str(), CHS_IMEFILENAME_SOGOUPY) == 0 ||
      _stricmp(imeUtf8.c_str(), CHS_IMEFILENAME_GOOGLEPINYIN2) == 0) {
    ms_bUseIMMCandidate = true;
  }

  if (ms_bUILessMode)
    return;

  SAFE_FREE_LIBRARY(ms_hCurrentImeDll);
  ms_hCurrentImeDll = LoadLibraryW(szImeFile);

  if (ms_hCurrentImeDll) {
    _GetReadingString =
        (UINT(WINAPI *)(HIMC, UINT, LPWSTR, PINT, BOOL *, PUINT))(
            GetProcAddress(ms_hCurrentImeDll, "GetReadingString"));
    _ShowReadingWindow = (BOOL(WINAPI *)(HIMC, BOOL))(
        GetProcAddress(ms_hCurrentImeDll, "ShowReadingWindow"));

    if (_ShowReadingWindow) {
      HIMC hImc = ImmGetContext(ms_hWnd);
      if (hImc) {
        _ShowReadingWindow(hImc, false);
        ImmReleaseContext(ms_hWnd, hImc);
      }
    }
  }
}

static unsigned long _strtoul(LPCSTR psz, LPTSTR *, int) {
  if (!psz)
    return 0;

  ULONG ulRet = 0;
  if (psz[0] == '0' && (psz[1] == 'x' || psz[1] == 'X')) {
    psz += 2;
    ULONG ul = 0;
    while (*psz) {
      if ('0' <= *psz && *psz <= '9')
        ul = *psz - '0';
      else if ('A' <= *psz && *psz <= 'F')
        ul = *psz - 'A' + 10;
      else if ('a' <= *psz && *psz <= 'f')
        ul = *psz - 'a' + 10;
      else
        break;
      ulRet = ulRet * 16 + ul;
      psz++;
    }
  } else {
    while (*psz && ('0' <= *psz && *psz <= '9')) {
      ulRet = ulRet * 10 + (*psz - '0');
      psz++;
    }
  }
  return ulRet;
}

void CIME::CheckInputLocale() {
  static HKL s_hklPrev = NULL;

  ms_hklCurrent = GetKeyboardLayout(0);
  if (s_hklPrev == ms_hklCurrent)
    return;
  s_hklPrev = ms_hklCurrent;

  GetKeyboardLayoutNameW(ms_szKeyboardLayout);

  switch (GETPRIMLANG()) {
  case LANG_KOREAN:
    ms_bVerticalCandidate = false;
    ms_wszCurrentIndicator = s_aszIndicator[INDICATOR_KOREAN];
    break;

  case LANG_JAPANESE:
    ms_bVerticalCandidate = true;
    ms_wszCurrentIndicator = s_aszIndicator[INDICATOR_JAPANESE];
    break;

  case LANG_CHINESE:
    ms_bVerticalCandidate = true;
    switch (GETSUBLANG()) {
    case SUBLANG_CHINESE_SIMPLIFIED:
    case SUBLANG_CHINESE_SINGAPORE:
      ms_bVerticalCandidate = false;
      ms_wszCurrentIndicator = s_aszIndicator[INDICATOR_CHS];
      break;

    case SUBLANG_CHINESE_TRADITIONAL:
    case SUBLANG_CHINESE_HONGKONG:
    case SUBLANG_CHINESE_MACAU:
      ms_wszCurrentIndicator = s_aszIndicator[INDICATOR_CHT];
      break;

    default:
      ms_wszCurrentIndicator = s_aszIndicator[INDICATOR_NON_IME];
      break;
    }
    break;

  default:
    ms_wszCurrentIndicator = s_aszIndicator[INDICATOR_NON_IME];
    break;
  }

  if (ms_wszCurrentIndicator == s_aszIndicator[INDICATOR_NON_IME]) {
    wchar_t szLang[10]{};
    GetLocaleInfoW(MAKELCID(GETLANG(), SORT_DEFAULT), LOCALE_SABBREVLANGNAME,
                   szLang, _countof(szLang));

    ms_wszCurrentIndicator[0] = szLang[0];
    ms_wszCurrentIndicator[1] = towlower(szLang[1]);
  }

  if (ms_compLen > 0)
    FinalizeString(false);
}

void CIME::CheckToggleState() {
  CheckInputLocale();

  // In Vista, we have to use TSF since few IMM functions don't work as
  // expected. WARNING: Because of timing, g_dwState and g_bChineseIME may not
  // be updated immediately after the change on IME states by user.
  if (ms_bUILessMode)
    return;

  /* Check Toggle State */
  bool bIme =
      ImmIsIME(ms_hklCurrent) != 0 &&
      ((0xF0000000 & (DWORD)ms_hklCurrent) ==
       0xE0000000); // Hack to detect IME correctly. When IME is running as TIP,
                    // ImmIsIME() returns true for CHT US keyboard.
  ms_bChineseIME = (GETPRIMLANG() == LANG_CHINESE) && bIme;

  HIMC himc;
  if (NULL != (himc = ImmGetContext(ms_hWnd))) {
    if (ms_bChineseIME) {
      DWORD dwConvMode, dwSentMode;
      ImmGetConversionStatus(himc, &dwConvMode, &dwSentMode);
      ms_dwImeState = (dwConvMode & IME_CMODE_NATIVE) ? IMEUI_STATE_ON
                                                      : IMEUI_STATE_ENGLISH;
    } else {
      ms_dwImeState = (bIme && ImmGetOpenStatus(himc) != 0) ? IMEUI_STATE_ON
                                                            : IMEUI_STATE_OFF;
    }
    ImmReleaseContext(ms_hWnd, himc);
  } else
    ms_dwImeState = IMEUI_STATE_OFF;
}

///////////////////////////////////////////////////////////////////////////////
//
//	CTsfUiLessMode methods
//
///////////////////////////////////////////////////////////////////////////////

//
//	SetupSinks()
//	Set up sinks. A sink is used to receive a Text Service Framework event.
//  CUIElementSink implements multiple sink interfaces to receive few different
//  TSF events.
//
BOOL CTsfUiLessMode::SetupSinks() {
  // ITfThreadMgrEx is available on Vista or later.
  HRESULT hr;
  hr = CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER,
                        __uuidof(ITfThreadMgrEx), (void **)&m_tm);

  if (hr != S_OK) {
    return FALSE;
  }

  // ready to start interacting
  TfClientId cid; // not used
  if (FAILED(m_tm->ActivateEx(&cid, TF_TMAE_UIELEMENTENABLEDONLY))) {
    return FALSE;
  }

  // Setup sinks
  BOOL bRc = FALSE;
  m_TsfSink = new CUIElementSink();
  if (m_TsfSink) {
    ITfSource *srcTm;
    if (SUCCEEDED(
            hr = m_tm->QueryInterface(__uuidof(ITfSource), (void **)&srcTm))) {
      // Sink for reading window change
      if (SUCCEEDED(hr = srcTm->AdviseSink(__uuidof(ITfUIElementSink),
                                           (ITfUIElementSink *)m_TsfSink,
                                           &m_dwUIElementSinkCookie))) {
        // Sink for input locale change
        if (SUCCEEDED(hr = srcTm->AdviseSink(
                          __uuidof(ITfInputProcessorProfileActivationSink),
                          (ITfInputProcessorProfileActivationSink *)m_TsfSink,
                          &m_dwAlpnSinkCookie))) {
          if (SetupCompartmentSinks()) // Setup compartment sinks for the first
                                       // time
          {
            bRc = TRUE;
          }
        }
      }
      srcTm->Release();
    }
  }
  return bRc;
}

void CTsfUiLessMode::ReleaseSinks() {
  HRESULT hr;
  ITfSource *source;

  // Remove all sinks
  if (m_tm &&
      SUCCEEDED(m_tm->QueryInterface(__uuidof(ITfSource), (void **)&source))) {
    hr = source->UnadviseSink(m_dwUIElementSinkCookie);
    hr = source->UnadviseSink(m_dwAlpnSinkCookie);
    source->Release();
    SetupCompartmentSinks(TRUE); // Remove all compartment sinks
    m_tm->Deactivate();
    SAFE_RELEASE(m_tm);
    SAFE_RELEASE(m_TsfSink);
  }
}

CTsfUiLessMode::CUIElementSink::CUIElementSink() { _cRef = 1; }

CTsfUiLessMode::CUIElementSink::~CUIElementSink() {}

STDAPI CTsfUiLessMode::CUIElementSink::QueryInterface(REFIID riid,
                                                      void **ppvObj) {
  if (ppvObj == NULL)
    return E_INVALIDARG;

  *ppvObj = NULL;

  if (IsEqualIID(riid, IID_IUnknown)) {
    *ppvObj = reinterpret_cast<IUnknown *>(this);
  } else if (IsEqualIID(riid, __uuidof(ITfUIElementSink))) {
    *ppvObj = (ITfUIElementSink *)this;
  } else if (IsEqualIID(riid,
                        __uuidof(ITfInputProcessorProfileActivationSink))) {
    *ppvObj = (ITfInputProcessorProfileActivationSink *)this;
  } else if (IsEqualIID(riid, __uuidof(ITfCompartmentEventSink))) {
    *ppvObj = (ITfCompartmentEventSink *)this;
  }

  if (*ppvObj) {
    AddRef();
    return S_OK;
  }

  return E_NOINTERFACE;
}

STDAPI_(ULONG) CTsfUiLessMode::CUIElementSink::AddRef() { return ++_cRef; }

STDAPI_(ULONG) CTsfUiLessMode::CUIElementSink::Release() {
  LONG cr = --_cRef;

  if (_cRef == 0) {
    delete this;
  }

  return cr;
}

STDAPI CTsfUiLessMode::CUIElementSink::BeginUIElement(DWORD dwUIElementId,
                                                      BOOL *pbShow) {
  ITfUIElement *pElement = GetUIElement(dwUIElementId);
  if (!pElement)
    return E_INVALIDARG;

  ITfReadingInformationUIElement *preading = NULL;
  ITfCandidateListUIElement *pcandidate = NULL;
  *pbShow = FALSE;

  // BSTR bstrDesc;
  // OutputDebugStringW(L"BEGINUI: ");
  // pElement->GetDescription(&bstrDesc);
  // OutputDebugStringW(bstrDesc);
  // OutputDebugStringW(L"\n");

  if (SUCCEEDED(pElement->QueryInterface(
          __uuidof(ITfReadingInformationUIElement), (void **)&preading))) {
    MakeReadingInformationString(preading);
    if (CIME::ms_pEvent)
      CIME::ms_pEvent->OnOpenReadingWnd();
    preading->Release();
  } else if (SUCCEEDED(pElement->QueryInterface(
                 __uuidof(ITfCandidateListUIElement), (void **)&pcandidate))) {
    m_nCandidateRefCount++;
    MakeCandidateStrings(pcandidate);
    if (CIME::ms_pEvent)
      CIME::ms_pEvent->OnOpenCandidateList();
    pcandidate->Release();
  }

  pElement->Release();
  return S_OK;
}

STDAPI CTsfUiLessMode::CUIElementSink::UpdateUIElement(DWORD dwUIElementId) {
  ITfUIElement *pElement = GetUIElement(dwUIElementId);
  if (!pElement)
    return E_INVALIDARG;

  ITfReadingInformationUIElement *preading = NULL;
  ITfCandidateListUIElement *pcandidate = NULL;

  // BSTR bstrDesc;
  // pElement->GetDescription(&bstrDesc);
  // OutputDebugStringW(L"UPDATEUI: ");
  // OutputDebugStringW(bstrDesc);
  // OutputDebugStringW(L"\n");

  if (SUCCEEDED(pElement->QueryInterface(
          __uuidof(ITfReadingInformationUIElement), (void **)&preading))) {
    MakeReadingInformationString(preading);
    if (CIME::ms_pEvent)
      CIME::ms_pEvent->OnOpenReadingWnd();
    preading->Release();
  } else if (SUCCEEDED(pElement->QueryInterface(
                 __uuidof(ITfCandidateListUIElement), (void **)&pcandidate))) {
    MakeCandidateStrings(pcandidate);
    if (CIME::ms_pEvent)
      CIME::ms_pEvent->OnOpenCandidateList();
    pcandidate->Release();
  }

  pElement->Release();
  return S_OK;
}

STDAPI CTsfUiLessMode::CUIElementSink::EndUIElement(DWORD dwUIElementId) {
  ITfUIElement *pElement = GetUIElement(dwUIElementId);
  if (!pElement)
    return E_INVALIDARG;

  // BSTR bstrDesc;
  // OutputDebugStringW(L"ENDUI: ");
  // pElement->GetDescription(&bstrDesc);
  // OutputDebugStringW(bstrDesc);
  // OutputDebugStringW(L"\n");

  ITfReadingInformationUIElement *preading = NULL;
  if (SUCCEEDED(pElement->QueryInterface(
          __uuidof(ITfReadingInformationUIElement), (void **)&preading))) {
    CIME::CloseReadingInformation();
    preading->Release();
  }

  ITfCandidateListUIElement *pcandidate = NULL;
  if (SUCCEEDED(pElement->QueryInterface(__uuidof(ITfCandidateListUIElement),
                                         (void **)&pcandidate))) {
    m_nCandidateRefCount--;
    if (m_nCandidateRefCount == 0)
      CIME::CloseCandidateList();
    pcandidate->Release();
  }

  pElement->Release();
  return S_OK;
}

void CTsfUiLessMode::UpdateImeState(BOOL bResetCompartmentEventSink) {
  ITfCompartmentMgr *pcm;
  ITfCompartment *pTfOpenMode = NULL;
  ITfCompartment *pTfConvMode = NULL;
  if (GetCompartments(&pcm, &pTfOpenMode, &pTfConvMode)) {
    VARIANT valOpenMode;
    VARIANT valConvMode;
    pTfOpenMode->GetValue(&valOpenMode);
    pTfConvMode->GetValue(&valConvMode);
    if (valOpenMode.vt == VT_I4) {
      if (CIME::ms_bChineseIME) {
        CIME::ms_dwImeState = valOpenMode.lVal != 0 && valConvMode.lVal != 0
                                  ? IMEUI_STATE_ON
                                  : IMEUI_STATE_ENGLISH;
      } else {
        CIME::ms_dwImeState =
            valOpenMode.lVal != 0 ? IMEUI_STATE_ON : IMEUI_STATE_OFF;
      }
    }
    VariantClear(&valOpenMode);
    VariantClear(&valConvMode);

    if (bResetCompartmentEventSink) {
      SetupCompartmentSinks(FALSE, pTfOpenMode,
                            pTfConvMode); // Reset compartment sinks
    }
    pTfOpenMode->Release();
    pTfConvMode->Release();
    pcm->Release();
  }
}

STDAPI CTsfUiLessMode::CUIElementSink::OnActivated(
    DWORD dwProfileType, LANGID langid, REFCLSID clsid, REFGUID catid,
    REFGUID guidProfile, HKL hkl, DWORD dwFlags) {
  static GUID TF_PROFILE_DAYI = {0x037B2C25, 0x480C, 0x4D7F, 0xB0, 0x27, 0xD6,
                                 0xCA,       0x6B,   0x69,   0x78, 0x8A};
  CIME::ms_iCandListIndexBase =
      IsEqualGUID(TF_PROFILE_DAYI, guidProfile) ? 0 : 1;
  if (IsEqualIID(catid, GUID_TFCAT_TIP_KEYBOARD) &&
      (dwFlags & TF_IPSINK_FLAG_ACTIVE)) {
    CIME::ms_bChineseIME =
        (dwProfileType & TF_PROFILETYPE_INPUTPROCESSOR) && langid == LANG_CHT;
    if (dwProfileType & TF_PROFILETYPE_INPUTPROCESSOR) {
      UpdateImeState(TRUE);
    } else
      CIME::ms_dwImeState = IMEUI_STATE_OFF;
    CIME::ChangeInputLanguage();
  }
  return S_OK;
}

STDAPI CTsfUiLessMode::CUIElementSink::OnChange(REFGUID rguid) {
  UpdateImeState();
  return S_OK;
}

void CTsfUiLessMode::MakeReadingInformationString(
    ITfReadingInformationUIElement *preading) {
  UINT cchMax;
  UINT uErrorIndex = 0;
  BOOL fVertical;
  DWORD dwFlags;

  CIME::ms_wstrReading.resize(0);
  preading->GetUpdatedFlags(&dwFlags);
  preading->GetMaxReadingStringLength(&cchMax);
  preading->GetErrorIndex(&uErrorIndex); // errorIndex is zero-based
  preading->IsVerticalOrderPreferred(&fVertical);
  CIME::ms_iReadingError = (int)uErrorIndex;
  CIME::ms_bHorizontalReading = !fVertical;
  CIME::ms_bReadingInformation = true;
  BSTR bstr;
  if (SUCCEEDED(preading->GetString(&bstr))) {
    if (bstr) {
      CIME::ms_wstrReading.assign((wchar_t *)bstr,
                                  (wchar_t *)bstr + lstrlenW(bstr));
      LPCWSTR pszSource = &(CIME::ms_wstrReading[0]);
      if (fVertical) {
        CIME::ms_dwCandidatePageSize = CIME::MAX_CANDLIST;
        // ms_iReadingError is used only in horizontal window, and has to be -1
        // if there's no error.
        CIME::ms_dwCandidateSelection =
            CIME::ms_iReadingError ? CIME::ms_iReadingError - 1 : (DWORD)-1;
        CIME::ms_dwCandidateCount = cchMax;
        // for vertical reading window, copy each character to g_szCandidate
        // array.
        for (UINT i = 0; i < cchMax; i++) {
          LPWSTR pszDest = CIME::ms_wszCandidate[i];
          if (*pszSource) {
            LPWSTR pszNextSrc = CharNextW(pszSource);
            SIZE_T cch =
                (SIZE_T)(pszNextSrc - pszSource); // WCHAR count (1 or 2)
            CopyMemory(pszDest, pszSource, cch * sizeof(WCHAR));
            pszSource = pszNextSrc;
            pszDest += cch;
          }
          *pszDest = 0;
        }
      }
      // else
      //{
      //	CIME::ms_wszCandidate[0][0] = L' ';	// hack to make
      // rendering happen
      // }
      SysFreeString(bstr);
    }
  }
}

void CTsfUiLessMode::MakeCandidateStrings(
    ITfCandidateListUIElement *pcandidate) {
  UINT uIndex = 0;
  UINT uCount = 0;
  UINT uCurrentPage = 0;
  UINT *IndexList = NULL;
  UINT uPageCnt = 0;
  DWORD dwPageStart = 0;
  DWORD dwPageSize = 0;
  BSTR bstr;

  pcandidate->GetSelection(&uIndex);
  pcandidate->GetCount(&uCount);
  pcandidate->GetCurrentPage(&uCurrentPage);
  CIME::ms_dwCandidateSelection = (DWORD)uIndex;
  CIME::ms_dwCandidateCount = (DWORD)uCount;
  CIME::ms_bCandidateList = true;

  pcandidate->GetPageIndex(NULL, 0, &uPageCnt);
  if (uPageCnt > 0) {
    IndexList = (UINT *)malloc(sizeof(UINT) * uPageCnt);
    if (IndexList) {
      pcandidate->GetPageIndex(IndexList, uPageCnt, &uPageCnt);
      dwPageStart = IndexList[uCurrentPage];
      dwPageSize =
          (uCurrentPage < uPageCnt - 1)
              ? std::min(uCount, IndexList[uCurrentPage + 1]) - dwPageStart
              : uCount - dwPageStart;
    }
  }

  CIME::ms_dwCandidatePageSize =
      std::min(dwPageSize, (DWORD)CIME::MAX_CANDLIST);
  CIME::ms_dwCandidateSelection = CIME::ms_dwCandidateSelection - dwPageStart;

  memset(&CIME::ms_wszCandidate, 0, sizeof(CIME::ms_wszCandidate));
  for (UINT i = dwPageStart, j = 0;
       (DWORD)i < CIME::ms_dwCandidateCount && j < CIME::ms_dwCandidatePageSize;
       i++, j++) {
    if (SUCCEEDED(pcandidate->GetString(i, &bstr))) {
      if (bstr) {
        wcsncpy_s(CIME::ms_wszCandidate[j], CIME::MAX_CANDIDATE_LENGTH, bstr,
                  _TRUNCATE);
        SysFreeString(bstr);
      }
    }
  }
  // OutputDebugStringW( L"\n" );

  if (GETPRIMLANG() == LANG_KOREAN) {
    CIME::ms_dwCandidateSelection = (DWORD)-1;
  }

  if (IndexList) {
    free(IndexList);
  }
}

ITfUIElement *CTsfUiLessMode::GetUIElement(DWORD dwUIElementId) {
  ITfUIElementMgr *puiem;
  ITfUIElement *pElement = NULL;

  if (SUCCEEDED(
          m_tm->QueryInterface(__uuidof(ITfUIElementMgr), (void **)&puiem))) {
    puiem->GetUIElement(dwUIElementId, &pElement);
    puiem->Release();
  }

  return pElement;
}

BOOL CTsfUiLessMode::CurrentInputLocaleIsIme() {
  BOOL ret = FALSE;
  HRESULT hr;

  ITfInputProcessorProfiles *pProfiles;
  hr = CoCreateInstance(
      CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
      __uuidof(ITfInputProcessorProfiles), (LPVOID *)&pProfiles);
  if (SUCCEEDED(hr)) {
    ITfInputProcessorProfileMgr *pProfileMgr;
    hr = pProfiles->QueryInterface(__uuidof(ITfInputProcessorProfileMgr),
                                   (LPVOID *)&pProfileMgr);
    if (SUCCEEDED(hr)) {
      TF_INPUTPROCESSORPROFILE tip;
      hr = pProfileMgr->GetActiveProfile(GUID_TFCAT_TIP_KEYBOARD, &tip);
      if (SUCCEEDED(hr)) {
        ret = (tip.dwProfileType & TF_PROFILETYPE_INPUTPROCESSOR) != 0;
      }
      pProfileMgr->Release();
    }
    pProfiles->Release();
  }
  return ret;
}

// Sets up or removes sink for UI element.
// UI element sink should be removed when IME is disabled,
// otherwise the sink can be triggered when a game has multiple instances of IME
// UI library.
void CTsfUiLessMode::EnableUiUpdates(bool bEnable) {
  if (m_tm == NULL ||
      (bEnable && m_dwUIElementSinkCookie != TF_INVALID_COOKIE) ||
      (!bEnable && m_dwUIElementSinkCookie == TF_INVALID_COOKIE)) {
    return;
  }
  ITfSource *srcTm = NULL;
  HRESULT hr = E_FAIL;
  if (SUCCEEDED(
          hr = m_tm->QueryInterface(__uuidof(ITfSource), (void **)&srcTm))) {
    if (bEnable) {
      hr = srcTm->AdviseSink(__uuidof(ITfUIElementSink),
                             (ITfUIElementSink *)m_TsfSink,
                             &m_dwUIElementSinkCookie);
    } else {
      hr = srcTm->UnadviseSink(m_dwUIElementSinkCookie);
      m_dwUIElementSinkCookie = TF_INVALID_COOKIE;
    }
    srcTm->Release();
  }
}

// Returns open mode compartments and compartment manager.
// Function fails if it fails to acquire any of the objects to be returned.
BOOL CTsfUiLessMode::GetCompartments(ITfCompartmentMgr **ppcm,
                                     ITfCompartment **ppTfOpenMode,
                                     ITfCompartment **ppTfConvMode) {
  ITfCompartmentMgr *pcm = NULL;
  ITfCompartment *pTfOpenMode = NULL;
  ITfCompartment *pTfConvMode = NULL;

  static GUID _GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION = {
      0xCCF05DD8, 0x4A87, 0x11D7, 0xA6, 0xE2, 0x00,
      0x06,       0x5B,   0x84,   0x43, 0x5C};

  HRESULT hr;
  if (SUCCEEDED(
          hr = m_tm->QueryInterface(IID_ITfCompartmentMgr, (void **)&pcm))) {
    if (SUCCEEDED(hr = pcm->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE,
                                           &pTfOpenMode))) {
      if (SUCCEEDED(hr = pcm->GetCompartment(
                        _GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION,
                        &pTfConvMode))) {
        *ppcm = pcm;
        *ppTfOpenMode = pTfOpenMode;
        *ppTfConvMode = pTfConvMode;
        return TRUE;
      }
      pTfOpenMode->Release();
    }
    pcm->Release();
  }
  return FALSE;
}

// There are three ways to call this function:
// SetupCompartmentSinks() : initialization
// SetupCompartmentSinks(FALSE, openmode, convmode) : Resetting sinks. This is
// necessary as DaYi and Array IME resets compartment on switching input locale
// SetupCompartmentSinks(TRUE) : clean up sinks
BOOL CTsfUiLessMode::SetupCompartmentSinks(BOOL bRemoveOnly,
                                           ITfCompartment *pTfOpenMode,
                                           ITfCompartment *pTfConvMode) {
  bool bLocalCompartments = false;
  ITfCompartmentMgr *pcm = NULL;
  BOOL bRc = FALSE;
  HRESULT hr = E_FAIL;

  if (!pTfOpenMode && !pTfConvMode) {
    bLocalCompartments = true;
    GetCompartments(&pcm, &pTfOpenMode, &pTfConvMode);
  }
  if (!(pTfOpenMode && pTfConvMode)) {
    // Invalid parameters or GetCompartments() has failed.
    return FALSE;
  }
  ITfSource *srcOpenMode = NULL;
  if (SUCCEEDED(hr = pTfOpenMode->QueryInterface(IID_ITfSource,
                                                 (void **)&srcOpenMode))) {
    // Remove existing sink for open mode
    if (m_dwOpenModeSinkCookie != TF_INVALID_COOKIE) {
      srcOpenMode->UnadviseSink(m_dwOpenModeSinkCookie);
      m_dwOpenModeSinkCookie = TF_INVALID_COOKIE;
    }
    // Setup sink for open mode (toggle state) change
    if (bRemoveOnly || SUCCEEDED(hr = srcOpenMode->AdviseSink(
                                     IID_ITfCompartmentEventSink,
                                     (ITfCompartmentEventSink *)m_TsfSink,
                                     &m_dwOpenModeSinkCookie))) {
      ITfSource *srcConvMode = NULL;
      if (SUCCEEDED(hr = pTfConvMode->QueryInterface(IID_ITfSource,
                                                     (void **)&srcConvMode))) {
        // Remove existing sink for open mode
        if (m_dwConvModeSinkCookie != TF_INVALID_COOKIE) {
          srcConvMode->UnadviseSink(m_dwConvModeSinkCookie);
          m_dwConvModeSinkCookie = TF_INVALID_COOKIE;
        }
        // Setup sink for open mode (toggle state) change
        if (bRemoveOnly || SUCCEEDED(hr = srcConvMode->AdviseSink(
                                         IID_ITfCompartmentEventSink,
                                         (ITfCompartmentEventSink *)m_TsfSink,
                                         &m_dwConvModeSinkCookie))) {
          bRc = TRUE;
        }
        srcConvMode->Release();
      }
    }
    srcOpenMode->Release();
  }
  if (bLocalCompartments) {
    pTfOpenMode->Release();
    pTfConvMode->Release();
    pcm->Release();
  }
  return bRc;
}

/* IME Message Handler */
LRESULT CIME::WMInputLanguage(HWND hWnd, UINT /*uiMsg*/, WPARAM /*wParam*/,
                              LPARAM lParam) {
  ChangeInputLanguage();
  return 0;
}

LRESULT CIME::WMStartComposition(HWND /*hWnd*/, UINT /*uiMsg*/,
                                 WPARAM /*wParam*/, LPARAM /*lParam*/) {
  return 1L;
}

LRESULT CIME::WMComposition(HWND hWnd, UINT /*uiMsg*/, WPARAM /*wParam*/,
                            LPARAM lParam) {
  LRESULT result = 0;

  if (ms_bCaptureInput == false)
    return 0;

  HIMC hImc = ImmGetContext(hWnd);

  if (hImc == NULL)
    return 0;

  if (lParam & GCS_RESULTSTR)
    ResultProcess(hImc);

  if (lParam & GCS_COMPATTR)
    AttributeProcess(hImc);

  if (lParam & GCS_COMPSTR) {
    CompositionProcess(hImc);
  }

  if (lParam & GCS_CURSORPOS) {
    LONG pos = ImmGetCompositionStringW(hImc, GCS_CURSORPOS, nullptr, 0);
    ms_compCaret = (pos < 0) ? 0 : (int)pos;
  }

  ImmReleaseContext(hWnd, hImc);

  if (ms_pEvent)
    ms_pEvent->OnUpdate();

  return (result);
}

LRESULT CIME::WMEndComposition(HWND /*hWnd*/, UINT /*uiMsg*/, WPARAM /*wParam*/,
                               LPARAM /*lParam*/) {
  ms_compLen = 0;
  ms_ulbegin = 0;
  ms_ulend = 0;

  if (ms_pEvent)
    ms_pEvent->OnUpdate();

  return 0L;
}

LRESULT CIME::WMNotify(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam) {
  LRESULT result = 0;

  if (ms_bCaptureInput == false)
    return 0;
  switch (wParam) {

  case IMN_OPENCANDIDATE:
  case IMN_CHANGECANDIDATE: {
    if (ms_bUILessMode && !ms_bUseIMMCandidate)
      break;
    HIMC hImc = ImmGetContext(hWnd);
    if (hImc == NULL)
      break;
    CandidateProcess(hImc);
    if (!m_bUseDefaultIME) {
      if (ms_pEvent)
        ms_pEvent->OnOpenCandidateList();
    } else
      result = ::DefWindowProc(hWnd, uiMsg, wParam, lParam);
    ImmReleaseContext(hWnd, hImc);
    break;
  }
  case IMN_CLOSECANDIDATE:
    if (ms_bUILessMode && !ms_bUseIMMCandidate)
      break;
    if (!m_bUseDefaultIME)
      CloseCandidateList();
    else
      result = DefWindowProc(hWnd, uiMsg, wParam, lParam);
    break;

  case IMN_SETCONVERSIONMODE:
  case IMN_SETOPENSTATUS:
    if (ms_bUILessMode)
      break;
    CheckToggleState();
    break;

  case IMN_PRIVATE: {
    if (ms_bUILessMode)
      break;
    HIMC hImc = ImmGetContext(hWnd);
    if (hImc == NULL)
      break;
    ReadingProcess(hImc);

    // Trap some messages to hide reading window
    switch (ms_adwId[0]) {
    case IMEID_CHT_VER42:
    case IMEID_CHT_VER43:
    case IMEID_CHT_VER44:
    case IMEID_CHS_VER41:
    case IMEID_CHS_VER42:
      if ((lParam == 1) || (lParam == 2))
        return true;
      break;

    case IMEID_CHT_VER50:
    case IMEID_CHT_VER51:
    case IMEID_CHT_VER52:
    case IMEID_CHT_VER60:
    case IMEID_CHS_VER53:
      if ((lParam == 16) || (lParam == 17) || (lParam == 26) ||
          (lParam == 27) || (lParam == 28))
        return true;
      break;
    }
    ImmReleaseContext(hWnd, hImc);
    break;
  }
  }

  if (ms_pEvent)
    ms_pEvent->OnUpdate();

  return result;
}

LRESULT CIME::WMChar(HWND /*hWnd*/, UINT /*uiMsg*/, WPARAM wParam,
                     LPARAM lParam) {
  wchar_t wc = (wchar_t)wParam;

  switch ((unsigned int)wc) {
  case 8:
    if (ms_bCaptureInput == false)
      return 0;

    // If something is selected, backspace deletes the whole selection
    if (ms_selbegin != ms_selend) {
      PushUndoState();
      DeleteSelection();

      if (ms_pEvent)
        ms_pEvent->OnUpdate();

      return 0;
    }

    // Otherwise delete one char to the left
    if (ms_curpos > 0) {
      PushUndoState();
      DecCurPos();
      DelCurPos();
    }

    if (ms_pEvent)
      ms_pEvent->OnUpdate();
    return 0;

  default:
    if (ms_pEvent) {
      if (ms_pEvent->OnWM_CHAR(wParam, lParam))
        break;
    }

    if (ms_bCaptureInput == false)
      return 0;

    OnChar(wc);
    if (wc == L'|')
      OnChar(wc);

    if (ms_pEvent)
      ms_pEvent->OnUpdate();

    break;
  }

  return 0;
}
