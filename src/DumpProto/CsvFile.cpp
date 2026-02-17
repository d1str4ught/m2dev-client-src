#include "CsvFile.h"
#include <fstream>
#include <algorithm>

#ifndef Assert
#include <assert.h>
#define Assert assert
#define LogToFile (void)(0);
#endif

namespace {
/// �Ľ̿� state ���Ű�
enum ParseState {
  STATE_NORMAL = 0, ///< �Ϲ� ����
  STATE_QUOTE       ///< ����ǥ ���� ����
};

/// ���ڿ� �¿��� ������ �����ؼ� ��ȯ�Ѵ�.
std::string Trim(std::string str) {
  str = str.erase(str.find_last_not_of(" \t\r\n") + 1);
  str = str.erase(0, str.find_first_not_of(" \t\r\n"));
  return str;
}

/// \brief �־��� ���忡 �ִ� ���ĺ��� ��� �ҹ��ڷ� �ٲ۴�.
std::string Lower(std::string original) {
  std::transform(original.begin(), original.end(), original.begin(), tolower);
  return original;
}
} // namespace

////////////////////////////////////////////////////////////////////////////////
/// \brief ���� �׼����� ��, ���� ��� �����
/// �̸��� ����Ѵ�.
/// \param name �� �̸�
/// \param index �� �ε���
////////////////////////////////////////////////////////////////////////////////
void cCsvAlias::AddAlias(const char *name, size_t index) {
  std::string converted(Lower(name));

  Assert(m_Name2Index.find(converted) == m_Name2Index.end());
  Assert(m_Index2Name.find(index) == m_Index2Name.end());

  m_Name2Index.insert(NAME2INDEX_MAP::value_type(converted, index));
  m_Index2Name.insert(INDEX2NAME_MAP::value_type(index, name));
}

////////////////////////////////////////////////////////////////////////////////
/// \brief ��� �����͸� �����Ѵ�.
////////////////////////////////////////////////////////////////////////////////
void cCsvAlias::Destroy() {
  m_Name2Index.clear();
  m_Index2Name.clear();
}

////////////////////////////////////////////////////////////////////////////////
/// \brief ���� �ε����� �̸����� ��ȯ�Ѵ�.
/// \param index ���� �ε���
/// \return const char* �̸�
////////////////////////////////////////////////////////////////////////////////
const char *cCsvAlias::operator[](size_t index) const {
  INDEX2NAME_MAP::const_iterator itr(m_Index2Name.find(index));
  if (itr == m_Index2Name.end()) {
    LogToFile(NULL, "cannot find suitable conversion for %d", index);
    Assert(false && "cannot find suitable conversion");
    return NULL;
  }

  return itr->second.c_str();
}

////////////////////////////////////////////////////////////////////////////////
/// \brief �̸��� ���� �ε����� ��ȯ�Ѵ�.
/// \param name �̸�
/// \return size_t ���� �ε���
////////////////////////////////////////////////////////////////////////////////
size_t cCsvAlias::operator[](const char *name) const {
  NAME2INDEX_MAP::const_iterator itr(m_Name2Index.find(Lower(name)));
  if (itr == m_Name2Index.end()) {
    LogToFile(NULL, "cannot find suitable conversion for %s", name);
    Assert(false && "cannot find suitable conversion");
    return 0;
  }

  return itr->second;
}

////////////////////////////////////////////////////////////////////////////////
/// \brief ������ �̸��� CSV ������ �ε��Ѵ�.
/// \param fileName CSV ���� �̸�
/// \param seperator �ʵ� �и��ڷ� ����� ����. �⺻���� ','�̴�.
/// \param quote ����ǥ�� ����� ����. �⺻���� '"'�̴�.
/// \return bool ������ �ε��ߴٸ� true, �ƴ϶�� false
////////////////////////////////////////////////////////////////////////////////
bool cCsvFile::Load(const char *fileName, const char seperator,
                    const char quote) {
  Assert(seperator != quote);

  // Open file in binary mode to preserve UTF-8 encoding
  std::ifstream file(fileName, std::ios::in | std::ios::binary);
  if (!file)
    return false;

  Destroy(); // ������ �����͸� ����

  cCsvRow *row = NULL;
  ParseState state = STATE_NORMAL;
  std::string token = "";
  char buf[2048 + 1] = {
      0,
  };

  while (file.good()) {
    file.getline(buf, 2048);
    buf[sizeof(buf) - 1] = 0;

    std::string line(Trim(buf));
    if (line.empty() || (state == STATE_NORMAL && line[0] == '#'))
      continue;

    std::string text = std::string(line) + "  "; // �Ľ� lookahead ������ �ٿ��ش�.
    size_t cur = 0;

    while (cur < text.size()) {
      // ���� ��尡 QUOTE ����� ��,
      if (state == STATE_QUOTE) {
        // '"' ������ ������ �� �����̴�.
        // 1. �� ���ο� Ư�� ���ڰ� ���� ��� �̸� �˸��� �� �¿��� ��
        // 2. �� ������ '"' ���ڰ� '"' 2���� ġȯ�� ��
        // �� �� ù��° ����� ������ �ִ� ���� CSV ������ �������̶��,
        // ������ STATE_NORMAL�� �ɸ��� �Ǿ��ִ�.
        // �׷��Ƿ� ���⼭ �ɸ��� ���� 1���� ���� ��쳪, 2��
        // ��� ���̴�. 2���� ��쿡�� ������ '"' ���ڰ� 2���� ��Ÿ����. ������ 1����
        // ���� ��쿡�� �ƴϴ�. �̸� �������� �ؼ� �ڵ带 ¥��...
        if (text[cur] == quote) {
          // ���� ���ڰ� '"' ���ڶ��, �� ���ӵ� '"' ���ڶ��
          // �̴� �� ������ '"' ���ڰ� ġȯ�� ���̴�.
          if (text[cur + 1] == quote) {
            token += quote;
            ++cur;
          }
          // ���� ���ڰ� '"' ���ڰ� �ƴ϶��
          // ������ '"'���ڴ� ���� ���� �˸��� ���ڶ�� �� �� �ִ�.
          else {
            state = STATE_NORMAL;
          }
        } else {
          token += text[cur];
        }
      }
      // ���� ��尡 NORMAL ����� ��,
      else if (state == STATE_NORMAL) {
        if (row == NULL)
          row = new cCsvRow();

        // ',' ���ڸ� �����ٸ� ���� ���� �ǹ��Ѵ�.
        // ��ū���μ� �� ����Ʈ���ٰ� ����ְ�, ��ū�� �ʱ�ȭ�Ѵ�.
        if (text[cur] == seperator) {
          row->push_back(token);
          token.clear();
        }
        // '"' ���ڸ� �����ٸ�, QUOTE ���� ��ȯ�Ѵ�.
        else if (text[cur] == quote) {
          state = STATE_QUOTE;
        }
        // �ٸ� �Ϲ� ���ڶ�� ���� ��ū���ٰ� �����δ�.
        else {
          token += text[cur];
        }
      }

      ++cur;
    }

    // ������ ���� ���� ',' ���ڰ� ���� ������ ���⼭ �߰�������Ѵ�.
    // ��, ó���� �Ľ� lookahead ������ ���� �����̽� ���� �� ���� ����.
    if (state == STATE_NORMAL) {
      Assert(row != NULL);
      row->push_back(token.substr(0, token.size() - 2));
      m_Rows.push_back(row);
      token.clear();
      row = NULL;
    } else {
      token = token.substr(0, token.size() - 2) + "\r\n";
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// \brief ������ �ִ� ������ CSV ���Ͽ��� �����Ѵ�.
/// \param fileName CSV ���� �̸�
/// \param append true�� ���, ������ ���Ͽ��� �����δ�. false�� ��쿡��
/// ������ ���� ������ �����ϰ�, ���� ����.
/// \param seperator �ʵ� �и��ڷ� ����� ����. �⺻���� ','�̴�.
/// \param quote ����ǥ�� ����� ����. �⺻���� '"'�̴�.
/// \return bool ������ �����ߴٸ� true, ������ ���� ��쿡�� false
////////////////////////////////////////////////////////////////////////////////
bool cCsvFile::Save(const char *fileName, bool append, char seperator,
                    char quote) const {
  Assert(seperator != quote);

  // ��� ��忡 ���� ������ ������
  // �÷��׷� �����Ѵ�.
  std::ofstream file;
  if (append) {
    file.open(fileName, std::ios::out | std::ios::app);
  } else {
    file.open(fileName, std::ios::out | std::ios::trunc);
  }

  // ������ ���� ���ߴٸ�, false�� �����Ѵ�.
  if (!file)
    return false;

  char special_chars[5] = {seperator, quote, '\r', '\n', 0};
  char quote_escape_string[3] = {quote, quote, 0};

  // ��� ���� Ⱦ���ϸ鼭...
  for (size_t i = 0; i < m_Rows.size(); i++) {
    const cCsvRow &row = *((*this)[i]);

    std::string line;

    // �� ���� ��� ��ū�� Ⱦ���ϸ鼭...
    for (size_t j = 0; j < row.size(); j++) {
      const std::string &token = row[j];

      // �Ϲ�����('"' �Ǵ� ','�� �������� ����)
      // ��ū�̶�� �׳� �����ϸ� �ȴ�.
      if (token.find_first_of(special_chars) == std::string::npos) {
        line += token;
      }
      // Ư�����ڸ� ������ ��ū�̶�� ���ڿ� �¿쿡 '"'�� �ٿ��ְ�,
      // ���ڿ� ������ '"'�� �� ���� ���������Ѵ�.
      else {
        line += quote;

        for (size_t k = 0; k < token.size(); k++) {
          if (token[k] == quote)
            line += quote_escape_string;
          else
            line += token[k];
        }

        line += quote;
      }

      // ������ ���� �ƴ϶�� ','�� ��ū�� �ڿ��� �ٿ�����Ѵ�.
      if (j != row.size() - 1) {
        line += seperator;
      }
    }

    // ������ ����Ѵ�.
    file << line << std::endl;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// \brief ��� �����͸� �޸𸮿���
/// �����Ѵ�.
////////////////////////////////////////////////////////////////////////////////
void cCsvFile::Destroy() {
  for (ROWS::iterator itr(m_Rows.begin()); itr != m_Rows.end(); ++itr)
    delete *itr;

  m_Rows.clear();
}

////////////////////////////////////////////////////////////////////////////////
/// \brief �ش��ϴ� �ε����� ���� ��ȯ�Ѵ�.
/// \param index �ε���
/// \return cCsvRow* �ش� ��
////////////////////////////////////////////////////////////////////////////////
cCsvRow *cCsvFile::operator[](size_t index) {
  Assert(index < m_Rows.size());
  return m_Rows[index];
}

////////////////////////////////////////////////////////////////////////////////
/// \brief �ش��ϴ� �ε����� ���� ��ȯ�Ѵ�.
/// \param index �ε���
/// \return const cCsvRow* �ش� ��
////////////////////////////////////////////////////////////////////////////////
const cCsvRow *cCsvFile::operator[](size_t index) const {
  Assert(index < m_Rows.size());
  return m_Rows[index];
}

////////////////////////////////////////////////////////////////////////////////
/// \brief ������
////////////////////////////////////////////////////////////////////////////////
cCsvTable::cCsvTable() : m_CurRow(-1) {}

////////////////////////////////////////////////////////////////////////////////
/// \brief �Ҹ���
////////////////////////////////////////////////////////////////////////////////
cCsvTable::~cCsvTable() {}

////////////////////////////////////////////////////////////////////////////////
/// \brief ������ �̸��� CSV ������ �ε��Ѵ�.
/// \param fileName CSV ���� �̸�
/// \param seperator �ʵ� �и��ڷ� ����� ����. �⺻���� ','�̴�.
/// \param quote ����ǥ�� ����� ����. �⺻���� '"'�̴�.
/// \return bool ������ �ε��ߴٸ� true, �ƴ϶�� false
////////////////////////////////////////////////////////////////////////////////
bool cCsvTable::Load(const char *fileName, const char seperator,
                     const char quote) {
  Destroy();
  return m_File.Load(fileName, seperator, quote);
}

////////////////////////////////////////////////////////////////////////////////
/// \brief ���� ������ �Ѿ��.
/// \return bool ���� ������ ������ �Ѿ ��� true�� ��ȯ�ϰ�,
/// �� �̻�
/// �Ѿ ���� �������� �ʴ� ��쿡��
/// false�� ��ȯ�Ѵ�.
////////////////////////////////////////////////////////////////////////////////
bool cCsvTable::Next() {
  // 20��� ���� ȣ���ϸ� �����÷ΰ�
  // �Ͼ�ٵ�...��������?
  return ++m_CurRow < (int)m_File.GetRowCount() ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
/// \brief ���� ���� �� ���ڸ� ��ȯ�Ѵ�.
/// \return size_t ���� ���� �� ����
////////////////////////////////////////////////////////////////////////////////
size_t cCsvTable::ColCount() const { return CurRow()->size(); }

////////////////////////////////////////////////////////////////////////////////
/// \brief �ε����� �̿��� int ������ �� ���� ��ȯ�Ѵ�.
/// \param index �� �ε���
/// \return int �� ��
////////////////////////////////////////////////////////////////////////////////
int cCsvTable::AsInt(size_t index) const {
  const cCsvRow *const row = CurRow();
  Assert(row);
  Assert(index < row->size());
  return row->AsInt(index);
}

////////////////////////////////////////////////////////////////////////////////
/// \brief �ε����� �̿��� double ������ �� ���� ��ȯ�Ѵ�.
/// \param index �� �ε���
/// \return double �� ��
////////////////////////////////////////////////////////////////////////////////
double cCsvTable::AsDouble(size_t index) const {
  const cCsvRow *const row = CurRow();
  Assert(row);
  Assert(index < row->size());
  return row->AsDouble(index);
}

////////////////////////////////////////////////////////////////////////////////
/// \brief �ε����� �̿��� std::string ������ �� ���� ��ȯ�Ѵ�.
/// \param index �� �ε���
/// \return const char* �� ��
////////////////////////////////////////////////////////////////////////////////
const char *cCsvTable::AsStringByIndex(size_t index) const {
  const cCsvRow *const row = CurRow();
  Assert(row);
  Assert(index < row->size());
  return row->AsString(index);
}

////////////////////////////////////////////////////////////////////////////////
/// \brief alias�� ������ ��� �����͸�
/// �����Ѵ�.
////////////////////////////////////////////////////////////////////////////////
void cCsvTable::Destroy() {
  m_File.Destroy();
  m_Alias.Destroy();
  m_CurRow = -1;
}

////////////////////////////////////////////////////////////////////////////////
/// \brief ���� ���� ��ȯ�Ѵ�.
/// \return const cCsvRow* �׼����� ������ ���� ���� �����ϴ� ��쿡��
/// �� ����
/// �����͸� ��ȯ�ϰ�, �� �̻� �׼��� ������ ���� ����
/// ��쿡�� NULL�� ��ȯ�Ѵ�.
////////////////////////////////////////////////////////////////////////////////
const cCsvRow *const cCsvTable::CurRow() const {
  if (m_CurRow < 0) {
    Assert(false && "call Next() first!");
    return NULL;
  } else if (m_CurRow >= (int)m_File.GetRowCount()) {
    Assert(false && "no more rows!");
    return NULL;
  }

  return m_File[m_CurRow];
}
