#include "StdAfx.h"
#include "GrpTextInstance.h"
#include "StateManager.h"
#include "IME.h"
#include "TextTag.h"
#include "EterBase/Utils.h"
#include "EterLocale/Arabic.h"

#include <unordered_map>
#include <utf8.h>

// Forward declaration to avoid header conflicts
extern bool IsRTL();

const float c_fFontFeather = 0.5f;

CDynamicPool<CGraphicTextInstance> CGraphicTextInstance::ms_kPool;

static int gs_mx = 0;
static int gs_my = 0;

static std::wstring gs_hyperlinkText;

void CGraphicTextInstance::Hyperlink_UpdateMousePos(int x, int y)
{
	gs_mx = x;
	gs_my = y;
	gs_hyperlinkText = L"";
}

int CGraphicTextInstance::Hyperlink_GetText(char* buf, int len)
{
	if (gs_hyperlinkText.empty())
		return 0;

	int written = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, gs_hyperlinkText.c_str(), (int)gs_hyperlinkText.length(), buf, len, nullptr, nullptr);

	return (written > 0) ? written : 0;
}

int CGraphicTextInstance::__DrawCharacter(CGraphicFontTexture * pFontTexture, wchar_t text, DWORD dwColor)
{
	CGraphicFontTexture::TCharacterInfomation* pInsCharInfo = pFontTexture->GetCharacterInfomation(text);

	if (pInsCharInfo)
	{
		m_dwColorInfoVector.push_back(dwColor);
		m_pCharInfoVector.push_back(pInsCharInfo);

		m_textWidth += pInsCharInfo->advance;
		m_textHeight = std::max((WORD)pInsCharInfo->height, m_textHeight);
		return pInsCharInfo->advance;
	}

	return 0;
}

void CGraphicTextInstance::__GetTextPos(DWORD index, float* x, float* y)
{
	index = std::min((size_t)index, m_pCharInfoVector.size());

	float sx = 0;
	float sy = 0;
	float fFontMaxHeight = 0;

	for(DWORD i=0; i<index; ++i)
	{
		if (sx+float(m_pCharInfoVector[i]->width) > m_fLimitWidth)
		{
			sx = 0;
			sy += fFontMaxHeight;
		}

		sx += float(m_pCharInfoVector[i]->advance);
		fFontMaxHeight = std::max(float(m_pCharInfoVector[i]->height), fFontMaxHeight);
	}

	*x = sx;
	*y = sy;
}

void CGraphicTextInstance::Update()
{
	if (m_isUpdate)
		return;

	// Get space height first for empty text cursor rendering
	int spaceHeight = 12; // default fallback
	if (!m_roText.IsNull() && !m_roText->IsEmpty())
	{
		CGraphicFontTexture* pFontTexture = m_roText->GetFontTexturePointer();
		if (pFontTexture)
		{
			CGraphicFontTexture::TCharacterInfomation* pSpaceInfo = pFontTexture->GetCharacterInfomation(L' ');
			spaceHeight = pSpaceInfo ? pSpaceInfo->height : 12;
		}
	}

	auto ResetState = [&, spaceHeight]()
		{
			m_pCharInfoVector.clear();
			m_dwColorInfoVector.clear();
			m_hyperlinkVector.clear();
			m_textWidth = 0;
			m_textHeight = spaceHeight; // Use space height instead of 0 for cursor rendering
			m_computedRTL = IsRTL(); // Use global RTL setting
			m_isUpdate = true;
		};

	if (m_roText.IsNull() || m_roText->IsEmpty())
	{
		ResetState();
		return;
	}

	CGraphicFontTexture* pFontTexture = m_roText->GetFontTexturePointer();
	if (!pFontTexture)
	{
		ResetState();
		return;
	}

	m_pCharInfoVector.clear();
	m_dwColorInfoVector.clear();
	m_hyperlinkVector.clear();

	m_textWidth = 0;
	m_textHeight = spaceHeight;

	const char* utf8 = m_stText.c_str();
	const int utf8Len = (int)m_stText.size();
	DWORD dwColor = m_dwTextColor;

	// UTF-8 -> UTF-16 conversion
	std::vector<wchar_t> wTextBuf((size_t)utf8Len + 1u, 0);
	int wTextLen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, utf8Len, wTextBuf.data(), (int)wTextBuf.size());

	// If strict UTF-8 conversion fails, try lenient mode (replaces invalid sequences)
	if (wTextLen <= 0)
	{
		// Try lenient conversion (no MB_ERR_INVALID_CHARS flag)
		wTextLen = MultiByteToWideChar(CP_UTF8, 0, utf8, utf8Len, wTextBuf.data(), (int)wTextBuf.size());

		if (wTextLen <= 0)
		{
			ResetState();
			return;
		}
	}

	// Set computed RTL based on global setting
	m_computedRTL = IsRTL();

	// Secret mode: draw '*' instead of actual characters
	if (m_isSecret)
	{
		for (int i = 0; i < wTextLen; ++i)
			__DrawCharacter(pFontTexture, L'*', dwColor);

		pFontTexture->UpdateTexture();
		m_isUpdate = true;
		return;
	}

	// === RENDERING APPROACH ===
	// Use BuildVisualBidiText_Tagless() and BuildVisualChatMessage() from utf8.h
	// These functions handle Arabic shaping, BiDi reordering, and chat formatting properly

	// Special handling for chat messages
	if (m_isChatMessage && !m_chatName.empty() && !m_chatMessage.empty())
	{
		std::wstring wName = Utf8ToWide(m_chatName);
		std::wstring wMsg = Utf8ToWide(m_chatMessage);

		// Check if message has tags (hyperlinks)
		bool msgHasTags = (std::find(wMsg.begin(), wMsg.end(), L'|') != wMsg.end());

		if (!msgHasTags)
		{
			// No tags: Use BuildVisualChatMessage() for simple BiDi
			std::vector<wchar_t> visual = BuildVisualChatMessage(
				wName.data(), (int)wName.size(),
				wMsg.data(), (int)wMsg.size(),
				m_computedRTL);

			for (size_t i = 0; i < visual.size(); ++i)
				__DrawCharacter(pFontTexture, visual[i], dwColor);

			pFontTexture->UpdateTexture();
			m_isUpdate = true;
			return;
		}
		else
		{
			// Has tags (hyperlinks): Rebuild as "Message : Name" or "Name : Message"
			// then use tag-aware rendering below
			if (m_computedRTL)
			{
				// RTL: "Message : Name"
				m_stText = m_chatMessage + " : " + m_chatName;
			}
			else
			{
				// LTR: "Name : Message" (original format)
				m_stText = m_chatName + " : " + m_chatMessage;
			}

			// Re-convert to wide chars for tag-aware processing below
			const char* utf8 = m_stText.c_str();
			const int utf8Len = (int)m_stText.size();
			wTextBuf.clear();
			wTextBuf.resize((size_t)utf8Len + 1u, 0);
			wTextLen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, utf8Len, wTextBuf.data(), (int)wTextBuf.size());

			if (wTextLen <= 0)
			{
				wTextLen = MultiByteToWideChar(CP_UTF8, 0, utf8, utf8Len, wTextBuf.data(), (int)wTextBuf.size());
				if (wTextLen <= 0)
				{
					ResetState();
					return;
				}
			}
			// Fall through to tag-aware rendering below
		}
	}

	// Check if text contains tags or RTL
	const bool hasTags = (std::find(wTextBuf.begin(), wTextBuf.begin() + wTextLen, L'|') != (wTextBuf.begin() + wTextLen));
	bool hasRTL = false;
	for (int i = 0; i < wTextLen; ++i)
	{
		if (IsRTLCodepoint(wTextBuf[i]))
		{
			hasRTL = true;
			break;
		}
	}

	// Tag-aware BiDi rendering: Parse tags, apply BiDi per segment, track colors/hyperlinks
	// OPTIMIZED: Use helper lambda to eliminate code duplication (was repeated 5+ times)
	if (hasRTL || hasTags)
	{
		DWORD currentColor = dwColor;
		int hyperlinkStep = 0; // 0=normal, 1=collecting metadata, 2=visible hyperlink
		std::wstring hyperlinkMetadata;

		// Use thread-local buffer to avoid per-call allocation
		thread_local static std::vector<wchar_t> s_currentSegment;
		s_currentSegment.clear();

		SHyperlink currentHyperlink;
		currentHyperlink.sx = currentHyperlink.ex = 0;

		// In chat RTL, force RTL base direction so prefixes like "[hyperlink]" don't flip the paragraph to LTR.
		const bool forceRTLForBidi = (m_isChatMessage && m_computedRTL);

		// OPTIMIZED: Single helper function for flushing segments (eliminates 5x code duplication)
		auto FlushSegment = [&](DWORD segColor) -> int
		{
			if (s_currentSegment.empty())
				return 0;

			int totalWidth = 0;

			// Apply BiDi transformation using optimized BuildVisualBidiText_Tagless
			std::vector<wchar_t> visual = BuildVisualBidiText_Tagless(
				s_currentSegment.data(), (int)s_currentSegment.size(), forceRTLForBidi);

			for (size_t j = 0; j < visual.size(); ++j)
			{
				int w = __DrawCharacter(pFontTexture, visual[j], segColor);
				totalWidth += w;
			}

			s_currentSegment.clear();
			return totalWidth;
		};

		// Prepend glyphs to the already-built draw list (used to place hyperlink before message in RTL chat).
		auto PrependGlyphs = [&](CGraphicFontTexture* pFontTexture,
		                         const std::vector<wchar_t>& chars,
		                         DWORD color,
		                         int& outWidth)
		{
			outWidth = 0;

			// Use thread-local buffers to avoid allocation
			thread_local static std::vector<CGraphicFontTexture::TCharacterInfomation*> s_newCharInfos;
			thread_local static std::vector<DWORD> s_newColors;
			s_newCharInfos.clear();
			s_newColors.clear();
			s_newCharInfos.reserve(chars.size());
			s_newColors.reserve(chars.size());

			for (size_t k = 0; k < chars.size(); ++k)
			{
				auto* pInfo = pFontTexture->GetCharacterInfomation(chars[k]);
				if (!pInfo)
					continue;

				s_newCharInfos.push_back(pInfo);
				s_newColors.push_back(color);

				outWidth += pInfo->advance;
				m_textHeight = std::max((WORD)pInfo->height, m_textHeight);
			}

			// Insert at the beginning of the draw list.
			m_pCharInfoVector.insert(m_pCharInfoVector.begin(), s_newCharInfos.begin(), s_newCharInfos.end());
			m_dwColorInfoVector.insert(m_dwColorInfoVector.begin(), s_newColors.begin(), s_newColors.end());

			// Shift any already-recorded hyperlinks to the right.
			for (auto& link : m_hyperlinkVector)
			{
				link.sx += outWidth;
				link.ex += outWidth;
			}

			m_textWidth += outWidth;
		};

		// Parse text with tags
		for (int i = 0; i < wTextLen;)
		{
			int tagLen = 0;
			std::wstring tagExtra;
			int tagType = GetTextTag(&wTextBuf[i], wTextLen - i, tagLen, tagExtra);

			if (tagType == TEXT_TAG_COLOR)
			{
				// Flush current segment before changing color
				currentHyperlink.ex += FlushSegment(currentColor);
				currentColor = htoi(tagExtra.c_str(), 8);
				i += tagLen;
			}
			else if (tagType == TEXT_TAG_RESTORE_COLOR)
			{
				// Flush segment before restoring color
				currentHyperlink.ex += FlushSegment(currentColor);
				currentColor = dwColor;
				i += tagLen;
			}
			else if (tagType == TEXT_TAG_HYPERLINK_START)
			{
				hyperlinkStep = 1;
				hyperlinkMetadata.clear();
				i += tagLen;
			}
			else if (tagType == TEXT_TAG_HYPERLINK_END)
			{
				if (hyperlinkStep == 1)
				{
					// End of metadata, start visible section
					// Flush any pending non-hyperlink segment first
					currentHyperlink.ex += FlushSegment(currentColor);

					hyperlinkStep = 2;
					currentHyperlink.text = hyperlinkMetadata;
					currentHyperlink.sx = currentHyperlink.ex; // Start hyperlink at current cursor position
				}
				else if (hyperlinkStep == 2)
				{
					// End of visible section - render hyperlink text with proper Arabic handling
					// In RTL chat: we want the hyperlink chunk to appear BEFORE the message, even if logically appended.
					if (!s_currentSegment.empty())
					{
						// OPTIMIZED: Use thread-local buffer for visible rendering
						thread_local static std::vector<wchar_t> s_visibleToRender;
						s_visibleToRender.clear();

						// Find bracket positions: [ ... ]
						int openBracket = -1, closeBracket = -1;
						for (size_t idx = 0; idx < s_currentSegment.size(); ++idx)
						{
							if (s_currentSegment[idx] == L'[' && openBracket == -1)
								openBracket = (int)idx;
							else if (s_currentSegment[idx] == L']' && closeBracket == -1)
								closeBracket = (int)idx;
						}

						if (openBracket >= 0 && closeBracket > openBracket)
						{
							// Keep '['
							s_visibleToRender.push_back(L'[');

							// Extract inside content and apply BiDi
							thread_local static std::vector<wchar_t> s_content;
							s_content.assign(
								s_currentSegment.begin() + openBracket + 1,
								s_currentSegment.begin() + closeBracket);

							// FIX: Use false to let BiDi auto-detect direction from content
							// This ensures English items like [Sword+9] stay LTR
							// while Arabic items like [درع فولاذي+9] are properly RTL
							std::vector<wchar_t> visual = BuildVisualBidiText_Tagless(
								s_content.data(), (int)s_content.size(), false);

							s_visibleToRender.insert(s_visibleToRender.end(), visual.begin(), visual.end());

							// Keep ']'
							s_visibleToRender.push_back(L']');
						}
						else
						{
							// No brackets: apply BiDi to whole segment
							// FIX: Use false to let BiDi auto-detect direction from content
							std::vector<wchar_t> visual = BuildVisualBidiText_Tagless(
								s_currentSegment.data(), (int)s_currentSegment.size(), false);

							s_visibleToRender.insert(s_visibleToRender.end(), visual.begin(), visual.end());
						}

						// Ensure a space AFTER the hyperlink chunk (so it becomes "[hyperlink] اختبار...")
						s_visibleToRender.push_back(L' ');

						// Key behavior:
						// In RTL chat, place hyperlink BEFORE the message by prepending glyphs.
						if (m_isChatMessage && m_computedRTL)
						{
							int addedWidth = 0;
							PrependGlyphs(pFontTexture, s_visibleToRender, currentColor, addedWidth);

							// Record the hyperlink range at the beginning (0..addedWidth)
							currentHyperlink.sx = 0;
							currentHyperlink.ex = addedWidth;
							m_hyperlinkVector.push_back(currentHyperlink);
						}
						else
						{
							// LTR or non-chat: keep original "append" behavior
							currentHyperlink.sx = currentHyperlink.ex;
							for (size_t j = 0; j < s_visibleToRender.size(); ++j)
							{
								int w = __DrawCharacter(pFontTexture, s_visibleToRender[j], currentColor);
								currentHyperlink.ex += w;
							}
							m_hyperlinkVector.push_back(currentHyperlink);
						}
					}

					hyperlinkStep = 0;
					s_currentSegment.clear();
				}
				i += tagLen;
			}
			else // TEXT_TAG_PLAIN or TEXT_TAG_TAG
			{
				if (hyperlinkStep == 1)
				{
					// Collecting hyperlink metadata (hidden)
					hyperlinkMetadata.push_back(wTextBuf[i]);
				}
				else
				{
					// Add to current segment
					// Will be BiDi-processed for normal text, or rendered directly for hyperlinks
					s_currentSegment.push_back(wTextBuf[i]);
				}
				i += tagLen;
			}
		}

		// Flush any remaining segment using optimized helper
		currentHyperlink.ex += FlushSegment(currentColor);

		pFontTexture->UpdateTexture();
		m_isUpdate = true;
		return;
	}

	// Simple LTR rendering for plain text (no tags, no RTL)
	// Just draw characters in logical order
	for (int i = 0; i < wTextLen; ++i)
		__DrawCharacter(pFontTexture, wTextBuf[i], dwColor);

	pFontTexture->UpdateTexture();
	m_isUpdate = true;
}

void CGraphicTextInstance::Render(RECT * pClipRect)
{
	if (!m_isUpdate)
		return;

	CGraphicText* pkText=m_roText.GetPointer();
	if (!pkText)
		return;

	CGraphicFontTexture* pFontTexture = pkText->GetFontTexturePointer();
	if (!pFontTexture)
		return;

	float fStanX = m_v3Position.x;
	float fStanY = m_v3Position.y + 1.0f;

	// Use the computed direction for this text instance, not the global UI direction
	if (m_computedRTL)
	{
		switch (m_hAlign)
		{
			case HORIZONTAL_ALIGN_LEFT:
				fStanX -= m_textWidth;
				break;

			case HORIZONTAL_ALIGN_CENTER:
				fStanX -= float(m_textWidth / 2);
				break;
		}
	}
	else
	{
		switch (m_hAlign)
		{
			case HORIZONTAL_ALIGN_RIGHT:
				fStanX -= m_textWidth;
				break;

			case HORIZONTAL_ALIGN_CENTER:
				fStanX -= float(m_textWidth / 2);
				break;
		}
	}

	switch (m_vAlign)
	{
		case VERTICAL_ALIGN_BOTTOM:
			fStanY -= m_textHeight;
			break;

		case VERTICAL_ALIGN_CENTER:
			fStanY -= float(m_textHeight) / 2.0f;
			break;
	}

	static std::unordered_map<LPDIRECT3DTEXTURE9, std::vector<SVertex>> s_vtxBatches;
	for (auto& [pTexture, vtxBatch] : s_vtxBatches) {
		vtxBatch.clear();
	}

	STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	DWORD dwFogEnable = STATEMANAGER.GetRenderState(D3DRS_FOGENABLE);
	DWORD dwLighting = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);
	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

	STATEMANAGER.SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2,	D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2,	D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_MODULATE);

	{
		const float fFontHalfWeight=1.0f;

		float fCurX;
		float fCurY;

		float fFontSx;
		float fFontSy;
		float fFontEx;
		float fFontEy;
		float fFontWidth;
		float fFontHeight;
		float fFontMaxHeight;
		float fFontAdvance;

		SVertex akVertex[4];
		akVertex[0].z=m_v3Position.z;
		akVertex[1].z=m_v3Position.z;
		akVertex[2].z=m_v3Position.z;
		akVertex[3].z=m_v3Position.z;

		CGraphicFontTexture::TCharacterInfomation* pCurCharInfo;

		if (m_isOutline)
		{
			fCurX=fStanX;
			fCurY=fStanY;
			fFontMaxHeight=0.0f;

			CGraphicFontTexture::TPCharacterInfomationVector::iterator i;
			for (i=m_pCharInfoVector.begin(); i!=m_pCharInfoVector.end(); ++i)
			{
				pCurCharInfo = *i;

				fFontWidth=float(pCurCharInfo->width);
				fFontHeight=float(pCurCharInfo->height);
				fFontAdvance=float(pCurCharInfo->advance);

				if ((fCurX+fFontWidth)-m_v3Position.x > m_fLimitWidth) [[unlikely]] {
					if (m_isMultiLine)
					{
						fCurX=fStanX;
						fCurY+=fFontMaxHeight;
					}
					else
					{
						break;
					}
				}

				if (pClipRect)
				{
					if (fCurY <= pClipRect->top)
					{
						fCurX += fFontAdvance;
						continue;
					}
				}

				fFontSx = fCurX - 0.5f;
				fFontSy = fCurY - 0.5f;
				fFontEx = fFontSx + fFontWidth;
				fFontEy = fFontSy + fFontHeight;

				pFontTexture->SelectTexture(pCurCharInfo->index);
				std::vector<SVertex>& vtxBatch = s_vtxBatches[pFontTexture->GetD3DTexture()];

				akVertex[0].u=pCurCharInfo->left;
				akVertex[0].v=pCurCharInfo->top;
				akVertex[1].u=pCurCharInfo->left;
				akVertex[1].v=pCurCharInfo->bottom;
				akVertex[2].u=pCurCharInfo->right;
				akVertex[2].v=pCurCharInfo->top;
				akVertex[3].u=pCurCharInfo->right;
				akVertex[3].v=pCurCharInfo->bottom;

				akVertex[3].color = akVertex[2].color = akVertex[1].color = akVertex[0].color = m_dwOutLineColor;

				float feather = 0.0f; // m_fFontFeather

				akVertex[0].y=fFontSy-feather;
				akVertex[1].y=fFontEy+feather;
				akVertex[2].y=fFontSy-feather;
				akVertex[3].y=fFontEy+feather;

				// 왼
				akVertex[0].x=fFontSx-fFontHalfWeight-feather;
				akVertex[1].x=fFontSx-fFontHalfWeight-feather;
				akVertex[2].x=fFontEx-fFontHalfWeight+feather;
				akVertex[3].x=fFontEx-fFontHalfWeight+feather;

				vtxBatch.push_back(akVertex[0]); vtxBatch.push_back(akVertex[1]); vtxBatch.push_back(akVertex[2]);
				vtxBatch.push_back(akVertex[2]); vtxBatch.push_back(akVertex[1]); vtxBatch.push_back(akVertex[3]);

				// 오른
				akVertex[0].x=fFontSx+fFontHalfWeight-feather;
				akVertex[1].x=fFontSx+fFontHalfWeight-feather;
				akVertex[2].x=fFontEx+fFontHalfWeight+feather;
				akVertex[3].x=fFontEx+fFontHalfWeight+feather;

				vtxBatch.push_back(akVertex[0]); vtxBatch.push_back(akVertex[1]); vtxBatch.push_back(akVertex[2]);
				vtxBatch.push_back(akVertex[2]); vtxBatch.push_back(akVertex[1]); vtxBatch.push_back(akVertex[3]);

				akVertex[0].x=fFontSx-feather;
				akVertex[1].x=fFontSx-feather;
				akVertex[2].x=fFontEx+feather;
				akVertex[3].x=fFontEx+feather;

				// 위
				akVertex[0].y=fFontSy-fFontHalfWeight-feather;
				akVertex[1].y=fFontEy-fFontHalfWeight+feather;
				akVertex[2].y=fFontSy-fFontHalfWeight-feather;
				akVertex[3].y=fFontEy-fFontHalfWeight+feather;

				vtxBatch.push_back(akVertex[0]); vtxBatch.push_back(akVertex[1]); vtxBatch.push_back(akVertex[2]);
				vtxBatch.push_back(akVertex[2]); vtxBatch.push_back(akVertex[1]); vtxBatch.push_back(akVertex[3]);

				// 아래
				akVertex[0].y=fFontSy+fFontHalfWeight-feather;
				akVertex[1].y=fFontEy+fFontHalfWeight+feather;
				akVertex[2].y=fFontSy+fFontHalfWeight-feather;
				akVertex[3].y=fFontEy+fFontHalfWeight+feather;

				vtxBatch.push_back(akVertex[0]); vtxBatch.push_back(akVertex[1]); vtxBatch.push_back(akVertex[2]);
				vtxBatch.push_back(akVertex[2]); vtxBatch.push_back(akVertex[1]); vtxBatch.push_back(akVertex[3]);

				fCurX += fFontAdvance;
			}
		}

		fCurX=fStanX;
		fCurY=fStanY;
		fFontMaxHeight=0.0f;

		for (int i = 0; i < m_pCharInfoVector.size(); ++i)
		{
			pCurCharInfo = m_pCharInfoVector[i];

			fFontWidth=float(pCurCharInfo->width);
			fFontHeight=float(pCurCharInfo->height);
			fFontMaxHeight=(std::max)(fFontHeight, (float)pCurCharInfo->height);
			fFontAdvance=float(pCurCharInfo->advance);

			if ((fCurX + fFontWidth) - m_v3Position.x > m_fLimitWidth) [[unlikely]] {
				if (m_isMultiLine)
				{
					fCurX=fStanX;
					fCurY+=fFontMaxHeight;
				}
				else
				{
					break;
				}
			}

			if (pClipRect)
			{
				if (fCurY <= pClipRect->top)
				{
					fCurX += fFontAdvance;
					continue;
				}
			}

			fFontSx = fCurX-0.5f;
			fFontSy = fCurY-0.5f;
			fFontEx = fFontSx + fFontWidth;
			fFontEy = fFontSy + fFontHeight;

			pFontTexture->SelectTexture(pCurCharInfo->index);
			std::vector<SVertex>& vtxBatch = s_vtxBatches[pFontTexture->GetD3DTexture()];

			akVertex[0].x=fFontSx;
			akVertex[0].y=fFontSy;
			akVertex[0].u=pCurCharInfo->left;
			akVertex[0].v=pCurCharInfo->top;

			akVertex[1].x=fFontSx;
			akVertex[1].y=fFontEy;
			akVertex[1].u=pCurCharInfo->left;
			akVertex[1].v=pCurCharInfo->bottom;

			akVertex[2].x=fFontEx;
			akVertex[2].y=fFontSy;
			akVertex[2].u=pCurCharInfo->right;
			akVertex[2].v=pCurCharInfo->top;

			akVertex[3].x=fFontEx;
			akVertex[3].y=fFontEy;
			akVertex[3].u=pCurCharInfo->right;
			akVertex[3].v=pCurCharInfo->bottom;

			akVertex[0].color = akVertex[1].color = akVertex[2].color = akVertex[3].color = m_dwColorInfoVector[i];

			vtxBatch.push_back(akVertex[0]); vtxBatch.push_back(akVertex[1]); vtxBatch.push_back(akVertex[2]);
			vtxBatch.push_back(akVertex[2]); vtxBatch.push_back(akVertex[1]); vtxBatch.push_back(akVertex[3]);

			fCurX += fFontAdvance;
		}
	}

	// --- Selection background (Ctrl+A / shift-select) ---
	if (m_isCursor && CIME::ms_bCaptureInput)
	{
		int selBegin = CIME::GetSelBegin();
		int selEnd = CIME::GetSelEnd();

		if (selBegin > selEnd) std::swap(selBegin, selEnd);

		if (selBegin != selEnd)
		{
			float sx, sy, ex, ey;

			// Convert logical selection positions to visual positions (handles tags)
			int visualSelBegin = selBegin;
			int visualSelEnd = selEnd;
			if (!m_logicalToVisualPos.empty())
			{
				if (selBegin >= 0 && selBegin < (int)m_logicalToVisualPos.size())
					visualSelBegin = m_logicalToVisualPos[selBegin];
				if (selEnd >= 0 && selEnd < (int)m_logicalToVisualPos.size())
					visualSelEnd = m_logicalToVisualPos[selEnd];
			}

			__GetTextPos(visualSelBegin, &sx, &sy);
			__GetTextPos(visualSelEnd,   &ex, &sy);

			// Handle RTL - use the computed direction for this text instance
			if (m_computedRTL)
			{
				sx += m_v3Position.x - m_textWidth;
				ex += m_v3Position.x - m_textWidth;
			}
			else
			{
				sx += m_v3Position.x;
				ex += m_v3Position.x;
			}

			// Apply vertical alignment
			float top = m_v3Position.y;
			float bot = m_v3Position.y + m_textHeight;

			switch (m_vAlign)
			{
				case VERTICAL_ALIGN_BOTTOM:
					top -= m_textHeight;
					bot -= m_textHeight;
					break;

				case VERTICAL_ALIGN_CENTER:
					top -= float(m_textHeight) / 2.0f;
					bot -= float(m_textHeight) / 2.0f;
					break;
			}

			TPDTVertex vertices[4];
			vertices[0].diffuse = 0x80339CFF;
			vertices[1].diffuse = 0x80339CFF;
			vertices[2].diffuse = 0x80339CFF;
			vertices[3].diffuse = 0x80339CFF;

			vertices[0].position = TPosition(sx, top, 0.0f);
			vertices[1].position = TPosition(ex, top, 0.0f);
			vertices[2].position = TPosition(sx, bot, 0.0f);
			vertices[3].position = TPosition(ex, bot, 0.0f);

			STATEMANAGER.SetTexture(0, NULL);
			CGraphicBase::SetDefaultIndexBuffer(CGraphicBase::DEFAULT_IB_FILL_RECT);
			if (CGraphicBase::SetPDTStream(vertices, 4))
				STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);
		}
	}

	for (const auto& [pTexture, vtxBatch] : s_vtxBatches) {
		if (vtxBatch.empty())
			continue;

		STATEMANAGER.SetTexture(0, pTexture);
		STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLELIST, vtxBatch.size() / 3, vtxBatch.data(), sizeof(SVertex));
	}

	if (m_isCursor)
	{
		// Draw Cursor
		float sx, sy, ex, ey;
		TDiffuse diffuse;

		int curpos = CIME::GetCurPos();
		int compend = curpos + CIME::GetCompLen();

		// Convert logical cursor position to visual position (handles tags)
		int visualCurpos = curpos;
		int visualCompend = compend;
		if (!m_logicalToVisualPos.empty())
		{
			if (curpos >= 0 && curpos < (int)m_logicalToVisualPos.size())
				visualCurpos = m_logicalToVisualPos[curpos];
			if (compend >= 0 && compend < (int)m_logicalToVisualPos.size())
				visualCompend = m_logicalToVisualPos[compend];
		}

		__GetTextPos(visualCurpos, &sx, &sy);

		// If Composition
		if(visualCurpos<visualCompend)
		{
			diffuse = 0x7fffffff;
			__GetTextPos(visualCompend, &ex, &sy);
		}
		else
		{
			diffuse = 0xffffffff;
			ex = sx + 2;
		}

		// FOR_ARABIC_ALIGN
		// Use the computed direction for this text instance, not the global UI direction
		if (m_computedRTL)
		{
			sx += m_v3Position.x - m_textWidth;
			ex += m_v3Position.x - m_textWidth;
			sy += m_v3Position.y;
		}
		else
		{
			sx += m_v3Position.x;
			sy += m_v3Position.y;
			ex += m_v3Position.x;
		}

		// Apply vertical alignment adjustment BEFORE calculating ey
		switch (m_vAlign)
		{
			case VERTICAL_ALIGN_BOTTOM:
				sy -= m_textHeight;
				break;

			case VERTICAL_ALIGN_CENTER:
				sy -= float(m_textHeight) / 2.0f;
				break;
		}

		// NOW calculate ey after sy has been adjusted
		ey = sy + m_textHeight;

		TPDTVertex vertices[4];
		vertices[0].diffuse = diffuse;
		vertices[1].diffuse = diffuse;
		vertices[2].diffuse = diffuse;
		vertices[3].diffuse = diffuse;
		vertices[0].position = TPosition(sx, sy, 0.0f);
		vertices[1].position = TPosition(ex, sy, 0.0f);
		vertices[2].position = TPosition(sx, ey, 0.0f);
		vertices[3].position = TPosition(ex, ey, 0.0f);

		STATEMANAGER.SetTexture(0, NULL);

		CGraphicBase::SetDefaultIndexBuffer(CGraphicBase::DEFAULT_IB_FILL_RECT);
		if (CGraphicBase::SetPDTStream(vertices, 4))
			STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);

		int ulbegin = CIME::GetULBegin();
		int ulend = CIME::GetULEnd();

		if (ulbegin < ulend)
		{
			__GetTextPos(curpos+ulbegin, &sx, &sy);
			__GetTextPos(curpos+ulend, &ex, &sy);

			sx += m_v3Position.x;
			sy += m_v3Position.y + m_textHeight;
			ex += m_v3Position.x;
			ey = sy + 2;

			vertices[0].diffuse = 0xFFFF0000;
			vertices[1].diffuse = 0xFFFF0000;
			vertices[2].diffuse = 0xFFFF0000;
			vertices[3].diffuse = 0xFFFF0000;
			vertices[0].position = TPosition(sx, sy, 0.0f);
			vertices[1].position = TPosition(ex, sy, 0.0f);
			vertices[2].position = TPosition(sx, ey, 0.0f);
			vertices[3].position = TPosition(ex, ey, 0.0f);

			STATEMANAGER.DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 4, 2, c_FillRectIndices, D3DFMT_INDEX16, vertices, sizeof(TPDTVertex));
		}
	}

	STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
	STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);

	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, dwFogEnable);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, dwLighting);

	if (m_hyperlinkVector.size() != 0)
	{
		// FOR_ARABIC_ALIGN: RTL text is drawn with offset (m_v3Position.x - m_textWidth)
		// Use the computed direction for this text instance, not the global UI direction
		int textOffsetX = m_computedRTL ? (int)(m_v3Position.x - m_textWidth) : (int)m_v3Position.x;

		int lx = gs_mx - textOffsetX;
		int ly = gs_my - (int)m_v3Position.y;

		if (lx >= 0 && ly >= 0 && lx < m_textWidth && ly < m_textHeight)
		{
			std::vector<SHyperlink>::iterator it = m_hyperlinkVector.begin();

			while (it != m_hyperlinkVector.end())
			{
				SHyperlink & link = *it++;
				if (lx >= link.sx && lx < link.ex)
				{
					gs_hyperlinkText = link.text;
					break;
				}
			}
		}
	}
}

void CGraphicTextInstance::CreateSystem(UINT uCapacity)
{
	ms_kPool.Create(uCapacity);
}

void CGraphicTextInstance::DestroySystem()
{
	ms_kPool.Destroy();
}

CGraphicTextInstance* CGraphicTextInstance::New()
{
	return ms_kPool.Alloc();
}

void CGraphicTextInstance::Delete(CGraphicTextInstance* pkInst)
{
	pkInst->Destroy();
	ms_kPool.Free(pkInst);
}

void CGraphicTextInstance::ShowCursor()
{
	m_isCursor = true;
}

void CGraphicTextInstance::HideCursor()
{
	m_isCursor = false;
}

void CGraphicTextInstance::ShowOutLine()
{
	m_isOutline = true;
}

void CGraphicTextInstance::HideOutLine()
{
	m_isOutline = false;
}

void CGraphicTextInstance::SetColor(DWORD color)
{
	if (m_dwTextColor != color)
	{
		for (int i = 0; i < m_pCharInfoVector.size(); ++i)
			if (m_dwColorInfoVector[i] == m_dwTextColor)
				m_dwColorInfoVector[i] = color;

		m_dwTextColor = color;
	}
}

void CGraphicTextInstance::SetColor(float r, float g, float b, float a)
{
	SetColor(D3DXCOLOR(r, g, b, a));
}

void CGraphicTextInstance::SetOutLineColor(DWORD color)
{
	m_dwOutLineColor=color;
}

void CGraphicTextInstance::SetOutLineColor(float r, float g, float b, float a)
{
	m_dwOutLineColor=D3DXCOLOR(r, g, b, a);
}

void CGraphicTextInstance::SetSecret(bool Value)
{
	m_isSecret = Value;
}

void CGraphicTextInstance::SetOutline(bool Value)
{
	m_isOutline = Value;
}

void CGraphicTextInstance::SetFeather(bool Value)
{
	if (Value)
	{
		m_fFontFeather = c_fFontFeather;
	}
	else
	{
		m_fFontFeather = 0.0f;
	}
}

void CGraphicTextInstance::SetMultiLine(bool Value)
{
	m_isMultiLine = Value;
}

void CGraphicTextInstance::SetHorizonalAlign(int hAlign)
{
	m_hAlign = hAlign;
}

void CGraphicTextInstance::SetVerticalAlign(int vAlign)
{
	m_vAlign = vAlign;
}

void CGraphicTextInstance::SetMax(int iMax)
{
	m_iMax = iMax;
}

void CGraphicTextInstance::SetLimitWidth(float fWidth)
{
	m_fLimitWidth = fWidth;
}

void CGraphicTextInstance::SetValueString(const std::string& c_stValue)
{
	if (0 == m_stText.compare(c_stValue))
		return;

	m_stText = c_stValue;
	m_isUpdate = false;
}

void CGraphicTextInstance::SetValue(const char* c_szText, size_t len)
{
	if (0 == m_stText.compare(c_szText))
		return;

	m_stText = c_szText;
	m_isChatMessage = false; // Reset chat mode
	m_isUpdate = false;
}

void CGraphicTextInstance::SetChatValue(const char* c_szName, const char* c_szMessage)
{
	if (!c_szName || !c_szMessage)
		return;

	// Store separated components
	m_chatName = c_szName;
	m_chatMessage = c_szMessage;
	m_isChatMessage = true;

	// Build combined text for rendering (will be processed by Update())
	// Use BuildVisualChatMessage in Update() instead of BuildVisualBidiText_Tagless
	m_stText = std::string(c_szName) + " : " + std::string(c_szMessage);
	m_isUpdate = false;
}

void CGraphicTextInstance::SetPosition(float fx, float fy, float fz)
{
	m_v3Position.x = fx;
	m_v3Position.y = fy;
	m_v3Position.z = fz;
}

void CGraphicTextInstance::SetTextPointer(CGraphicText* pText)
{
	m_roText = pText;
}

void CGraphicTextInstance::SetTextDirection(ETextDirection dir)
{
	if (m_direction == dir)
		return;

	m_direction = dir;
	m_isUpdate = false;
}

const std::string & CGraphicTextInstance::GetValueStringReference()
{
	return m_stText;
}

WORD CGraphicTextInstance::GetTextLineCount()
{
	CGraphicFontTexture::TCharacterInfomation* pCurCharInfo;
	CGraphicFontTexture::TPCharacterInfomationVector::iterator itor;

	float fx = 0.0f;
	WORD wLineCount = 1;
	for (itor=m_pCharInfoVector.begin(); itor!=m_pCharInfoVector.end(); ++itor)
	{
		pCurCharInfo = *itor;

		float fFontWidth=float(pCurCharInfo->width);
		float fFontAdvance=float(pCurCharInfo->advance);
		//float fFontHeight=float(pCurCharInfo->height);

		// NOTE : 폰트 출력에 Width 제한을 둡니다. - [levites]
		if (fx+fFontWidth > m_fLimitWidth)
		{
			fx = 0.0f;
			++wLineCount;
		}

		fx += fFontAdvance;
	}

	return wLineCount;
}

void CGraphicTextInstance::GetTextSize(int* pRetWidth, int* pRetHeight)
{
	*pRetWidth = m_textWidth;
	*pRetHeight = m_textHeight;
}

int CGraphicTextInstance::PixelPositionToCharacterPosition(int iPixelPosition)
{
	// Clamp to valid range [0, textWidth]
	int adjustedPixelPos = iPixelPosition;
	if (adjustedPixelPos < 0)
		adjustedPixelPos = 0;
	if (adjustedPixelPos > m_textWidth)
		adjustedPixelPos = m_textWidth;

	// RTL: interpret click from right edge of rendered text
	if (m_computedRTL)
		adjustedPixelPos = m_textWidth - adjustedPixelPos;

	int icurPosition = 0;
	int visualPos = -1;

	for (int i = 0; i < (int)m_pCharInfoVector.size(); ++i)
	{
		CGraphicFontTexture::TCharacterInfomation* pCurCharInfo = m_pCharInfoVector[i];

		// Use advance instead of width (width is not reliable for cursor hit-testing)
		int adv = pCurCharInfo->advance;
		if (adv <= 0)
			adv = pCurCharInfo->width;

		icurPosition += adv;

		if (adjustedPixelPos < icurPosition)
		{
			visualPos = i;
			break;
		}
	}

	if (visualPos < 0)
		visualPos = (int)m_pCharInfoVector.size();

	if (!m_visualToLogicalPos.empty() && visualPos >= 0 && visualPos < (int)m_visualToLogicalPos.size())
		return m_visualToLogicalPos[visualPos];

	return visualPos;
}

int CGraphicTextInstance::GetHorizontalAlign()
{
	return m_hAlign;
}

void CGraphicTextInstance::__Initialize()
{
	m_roText = NULL;

	m_hAlign = HORIZONTAL_ALIGN_LEFT;
	m_vAlign = VERTICAL_ALIGN_TOP;

	m_iMax = 0;
	m_fLimitWidth = 1600.0f;

	m_isCursor = false;
	m_isSecret = false;
	m_isMultiLine = false;

	m_isOutline = false;
	m_fFontFeather = c_fFontFeather;

	m_isUpdate = false;
	// Use Auto direction for input fields - they should auto-detect based on content
	// Only chat messages should be explicitly set to RTL
	m_direction = ETextDirection::Auto;
	m_computedRTL = false;
	m_isChatMessage = false;
	m_chatName = "";
	m_chatMessage = "";

	m_textWidth = 0;
	m_textHeight = 0;

	m_v3Position.x = m_v3Position.y = m_v3Position.z = 0.0f;

	m_dwOutLineColor=0xff000000;
}

void CGraphicTextInstance::Destroy()
{
	m_stText="";
	m_pCharInfoVector.clear();
	m_dwColorInfoVector.clear();
	m_hyperlinkVector.clear();
	m_logicalToVisualPos.clear();
	m_visualToLogicalPos.clear();

	__Initialize();
}

CGraphicTextInstance::CGraphicTextInstance()
{
	__Initialize();
}

CGraphicTextInstance::~CGraphicTextInstance()
{
	Destroy();
}
