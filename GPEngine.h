
// GPEngine.h: archivo de encabezado principal para la aplicación GPEngine
//
#pragma once

#ifndef __AFXWIN_H__
	#error "incluir 'stdafx.h' antes de incluir este archivo para PCH"
#endif

#include "resource.h"       // Símbolos principales


// CGPEngineApp:
// Consulte la sección GPEngine.cpp para obtener información sobre la implementación de esta clase
//

class CGPEngineApp : public CWinAppEx
{
public:
	CGPEngineApp();


// Reemplazos
public:
	virtual BOOL InitInstance();

// Implementación
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CGPEngineApp theApp;
