
// GPEngine.h: archivo de encabezado principal para la aplicaci�n GPEngine
//
#pragma once

#ifndef __AFXWIN_H__
	#error "incluir 'stdafx.h' antes de incluir este archivo para PCH"
#endif

#include "resource.h"       // S�mbolos principales


// CGPEngineApp:
// Consulte la secci�n GPEngine.cpp para obtener informaci�n sobre la implementaci�n de esta clase
//

class CGPEngineApp : public CWinAppEx
{
public:
	CGPEngineApp();


// Reemplazos
public:
	virtual BOOL InitInstance();

// Implementaci�n
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CGPEngineApp theApp;
