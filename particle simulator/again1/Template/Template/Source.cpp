#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "vmath.h"
#include "glFiles.h"
#include "shader.h"
#include "resource.h"


#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"glu32.lib")

using namespace vmath;

//global vars
HWND ghwnd;
HDC ghdc;
HGLRC ghrc;
FILE *gpFile = NULL;

DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

bool gbIsActiveWindow = false;
bool gbIsEscapeKeyPressed = false;
bool gbFullScreen = false;
bool gbLight= false;
bool gbAnimate = false;

GLfloat gCubeAngle = 0.0f;
enum
{
	GAB_ATTRIBUTE_VERTEX = 0,
	GAB_ATTRIBUTE_COLOR,
	GAB_ATTRIBUTE_NORMAL ,
	GAB_ATTRIBUTE_TEXTURE0,
};

enum
{
	PARTICLE_GROUP_SIZE = 128,
	PARTICLE_GROUP_COUNT = 2048,
	PARTICLE_COUNT = (PARTICLE_GROUP_SIZE * PARTICLE_GROUP_COUNT),
	MAX_ATTRACTORS = 64
};

mat4 gPerspectiveProjectionMatrix;
GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;
GLuint gVao_cube;
GLuint gVbo_position;
GLuint gVbo_normals;
GLuint gModelMatrixUniform;	//only model view matrix
GLuint gProjectionMatrixUniform;//only projection matrix
GLuint gViewMatrixUniform;
GLuint gLdUniform, gKdUniform, gLightPositionUniform;	//for light diffuse  constant , light constant , light position
GLuint gLKeyPressedUniform;//L key notification to enable lights

GLuint gComputeShaderProgramObject;
GLuint gComputeShaderObject;
GLuint imageInput;
GLuint imageOutput;

float timer;

//position and velocity buffers
static union
{
	struct
	{
		GLuint position_buffer;
		GLuint velocity_buffer;
	};
	GLuint buffers[2];
};

//tbos

static union
{
	struct
	{
		GLuint position_tbo;
		GLuint velocity_tbo;
	};

	GLuint tbos[2];
};

//Attractor UBO
GLuint attractor_buffer;

//Program , vao and vbo to render a full screen quad
GLuint render_prog;
GLuint render_vao;
GLuint render_vbo;

GLuint dt_location;


float attractor_masses[MAX_ATTRACTORS];

static inline float random_float()
{
	float res;
	unsigned int tmp;
	static unsigned int seed = 0xFFFF0C59;

	seed *= 16807;
	tmp = seed ^ (seed >> 4) ^ (seed << 15);
	*((unsigned int*)&res) = (tmp >> 9) | 0x3F800000;

	return (res - 1.0f);
}

static vec3 random_vector(float minmag = 0.0f, float maxmag = 1.0f)
{
	vec3 randomvec(random_float()* 2.0f - 1.0, random_float() * 2.0f - 1.0f, random_float() * 2.0f - 1.0f);
	randomvec = normalize(randomvec);
	randomvec *= (random_float() * (maxmag - minmag) + minmag);

	return randomvec;
}

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR nCmdLine, int nCmdShow)
{
	void initialize(void);
	void uninitialize(void);
	void resize(int, int);
	void display(void);
	void update(void);

	bool bDone = false;
	HWND hwnd;
	WNDCLASSEX wndclass;
	MSG msg;
	TCHAR szAppName[] = TEXT("Perspective Triangle");

	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClassEx(&wndclass);

	hwnd = CreateWindowEx(WS_EX_APPWINDOW, szAppName, TEXT("OGL window"), WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE, 0, 0, WIN_WIDTH, WIN_HEIGHT, NULL, NULL, hInstance, NULL);

	if (hwnd == NULL)
	{
		MessageBox(NULL, TEXT("Failed to create window"), TEXT("error"), MB_OK);
		return 1;
	}

	ghwnd = hwnd;
	initialize();

	ShowWindow(hwnd, SW_SHOW);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);


	while (bDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bDone = true;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbIsActiveWindow == true)
			{
				if (gbIsEscapeKeyPressed == true)
				{
					bDone = true;
				}
			}

			if (gbAnimate == true)
			{
				update();
			}
			display();
		}

	}

	uninitialize();
	return(msg.wParam);

}

void initialize(void)
{
	void uninitialize(void);
	int LoadGLTextures(GLuint *texture, TCHAR imageResourceId[]);
	void resize(int, int);

	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormat;
	int err = fopen_s(&gpFile, "log.txt", "w");
	if (err != 0)
	{
		MessageBox(NULL, TEXT("file open failed"), TEXT("error"), MB_OK);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "File Created Successfully");
	}


	//set required pfd
	pfd.nVersion = 0;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cBlueBits = 8;
	pfd.cGreenBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 64;

	ghdc = GetDC(ghwnd);

	iPixelFormat = ChoosePixelFormat(ghdc, &pfd);

	if (iPixelFormat == 0)
	{
		ReleaseDC(ghwnd, ghdc);
		MessageBox(NULL, TEXT("iPixelFormat is 0"), TEXT("Error"), MB_OK);
		exit(1);
	}

	if (SetPixelFormat(ghdc, iPixelFormat, &pfd) == NULL)
	{
		ReleaseDC(ghwnd, ghdc);
		MessageBox(NULL, TEXT("SetPixelFormat failed"), TEXT("Error"), MB_OK);
		exit(1);
	}

	ghrc = wglCreateContext(ghdc);

	if (ghrc == NULL)
	{
		wglDeleteContext(ghrc);
		ReleaseDC(ghwnd, ghdc);
		MessageBox(NULL, TEXT("Failed to Create Context"), TEXT("Error"), MB_OK);
		exit(1);
	}

	if ((wglMakeCurrent(ghdc, ghrc)) == FALSE)
	{
		ReleaseDC(ghwnd, ghdc);
		wglDeleteContext(ghrc);
		exit(1);
	}

	GLenum err_no = glewInit();
	if (err_no != GLEW_OK)
	{
		fprintf(gpFile, "Error: %s\n", glewGetErrorString(err));
		fclose(gpFile);
		exit(0);
	}
	const GLubyte *version, *glslVersion, *vendor;

	version = glGetString(GL_VERSION);
	vendor = glGetString(GL_VENDOR);
	glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

	fprintf(gpFile, "OpenGL vendor: %s\n", vendor);
	fprintf(gpFile, "OpenGL version: %s\n", version);
	fprintf(gpFile, "GLSLVersion: %s\n", glslVersion);

	//Create vertex shader
	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
	gVertexShaderObject = setShader("VERTEX_SHADER", "vertexShader.glsl", gpFile);

	//Create fragment shader
	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	gFragmentShaderObject = setShader("FRAGMENT_SHADER","fragmentShader.glsl",gpFile);

	gShaderProgramObject = setProgram(gVertexShaderObject, gFragmentShaderObject , gpFile);

	gModelMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
	gProjectionMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");
	gViewMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_view_matrix");

	dt_location = glGetUniformLocation(gShaderProgramObject, "dt");

	//create compute shader
	gComputeShaderObject = glCreateShader(GL_COMPUTE_SHADER);
	gComputeShaderProgramObject = setComputeProgram("computeShader.glsl" , gpFile);

	GLint arrSize[3];
	GLint x, y, z;

	glGetProgramiv(gComputeShaderProgramObject, GL_MAX_COMPUTE_WORK_GROUP_SIZE, arrSize);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &x);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &y);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &z);

	
	fprintf(gpFile, "\nCOMPUTE SHADER: \nLocal work group max size\n");
	fprintf(gpFile, "Size x: %d , Size y: %d , Size z: %d \n", x, y, z);
	
	glGenVertexArrays(1, &render_vao);
	glBindVertexArray(render_vao);

	glGenBuffers(2, buffers);
	glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
	glBufferData(GL_ARRAY_BUFFER, PARTICLE_COUNT * sizeof(vec4), NULL, GL_DYNAMIC_COPY);

	vec4 *positions = (vec4*)glMapBufferRange(GL_ARRAY_BUFFER, 0, PARTICLE_COUNT * sizeof(vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	for (int i = 0; i < PARTICLE_COUNT; i++)
	{
		positions[i] = vec4(random_vector(-10.0f, 10.0f), random_float());
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);
	glGenTextures(2, tbos);

	for (int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_BUFFER, tbos[i]);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, buffers[i]);
	}
	glGenBuffers(1, &attractor_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, attractor_buffer);
	glBufferData(GL_UNIFORM_BUFFER, 32 * sizeof(vec4), NULL, GL_STATIC_DRAW);

	for (int i = 0; i < MAX_ATTRACTORS; i++)
	{
		attractor_masses[i] = 0.5f + random_float() * 0.5f;
	}

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, attractor_buffer);


	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glShadeModel(GL_SMOOTH);
	
	gPerspectiveProjectionMatrix = mat4::identity();

	resize(WIN_WIDTH, WIN_HEIGHT);
}


int LoadGLTextures(GLuint *texture, TCHAR imageResourceId[])
{
	HBITMAP hBitmap;
	BITMAP bmp;
	int iStatus = -1;

	hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), imageResourceId, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

	if (hBitmap)
	{
		iStatus = 0;

		GetObject(hBitmap, sizeof(bmp),&bmp);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glGenTextures(1, texture);
		glBindTexture(GL_TEXTURE_2D, *texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		 
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB , 512, 512, 0, GL_BGR, GL_UNSIGNED_BYTE, bmp.bmBits);
	}

	return iStatus;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	void resize(int, int);
	void ToggleFullScreen(void);

	static bool bLKeyIsPressed = false;
	static bool bAKeyIsPressed = false;


	switch (iMsg)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 0x41:
			if (bAKeyIsPressed == false)
			{
				gbAnimate = true;
				bAKeyIsPressed = true;
			}
			else
			{
				gbAnimate = false;
				bAKeyIsPressed = false;
			}
			break;
		case 0x4C:
			if (bLKeyIsPressed == false)
			{
				gbLight = true;
				bLKeyIsPressed = true;
			}
			else
			{
				gbLight = false;
				bLKeyIsPressed = false;
			}
			break;
		case 0x46:
			if (gbFullScreen == false)
			{
				ToggleFullScreen();
				gbFullScreen = true;
			}
			else
			{
				ToggleFullScreen();
				gbFullScreen = false;
			}
			break;
		case VK_ESCAPE:
			gbIsEscapeKeyPressed = true;
			break;
		default:
			break;
		}
		break;
	case WM_ACTIVATE:
		if (HIWORD(wParam) == 0)
		{
			gbIsActiveWindow = true;
		}
		else
		{
			gbIsActiveWindow = false;
		}
		break;
	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void display(void)

{
	
	static const GLuint start_ticks = timer - 100000;
	GLuint current_ticks = timer;
	static GLuint last_ticks = current_ticks;

	float time = ((start_ticks - current_ticks) & 0xFFFFF) / float(0xFFFFF);
	float delta_time = (float)(current_ticks - last_ticks) * 0.075f;
	
	if (delta_time < 0.01f)
	{
		return;
	}

	vec4 *attractors = (vec4*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, 32 * sizeof(vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	int i;

	for (i = 0; i < 32; i++)
	{
		attractors[i] = vec4(
			sinf(time*(float)(i + 4)*7.5f * 20.0f) * 50.0f,
			cosf(time*(float)(i + 7)*3.9f * 20.0f) * 50.0f,
			sinf(time*(float)(i + 3)*5.3f*20.0f)*cos(time*(float)(i + 5)*9.1f)*100.0f,
			attractor_masses[i]);
	}
	glUnmapBuffer(GL_UNIFORM_BUFFER);

	if (delta_time >= 2.0f)
	{
		delta_time = 2.0f;
	}

	
	glUseProgram(gComputeShaderProgramObject);
	
	glBindImageTexture(0, velocity_tbo, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(0, position_tbo, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glUniform1f(dt_location, delta_time);
	glDispatchCompute(PARTICLE_GROUP_COUNT, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glUseProgram(0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(gShaderProgramObject);

	mat4 modelMatrix = mat4::identity();
	mat4 viewMatrix = mat4::identity();
	mat4 rotationMatrix = mat4::identity();

	//translate back
	modelMatrix = translate(0.0f, 0.0f, -3.0f);
	modelMatrix *= rotate(time*1000.0f, vec3(0.0f, 1.0f, 0.0f));

	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);

	glBindVertexArray(render_vao);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glDrawArrays(GL_POINTS, 0, PARTICLE_COUNT);

	glBindVertexArray(0);

	glUseProgram(0);

	//last_ticks = current_ticks;

	SwapBuffers(ghdc);
}

void update(void)
{
	timer += 1.0;
}

void resize(int width, int height)
{
	if (height == 0)
	{
		height = 1;
	}

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	
	gPerspectiveProjectionMatrix = perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);


}


void ToggleFullScreen(void)
{
	MONITORINFO mi;

	if (gbFullScreen == false)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
		ShowCursor(FALSE);
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
	}
}

void uninitialize(void)
{



	if (gbFullScreen)
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		ShowCursor(TRUE);
	}

	if (render_vao)
	{
		glDeleteVertexArrays(1, &render_vao);
	}
	if (render_vbo)
	{
		glDeleteBuffers(1, &render_vbo);
	}
	
	glDetachShader(gShaderProgramObject, gVertexShaderObject);

	glDetachShader(gShaderProgramObject, gFragmentShaderObject);

	glDeleteShader(gVertexShaderObject);
	gVertexShaderObject = 0;
	glDeleteShader(gFragmentShaderObject);
	gFragmentShaderObject = 0;

	glDeleteShader(gComputeShaderObject);
	glDetachShader(gShaderProgramObject, gComputeShaderObject);
	gComputeShaderObject = 0;

	glDeleteProgram(gShaderProgramObject);
	gShaderProgramObject = 0;

	

	glUseProgram(0);



	if (ghrc)
	{
		wglMakeCurrent(NULL, NULL);
	}
	wglDeleteContext(ghrc);
	ghrc = NULL;

	if (ghdc)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (gpFile)
	{
		fprintf(gpFile, "File Closed Successfully");
		fclose(gpFile);
	}


}