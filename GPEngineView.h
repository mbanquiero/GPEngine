
// GPEngineView.h: interfaz de la clase CGPEngineView
//

#pragma once

#include "gpe_math.h"

#define EV_NADA				0
#define EV_PAN_REALTIME		1
#define EV_DRAW_POLY		2
#define EV_DRAGING_BODY		3

class CGPEngineView : public CView
{
protected: // Crear sólo a partir de serialización
	CGPEngineView();
	DECLARE_DYNCREATE(CGPEngineView)

// Atributos
public:
	CGPEngineDoc* GetDocument() const;

	int ox,oy;
	float ex,ey;
	bool init;

	int eventoInterno;
	int mouse_x , mouse_y;

	int cant_pt;
	Vector2 pt[32];

	int drag_body;
	float ant_time;

	bool step_by_step;


// Operaciones
public:

// Reemplazos
public:
	virtual void OnDraw(CDC* pDC);  // Reemplazado para dibujar esta vista
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	void LoadStaticScene(int escena);

protected:

// Implementación
public:
	virtual ~CGPEngineView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Funciones de asignación de mensajes generadas
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};

#ifndef _DEBUG  // Versión de depuración en GPEngineView.cpp
inline CGPEngineDoc* CGPEngineView::GetDocument() const
   { return reinterpret_cast<CGPEngineDoc*>(m_pDocument); }
#endif

