#include <string>
#include "obj_parsing.h"


int main() {
	switch (1) {
	case 1: obj_parsing_main(".\\shapes\\cube.obj"); return 0;
	case 2: obj_parsing_main(".\\shapes\\tetrahedron.obj"); return 0;
	case 3: obj_parsing_main(".\\shapes\\dodecahedron.obj"); return 0;
	case 4: obj_parsing_main(".\\shapes\\octahedron.obj"); return 0;
	case 5: obj_parsing_main(".\\shapes\\sphere.obj"); return 0;
	case 6: obj_parsing_main(".\\shapes\\torus.obj"); return 0;
	}
	
	return 1;
}