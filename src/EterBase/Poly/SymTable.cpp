#include "StdAfx.h"
#include "SymTable.h"

CSymTable::CSymTable(int aTok, std::string aStr)
    : dVal(0), token(aTok), strlex(aStr) {}

CSymTable::~CSymTable() {}
