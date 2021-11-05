#include "shader.h"

char* readTextFile(char* fileName);

GLuint setShader(char *shaderType, char* shaderFile, FILE* gpFile)
{
	void uninitialize(void);

	int shaderID;
	char* shaderSourceFile = readTextFile(shaderFile);

	if (shaderType == "VERTEX_SHADER")
	{
		shaderID = glCreateShader(GL_VERTEX_SHADER);

	}
	else if (shaderType == "FRAGMENT_SHADER")
	{
		shaderID = glCreateShader(GL_FRAGMENT_SHADER);

	}
	else if (shaderType == "GEOMETRY_SHADER")
	{
		shaderID = glCreateShader(GL_GEOMETRY_SHADER);

	}
	else if (shaderType == "TESS_CONTROL_SHADER")
	{
		shaderID = glCreateShader(GL_TESS_CONTROL_SHADER);

	}
	else if (shaderType == "TESS_EVALULATION_SHADER")
	{	
		shaderID = glCreateShader(GL_TESS_EVALUATION_SHADER);
	}

	glShaderSource(shaderID,1,(const char**)&shaderSourceFile,NULL);
	glCompileShader(shaderID);

	GLint iCompileStatus;
	GLint iInfoLogLength;
	char *szInfoLog = NULL;

	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &iCompileStatus);
	if (iCompileStatus == GL_FALSE)
	{
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char*)malloc(iInfoLogLength);
			GLsizei written;
			glGetShaderInfoLog(shaderID, iInfoLogLength, &written, szInfoLog);
			fprintf(gpFile, "%s compile error Log : %s", shaderType,szInfoLog);
			free(szInfoLog);
			uninitialize();
			exit(0);
		}
	}

	return shaderID;
}


char* readTextFile(char* fileName)
{
	FILE *fp = fopen(fileName, "rb");
	char* content = NULL;
	long numVal;

	//go till end
	fseek(fp, 0L, SEEK_END);
	
	//get count bytes 
	numVal = ftell(fp);

	//reset
	fseek(fp, 0L, SEEK_SET);

	content = (char*)malloc(sizeof(char)*(numVal + 1));

	//read into buffer
	fread(content, 1, numVal, fp);
	content[numVal] = '\0';

	fclose(fp);
	return content;
}

GLuint setProgram(GLuint vertexShaderID, GLuint fragmentShaderID , FILE* gpFile)
{
	void uninitialize(void);

	GLint iCompileStatus;
	GLint iInfoLogLength;
	char *szInfoLog = NULL;

	GLuint program = glCreateProgram();

	glAttachShader(program, vertexShaderID);
	glAttachShader(program, fragmentShaderID);

	glLinkProgram(program);
	GLint iShaderLinkStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &iShaderLinkStatus);

	if (iShaderLinkStatus == GL_FALSE)
	{
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			GLsizei written;
			szInfoLog = (char*)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				glGetProgramInfoLog(program, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "program link error log : %s", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}


	return program;
}



GLuint setComputeProgram(char* shaderFile , FILE* gpFile)
{
	GLuint program = glCreateProgram();
	void uninitialize(void);

	char* shaderSourceFile = readTextFile(shaderFile);

	GLuint shaderID = glCreateShader(GL_COMPUTE_SHADER);

	glShaderSource(shaderID, 1, (const char**)&shaderSourceFile, NULL);
	glCompileShader(shaderID);

	GLint iCompileStatus;
	GLint iInfoLogLength;
	char *szInfoLog = NULL;
	

	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &iCompileStatus);
	if (iCompileStatus == GL_FALSE)
	{
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char*)malloc(iInfoLogLength);
			GLsizei written;
			glGetShaderInfoLog(shaderID, iInfoLogLength, &written, szInfoLog);
			fprintf(gpFile, "COMPUTE SHADER: compile error Log : %s", szInfoLog);
			free(szInfoLog);
			uninitialize();
			exit(0);
		}
	}


	glAttachShader(program,shaderID);
	
	glLinkProgram(program);
	GLint iShaderLinkStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &iShaderLinkStatus);

	if (iShaderLinkStatus == GL_FALSE)
	{
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			GLsizei written;
			szInfoLog = (char*)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				glGetProgramInfoLog(program, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "program link error log : %s", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}

	return program;
}










