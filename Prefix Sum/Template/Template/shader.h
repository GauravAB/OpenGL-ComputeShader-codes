#include <cstdlib>
#include <fstream>
#include "glFiles.h"


GLuint setShader(char *shaderType, char* shaderFile, FILE* gpFile);
GLuint setProgram(GLuint vertexshaderID, GLuint fragmentshaderId , FILE *gpFile);
GLuint setComputeProgram(char* shaderFile , FILE* gpFile);
