
#include <string>
#include "switcher.h"

#ifdef OBJ_PARSING
#include "obj_parsing.h"
#endif

#ifdef LAMBERT
#include "lighting.h"
#endif

#ifdef PHONG
#include "lighting.h"
#endif

#ifdef BLINN
#include"lighting.h"
#endif

#ifdef TOONSHADING
#include "lighting.h"
#endif

#ifdef OREN
#include "lighting.h"
#endif

#ifdef MINNAERT
#include "lighting.h"
#endif





int main() {
	int figuretype = 6;

	switch (figuretype) {
	case 1: task_main(".\\shapes\\cube.obj"); return 0;
	case 2: task_main(".\\shapes\\tetrahedron.obj"); return 0;
	case 3: task_main(".\\shapes\\dodecahedron.obj"); return 0;
	case 4: task_main(".\\shapes\\octahedron.obj"); return 0;
	case 5: task_main(".\\shapes\\sphere.obj"); return 0;
	case 6: task_main(".\\shapes\\torus.obj"); return 0;
	case 7: task_main(".\\shapes\\hexahedron.obj"); return 0;
	case 8: task_main(".\\shapes\\untitled.obj"); return 0;
	}
	
	
	return 1;
}