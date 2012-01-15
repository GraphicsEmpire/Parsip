#ifndef _GLOBALSETTINGS_H
#define _GLOBALSETTINGS_H

//Application Settings and important constant values

#define DEFAULT_EDGE_NORMALS_ANGLE 5.0f
#define DEFAULT_CELL_SIZE 0.25f
#define DEFAULT_BOUNDS 500
#define DEFAULT_NORMAL_LEN 5
#define CELL_SIZE_SCALE		100

#define DEFAULT_MOUSE_DRAG_SCALE 10
#define MOUSE_WHEEL_COEFF   0.01f
#define MOUSE_MOVE_COEFF    0.03f
#define MAX_UNDO_LEVEL	   10
#define MAX_NAME_LEN	   32
#define MAX_DATETIME_LEN   26
#define SCENE_FILE_VERSION 4

const std::string appname = "PARSIP";
const std::string companyname = "POURYASHIRAZIAN";

//Converge Iterations
const int RES =	30; 

//Polygonizer HashBIT
const int HASHBIT = 5;



#endif
