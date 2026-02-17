#ifndef __INC_SCRIPT_PARSER_H__
#define __INC_SCRIPT_PARSER_H__

#include <list>
#include <string>

namespace script {
typedef struct SArgumet {
  SArgumet(const std::string &c_stName, const std::string &c_stValue) {
    strName = c_stName;
    strValue = c_stValue;
  }

  SArgumet(const SArgumet &c_arg) {
    strName = c_arg.strName;
    strValue = c_arg.strValue;
  }

  void operator=(const SArgumet &c_arg) {
    strName = c_arg.strName;
    strValue = c_arg.strValue;
  }

  std::string strName;
  std::string strValue;
} TArg;

typedef std::list<TArg> TArgList;

typedef struct SCmd {
  std::string name;
  TArgList argList;

  SCmd() {}
  SCmd(const SCmd &c_cmd) {
    name = c_cmd.name;
    argList = c_cmd.argList;
  }

  void operator=(const SCmd &c_cmd) {
    name = c_cmd.name;
    argList = c_cmd.argList;
  }
} TCmd;

class Group {
public:
  Group();
  ~Group();

public:
  bool Create(const std::string &stSource);
  bool GetCmd(TCmd &cmd);
  bool ReadCmd(TCmd &cmd);
  std::string &GetError();

private:
  void SetError(const char *str);
  bool GetArg(const char *c_atr_base, int arg_len, TArgList &argList);

  std::string m_stError;
  std::list<TCmd> m_cmdList;
};
} // namespace script
#endif
