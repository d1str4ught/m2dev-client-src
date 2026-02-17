#include "StdAfx.h"
#include <string>
#include <assert.h>

#include "Poly.h"
#include <cmath>
#include <cctype>
#include <cstdlib>

using namespace std;

double _random() { return rand() / (RAND_MAX + 1.0); }

void CPoly::SetRandom(int iRandomType) { m_iRandomType = iRandomType; }

int CPoly::my_irandom(double start, double end) {
  switch (m_iRandomType) {
  case RANDOM_TYPE_FORCE_MIN:
    return int(start);
    break;
  case RANDOM_TYPE_FORCE_MAX:
    return int(end);
    break;
  }

  // Make range as inclusive-exclusive
  int is = int(start + 0.5);
  int ie = int(end - start + 0.5) + 1;

  return int(_random() * ie + is);
}

double CPoly::my_frandom(double start, double end) {
  return _random() * (end - start) + start;
}

CPoly::CPoly() {
  m_iRandomType = RANDOM_TYPE_FREELY;
  uiLookPos = 0;
  ErrorOccur = true;
  lSymbol.clear();
  STSize = 0;
  MathSymbolCount = 0;
  lSymbol.reserve(50);
  init();
}

CPoly::~CPoly() { Clear(); }

void CPoly::SetStr(const string &str) { strData = str; }

float CPoly::Eval() {
  int stNow;
  double save[POLY_MAXSTACK], t;
  int iSp = 0;
  if (ErrorOccur) {
    /*THROW(new CEvalException("Evaluate Error"));*/
    return 0;
  }

  // TEST
  // list<int>::iterator pos = tokenBase.begin();
  // list<double>::iterator posn = numBase.begin();
  vector<int>::iterator pos = tokenBase.begin();
  vector<double>::iterator posn = numBase.begin();
  while (pos != tokenBase.end()) {

    stNow = *pos;
    ++pos;
    switch (stNow) {
    case POLY_NUM:
      save[iSp++] = *posn++;
      break;
    case POLY_ID:
      save[iSp++] = lSymbol[*pos]->dVal;
      pos++;
      break;
      // case '+':
    case POLY_PLU:
      iSp--;
      save[iSp - 1] += save[iSp];
      break;
      // case '-':
    case POLY_MIN:
      iSp--;
      save[iSp - 1] -= save[iSp];
      break;
      // case '*':
    case POLY_MUL:
      iSp--;
      save[iSp - 1] *= save[iSp];
      break;
      // case '%':
    case POLY_MOD:
      iSp--;
      if (save[iSp] == 0) {
        // THROW(new CEvalException("Divide by 0"));
        return 0;
      }
      save[iSp - 1] = fmod(save[iSp - 1], save[iSp]);
      break;
      // case '/':
    case POLY_DIV:
      iSp--;
      if (save[iSp] == 0) {
        // THROW(new CEvalException("Divide by 0"));
        return 0;
      }
      save[iSp - 1] /= save[iSp];
      break;
      // case '^':
    case POLY_POW:
      iSp--;
      save[iSp - 1] = pow(save[iSp - 1], save[iSp]);
      break;
    case POLY_ROOT:
      if (save[iSp - 1] < 0) {
        // THROW(new CEvalException("Negative in root"));
        return 0;
      }
      save[iSp - 1] = sqrt(save[iSp - 1]);
      break;
    case POLY_COS:
      save[iSp - 1] = cos(save[iSp - 1]);
      break;
    case POLY_SIN:
      save[iSp - 1] = sin(save[iSp - 1]);
      break;
    case POLY_TAN:
      if (!(t = cos(save[iSp - 1]))) {
        // THROW (new CEvalException("Divide by 0"));
        return 0;
      }
      save[iSp - 1] = tan(save[iSp - 1]);
      break;
    case POLY_CSC:
      if (!(t = sin(save[iSp - 1]))) {
        // THROW(new CEvalException("Divide by 0"));
        return 0;
      }
      save[iSp - 1] = 1 / t;
      break;
    case POLY_SEC:
      if (!(t = cos(save[iSp - 1]))) {
        // THROW(new CEvalException("Divide by 0"));
        return 0;
      }
      save[iSp - 1] = 1 / t;
      break;
    case POLY_COT:
      if (!(t = sin(save[iSp - 1]))) {
        // THROW(new CEvalException("Divide by 0"));
        return 0;
      }
      save[iSp - 1] = cos(save[iSp - 1]) / t;
      break;
    case POLY_LN:
      if (save[iSp - 1] <= 0) {
        // THROW( new CEvalException("Call Log with minus number"));
        return 0;
      }
      save[iSp - 1] = log(save[iSp - 1]);
      break;
    case POLY_LOG10:
      if (save[iSp - 1] <= 0) {
        // THROW( new CEvalException("Call Log with minus number"));
        return 0;
      }
      save[iSp - 1] = log10(save[iSp - 1]);
      break;
    case POLY_LOG:
      if (save[iSp - 1] <= 0) {
        // THROW( new CEvalException("Call Log with minus number"));
        return 0;
      }
      if (save[iSp - 2] <= 0 || save[iSp - 2] == 1) {
        // THROW( new CEvalException("Call Log with minus number"));
        return 0;
      }

      save[iSp - 2] = log(save[iSp - 1]) / log(save[iSp - 2]);
      iSp--;
      break;
    case POLY_ABS:
      save[iSp - 1] = fabs(save[iSp - 1]);
      break;
    case POLY_FLOOR:
      save[iSp - 1] = floor(save[iSp - 1]);
      break;
    case POLY_IRAND:
      save[iSp - 2] = my_irandom(save[iSp - 2], save[iSp - 1]);
      iSp--;
      break;
    case POLY_FRAND:
      save[iSp - 2] = my_frandom(save[iSp - 2], save[iSp - 1]);
      iSp--;
      break;
    case POLY_MINF:
      save[iSp - 2] =
          (save[iSp - 2] < save[iSp - 1]) ? save[iSp - 2] : save[iSp - 1];
      iSp--;
      break;
    case POLY_MAXF:
      save[iSp - 2] =
          (save[iSp - 2] > save[iSp - 1]) ? save[iSp - 2] : save[iSp - 1];
      iSp--;
      break;
      /*case POLY_MOD:
        save[iSp-2]=fmod(save[iSp-2],save[iSp-1]);
        iSp--;
        break;*/
    default:
      return 0;
      // THROW(new CEvalException("Token Error"));
    }
  }
  return float(save[iSp - 1]);
}

int CPoly::Analyze(const char *pszStr) {
  if (pszStr)
    SetStr(pszStr);

  if (0 == strData.length())
    return true;

  // DisposeList();
  ErrorOccur = false;
  uiLookPos = 0;
  iLookAhead = lexan();

  expr();

  if (tokenBase.empty()) {
    // THROW(new CParseException("No Data"));
    return false;
  }

  return !ErrorOccur;
}

void CPoly::Clear() {
  int i;
  // while (!tokenBase.IsEmpty()) listBase.RemoveTail();
  // while (!numBase.IsEmpty()) numBase.RemoveTail();
  tokenBase.clear();
  numBase.clear();

  for (i = 0; i < STSize; ++i) {
    if (lSymbol[i])
      delete lSymbol[i];
    lSymbol[i] = NULL;
  }
  // lSymbol.FreeExtra();
  lSymbol.clear();
  SymbolIndex.clear();
  STSize = 0;
  MathSymbolCount = 0;
}

void CPoly::expr() {
  int t;

  switch (iLookAhead) {
  case '+':
  case '-':
    uiLookPos--;
    iLookAhead = POLY_NUM;
    iNumToken = iToken = 0;
  }

  term();

  while (!ErrorOccur) {
    switch (iLookAhead) {
    case '+':
    case '-':
      t = iLookAhead;
      match(t);
      term();
      emit(t, POLY_NONE);
      continue;
    case POLY_EOS:
    case ')':
    case ',':
      return;
    default:
      error();
      // THROW( new CParseException("Error Parsing"));
      return;
    }
  }
}

void CPoly::error() {
  iErrorPos = uiLookPos;
  ErrorOccur = true;
}

int CPoly::lexan() {
  int t;
  double tt;

  while (uiLookPos < strData.size()) {
    if (strData[uiLookPos] == ' ' || strData[uiLookPos] == '\t')
      ;
    else if (isdigit(strData[uiLookPos])) {
      t = 0;
      for (; uiLookPos < strData.size(); uiLookPos++) {
        if (isdigit(strData[uiLookPos]))
          t = t * 10 + strData[uiLookPos] - '0';
        else
          break;
      }
      iToken = t;
      tt = 0.1;
      iNumToken = 0;
      if (uiLookPos < strData.size() && strData[uiLookPos] == '.') {
        uiLookPos++;
        for (; uiLookPos < strData.size(); uiLookPos++, tt *= 0.1) {
          if (isdigit(strData[uiLookPos]))
            iNumToken += tt * (strData[uiLookPos] - '0');
          else
            break;
        }
      }
      iNumToken += iToken;
      return POLY_NUM;
    } else if (isalpha(strData[uiLookPos])) {
      string localSymbol("");
      while (uiLookPos < strData.size() && isalpha(strData[uiLookPos])) {
        localSymbol += strData[uiLookPos];
        uiLookPos++;
      }
      iToken = find(localSymbol);
      if (iToken == -1) {
        iToken = insert(localSymbol, POLY_ID);
      }
      return lSymbol[(/*FindIndex*/ (iToken))]->token;
    } else {
      iToken = 0;
      return strData[uiLookPos++];
    }
    uiLookPos++;
  }
  return POLY_EOS;
}

void CPoly::term() {
  int t;
  factor();
  while (!ErrorOccur) {
    switch (iLookAhead) {
    case '*':
    case '/':
    case '%':
      t = iLookAhead;
      match(t);
      factor();
      emit(t, POLY_NONE);
      continue;
    default:
      return;
    }
  }
}

void CPoly::factor() {
  int t;
  expo();
  while (!ErrorOccur) {
    switch (iLookAhead) {
    case '^':
      t = iLookAhead;
      match(t);
      expo();
      emit(t, POLY_NONE);
      continue;
    default:
      return;
    }
  }
}

void CPoly::expo() {
  int t;
  switch (iLookAhead) {
  case '(':
    match('(');
    expr();
    match(')');
    break;
  case POLY_NUM:
    emit(POLY_NUM, iToken);
    match(POLY_NUM);
    break;
  case POLY_ID:
    emit(POLY_ID, (int)/*FindIndex*/ (iToken));
    match(POLY_ID);
    break;
  case POLY_ROOT:
  case POLY_SIN:
  case POLY_COT:
  case POLY_TAN:
  case POLY_CSC:
  case POLY_SEC:
  case POLY_LN:
  case POLY_LOG10:
  case POLY_COS:
  case POLY_ABS:
  case POLY_FLOOR:
    t = iLookAhead;
    match(iLookAhead);
    match('(');
    expr();
    match(')');
    emit(t, iToken);
    break;
  case POLY_LOG:
  case POLY_MINF:
  case POLY_MAXF:
  case POLY_IRAND:
  case POLY_FRAND:
  case POLY_MOD:
    t = iLookAhead;
    match(iLookAhead);
    match('(');
    expr();
    match(',');
    expr();
    match(')');
    emit(t, iToken);
    break;
  case POLY_EOS:
    break;
  default:
    error();
    // THROW( new CParseException("Error Parsing"));
  }
}

void CPoly::match(int t) {
  if (iLookAhead == t)
    iLookAhead = lexan();
  else
    error();
}

void CPoly::emit(int t, int tval) {
  switch (t) {
  case '+':
    tokenBase.push_back(POLY_PLU);
    break;
  case '-':
    tokenBase.push_back(POLY_MIN);
    break;
  case '*':
    tokenBase.push_back(POLY_MUL);
    break;
  case '/':
    tokenBase.push_back(POLY_DIV);
    break;
  case '%':
    tokenBase.push_back(POLY_MOD);
    break;
  case '^':
    tokenBase.push_back(POLY_POW);
    break;
  case POLY_ROOT:
  case POLY_SIN:
  case POLY_TAN:
  case POLY_COT:
  case POLY_COS:
  case POLY_CSC:
  case POLY_SEC:
  case POLY_LOG:
  case POLY_LN:
  case POLY_LOG10:
  case POLY_ABS:
  case POLY_MINF:
  case POLY_MAXF:
  case POLY_IRAND:
  case POLY_FRAND:
  case POLY_MOD:
  case POLY_FLOOR:
    tokenBase.push_back(t);
    break;
  case POLY_NUM:
    tokenBase.push_back(t);
    numBase.push_back(iNumToken);
    break;
  case POLY_ID:
    tokenBase.push_back(t);
    tokenBase.push_back(tval);
    break;
  default:
    error();
    Clear();
    // THROW( new CParseException("Error Parsing"));
    return;
  }
}

int CPoly::find(const string &s) {
  int l, m, r;

  l = 0;
  r = STSize - 1;

  while (l <= r) {
    m = (l + r) >> 1;

    if (lSymbol[SymbolIndex[m]]->strlex == s)
      return SymbolIndex[m];
    else if (lSymbol[SymbolIndex[m]]->strlex < s)
      l = m + 1;
    else
      r = m - 1;
  }
  return -1;
}

int CPoly::insert(const string &s, int tok) {
  int i;
  bool bAdded = false;

  lSymbol.push_back(new CSymTable(tok, s));
  for (i = 0; i < STSize; i++) {
    if (s < lSymbol[SymbolIndex[i]]->strlex) {
      SymbolIndex.insert(SymbolIndex.begin() + i, STSize);
      bAdded = true;
      break;
    }
  }
  if (!bAdded) {
    // SymbolIndex.SetAtGrow(STSize,STSize);
    SymbolIndex.push_back(STSize);
  }
  STSize++;
  return STSize - 1;
}

int CPoly::SetVar(const string &strName, double dVar) {

  if (ErrorOccur)
    return false;
  int index = find(strName);
  if (index == -1)
    return false;
  CSymTable *stVar = lSymbol[(/*FindIndex*/ (index))];
  stVar->dVal = dVar;
  return true;
}

int CPoly::GetVarCount() { return lSymbol.size() - MathSymbolCount; }

const char *CPoly::GetVarName(unsigned int dwIndex) {
  assert(dwIndex + MathSymbolCount < lSymbol.size());
  return lSymbol[dwIndex + MathSymbolCount]->strlex.c_str();
}

void CPoly::init() {
  insert("min", POLY_MINF);
  insert("max", POLY_MAXF);
  insert("number", POLY_IRAND);
  insert("irandom", POLY_IRAND);
  insert("irand", POLY_IRAND);
  insert("frandom", POLY_FRAND);
  insert("frand", POLY_FRAND);
  insert("rt", POLY_ROOT);
  insert("sqrt", POLY_ROOT);
  insert("cos", POLY_COS);
  insert("sin", POLY_SIN);
  insert("tan", POLY_TAN);
  insert("cot", POLY_COT);
  insert("csc", POLY_CSC);
  insert("cosec", POLY_COSEC);
  insert("sec", POLY_SEC);
  insert("pi", POLY_PI);
  SetVar(
      "pi",
      3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117068);
  insert("e", POLY_EXP);
  SetVar(
      "e",
      2.718281828459045235360287471352662497757247093699959574966967627724076630353547594571382178525166427);
  insert("log", POLY_LOG);
  insert("ln", POLY_LN);
  insert("log10", POLY_LOG10);
  insert("abs", POLY_ABS);
  insert("mod", POLY_MOD);
  insert("floor", POLY_FLOOR);
  MathSymbolCount = STSize;
}
