
//	HELPERS para Resolucion de colisiones
#pragma once

#include "stdafx.h"
#include "gpe_math.h"
#include "math.h"
#include "body.h"
#include "contact.h"

BOOL CircleCircle( Body *a, Body *b ,ContactSolver *contact_info);
BOOL CirclePolygon( Body *a, Body *b ,ContactSolver *contact_info);
BOOL PolygonPolygon( Body *a, Body *b ,ContactSolver *contact_info);


