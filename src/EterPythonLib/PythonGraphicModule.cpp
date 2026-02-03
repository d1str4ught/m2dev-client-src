#include "StdAfx.h"
#include "EterLib/Camera.h"
#include "EterLib/TextBar.h"

#include <string>
#include <utf8.h>
#include <shlobj.h>

PyObject* grpCreateTextBar(PyObject* poSelf, PyObject* poArgs)
{
	int iWidth;
	if (!PyTuple_GetInteger(poArgs, 0, &iWidth))
		return Py_BuildException();
	int iHeight;
	if (!PyTuple_GetInteger(poArgs, 1, &iHeight))
		return Py_BuildException();

	CTextBar * pTextBar = new CTextBar(12, false);
	if (!pTextBar->Create(iWidth, iHeight))
	{
		delete pTextBar;
		return Py_BuildValue("K", NULL);
	}

	return Py_BuildValue("K", pTextBar);
}

PyObject* grpCreateBigTextBar(PyObject* poSelf, PyObject* poArgs)
{
	int iWidth;
	if (!PyTuple_GetInteger(poArgs, 0, &iWidth))
		return Py_BuildException();
	int iHeight;
	if (!PyTuple_GetInteger(poArgs, 1, &iHeight))
		return Py_BuildException();

	int iFontSize;
	if (!PyTuple_GetInteger(poArgs, 2, &iFontSize))
		return Py_BuildException();

	CTextBar * pTextBar = new CTextBar(iFontSize, true);
	if (!pTextBar->Create(iWidth, iHeight))
	{
		delete pTextBar;
		return Py_BuildValue("K", NULL);
	}

	return Py_BuildValue("K", pTextBar);
}

PyObject* grpDestroyTextBar(PyObject* poSelf, PyObject* poArgs)
{
	CTextBar* pTextBar;
	if (!PyTuple_GetPointer(poArgs, 0, &pTextBar))
		return Py_BuildException();

	delete pTextBar;

	return Py_BuildNone();
}

PyObject* grpRenderTextBar(PyObject* poSelf, PyObject* poArgs)
{
	CTextBar* pTextBar;
	if (!PyTuple_GetPointer(poArgs, 0, &pTextBar))
		return Py_BuildException();
	int ix;
	if (!PyTuple_GetInteger(poArgs, 1, &ix))
		return Py_BuildException();
	int iy;
	if (!PyTuple_GetInteger(poArgs, 2, &iy))
		return Py_BuildException();

	if (pTextBar)
		pTextBar->Render(ix, iy);

	return Py_BuildNone();
}

PyObject* grpTextBarSetTextColor(PyObject* poSelf, PyObject* poArgs)
{
	CTextBar* pTextBar;
	if (!PyTuple_GetPointer(poArgs, 0, &pTextBar))
		return Py_BuildException();
	int r;
	if (!PyTuple_GetInteger(poArgs, 1, &r))
		return Py_BuildException();
	int g;
	if (!PyTuple_GetInteger(poArgs, 2, &g))
		return Py_BuildException();
	int b;
	if (!PyTuple_GetInteger(poArgs, 3, &b))
		return Py_BuildException();
	
	if (pTextBar)
		pTextBar->SetTextColor(r, g, b);

	return Py_BuildNone();
}

PyObject* grpTextBarGetTextExtent(PyObject* poSelf, PyObject* poArgs)
{
	CTextBar* pTextBar;
	if (!PyTuple_GetPointer(poArgs, 0, &pTextBar))
		return Py_BuildException();
	char * szText;
	if (!PyTuple_GetString(poArgs, 1, &szText))
		return Py_BuildException();

	SIZE size = {0, 0};
	if (pTextBar)
		pTextBar->GetTextExtent(szText, &size);

	return Py_BuildValue("ii", size.cx, size.cy);
}

PyObject* grpTextBarTextOut(PyObject* poSelf, PyObject* poArgs)
{
	CTextBar* pTextBar;
	if (!PyTuple_GetPointer(poArgs, 0, &pTextBar))
		return Py_BuildException();
	int ix;
	if (!PyTuple_GetInteger(poArgs, 1, &ix))
		return Py_BuildException();
	int iy;
	if (!PyTuple_GetInteger(poArgs, 2, &iy))
		return Py_BuildException();
	char * szText;
	if (!PyTuple_GetString(poArgs, 3, &szText))
		return Py_BuildException();

	if (pTextBar)
		pTextBar->TextOut(ix, iy, szText);

	return Py_BuildNone();
}

PyObject* grpClearTextBar(PyObject* poSelf, PyObject* poArgs)
{
	CTextBar* pTextBar;
	if (!PyTuple_GetPointer(poArgs, 0, &pTextBar))
		return Py_BuildException();

	if (pTextBar)
		pTextBar->ClearBar();

	return Py_BuildNone();
}

PyObject* grpSetTextBarClipRect(PyObject* poSelf, PyObject* poArgs)
{
	CTextBar* pTextBar;
	if (!PyTuple_GetPointer(poArgs, 0, &pTextBar))
		return Py_BuildException();
	int isx;
	if (!PyTuple_GetInteger(poArgs, 1, &isx))
		return Py_BuildException();
	int isy;
	if (!PyTuple_GetInteger(poArgs, 2, &isy))
		return Py_BuildException();
	int iex;
	if (!PyTuple_GetInteger(poArgs, 3, &iex))
		return Py_BuildException();
	int iey;
	if (!PyTuple_GetInteger(poArgs, 4, &iey))
		return Py_BuildException();

	if (pTextBar)
	{
		RECT rect;
		rect.left = isx;
		rect.top = isy;
		rect.right = iex;
		rect.bottom = iey;

		pTextBar->SetClipRect(rect);
	}

	return Py_BuildNone();
}

PyObject* grpCulling(PyObject* poSelf, PyObject* poArgs)
{
	CCullingManager::Instance().Process();
	return Py_BuildNone();
}

PyObject* grpInitScreenEffect(PyObject* poSelf, PyObject* poArgs)
{
	CPythonGraphic::Instance().InitScreenEffect();
	return Py_BuildNone();
}

PyObject* grpClearDepthBuffer(PyObject* poSelf, PyObject* poArgs)
{
	CPythonGraphic::Instance().ClearDepthBuffer();
	return Py_BuildNone();
}

PyObject* grpPushState(PyObject* poSelf, PyObject* poArgs)
{
	CPythonGraphic::Instance().PushState();
	return Py_BuildNone();
}

PyObject* grpPopState(PyObject* poSelf, PyObject* poArgs)
{
	CPythonGraphic::Instance().PopState();
	return Py_BuildNone();
}

PyObject* grpPushMatrix(PyObject* poSelf, PyObject* poArgs)
{
	CPythonGraphic::Instance().PushMatrix();
	return Py_BuildNone();
}

PyObject* grpPopMatrix(PyObject* poSelf, PyObject* poArgs)
{
	CPythonGraphic::Instance().PopMatrix();
	return Py_BuildNone();
}

PyObject* grpTranslate(PyObject* poSelf, PyObject* poArgs)
{
	float x;
	if (!PyTuple_GetFloat(poArgs, 0, &x))
		return Py_BuildException("grp.Translate:Wrong X");

	float y;
	if (!PyTuple_GetFloat(poArgs, 1, &y))
		return Py_BuildException("grp.Translate:Wrong Y");

	float z;
	if (!PyTuple_GetFloat(poArgs, 2, &z))
		return Py_BuildException("grp.Translate:Wrong Z");

	CPythonGraphic::Instance().Translate(x, y, z);

	return Py_BuildNone();
}

PyObject* grpRotate(PyObject* poSelf, PyObject* poArgs)
{
	float Degree;
	if (!PyTuple_GetFloat(poArgs, 0, &Degree))
		return Py_BuildException();

	float x;
	if (!PyTuple_GetFloat(poArgs, 1, &x))
		return Py_BuildException();

	float y;
	if (!PyTuple_GetFloat(poArgs, 2, &y))
		return Py_BuildException();

	float z;
	if (!PyTuple_GetFloat(poArgs, 3, &z))
		return Py_BuildException();

	CPythonGraphic::Instance().Rotate(Degree, x, y, z);

	return Py_BuildNone();
}

PyObject * grpSetColorRenderState(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildNone();
}

PyObject* grpSetAroundCamera(PyObject* poSelf, PyObject* poArgs)
{
	float distance;
	if (!PyTuple_GetFloat(poArgs, 0, &distance))
		return Py_BuildException();

	float pitch;	
	if (!PyTuple_GetFloat(poArgs, 1, &pitch))
		return Py_BuildException();

	float roll;
	if (!PyTuple_GetFloat(poArgs, 2, &roll))
		return Py_BuildException();
	
	float lookAtZ;
	if (!PyTuple_GetFloat(poArgs, 3, &lookAtZ))
		return Py_BuildException();

	CPythonGraphic::Instance().SetAroundCamera(distance, pitch, roll, lookAtZ);
	return Py_BuildNone();
}

PyObject* grpSetPositionCamera(PyObject* poSelf, PyObject* poArgs)
{
	float fx;
	if (!PyTuple_GetFloat(poArgs, 0, &fx))
		return Py_BuildException();

	float fy;	
	if (!PyTuple_GetFloat(poArgs, 1, &fy))
		return Py_BuildException();

	float fz;
	if (!PyTuple_GetFloat(poArgs, 2, &fz))
		return Py_BuildException();
	
	float distance;
	if (!PyTuple_GetFloat(poArgs, 3, &distance))
		return Py_BuildException();

	float pitch;
	if (!PyTuple_GetFloat(poArgs, 4, &pitch))
		return Py_BuildException();
	
	float roll;
	if (!PyTuple_GetFloat(poArgs, 5, &roll))
		return Py_BuildException();

	CPythonGraphic::Instance().SetPositionCamera(fx, fy, fz, distance, pitch, roll);
	return Py_BuildNone();
}

PyObject* grpSetEyeCamera(PyObject* poSelf, PyObject* poArgs)
{
	float xEye;
	if (!PyTuple_GetFloat(poArgs, 0, &xEye))
		return Py_BuildException();

	float yEye;	
	if (!PyTuple_GetFloat(poArgs, 1, &yEye))
		return Py_BuildException();

	float zEye;
	if (!PyTuple_GetFloat(poArgs, 2, &zEye))
		return Py_BuildException();
	
	float xCenter;
	if (!PyTuple_GetFloat(poArgs, 3, &xCenter))
		return Py_BuildException();

	float yCenter;	
	if (!PyTuple_GetFloat(poArgs, 4, &yCenter))
		return Py_BuildException();

	float zCenter;
	if (!PyTuple_GetFloat(poArgs, 5, &zCenter))
		return Py_BuildException();
	
	float xUp;
	if (!PyTuple_GetFloat(poArgs, 6, &xUp))
		return Py_BuildException();

	float yUp;
	if (!PyTuple_GetFloat(poArgs, 7, &yUp))
		return Py_BuildException();

	float zUp;
	if (!PyTuple_GetFloat(poArgs, 8, &zUp))
		return Py_BuildException();

	CPythonGraphic::Instance().SetEyeCamera(xEye, yEye, zEye, xCenter, yCenter, zCenter, xUp, yUp, zUp);
	return Py_BuildNone();
}


PyObject* grpSetPerspective(PyObject* poSelf, PyObject* poArgs)
{
	float fov;
	if (!PyTuple_GetFloat(poArgs, 0, &fov))
		return Py_BuildException();

	float aspect;	
	if (!PyTuple_GetFloat(poArgs, 1, &aspect))
		return Py_BuildException();

	float zNear;
	if (!PyTuple_GetFloat(poArgs, 2, &zNear))
		return Py_BuildException();

	float zFar;
	if (!PyTuple_GetFloat(poArgs, 3, &zFar))
		return Py_BuildException();
	
	CPythonGraphic::Instance().SetPerspective(fov, aspect, zNear, zFar);
	return Py_BuildNone();
}


PyObject* grpSetOrtho2D(PyObject* poSelf, PyObject* poArgs)
{
	float width;
	if (!PyTuple_GetFloat(poArgs, 0, &width))
		return Py_BuildException();

	float height;
	if (!PyTuple_GetFloat(poArgs, 1, &height))
		return Py_BuildException();

	float length;
	if (!PyTuple_GetFloat(poArgs, 2, &length))
		return Py_BuildException();

	CPythonGraphic::Instance().SetOrtho2D(width, height, length);
	return Py_BuildNone();
}


PyObject* grpSetOrtho3D(PyObject* poSelf, PyObject* poArgs)
{
	float width;
	if (!PyTuple_GetFloat(poArgs, 0, &width))
		return Py_BuildException();

	float height;
	if (!PyTuple_GetFloat(poArgs, 1, &height))
		return Py_BuildException();

	float zmin;
	if (!PyTuple_GetFloat(poArgs, 2, &zmin))
		return Py_BuildException();

	float zmax;
	if (!PyTuple_GetFloat(poArgs, 3, &zmax))
		return Py_BuildException();

	CPythonGraphic::Instance().SetOrtho3D(width, height, zmin, zmax);
	return Py_BuildNone();
}


PyObject* grpGenerateColor(PyObject* poSelf, PyObject* poArgs)
{
	float r;
	if (!PyTuple_GetFloat(poArgs, 0, &r))
		return Py_BuildException();

	float g;
	if (!PyTuple_GetFloat(poArgs, 1, &g))
		return Py_BuildException();

	float b;
	if (!PyTuple_GetFloat(poArgs, 2, &b))
		return Py_BuildException();

	float a;
	if (!PyTuple_GetFloat(poArgs, 3, &a))
		return Py_BuildException();
	
	return Py_BuildValue("l", CPythonGraphic::Instance().GenerateColor(r, g, b, a));
}

PyObject* grpSetColor(PyObject* poSelf, PyObject* poArgs)
{
	int color;
	if (!PyTuple_GetInteger(poArgs, 0, &color))
		return Py_BuildException();

	CPythonGraphic::Instance().SetDiffuseColor(DWORD(color));
	return Py_BuildNone();
}

PyObject* grpSetAlpha(PyObject* poSelf, PyObject* poArgs)
{
	float Alpha;
	if (!PyTuple_GetFloat(poArgs, 0, &Alpha))
		return Py_BuildException();

	//CPythonGraphic::Instance().SetAlpha(Alpha);
	return Py_BuildNone();
}

PyObject* grpGetCursorPosition3d(PyObject * poSelf, PyObject * poArgs)
{
	float x, y, z;
	CPythonGraphic::Instance().GetCursorPosition(&x, &y, &z);
	return Py_BuildValue("(fff)", x, y, z);
}

PyObject* grpSetCursorPosition(PyObject * poSelf, PyObject * poArgs)
{
	int ix;
	if (!PyTuple_GetInteger(poArgs, 0, &ix))
		return Py_BuildException();
	int iy;
	if (!PyTuple_GetInteger(poArgs, 1, &iy))
		return Py_BuildException();

	CPythonGraphic::Instance().SetCursorPosition(ix, iy);
	return Py_BuildNone();
}

PyObject* grpRenderLine(PyObject* poSelf, PyObject* poArgs)
{
	int x;
	if (!PyTuple_GetInteger(poArgs, 0, &x))
		return Py_BuildException();

	int y;
	if (!PyTuple_GetInteger(poArgs, 1, &y))
		return Py_BuildException();

	int width;
	if (!PyTuple_GetInteger(poArgs, 2, &width))
		return Py_BuildException();

	int height;
	if (!PyTuple_GetInteger(poArgs, 3, &height))
		return Py_BuildException();

	CPythonGraphic::Instance().RenderLine2d((float) x, (float) y, (float) x+width, (float) y+height);
	return Py_BuildNone();
}

PyObject* grpRenderRoundBox(PyObject* poSelf, PyObject* poArgs)
{
	int x;
	if (!PyTuple_GetInteger(poArgs, 0, &x))
		return Py_BuildException();

	int y;
	if (!PyTuple_GetInteger(poArgs, 1, &y))
		return Py_BuildException();

	int width;
	if (!PyTuple_GetInteger(poArgs, 2, &width))
		return Py_BuildException();

	int height;
	if (!PyTuple_GetInteger(poArgs, 3, &height))
		return Py_BuildException();

	float fx = float(x);
	float fy = float(y);
	float fWidth = float(width);
	float fHeight = float(height);

	CPythonGraphic::Instance().RenderLine2d(fx+2.0f, fy, fx+2.0f + (fWidth-3.0f), fy);
	CPythonGraphic::Instance().RenderLine2d(fx+2.0f, fy+fHeight, fx+2.0f + (fWidth-3.0f), fy+fHeight);
	CPythonGraphic::Instance().RenderLine2d(fx, fy+2.0f, fx, fy+2.0f + fHeight-4.0f);
	CPythonGraphic::Instance().RenderLine2d(fx+fWidth, fy+1.0f, fx+fWidth, fy+1.0f + fHeight-3.0f);
	CPythonGraphic::Instance().RenderLine2d(fx, fy+2.0f, fx+2.0f, fy);
	CPythonGraphic::Instance().RenderLine2d(fx, fy+fHeight-2.0f, fx+2.0f, fy+fHeight);
	CPythonGraphic::Instance().RenderLine2d(fx+fWidth-2.0f, fy, fx+fWidth, fy+2.0f);
	CPythonGraphic::Instance().RenderLine2d(fx+fWidth-2.0f, fy+fHeight, fx+fWidth, fy+fHeight-2.0f);
	return Py_BuildNone();
}

PyObject* grpRenderBox(PyObject* poSelf, PyObject* poArgs)
{
	int x;
	if (!PyTuple_GetInteger(poArgs, 0, &x))
		return Py_BuildException();

	int y;
	if (!PyTuple_GetInteger(poArgs, 1, &y))
		return Py_BuildException();

	int width;
	if (!PyTuple_GetInteger(poArgs, 2, &width))
		return Py_BuildException();

	int height;
	if (!PyTuple_GetInteger(poArgs, 3, &height))
		return Py_BuildException();

	CPythonGraphic::Instance().RenderBox2d((float) x, (float) y, (float) x+width, (float) y+height);
	return Py_BuildNone();
}

PyObject* grpRenderBar(PyObject* poSelf, PyObject* poArgs)
{
	int x;
	if (!PyTuple_GetInteger(poArgs, 0, &x))
		return Py_BuildException();

	int y;
	if (!PyTuple_GetInteger(poArgs, 1, &y))
		return Py_BuildException();

	int width;
	if (!PyTuple_GetInteger(poArgs, 2, &width))
		return Py_BuildException();

	int height;
	if (!PyTuple_GetInteger(poArgs, 3, &height))
		return Py_BuildException();

	CPythonGraphic::Instance().RenderBar2d((float) x, (float) y, (float) x+width, (float) y+height);
	return Py_BuildNone();
}

PyObject* grpRenderCube(PyObject* poSelf, PyObject* poArgs)
{
	float sx;
	if (!PyTuple_GetFloat(poArgs, 0, &sx))
		return Py_BuildException();

	float sy;
	if (!PyTuple_GetFloat(poArgs, 1, &sy))
		return Py_BuildException();

	float sz;
	if (!PyTuple_GetFloat(poArgs, 2, &sz))
		return Py_BuildException();

	float ex;
	if (!PyTuple_GetFloat(poArgs, 3, &ex))
		return Py_BuildException();

	float ey;
	if (!PyTuple_GetFloat(poArgs, 4, &ey))
		return Py_BuildException();

	float ez;
	if (!PyTuple_GetFloat(poArgs, 5, &ez))
		return Py_BuildException();

	CPythonGraphic::Instance().RenderCube(sx, sy, sz, ex, ey, ez);
	return Py_BuildNone();
}

PyObject* grpRenderBar3d(PyObject* poSelf, PyObject* poArgs)
{
	int sx;
	if (!PyTuple_GetInteger(poArgs, 0, &sx))
		return Py_BuildException();

	int sy;
	if (!PyTuple_GetInteger(poArgs, 1, &sy))
		return Py_BuildException();

	int sz;
	if (!PyTuple_GetInteger(poArgs, 2, &sz))
		return Py_BuildException();

	int ex;
	if (!PyTuple_GetInteger(poArgs, 3, &ex))
		return Py_BuildException();

	int ey;
	if (!PyTuple_GetInteger(poArgs, 4, &ey))
		return Py_BuildException();

	int ez;
	if (!PyTuple_GetInteger(poArgs, 5, &ez))
		return Py_BuildException();

	CPythonGraphic::Instance().RenderBar3d(sx, sy, sz, ex, ey, ez);
	return Py_BuildNone();
}

PyObject * grpRenderGradationBar(PyObject* poSelf, PyObject* poArgs)
{
	int x;
	if (!PyTuple_GetInteger(poArgs, 0, &x))
		return Py_BadArgument();

	int y;
	if (!PyTuple_GetInteger(poArgs, 1, &y))
		return Py_BadArgument();

	int width;
	if (!PyTuple_GetInteger(poArgs, 2, &width))
		return Py_BadArgument();

	int height;
	if (!PyTuple_GetInteger(poArgs, 3, &height))
		return Py_BadArgument();

	int iStartColor;
	if (!PyTuple_GetInteger(poArgs, 4, &iStartColor))
		return Py_BadArgument();

	int iEndColor;
	if (!PyTuple_GetInteger(poArgs, 5, &iEndColor))
		return Py_BadArgument();

	CPythonGraphic::Instance().RenderGradationBar2d((float)x, (float)y, (float)x+width, (float)y+height, iStartColor, iEndColor);
	return Py_BuildNone();
}

PyObject* grpRenderDownButton(PyObject* poSelf, PyObject* poArgs)
{
	int x;
	if (!PyTuple_GetInteger(poArgs, 0, &x))
		return Py_BuildException();

	int y;
	if (!PyTuple_GetInteger(poArgs, 1, &y))
		return Py_BuildException();

	int width;
	if (!PyTuple_GetInteger(poArgs, 2, &width))
		return Py_BuildException();

	int height;
	if (!PyTuple_GetInteger(poArgs, 3, &height))
		return Py_BuildException();

	CPythonGraphic::Instance().RenderDownButton((float) x, (float) y, (float) x+width, (float) y+height);
	return Py_BuildNone();
}

PyObject* grpRenderUpButton(PyObject* poSelf, PyObject* poArgs)
{
	int x;
	if (!PyTuple_GetInteger(poArgs, 0, &x))
		return Py_BuildException();

	int y;
	if (!PyTuple_GetInteger(poArgs, 1, &y))
		return Py_BuildException();

	int width;
	if (!PyTuple_GetInteger(poArgs, 2, &width))
		return Py_BuildException();

	int height;
	if (!PyTuple_GetInteger(poArgs, 3, &height))
		return Py_BuildException();

	CPythonGraphic::Instance().RenderUpButton((float) x, (float) y, (float) x+width, (float) y+height);
	return Py_BuildNone();
}

PyObject* grpRenderBox3d(PyObject* poSelf, PyObject* poArgs)
{
	float sx;
	if (!PyTuple_GetFloat(poArgs, 0, &sx))
		return Py_BuildException();

	float sy;
	if (!PyTuple_GetFloat(poArgs, 1, &sy))
		return Py_BuildException();

	float sz;
	if (!PyTuple_GetFloat(poArgs, 2, &sz))
		return Py_BuildException();

	float ex;
	if (!PyTuple_GetFloat(poArgs, 3, &ex))
		return Py_BuildException();

	float ey;
	if (!PyTuple_GetFloat(poArgs, 4, &ey))
		return Py_BuildException();

	float ez;
	if (!PyTuple_GetFloat(poArgs, 5, &ez))
		return Py_BuildException();

	CPythonGraphic::Instance().RenderBox3d((float) sx, (float) sy, (float) sz, (float) ex, (float) ey, (float) ez);
	return Py_BuildNone();
}

PyObject * grpSetClearColor(PyObject * poSelf, PyObject * poArgs)
{
	float fr;
	if (!PyTuple_GetFloat(poArgs, 0, &fr))
		return Py_BuildException();

	float fg;
	if (!PyTuple_GetFloat(poArgs, 1, &fg))
		return Py_BuildException();

	float fb;
	if (!PyTuple_GetFloat(poArgs, 2, &fb))
		return Py_BuildException();

	CPythonGraphic::Instance().SetClearColor(fr, fg, fb, 1.0f);
	return Py_BuildNone();
}

PyObject * grpSetDiffuseColor(PyObject * poSelf, PyObject * poArgs)
{
	float r;
	if (!PyTuple_GetFloat(poArgs, 0, &r))
		return Py_BuildException();

	float g;
	if (!PyTuple_GetFloat(poArgs, 1, &g))
		return Py_BuildException();

	float b;
	if (!PyTuple_GetFloat(poArgs, 2, &b))
		return Py_BuildException();

	float a;
	if (!PyTuple_GetFloat(poArgs, 3, &a))
		return Py_BuildException();

	CPythonGraphic::Instance().SetDiffuseColor(r, g, b, a);
	return Py_BuildNone();
}

PyObject * grpIdentity(PyObject * poSelf, PyObject * poArgs)
{
	CPythonGraphic::Instance().Identity();
	return Py_BuildNone();
}

PyObject * grpGetAvailableMemory(PyObject * poSelf, PyObject * poArgs)
{
	int iValue = CPythonGraphic::Instance().GetAvailableMemory();
	return Py_BuildValue("i", iValue);
}

// SCREENSHOT_CWDSAVE
PyObject * grpSaveScreenShotToPath(PyObject * poSelf, PyObject * poArgs)
{
	char * szBasePath;
	if (!PyTuple_GetString(poArgs, 0, &szBasePath))
		return Py_BuildException();

	struct tm * tmNow;
	time_t ct;

	ct = time(0);
	tmNow = localtime(&ct);

	char szPath[MAX_PATH + 256];
	snprintf(szPath, sizeof(szPath), "%s%02d%02d_%02d%02d%02d.jpg", 
			szBasePath,
			tmNow->tm_mon + 1,
			tmNow->tm_mday,
			tmNow->tm_hour,
			tmNow->tm_min,
			tmNow->tm_sec);

	BOOL bResult = CPythonGraphic::Instance().SaveScreenShot(szPath);
	return Py_BuildValue("(is)", bResult, szPath);
}
// END_OF_SCREENSHOT_CWDSAVE

PyObject* grpSaveScreenShot(PyObject* poSelf, PyObject* poArgs)
{
	time_t ct = time(nullptr);
	tm tmNow{};
	localtime_s(&tmNow, &ct);

	wchar_t wPath[MAX_PATH + 256]{};

	if (!SHGetSpecialFolderPathW(nullptr, wPath, CSIDL_PERSONAL, TRUE))
	{
		TraceError("SHGetSpecialFolderPathW failed");
		return Py_BuildValue("(is)", FALSE, "");
	}

	wcscat_s(wPath, L"\\METIN2\\");

	if (_waccess(wPath, 0) == -1)
	{
		if (!CreateDirectoryW(wPath, nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
		{
			TraceError("Failed to create directory [%s]", WideToUtf8(wPath).c_str());
			return Py_BuildValue("(is)", FALSE, "");
		}
	}

	wchar_t wFile[64]{};
	swprintf_s(
		wFile,
		L"%02d%02d_%02d%02d%02d.jpg",
		tmNow.tm_mon + 1,
		tmNow.tm_mday,
		tmNow.tm_hour,
		tmNow.tm_min,
		tmNow.tm_sec
	);

	wcscat_s(wPath, wFile);

	// If SaveScreenShot accepts wide paths:
	BOOL bResult = CPythonGraphic::Instance().SaveScreenShot(WideToUtf8(wPath).c_str());

	// Python expects bytes -> return UTF-8
	std::string pathUtf8 = WideToUtf8(wPath);
	return Py_BuildValue("(is)", bResult, pathUtf8.c_str());
}

PyObject * grpSetGamma(PyObject * poSelf, PyObject * poArgs)
{
	float gamma;

	if (!PyTuple_GetFloat(poArgs, 0, &gamma))
		return Py_BuildException();

	CPythonGraphic::Instance().SetGamma(gamma);
	return Py_BuildNone();
}

PyObject * grpSetInterfaceRenderState(PyObject * poSelf, PyObject * poArgs)
{
	CPythonGraphic::Instance().SetInterfaceRenderState();
	return Py_BuildNone();
}

PyObject * grpSetGameRenderState(PyObject * poSelf, PyObject * poArgs)
{
	CPythonGraphic::Instance().SetGameRenderState();
	return Py_BuildNone();
}

PyObject * grpSetViewport(PyObject * poSelf, PyObject * poArgs)
{
	float fx;
	if (!PyTuple_GetFloat(poArgs, 0, &fx))
		return Py_BuildException();
	float fy;
	if (!PyTuple_GetFloat(poArgs, 1, &fy))
		return Py_BuildException();

	float fWidth;
	if (!PyTuple_GetFloat(poArgs, 2, &fWidth))
		return Py_BuildException();
	float fHeight;
	if (!PyTuple_GetFloat(poArgs, 3, &fHeight))
		return Py_BuildException();

	UINT uWidth;
	UINT uHeight;
	CPythonGraphic::Instance().GetBackBufferSize(&uWidth, &uHeight);
	CPythonGraphic::Instance().SetViewport(fx*uWidth, fy*uHeight, fWidth*uWidth, fHeight*uHeight);
	return Py_BuildNone();
}

PyObject * grpRestoreViewport(PyObject * poSelf, PyObject * poArgs)
{
	CPythonGraphic::Instance().RestoreViewport();
	return Py_BuildNone();
}

PyObject * grpSetOmniLight(PyObject * poSelf, PyObject * poArgs)
{
	CPythonGraphic::Instance().SetOmniLight();
	return Py_BuildNone();
}

PyObject * grpGetCameraPosition(PyObject * poSelf, PyObject * poArgs)
{
	D3DXVECTOR3 v3Eye = CCameraManager::Instance().GetCurrentCamera()->GetEye();
	return Py_BuildValue("fff", v3Eye.x, v3Eye.y, v3Eye.z);
}

PyObject * grpGetTargetPosition(PyObject * poSelf, PyObject * poArgs)
{
	D3DXVECTOR3 v3Target = CCameraManager::Instance().GetCurrentCamera()->GetTarget();
	return Py_BuildValue("fff", v3Target.x, v3Target.y, v3Target.z);
}

void initgrp()
{
	static PyMethodDef s_methods[] =
	{
		{ "InitScreenEffect",			grpInitScreenEffect,			METH_VARARGS },
		{ "Culling",					grpCulling,						METH_VARARGS },
		{ "ClearDepthBuffer",			grpClearDepthBuffer,			METH_VARARGS },
		{ "Identity",					grpIdentity,					METH_VARARGS },
		{ "GenerateColor",				grpGenerateColor,				METH_VARARGS },
		{ "PopState",					grpPopState,					METH_VARARGS },
		{ "PushState",					grpPushState,					METH_VARARGS },
		{ "PushMatrix",					grpPushMatrix,					METH_VARARGS },
		{ "PopMatrix",					grpPopMatrix,					METH_VARARGS },
		{ "Translate",					grpTranslate,					METH_VARARGS },
		{ "Rotate",						grpRotate,						METH_VARARGS },
		{ "SetColorRenderState",		grpSetColorRenderState,			METH_VARARGS },
		{ "SetAroundCamera",			grpSetAroundCamera,				METH_VARARGS },
		{ "SetPositionCamera",			grpSetPositionCamera,			METH_VARARGS },
		{ "SetEyeCamera",				grpSetEyeCamera,				METH_VARARGS },
		{ "SetPerspective",				grpSetPerspective,				METH_VARARGS },
		{ "SetOrtho2d",					grpSetOrtho2D,					METH_VARARGS },
		{ "SetOrtho3d",					grpSetOrtho3D,					METH_VARARGS },
		{ "SetColor",					grpSetColor,					METH_VARARGS },
		{ "SetAlpha",					grpSetAlpha,					METH_VARARGS },
		{ "SetDiffuseColor",			grpSetDiffuseColor,				METH_VARARGS },
		{ "SetClearColor",				grpSetClearColor,				METH_VARARGS },
		{ "GetCursorPosition3d",		grpGetCursorPosition3d,			METH_VARARGS },
		{ "SetCursorPosition",			grpSetCursorPosition,			METH_VARARGS },
		{ "RenderLine",					grpRenderLine,					METH_VARARGS },
		{ "RenderBox",					grpRenderBox,					METH_VARARGS },
		{ "RenderRoundBox",				grpRenderRoundBox,				METH_VARARGS },
		{ "RenderBox3d",				grpRenderBox3d,					METH_VARARGS },
		{ "RenderBar",					grpRenderBar,					METH_VARARGS },
		{ "RenderBar3d",				grpRenderBar3d,					METH_VARARGS },
		{ "RenderGradationBar",			grpRenderGradationBar,			METH_VARARGS },
		{ "RenderCube",					grpRenderCube,					METH_VARARGS },
		{ "RenderDownButton",			grpRenderDownButton,			METH_VARARGS },
		{ "RenderUpButton",				grpRenderUpButton,				METH_VARARGS },
		{ "GetAvailableMemory",			grpGetAvailableMemory,			METH_VARARGS },
		{ "SaveScreenShot",				grpSaveScreenShot,				METH_VARARGS },
		{ "SaveScreenShotToPath",		grpSaveScreenShotToPath,		METH_VARARGS },
		{ "SetGamma",					grpSetGamma,					METH_VARARGS },
		{ "SetInterfaceRenderState",	grpSetInterfaceRenderState,		METH_VARARGS },
		{ "SetGameRenderState",			grpSetGameRenderState,			METH_VARARGS },
		{ "SetViewport",				grpSetViewport,					METH_VARARGS },
		{ "RestoreViewport",			grpRestoreViewport,				METH_VARARGS },
		{ "SetOmniLight",				grpSetOmniLight,				METH_VARARGS },
		{ "GetCameraPosition",			grpGetCameraPosition,			METH_VARARGS },
		{ "GetTargetPosition",			grpGetTargetPosition,			METH_VARARGS },

		{ "CreateTextBar",				grpCreateTextBar,				METH_VARARGS },
		{ "CreateBigTextBar",			grpCreateBigTextBar,			METH_VARARGS },
		{ "DestroyTextBar",				grpDestroyTextBar,				METH_VARARGS },
		{ "RenderTextBar",				grpRenderTextBar,				METH_VARARGS },
		{ "TextBarTextOut",				grpTextBarTextOut,				METH_VARARGS },
		{ "TextBarSetTextColor",		grpTextBarSetTextColor,			METH_VARARGS },
		{ "TextBarGetTextExtent",		grpTextBarGetTextExtent,		METH_VARARGS },
		{ "ClearTextBar",				grpClearTextBar,				METH_VARARGS },
		{ "SetTextBarClipRect",			grpSetTextBarClipRect,			METH_VARARGS },

		{ NULL,							NULL,							NULL		},
	};

	Py_InitModule("grp", s_methods);
}
