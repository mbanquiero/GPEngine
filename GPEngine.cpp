
// GPEngine.cpp : define los comportamientos de las clases para la aplicaci�n.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "GPEngine.h"
#include "MainFrm.h"

#include "GPEngineDoc.h"
#include "GPEngineView.h"

#include "world.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CGPEngineView *_engine_view = NULL;

// CGPEngineApp

BEGIN_MESSAGE_MAP(CGPEngineApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CGPEngineApp::OnAppAbout)
	// Comandos de documento est�ndar basados en archivo
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
END_MESSAGE_MAP()


// Construcci�n de CGPEngineApp

CGPEngineApp::CGPEngineApp()
{
	m_bHiColorIcons = TRUE;

	// TODO: reemplace la cadena de identificador de aplicaci�n siguiente por una cadena de identificador �nico; el formato
	// recomendado para la cadena es NombreCompa��a.NombreProducto.Subproducto.Informaci�nDeVersi�n
	SetAppID(_T("GPEngine.AppID.NoVersion"));

	// TODO: agregar aqu� el c�digo de construcci�n,
	// Colocar toda la inicializaci�n importante en InitInstance
}

// El �nico objeto CGPEngineApp

CGPEngineApp theApp;


// Inicializaci�n de CGPEngineApp

BOOL CGPEngineApp::InitInstance()
{
	// Windows XP requiere InitCommonControlsEx() si un manifiesto de
	// aplicaci�n especifica el uso de ComCtl32.dll versi�n 6 o posterior para habilitar
	// estilos visuales. De lo contrario, se generar� un error al crear ventanas.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Establecer para incluir todas las clases de control comunes que desee utilizar
	// en la aplicaci�n.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();


	EnableTaskbarInteraction(FALSE);

	// Se necesita AfxInitRichEdit2() para usar el control RichEdit	
	// AfxInitRichEdit2();

	// Inicializaci�n est�ndar
	// Si no utiliza estas caracter�sticas y desea reducir el tama�o
	// del archivo ejecutable final, debe quitar
	// las rutinas de inicializaci�n espec�ficas que no necesite
	// Cambie la clave del Registro en la que se almacena la configuraci�n
	// TODO: debe modificar esta cadena para que contenga informaci�n correcta
	// como el nombre de su compa��a u organizaci�n
	SetRegistryKey(_T("Aplicaciones generadas con el Asistente para aplicaciones local"));
	LoadStdProfileSettings(4);  // Cargar opciones de archivo INI est�ndar (incluidas las de la lista MRU)


	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Registrar las plantillas de documento de la aplicaci�n. Las plantillas de documento
	//  sirven como conexi�n entre documentos, ventanas de marco y vistas
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CGPEngineDoc),
		RUNTIME_CLASS(CMainFrame),       // Ventana de marco SDI principal
		RUNTIME_CLASS(CGPEngineView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// Analizar l�nea de comandos para comandos Shell est�ndar, DDE, Archivo Abrir
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Habilitar apertura de ejecuci�n DDE
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);


	// Enviar comandos especificados en la l�nea de comandos. Devolver� FALSE si
	// la aplicaci�n se inici� con los modificadores /RegServer, /Register, /Unregserver o /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;





	if(_engine_view!=NULL)
		_engine_view->init = true;

	// Se ha inicializado la �nica ventana; mostrarla y actualizarla
	m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
	m_pMainWnd->UpdateWindow();
	// Llamar a DragAcceptFiles s�lo si existe un sufijo
	//  En una aplicaci�n SDI, esto debe ocurrir despu�s de ProcessShellCommand
	// Habilitar apertura de arrastrar y colocar
	m_pMainWnd->DragAcceptFiles();





	return TRUE;
}

// Controladores de mensaje de CGPEngineApp


// Cuadro de di�logo CAboutDlg utilizado para el comando Acerca de

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Datos del cuadro de di�logo
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Compatibilidad con DDX/DDV

// Implementaci�n
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// Comando de la aplicaci�n para ejecutar el cuadro de di�logo
void CGPEngineApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// M�todos para cargar o guardar personalizaciones de  CGPEngineApp

void CGPEngineApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
}

void CGPEngineApp::LoadCustomState()
{
}

void CGPEngineApp::SaveCustomState()
{
}

// Controladores de mensaje de CGPEngineApp



