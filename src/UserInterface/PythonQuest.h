#pragma once

class CPythonQuest : public CSingleton<CPythonQuest> {
public:
  struct SQuestInstance {
    SQuestInstance() {
      dwIndex = 0;
      iClockValue = 0;
      iCounterValue = 0;
      iStartTime = 0;
    }

    DWORD dwIndex;

    std::string strIconFileName;
    std::string strTitle;

    std::string strClockName;
    std::string strCounterName;

    int iClockValue;
    int iCounterValue;

    int iStartTime;
  };
  typedef std::vector<SQuestInstance> TQuestInstanceContainer;

public:
  CPythonQuest();
  virtual ~CPythonQuest();

  void Clear();

  void RegisterQuestInstance(const SQuestInstance &c_rQuestInstance);
  void DeleteQuestInstance(DWORD dwIndex);

  bool IsQuest(DWORD dwIndex);
  void MakeQuest(DWORD dwIndex);

  void SetQuestTitle(DWORD dwIndex, const char *c_szTitle);
  void SetQuestClockName(DWORD dwIndex, const char *c_szClockName);
  void SetQuestCounterName(DWORD dwIndex, const char *c_szCounterName);
  void SetQuestClockValue(DWORD dwIndex, int iClockValue);
  void SetQuestCounterValue(DWORD dwIndex, int iCounterValue);
  void SetQuestIconFileName(DWORD dwIndex, const char *c_szIconFileName);

  int GetQuestCount();
  bool GetQuestInstancePtr(DWORD dwArrayIndex,
                           SQuestInstance **ppQuestInstance);

protected:
  void __Initialize();
  bool __GetQuestInstancePtr(DWORD dwQuestIndex,
                             SQuestInstance **ppQuestInstance);

protected:
  TQuestInstanceContainer m_QuestInstanceContainer;
};