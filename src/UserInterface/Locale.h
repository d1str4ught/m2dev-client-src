#pragma once

#include "Locale_inc.h"

const char *GetLocaleName();
const char *GetLocalePath();
const char *GetLocalePathCommon();
bool IsRTL();
int StringCompareCI(LPCSTR szStringLeft, LPCSTR szStringRight,
                    size_t sizeLength);
void LoadConfig(const char *fileName);
unsigned GetGuildLastExp(int level);
int GetSkillPower(unsigned level);
