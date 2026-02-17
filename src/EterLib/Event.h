#pragma once

class IEvent {
public:
  IEvent();
  ~IEvent();

  virtual void Run() = 0;

  void SetStartTime(float fTime) { m_fStartTime = fTime; }
  float GetStartTime() { return m_fStartTime; }

protected:
  float m_fStartTime;
};

class CEventManager : public CSingleton<CEventManager> {
public:
  CEventManager();
  virtual ~CEventManager() { Destroy(); }

  void Destroy() {
    while (!m_eventQueue.empty()) {
      IEvent *pEvent = m_eventQueue.top();
      m_eventQueue.pop();
      delete pEvent;
    }
  }

  void Register(IEvent *pEvent) { m_eventQueue.push(pEvent); }

  void Update(float fCurrentTime) {
    while (!m_eventQueue.empty()) {
      IEvent *pEvent = m_eventQueue.top();

      if (pEvent->GetStartTime() < fCurrentTime)
        break;

      m_eventQueue.pop();
      float fTime = pEvent->GetStartTime();
      pEvent->Run();
      delete pEvent;
    }
  }

protected:
  struct EventComparisonFunc {
    bool operator()(IEvent *left, IEvent *right) const {
      return left->GetStartTime() > right->GetStartTime();
    }
  };

  typedef std::priority_queue<IEvent *, std::vector<IEvent *>,
                              EventComparisonFunc>
      TEventQueue;
  TEventQueue m_eventQueue;
};
