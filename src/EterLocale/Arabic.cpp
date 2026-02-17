#include "StdAfx.h"
#include "Arabic.h"
#include <assert.h>
#include <vector>

enum ARABIC_CODE {
  ARABIC_CODE_BASE = 0x0621,
  ARABIC_CODE_LAST = 0x064a,
  ARABIC_CODE_COUNT = ARABIC_CODE_LAST - ARABIC_CODE_BASE + 1,
};

enum ARABIC_FORM_TYPE {
  DEBUG_CODE,
  ISOLATED,
  INITIAL,
  MEDIAL,
  FINAL,
  ARABIC_FORM_TYPE_NUM,
};

bool Arabic_IsInSpace(wchar_t code) {
  switch (code) {
  case ' ':
  case '\t':
    return true;
  default:
    return false;
  }
}

bool Arabic_IsInSymbol(wchar_t code) {
  return (code >= 0x20 && code <= 0x2f) || (code >= 0x3a && code <= 0x40) ||
         (code >= 0x5b && code <= 0x60) || (code >= 0x7b && code <= 0x7e);
}

bool Arabic_IsInPresentation(wchar_t code) {
  return (code >= 0xfb50 && code <= 0xfdff) ||
         (code >= 0xfe70 && code <= 0xfeff) || (code == 0x08);
}

bool Arabic_HasPresentation(wchar_t *codes, int last) {
  while (last > 0) {
    if (Arabic_IsInSpace(codes[last])) {
      last--;
    } else {
      if (Arabic_IsInPresentation(codes[last]))
        return true;
      else
        break;
    }
  }
  return false;
}

wchar_t Arabic_GetComposition(wchar_t cur, wchar_t next, ARABIC_FORM_TYPE pos) {
  static wchar_t LAM_ALEF_MADDA[ARABIC_FORM_TYPE_NUM] = {0x0622, 0xFEF5, 0, 0,
                                                         0xFEF6};
  static wchar_t LAM_ALEF_HAMZA_ABOVE[ARABIC_FORM_TYPE_NUM] = {0x0623, 0xFEF7,
                                                               0, 0, 0xFEF8};
  static wchar_t LAM_ALEF_HAMZA_BELOW[ARABIC_FORM_TYPE_NUM] = {0x0625, 0xFEF9,
                                                               0, 0, 0xFEFA};
  static wchar_t LAM_ALEF[ARABIC_FORM_TYPE_NUM] = {0x0627, 0xFEFB, 0, 0,
                                                   0xFEFC};

  switch (cur) {
  case 0x0644:
    switch (next) {
    case 0x0622:
      return LAM_ALEF_MADDA[pos];
      break;
    case 0x0623:
      return LAM_ALEF_HAMZA_ABOVE[pos];
      break;
    case 0x0625:
      return LAM_ALEF_HAMZA_BELOW[pos];
      break;
    case 0x0627:
      return LAM_ALEF[pos];
      break;
    }
    break;
  }
  return 0;
}

wchar_t Arabic_GetMap(wchar_t code, ARABIC_FORM_TYPE pos) {
  static wchar_t HAMZA[ARABIC_FORM_TYPE_NUM] = {0x0621, 0xFE80, 0, 0, 0};
  static wchar_t ALEF_MADDA[ARABIC_FORM_TYPE_NUM] = {0x0622, 0xFE81, 0, 0,
                                                     0xFE82};
  static wchar_t ALEF_HAMZA_ABOVE[ARABIC_FORM_TYPE_NUM] = {0x0623, 0xFE83, 0, 0,
                                                           0xFE84};
  static wchar_t WAW_HAMZA[ARABIC_FORM_TYPE_NUM] = {0x0624, 0xFE85, 0, 0,
                                                    0xFE86};
  static wchar_t ALEF_HAMZA_BELOW[ARABIC_FORM_TYPE_NUM] = {0x0625, 0xFE87, 0, 0,
                                                           0xFE88};
  static wchar_t YEH_HAMZA[ARABIC_FORM_TYPE_NUM] = {0x0626, 0xFE89, 0xFE8B,
                                                    0xFE8C, 0xFE8A};
  static wchar_t ALEF[ARABIC_FORM_TYPE_NUM] = {0x0627, 0xFE8D, 0, 0, 0xFE8E};
  static wchar_t BEH[ARABIC_FORM_TYPE_NUM] = {0x0628, 0xFE8F, 0xFE91, 0xFE92,
                                              0xFE90};
  static wchar_t TEH_MARBUTA[ARABIC_FORM_TYPE_NUM] = {0x0629, 0xFE93, 0, 0,
                                                      0xFE94};
  static wchar_t TEH[ARABIC_FORM_TYPE_NUM] = {0x062A, 0xFE95, 0xFE97, 0xFE98,
                                              0xFE96};
  static wchar_t THEH[ARABIC_FORM_TYPE_NUM] = {0x062B, 0xFE99, 0xFE9B, 0xFE9C,
                                               0xFE9A};
  static wchar_t JEEM[ARABIC_FORM_TYPE_NUM] = {0x062C, 0xFE9D, 0xFE9F, 0xFEA0,
                                               0xFE9E};
  static wchar_t HAH[ARABIC_FORM_TYPE_NUM] = {0x062D, 0xFEA1, 0xFEA3, 0xFEA4,
                                              0xFEA2};
  static wchar_t KHAH[ARABIC_FORM_TYPE_NUM] = {0x062E, 0xFEA5, 0xFEA7, 0xFEA8,
                                               0xFEA6};
  static wchar_t DAL[ARABIC_FORM_TYPE_NUM] = {0x062F, 0xFEA9, 0, 0, 0xFEAA};
  static wchar_t THAL[ARABIC_FORM_TYPE_NUM] = {0x0630, 0xFEAB, 0, 0, 0xFEAC};
  static wchar_t REH[ARABIC_FORM_TYPE_NUM] = {0x0631, 0xFEAD, 0, 0, 0xFEAE};
  static wchar_t ZAIN[ARABIC_FORM_TYPE_NUM] = {0x0632, 0xFEAF, 0, 0, 0xFEB0};
  static wchar_t SEEN[ARABIC_FORM_TYPE_NUM] = {0x0633, 0xFEB1, 0xFEB3, 0xFEB4,
                                               0xFEB2};
  static wchar_t SHEEN[ARABIC_FORM_TYPE_NUM] = {0x0634, 0xFEB5, 0xFEB7, 0xFEB8,
                                                0xFEB6};
  static wchar_t SAD[ARABIC_FORM_TYPE_NUM] = {0x0635, 0xFEB9, 0xFEBB, 0xFEBC,
                                              0xFEBA};
  static wchar_t DAD[ARABIC_FORM_TYPE_NUM] = {0x0636, 0xFEBD, 0xFEBF, 0xFEC0,
                                              0xFEBE};
  static wchar_t TAH[ARABIC_FORM_TYPE_NUM] = {0x0637, 0xFEC1, 0xFEC3, 0xFEC4,
                                              0xFEC2};
  static wchar_t ZAH[ARABIC_FORM_TYPE_NUM] = {0x0638, 0xFEC5, 0xFEC7, 0xFEC8,
                                              0xFEC6};
  static wchar_t AIN[ARABIC_FORM_TYPE_NUM] = {0x0639, 0xFEC9, 0xFECB, 0xFECC,
                                              0xFECA};
  static wchar_t GHAIN[ARABIC_FORM_TYPE_NUM] = {0x063A, 0xFECD, 0xFECF, 0xFED0,
                                                0xFECE};
  static wchar_t TATWEEL[ARABIC_FORM_TYPE_NUM] = {0x0640, 0x0640, 0, 0, 0};
  static wchar_t OX_FEH[ARABIC_FORM_TYPE_NUM] = {0x0641, 0xFED1, 0xFED3, 0xFED4,
                                                 0xFED2};
  static wchar_t QAF[ARABIC_FORM_TYPE_NUM] = {0x0642, 0xFED5, 0xFED7, 0xFED8,
                                              0xFED6};
  static wchar_t KAF[ARABIC_FORM_TYPE_NUM] = {0x0643, 0xFED9, 0xFEDB, 0xFEDC,
                                              0xFEDA};
  static wchar_t LAM[ARABIC_FORM_TYPE_NUM] = {0x0644, 0xFEDD, 0xFEDF, 0xFEE0,
                                              0xFEDE};
  static wchar_t MEEM[ARABIC_FORM_TYPE_NUM] = {0x0645, 0xFEE1, 0xFEE3, 0xFEE4,
                                               0xFEE2};
  static wchar_t NOON[ARABIC_FORM_TYPE_NUM] = {0x0646, 0xFEE5, 0xFEE7, 0xFEE8,
                                               0xFEE6};
  static wchar_t HEH[ARABIC_FORM_TYPE_NUM] = {0x0647, 0xFEE9, 0xFEEB, 0xFEEC,
                                              0xFEEA};
  static wchar_t WAW[ARABIC_FORM_TYPE_NUM] = {0x0648, 0xFEED, 0, 0, 0xFEEE};
  static wchar_t ALEF_MAKSURA[ARABIC_FORM_TYPE_NUM] = {0x0649, 0xFEEF, 0, 0,
                                                       0xFEF0};
  static wchar_t YEH[ARABIC_FORM_TYPE_NUM] = {0x064A, 0xFEF1, 0xFEF3, 0xFEF4,
                                              0xFEF2};

  switch (code) {
  case 0x0621:
    return HAMZA[pos];
    break;
  case 0x0622:
    return ALEF_MADDA[pos];
    break;
  case 0x0623:
    return ALEF_HAMZA_ABOVE[pos];
    break;
  case 0x0624:
    return WAW_HAMZA[pos];
    break;
  case 0x0625:
    return ALEF_HAMZA_BELOW[pos];
    break;
  case 0x0626:
    return YEH_HAMZA[pos];
    break;
  case 0x0627:
    return ALEF[pos];
    break;
  case 0x0628:
    return BEH[pos];
    break;
  case 0x0629:
    return TEH_MARBUTA[pos];
    break;
  case 0x062A:
    return TEH[pos];
    break;
  case 0x062B:
    return THEH[pos];
    break;
  case 0x062C:
    return JEEM[pos];
    break;
  case 0x062D:
    return HAH[pos];
    break;
  case 0x062E:
    return KHAH[pos];
    break;
  case 0x062F:
    return DAL[pos];
    break;
  case 0x0630:
    return THAL[pos];
    break;
  case 0x0631:
    return REH[pos];
    break;
  case 0x0632:
    return ZAIN[pos];
    break;
  case 0x0633:
    return SEEN[pos];
    break;
  case 0x0634:
    return SHEEN[pos];
    break;
  case 0x0635:
    return SAD[pos];
    break;
  case 0x0636:
    return DAD[pos];
    break;
  case 0x0637:
    return TAH[pos];
    break;
  case 0x0638:
    return ZAH[pos];
    break;
  case 0x0639:
    return AIN[pos];
    break;
  case 0x063A:
    return GHAIN[pos];
    break;
  case 0x0640:
    return TATWEEL[pos];
    break;
  case 0x0641:
    return OX_FEH[pos];
    break;
  case 0x0642:
    return QAF[pos];
    break;
  case 0x0643:
    return KAF[pos];
    break;
  case 0x0644:
    return LAM[pos];
    break;
  case 0x0645:
    return MEEM[pos];
    break;
  case 0x0646:
    return NOON[pos];
    break;
  case 0x0647:
    return HEH[pos];
    break;
  case 0x0648:
    return WAW[pos];
    break;
  case 0x0649:
    return ALEF_MAKSURA[pos];
    break;
  case 0x064A:
    return YEH[pos];
    break;
  }
  return 0;
}

bool Arabic_IsInMap(wchar_t code) {
  return (code >= ARABIC_CODE_BASE && code <= ARABIC_CODE_LAST);
}

bool Arabic_IsInComposing(wchar_t code) {
  switch (code) {
  case 0x064B: // FATHATAN
  case 0x064C: // DAMMATAN
  case 0x064D: // KASRATAN
  case 0x064E: // FATHA
  case 0x064F: // DAMMA
  case 0x0650: // KASRA
  case 0x0651: // SHADDA
  case 0x0652: // SUKUN
  case 0x0653: // MADDAH ABOVE
  case 0x0654: // HAMZA ABOVE
  case 0x0655: // HAMZA BELOW
  case 0x0670: // SUPERSCRIPT ALEF
  case 0x06D6: // HIGH LIG. SAD WITH LAM WITH ALEF MAKSURA
  case 0x06D7: // HIGH LIG. QAF WITH LAM WITH ALEF MAKSURA
  case 0x06D8: // HIGH MEEM INITIAL FORM
  case 0x06D9: // HIGH LAM ALEF
  case 0x06DA: // HIGH JEEM
  case 0x06DB: // HIGH THREE DOTS
  case 0x06DC: // HIGH SEEN

    // The 2 entires below should not be here - contact unicode.org !!
    // case 0x06DD:         // END OF AYAH
    // case 0x06DE:         // START OF RUB EL HIZB

  case 0x06DF: // HIGH ROUNDED ZERO
  case 0x06E0: // HIGH UPRIGHT RECTANGULAR ZERO
  case 0x06E1: // HIGH DOTLESS HEAD OF KHAH
  case 0x06E2: // HIGH MEEM ISOLATED FORM
  case 0x06E3: // LOW SEEN
  case 0x06E4: // HIGH MADDA
  case 0x06E7: // HIGH YEH
  case 0x06E8: // HIGH NOON
  case 0x06EA: // EMPTY CENTRE LOW STOP
  case 0x06EB: // EMPTY CENTRE HIGH STOP
  case 0x06EC: // HIGH STOP WITH FILLED CENTRE
  case 0x06ED: // LOW MEEM
    return true;
  }
  return false;
}

bool Arabic_IsNext(wchar_t code) { return (code == 0x0640); }

bool Arabic_IsComb1(wchar_t code) { return (code == 0x0644); }

bool Arabic_IsComb2(wchar_t code) {
  switch (code) {
  case 0x0622:
  case 0x0623:
  case 0x0625:
  case 0x0627:
    return true;
  }

  return false;
}

// Helper: Check if a character can join to the right (has INITIAL or MEDIAL
// form)
static inline bool Arabic_CanJoinRight(wchar_t code) {
  if (!Arabic_IsInMap(code))
    return false;
  return Arabic_GetMap(code, INITIAL) != 0 || Arabic_GetMap(code, MEDIAL) != 0;
}

// Helper: Check if a character can join to the left (has MEDIAL or FINAL form)
static inline bool Arabic_CanJoinLeft(wchar_t code) {
  if (!Arabic_IsInMap(code))
    return false;
  return Arabic_GetMap(code, MEDIAL) != 0 || Arabic_GetMap(code, FINAL) != 0 ||
         Arabic_IsNext(code);
}

// Optimized O(n) Arabic shaping algorithm
// Previous: O(n²) due to backward/forward scans for each character
// Now: O(n) single forward pass with state tracking
size_t Arabic_MakeShape(wchar_t *src, size_t srcLen, wchar_t *dst,
                        size_t dstLen) {
  // Runtime validation
  if (!src || !dst || srcLen == 0 || dstLen < srcLen)
    return 0;

  // Phase 1: Pre-scan to find the next non-composing Arabic letter for each
  // position This converts O(n) inner loops into O(1) lookups Use thread-local
  // buffer to avoid per-call allocation
  thread_local static std::vector<size_t> s_nextArabic;
  if (s_nextArabic.size() < srcLen + 1)
    s_nextArabic.resize(srcLen + 1);

  // Build next-arabic lookup (reverse scan)
  size_t nextArabicIdx = srcLen; // Invalid index = no next arabic
  for (size_t i = srcLen; i > 0; --i) {
    size_t idx = i - 1;
    s_nextArabic[idx] = nextArabicIdx;

    wchar_t ch = src[idx];
    if (Arabic_IsInMap(ch) && !Arabic_IsInComposing(ch))
      nextArabicIdx = idx;
  }
  s_nextArabic[srcLen] = srcLen; // Sentinel

  // Phase 2: Single forward pass with state tracking
  size_t dstIndex = 0;
  bool prevJoins = false; // Does previous Arabic letter join to the right?

  for (size_t srcIndex = 0; srcIndex < srcLen; ++srcIndex) {
    wchar_t cur = src[srcIndex];

    // Composing marks: copy directly, don't affect joining state
    if (Arabic_IsInComposing(cur)) {
      if (dstIndex < dstLen)
        dst[dstIndex++] = cur;
      continue;
    }

    if (Arabic_IsInMap(cur)) {
      // Find next joinable Arabic letter using pre-computed lookup
      wchar_t next = 0;
      size_t nextIdx = s_nextArabic[srcIndex];
      if (nextIdx < srcLen) {
        wchar_t nextChar = src[nextIdx];
        if (Arabic_CanJoinLeft(nextChar))
          next = nextChar;
      }

      // Handle LAM-ALEF composition
      if (Arabic_IsComb1(cur) && nextIdx < srcLen &&
          Arabic_IsComb2(src[nextIdx])) {
        wchar_t composed;
        if (prevJoins)
          composed = Arabic_GetComposition(cur, src[nextIdx], FINAL);
        else
          composed = Arabic_GetComposition(cur, src[nextIdx], ISOLATED);

        if (dstIndex < dstLen)
          dst[dstIndex++] = composed;

        // Skip the ALEF that was combined
        srcIndex = nextIdx;
        // LAM-ALEF doesn't join to the right
        prevJoins = false;
        continue;
      }

      // Determine form based on joining context
      wchar_t shaped = 0;
      bool curJoinsRight = false;

      if (prevJoins && next) {
        // Both sides join: MEDIAL
        shaped = Arabic_GetMap(cur, MEDIAL);
        if (shaped)
          curJoinsRight = Arabic_CanJoinRight(cur);
      }

      if (!shaped && prevJoins) {
        // Only left joins: FINAL
        shaped = Arabic_GetMap(cur, FINAL);
        // FINAL form doesn't extend to the right
        curJoinsRight = false;
      }

      if (!shaped && next) {
        // Only right joins: INITIAL
        shaped = Arabic_GetMap(cur, INITIAL);
        if (shaped)
          curJoinsRight = Arabic_CanJoinRight(cur);
      }

      if (!shaped) {
        // No joining: ISOLATED
        shaped = Arabic_GetMap(cur, ISOLATED);
        curJoinsRight = false;
      }

      if (!shaped)
        shaped = cur; // Fallback to original if no mapping

      if (dstIndex < dstLen)
        dst[dstIndex++] = shaped;

      // Update state for next character
      prevJoins = curJoinsRight;
    } else {
      // Non-Arabic character: copy directly, breaks joining
      if (dstIndex < dstLen)
        dst[dstIndex++] = cur;
      prevJoins = false;
    }
  }

  return dstIndex;
}

wchar_t Arabic_ConvSymbol(wchar_t c) {
  switch (c) {
  case '(':
    return ')';
    break;
  case ')':
    return '(';
    break;
  case '<':
    return '>';
    break;
  case '>':
    return '<';
    break;
  case '{':
    return '}';
    break;
  case '}':
    return '{';
    break;
  case '[':
    return ']';
    break;
  case ']':
    return '[';
    break;
  default:
    return c;
  }
}
