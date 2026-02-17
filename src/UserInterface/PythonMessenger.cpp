#include "stdafx.h"
#include "PythonMessenger.h"

void CPythonMessenger::RemoveFriend(const char *c_szKey) {
  m_FriendNameMap.erase(c_szKey);

  if (m_poMessengerHandler)
    PyCallClassMemberFunc(
        m_poMessengerHandler, "OnRemoveList",
        Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_FRIEND, c_szKey));
}

void CPythonMessenger::OnFriendLogin(
    const char *c_szKey /*, const char * c_szName*/) {
  m_FriendNameMap.insert(c_szKey);

  if (m_poMessengerHandler)
    PyCallClassMemberFunc(
        m_poMessengerHandler, "OnLogin",
        Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_FRIEND, c_szKey));
}

void CPythonMessenger::OnFriendLogout(const char *c_szKey) {
  m_FriendNameMap.insert(c_szKey);

  if (m_poMessengerHandler)
    PyCallClassMemberFunc(
        m_poMessengerHandler, "OnLogout",
        Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_FRIEND, c_szKey));
}

BOOL CPythonMessenger::IsFriendByKey(const char *c_szKey) {
  return m_FriendNameMap.end() != m_FriendNameMap.find(c_szKey);
}

BOOL CPythonMessenger::IsFriendByName(const char *c_szName) {
  return IsFriendByKey(c_szName);
}

void CPythonMessenger::AppendGuildMember(const char *c_szName) {
  if (m_GuildMemberStateMap.end() != m_GuildMemberStateMap.find(c_szName))
    return;

  LogoutGuildMember(c_szName);
}

void CPythonMessenger::RemoveGuildMember(const char *c_szName) {
  m_GuildMemberStateMap.erase(c_szName);

  if (m_poMessengerHandler)
    PyCallClassMemberFunc(
        m_poMessengerHandler, "OnRemoveList",
        Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_GUILD, c_szName));
}

void CPythonMessenger::RemoveAllGuildMember() {
  m_GuildMemberStateMap.clear();

  if (m_poMessengerHandler)
    PyCallClassMemberFunc(m_poMessengerHandler, "OnRemoveAllList",
                          Py_BuildValue("(i)", MESSENGER_GRUOP_INDEX_GUILD));
}

void CPythonMessenger::LoginGuildMember(const char *c_szName) {
  m_GuildMemberStateMap[c_szName] = 1;
  if (m_poMessengerHandler)
    PyCallClassMemberFunc(
        m_poMessengerHandler, "OnLogin",
        Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_GUILD, c_szName));
}

void CPythonMessenger::LogoutGuildMember(const char *c_szName) {
  m_GuildMemberStateMap[c_szName] = 0;
  if (m_poMessengerHandler)
    PyCallClassMemberFunc(
        m_poMessengerHandler, "OnLogout",
        Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_GUILD, c_szName));
}

void CPythonMessenger::RefreshGuildMember() {
  for (TGuildMemberStateMap::iterator itor = m_GuildMemberStateMap.begin();
       itor != m_GuildMemberStateMap.end(); ++itor) {
    if (itor->second)
      PyCallClassMemberFunc(m_poMessengerHandler, "OnLogin",
                            Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_GUILD,
                                          (itor->first).c_str()));
    else
      PyCallClassMemberFunc(m_poMessengerHandler, "OnLogout",
                            Py_BuildValue("(is)", MESSENGER_GRUOP_INDEX_GUILD,
                                          (itor->first).c_str()));
  }
}

void CPythonMessenger::Destroy() {
  m_FriendNameMap.clear();
  m_GuildMemberStateMap.clear();
}

void CPythonMessenger::SetMessengerHandler(PyObject *poHandler) {
  m_poMessengerHandler = poHandler;
}

CPythonMessenger::CPythonMessenger() : m_poMessengerHandler(NULL) {}

CPythonMessenger::~CPythonMessenger() {}

///////////////////////////////////////////////////////////////////////////////////////////////////

PyObject *messengerRemoveFriend(PyObject *poSelf, PyObject *poArgs) {
  char *szKey;
  if (!PyTuple_GetString(poArgs, 0, &szKey))
    return Py_BuildException();

  CPythonMessenger::Instance().RemoveFriend(szKey);
  return Py_BuildNone();
}

PyObject *messengerIsFriendByName(PyObject *poSelf, PyObject *poArgs) {
  char *szName;
  if (!PyTuple_GetString(poArgs, 0, &szName))
    return Py_BuildException();

  return Py_BuildValue("i",
                       CPythonMessenger::Instance().IsFriendByName(szName));
}

PyObject *messengerDestroy(PyObject *poSelf, PyObject *poArgs) {
  CPythonMessenger::Instance().Destroy();
  return Py_BuildNone();
}

PyObject *messengerRefreshGuildMember(PyObject *poSelf, PyObject *poArgs) {
  CPythonMessenger::Instance().RefreshGuildMember();
  return Py_BuildNone();
}

PyObject *messengerSetMessengerHandler(PyObject *poSelf, PyObject *poArgs) {
  PyObject *poEventHandler;
  if (!PyTuple_GetObject(poArgs, 0, &poEventHandler))
    return Py_BuildException();

  CPythonMessenger::Instance().SetMessengerHandler(poEventHandler);
  return Py_BuildNone();
}

void initMessenger() {
  static PyMethodDef s_methods[] = {
      {"RemoveFriend", messengerRemoveFriend, METH_VARARGS},
      {"IsFriendByName", messengerIsFriendByName, METH_VARARGS},
      {"Destroy", messengerDestroy, METH_VARARGS},
      {"RefreshGuildMember", messengerRefreshGuildMember, METH_VARARGS},
      {"SetMessengerHandler", messengerSetMessengerHandler, METH_VARARGS},

      {NULL, NULL, NULL},
  };

  Py_InitModule("messenger", s_methods);
}
