
// GPEngineView.cpp: implementación de la clase CGPEngineView
//

#include "stdafx.h"
// Se pueden definir SHARED_HANDLERS en un proyecto ATL implementando controladores de vista previa, miniatura
// y filtro de búsqueda, y permiten compartir código de documentos con ese proyecto.
#ifndef SHARED_HANDLERS
#include "GPEngine.h"
#endif

#include "GPEngineDoc.h"
#include "GPEngineView.h"

#include "world.h"

#define atx(X) ox + (X) * ex
#define aty(Y) oy + (Y) * ey

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGPEngineView

extern CGPEngineView *_engine_view;
extern void RagDollBody(int X,int Y);


IMPLEMENT_DYNCREATE(CGPEngineView, CView)

BEGIN_MESSAGE_MAP(CGPEngineView, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_CHAR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

// Construcción o destrucción de CGPEngineView

CGPEngineView::CGPEngineView()
{
	ex = ey = 10;
	ox = oy = 20;
	init = false;
	eventoInterno = EV_NADA;

	_engine_view = this;

	step_by_step = false;

}

CGPEngineView::~CGPEngineView()
{
}

BOOL CGPEngineView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: modificar aquí la clase Window o los estilos cambiando
	//  CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// Dibujo de CGPEngineView

void CGPEngineView::OnDraw(CDC* pDC)
{
	CGPEngineDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	if(!init)
		return;

	// Render LOOP
	BOOL seguir = TRUE;
	float time = 0;
	LARGE_INTEGER F, T0, T1;   // address of current frequency
	QueryPerformanceFrequency(&F);
	QueryPerformanceCounter(&T0);

	// Back buffering
	CRect rc;
	GetClientRect(&rc);
	int width = rc.Width();
	int height = rc.Height();
	HBITMAP hbm = CreateCompatibleBitmap(pDC->m_hDC, width,height);   
	HDC hTmpDC = CreateCompatibleDC(pDC->m_hDC);       
	HBITMAP hOldBm = (HBITMAP)SelectObject(hTmpDC, hbm);
	HBRUSH hbr = CreateSolidBrush(RGB(255,255,255));
	double elapsed_time = 0;

	while (seguir)
	{

		QueryPerformanceCounter(&T1);
		elapsed_time += (double)(T1.QuadPart - T0.QuadPart) / (double)F.QuadPart;
		time += (float)elapsed_time;
		T0 = T1;


		if(!step_by_step)
		{
			if(elapsed_time>=fixed_dt)
			{
				_world.Update(fixed_dt);
				elapsed_time -= fixed_dt;
			}
		}

		FillRect(hTmpDC,&rc,hbr);
		_world.Render(hTmpDC,ox,oy,ex,ey);

		if(cant_pt!=0)
		{
			// Esta dibujando un poligono
			CDC *ptmpDC = CDC::FromHandle(hTmpDC);
			CPen pen,*penOld;
			float dE = 10 / ex;		// 10 pixeles
			// verifico si esta cerrado, es decir si el punto del mouse esta cerca del primero, si hace click, ya puede
			// cerrar el poligono
			Vector2 Q = Vector2((mouse_x-ox)/ex, (mouse_y-oy)/oy);
			bool cerrado = cant_pt >=3 && (pt[0]-Q).Length()<dE ? true : false;

			pen.CreatePen(PS_SOLID,cerrado ? 3 : 1,RGB(128,0,0));
			penOld = ptmpDC->SelectObject(&pen);

			ptmpDC->MoveTo(atx(pt[0].x) , aty(pt[0].y));
			for(int i=1;i<cant_pt;++i)
			{
				ptmpDC->LineTo(atx(pt[i].x) , aty(pt[i].y));
			}
			ptmpDC->LineTo(mouse_x, mouse_y);
			ptmpDC->SelectObject(penOld);
		}

		BitBlt (pDC->m_hDC, 0, 0, width,height, hTmpDC, 0,0, SRCCOPY);	

		MSG Msg;
		ZeroMemory(&Msg, sizeof(Msg));
		if (PeekMessage(&Msg, NULL, 0U, 0U, PM_REMOVE))
		{

			if (Msg.message == WM_QUIT || Msg.message == WM_CLOSE)
			{
				seguir = FALSE;
				exit(0);
			}

			// dejo que windows procese el mensaje
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
	}

	DeleteObject(hbr);
	SelectObject(hTmpDC, hOldBm);   
	DeleteObject(hbm);
	DeleteDC(hTmpDC);

}

void CGPEngineView::OnRButtonUp(UINT  nFlags , CPoint point)
{
	ClientToScreen(&point);
}

void CGPEngineView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// Diagnósticos de CGPEngineView

#ifdef _DEBUG
void CGPEngineView::AssertValid() const
{
	CView::AssertValid();
}

void CGPEngineView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CGPEngineDoc* CGPEngineView::GetDocument() const // La versión de no depuración es en línea
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGPEngineDoc)));
	return (CGPEngineDoc*)m_pDocument;
}
#endif //_DEBUG


// Controladores de mensaje de CGPEngineView


void CGPEngineView::OnMButtonDown(UINT nFlags, CPoint point)
{
	// inicio evento de pan realtime
	eventoInterno = EV_PAN_REALTIME;
	// almaceno la posicion inical del mouse
	mouse_x = point.x; 
	mouse_y = point.y; 

	CView::OnMButtonDown(nFlags, point);
}


void CGPEngineView::OnMButtonUp(UINT nFlags, CPoint point)
{
	// termino event de pan realtime
	eventoInterno = EV_NADA;
	CView::OnMButtonUp(nFlags, point);
}


void CGPEngineView::OnMouseMove(UINT nFlags, CPoint point)
{

	float xPos = point.x; 
	float yPos = point.y; 
	switch(eventoInterno)
	{
		case EV_PAN_REALTIME:
			ox += xPos-mouse_x;
			oy += yPos-mouse_y;
			break;

		case EV_DRAGING_BODY:
			if(drag_body!=-1)
			{
				Vector2 ant_pos = _world.bodies[drag_body]->position;
				_world.bodies[drag_body]->position = Vector2((xPos-ox)/ex, (yPos-oy)/ey);
				_world.bodies[drag_body]->angularVelocity = 0;
				_world.bodies[drag_body]->inverseMass = 0;
				float dt = _world.time - ant_time;
				if(dt)
					_world.bodies[drag_body]->velocity = (_world.bodies[drag_body]->position - ant_pos) / dt;
				ant_time = _world.time;
			}
			break;


	}

	// actualizo la posicion del mouse
	mouse_x = xPos; 
	mouse_y = yPos; 
	

	CView::OnMouseMove(nFlags, point);
}


BOOL CGPEngineView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	int xPos = pt.x; 
	int yPos = pt.y; 
	float x = (xPos-ox)/ex;
	float y = (yPos-oy)/ey;
	// ajusto la escala
	float k = 1 + (float)zDelta/1200.0;
	ex*=k;
	ey*=k;
	// ajusto el origen
	ox += x*ex*(1.0/k-1);
	oy += y*ey*(1.0/k-1);
	
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}


void CGPEngineView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	int xPos = (mouse_x-ox)/ ex; 
	int yPos = (mouse_y-oy)/ ey; 
	switch(nChar)
	{
		case 32:
			if(step_by_step)
			{
				_world.Update(fixed_dt);
			}
			break;
		case 'c':
		case 'C':
			// Agregar circulo
			{
				Body *p = _world.AddCircle(xPos,yPos,Random(1,10));
				p->velocity = Vector2(0, Random(1,15));
			}
			break;

		case 'r':
		case 'R':
			// Agregar Rectangulo
			{
				Body *p = _world.AddBox(xPos,yPos,Random(1,10),Random(1,10));
				p->velocity = Vector2(0, 5);
			}
			break;

		case 'b':
		case 'B':
			// Agregar Ragg Dall Body
			RagDollBody(xPos,yPos);
			break;


		case 't':
		case 'T':
			// Agregar Triangulo
			{
				float r = Random(1,10);
				Body *p = _world.AddTri(-r,r,-r,-r, r,2*r);
				p->position = Vector2(xPos, yPos);
				p->velocity = Vector2(0, 5);
			}
			break;

		case 'p':
		case 'P':
			{
				// creo un piso
				Body *floor = _world.AddBox(0,50,600,1);
				floor->SetStatic();
			}
			break;

		// escenas standard
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			LoadStaticScene(nChar-'0' - 1);
			break;

		case '0':
			LoadStaticScene(9);
			break;


	}

	

	CView::OnChar(nChar, nRepCnt, nFlags);
}


void CGPEngineView::OnLButtonDown(UINT nFlags, CPoint point)
{
	float xPos = (mouse_x-ox)/ ex; 
	float yPos = (mouse_y-oy)/ ey; 

	switch(eventoInterno)
	{
		case EV_NADA:

			// verifico si toca contra algun cuerpo
			if((drag_body = _world.PointNearBodyCM(Vector2(xPos,yPos),20/ex))!=-1)
			{
				// inicio un evento de arrastrar cuerpo
				eventoInterno = EV_DRAGING_BODY;
				ant_time = _world.time;
			}
			/*
			else
			{
				// inicio un evento de dibujar polygono
				eventoInterno = EV_DRAW_POLY;
				// agrego el primer punto a la secuencia
				cant_pt = 0;
				pt[cant_pt++] = Vector2(xPos,yPos);
			}*/
			break;
		case EV_DRAW_POLY:
			// agrego mas puntos a la secuencia
			if(cant_pt<MAX_VERTEX_COUNT-2)
			{
				Vector2 Q = Vector2(xPos,yPos);
				// verifico si el pto Q esta cerca del punto cero
				float dE = 10 / ex;		// 10 pixeles
				if((pt[0]-Q).Length()<dE)
				{
					// agrego el poligono
					Body *b = _world.AddPoly(pt,cant_pt);
					// limpio el poligono
					cant_pt = 0;
					// termino el evento
					eventoInterno = EV_NADA;
				}
				else
					pt[cant_pt++] = Vector2(xPos,yPos);
			}
			break;

	}

	CView::OnLButtonDown(nFlags, point);
}


void CGPEngineView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// termino event de draggin
	if(eventoInterno==EV_DRAGING_BODY)
	{
		_world.bodies[drag_body]->inverseMass = 1 / _world.bodies[drag_body]->mass;
		eventoInterno = EV_NADA;
	}
	CView::OnLButtonUp(nFlags, point);
}


// Carga escena estaticas
void CGPEngineView::LoadStaticScene(int escena)
{
	// inicializo el mundo
	init = false;
	_world.Clear();
	Body *p;
	gravity = Vector2 ( 0, 10.0f * gravityScale );
	switch(escena)
	{
		case 0:

			/*
			// 2 triangulos de  base y uno que cae 
			p = _world.AddTri(0, 5, 0, 0, 40, 5);
			p->position = Vector2(0, 40);
			p->SetStatic();

			p = _world.AddTri(0, 5, 40,0 ,40, 5 );
			p->position = Vector2(50, 40);
			p->SetStatic();

			p = _world.AddTri(10-5,10+5,10-5,10-5, 10+5,10+2*5);
			p->position = Vector2(15.46 , 34.069);
			p->velocity = Vector2(9.06 , 15.48);
			p->SetOrient( -1.248819);
			p->angularVelocity = -2.0813;
			*/


			p = _world.AddBox(48, 62, 100, 4);
			p->SetStatic();

			// base de la torre
			p = _world.AddBox(85, 58, 30, 4);
			p->SetStatic();

			// torre inferior
			p = _world.AddBox(75, 40, 2, 16);

			break;
		case 1:
			// pendulo
			{
				Body *A = _world.AddCircle(40,2,1);
				A->velocity = Vector2(5 , 0);
				A->SetStatic();

				p = _world.AddCircle(30,20,5);
				p->velocity = Vector2(9.06 , 0);
				p->restitution = 1;
				_world.AddConstraint(p,A,25);

				p = _world.AddCircle(50,20,5);
				p->velocity = Vector2(-9.06 , 0);
				p->restitution = 1;
				_world.AddConstraint(p,A,25);

				p = _world.AddCircle(40,20,5);
				p->velocity = Vector2(-9.06 , 0);
				p->restitution = 1;
				_world.AddConstraint(p,A,25);
			}
			break;

		case 2:
			{
				// cadena 
				int id = 0;
				for(int j= 0;j<5;++j)
				{
					Body *A = _world.AddCircle(10+j*15,2,1);
					A->SetStatic();
					A->id = id++;
					for(int i= 0;i<15;++i)
					{
						p = _world.AddCircle(35-i*2+j*15,10+i*2,1);
						p->restitution = 1;
						p->id = id++;
						_world.AddConstraint(p,A,3);
						A = p;
					}
				}
			}
			break;

		case 3:
			{
				// triple pendulo
				Body *A = _world.AddCircle(10,2,1);
				A->SetStatic();
				p = _world.AddCircle(9,3,1);
				p->restitution = 1;
				_world.AddConstraint(p,A,4);

				A = p;
				p = _world.AddCircle(10,9,1);
				p->restitution = 1;
				_world.AddConstraint(p,A,5);

			}
			break;

		case 4:
			{
				// triangulo con 2 circulos atados
				p = _world.AddTri(0, 30, 30, 0, 60, 30);
				p->position = Vector2(50, 50);
				p->SetStatic();

				Body *A = _world.AddCircle(10,10,5);
				A->restitution = 1;
				Body *B = _world.AddCircle(60,10,5);
				B->restitution = 1;
				_world.AddConstraint(A,B,50);
			}
			break;
		case 5:
			{
				// pendulo fisico
				Body *A = _world.AddCircle(40,2,1);
				A->velocity = Vector2(5 , 0);
				A->SetStatic();

				p = _world.AddBox(30,20,6,3);
				p->velocity = Vector2(9.06 , 10);
				p->restitution = 1;
				Constraint *pC = _world.AddConstraint(p,A,25);
				pC->ptA = Vector2(-3,-1.5);
				//pC = _world.AddConstraint(p,A,25);
				//pC->ptA = Vector2(3,-1.5);
			}
			break;

		case 6:
			{
				// cuerda
				Body *p = _world.AddBox(0,10,2,6);
				p->SetStatic();
				int cant_eslabones = 30;
				float dx = 80.0/(float)cant_eslabones;
				int j;
				for(j= 0;j<cant_eslabones;++j)
				{
					Body *A = _world.AddCircle(j*dx,10,1);
					Constraint *C = _world.AddConstraint(p,A,dx/2);
					C->exact_distance = true;
					p = A;
				}
				Body *B = _world.AddBox(j*dx,10,2,6);
				B->SetStatic();
				_world.AddConstraint(p,B,dx*1.1);

			}
			break;

		case 7:
			{
				gravity.y = 10;
				float Y = 10;
				float X = 20;
				for(int s = 0;s<10;++s)
				{
					RagDollBody(X,Y);
					X+=5;
					Y+=3;
				}

				Body *floor = _world.AddBox(50,50,100,1);
				floor->SetStatic();

				Body *wall = _world.AddBox(0,25,1,50);
				wall->SetStatic();

				wall = _world.AddBox(100,25,1,50);
				wall->SetStatic();

			}
			break;

		case 8:
			{
				// particle simulated rigid body
				Body *p0 = _world.AddCircle(10,10,1);
				Body *p1 = _world.AddCircle(20,10,1);
				Body *p2 = _world.AddCircle(20,20,1);
				Body *p3 = _world.AddCircle(10,20,1);
				
				Constraint *lado_0 = _world.AddJointConstraint(p0, p1);
				Constraint *lado_1 = _world.AddJointConstraint(p1, p2);
				Constraint *lado_2 = _world.AddJointConstraint(p2, p3);
				Constraint *lado_3 = _world.AddJointConstraint(p3, p0);
				Constraint *diag_0 = _world.AddJointConstraint(p0, p2);
				Constraint *diag_1 = _world.AddJointConstraint(p1, p3);


				Body *floor = _world.AddBox(50,50,100,1);
				floor->SetStatic();
				Body *wall = _world.AddBox(0,25,1,50);
				wall->SetStatic();
				wall = _world.AddBox(100,25,1,50);
				wall->SetStatic();

			}
			break;

		case 9:
			/*
			{
				// sube y baja
				Body *A = _world.AddCircle(40,40,5);
				A->SetStatic();
				Body *B = _world.AddBox(40,35,60,2);
				_world.AddJointConstraint(A, B);

				RagDollBody(25,10);
				RagDollBody(55,10);
			}
			*/
			{
				Body *p0 = _world.AddCircle(10,10,1.5);
				Body *p1 = _world.AddCircle(20,10,1.5);
				Body *B = _world.AddBox(15,5,20,6);
				_world.AddDistanceConstraint(p0,Vector2(0,0),B,Vector2(-4,0));
				_world.AddDistanceConstraint(p1,Vector2(0,0),B,Vector2(4,0));
				_world.AddDistanceConstraint(p0,Vector2(0,0),p1,Vector2(0,0));

				_world.AddDistanceConstraint(p1,Vector2(0,0),B,Vector2(-4,0));
				_world.AddDistanceConstraint(p0,Vector2(0,0),B,Vector2(4,0));



				Body *floor = _world.AddTri(0, 5, 0, 0, 40, 5);
				floor->position = Vector2(0, 40);
				floor->SetStatic();

				floor = _world.AddTri(0, 5, 40,0 ,40, 5 );
				floor->position = Vector2(50, 40);
				floor->SetStatic();

			}

			break;


	}

	init = true;
}


// helper global : Crea un ragdoll body
void RagDollBody(int X,int Y)
{
	// Ragdoll
	float shouldersDistance = 5,
		upperArmLength = 4,
		lowerArmLength = 4,
		upperArmSize = 2,
		lowerArmSize = 2,
		neckLength = 0.5,
		headRadius = 2.5,
		upperBodyLength = 6,
		pelvisLength = 4,
		upperLegLength = 5,
		upperLegSize = 2,
		lowerLegSize = 2,
		lowerLegLength = 5;

	float dist = 0.5;
	float r = 1;

	// Lower legs
	Body *lowerLeftLeg = _world.AddCircle(X-shouldersDistance/2,Y-lowerLegLength / 2,r);
	Body *lowerRightLeg = _world.AddCircle(X+shouldersDistance/2,Y-lowerLegLength / 2,r);

	// Upper legs
	Body *upperLeftLeg = _world.AddCircle(X-shouldersDistance/2,lowerLeftLeg->position.y-lowerLegLength/2-upperLegLength/2,r);
	Body *upperRightLeg = _world.AddCircle(X+shouldersDistance/2,lowerRightLeg->position.y-lowerLegLength/2-upperLegLength/2,r);

	// Pelvis
	Body *pelvis = _world.AddCircle(X, upperLeftLeg->position.y-upperLegLength/2-pelvisLength/2,r);


	// Upper body
	Body *upperBody = _world.AddCircle(X,pelvis->position.y-pelvisLength/2-upperBodyLength/2,r);

	// Head
	Body *head = _world.AddCircle(X,upperBody->position.y-upperBodyLength/2-headRadius-neckLength,r);

	// Upper arms
	Body *upperLeftArm = _world.AddCircle(X-shouldersDistance/2-upperArmLength/2, upperBody->position.y-upperBodyLength/2,r);
	Body *upperRightArm = _world.AddCircle(X+shouldersDistance/2+upperArmLength/2, upperBody->position.y-upperBodyLength/2,r);

	// lower arms
	Body *lowerLeftArm = _world.AddCircle(upperLeftArm->position.x - lowerArmLength/2 - upperArmLength/2,
		upperLeftArm->position.y,r);
	Body *lowerRightArm = _world.AddCircle(upperRightArm->position.x + lowerArmLength/2 + upperArmLength/2,
		upperRightArm->position.y,r);


	// Neck joint
	Constraint *neckJoint = _world.AddJointConstraint(head, upperBody);

	// Knee joints
	Constraint *leftKneeJoint = _world.AddJointConstraint(lowerLeftLeg, upperLeftLeg);
	Constraint *rightKneeJoint = _world.AddJointConstraint(lowerRightLeg, upperRightLeg);
	// Hip joints
	Constraint *leftHipJoint = _world.AddJointConstraint(upperLeftLeg, pelvis);
	Constraint *rightHipJoint = _world.AddJointConstraint(upperRightLeg, pelvis);
	// Spine
	Constraint *spineJoint = _world.AddJointConstraint(pelvis, upperBody);

	// Shoulders
	Constraint *leftShoulder = _world.AddJointConstraint(upperBody, upperLeftArm);
	Constraint *rightShoulder= _world.AddJointConstraint(upperBody, upperRightArm);

	// Elbow joint
	Constraint *leftElbowJoint = _world.AddJointConstraint(lowerLeftArm, upperLeftArm);
	Constraint *rightElbowJoint= _world.AddJointConstraint(lowerRightArm, upperRightArm);

	// constrains virtuales
	Constraint *c = _world.AddJointConstraint(lowerLeftLeg, head);
	c->visible = 0;
	c = _world.AddJointConstraint(lowerRightLeg, head);
	c->visible = 0;
	c = _world.AddJointConstraint(upperLeftArm, head);
	c->visible = 0;
	c = _world.AddJointConstraint(upperRightArm, head);
	c->visible = 0;
}