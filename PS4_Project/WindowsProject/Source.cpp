#include <Windows.h>
#include "../StaticLibrary/Common.hpp"
#include "GL\glew.h"
#include "GL\wglew.h"
#include <cassert>
#include "stdio.h" 
#define GLEW_STATIC
#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION

#pragma comment (lib, "Winmm.lib")

void RenderWindows(GameAAA::RenderCommands renderCommands);
/*
int __stdcall WinMain(__in HINSTANCE, __in_opt HINSTANCE, __in_opt LPSTR, __in int)
{
	return 0;
}*/

void drawFilledCircle(GLfloat x, GLfloat y, GLfloat radius) {
	int i;
	int triangleAmount = 20; //# of triangles used to draw circle

							 //GLfloat radius = 0.8f; //radius
	GLfloat twicePi = 2.0f * 3.1415;

	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x, y); // center of circle
	for (i = 0; i <= triangleAmount; i++) {
		glVertex2f(
			x + (radius * cos(i *  twicePi / triangleAmount)),
			y + (radius * sin(i * twicePi / triangleAmount))
		);
	}
	glEnd();

	glColor3f(0.0f, 0.0f, 0.0f);
}

void RenderWindows(GameAAA::RenderCommands renderCommands)
{
	//draw balls
	for (int i = 0; i < renderCommands.elements.size(); i++)
	{
		if (renderCommands.elements[i].whatKind == 0)
		{
			float centerX = renderCommands.elements[i].x;
			float centerY = renderCommands.elements[i].y;
			float radius = renderCommands.elements[i].radi;

			int i;
			int triangleAmount = 20; //# of triangles used to draw circle

									 //GLfloat radius = 0.8f; //radius
			GLfloat twicePi = 2.0f * 3.1415f;

			glBegin(GL_TRIANGLE_FAN);
			glVertex2f(centerX, centerY); // center of circle
			for (i = 0; i <= triangleAmount; i++) {
				glVertex2f(
					centerX + (radius * cos(i *  twicePi / triangleAmount)),
					centerY + (radius * sin(i * twicePi / triangleAmount))
				);
			}
			glEnd();
		}
		else if (renderCommands.elements[i].whatKind == 1) //RECT 
		{
			glRectf(renderCommands.elements[i].x, renderCommands.elements[i].y, renderCommands.elements[i].x2, renderCommands.elements[i].y2);
		}
	}
}

static HGLRC s_OpenGLRenderingContext = nullptr;
static HDC s_WindowHandleToDeviceContext;

static bool windowActive = true;
static size_t screenWidth = 640, screenHeight = 480;

LARGE_INTEGER l_LastFrameTime;
int64_t l_PerfCountFrequency;

int64_t l_TicksPerFrame;
LARGE_INTEGER PerfCountFrequencyResult;

//shader declaration
bool dataInitialized = false;
GLint passThroughId;

struct VAO
{
	GLuint vao;
	GLuint elementArrayBufferObject, vertexBufferObject;
	int numIndices;
};


struct RendererData
{
	VAO quadVAO;
};

RendererData rendererData;

//input data
bool keyboard[256] = {};

inline void GLAPIENTRY openGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	OutputDebugStringA("openGL Debug Callback : [");
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
		OutputDebugStringA("ERROR");
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		OutputDebugStringA("DEPRECATED_BEHAVIOR");
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		OutputDebugStringA("UNDEFINED_BEHAVIOR");
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		OutputDebugStringA("PORTABILITY");
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		OutputDebugStringA("PERFORMANCE");
		break;
	case GL_DEBUG_TYPE_OTHER:
		OutputDebugStringA("OTHER");
		break;
	default:
		OutputDebugStringA("????");
		break;
	}
	OutputDebugStringA(":");
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_LOW:
		OutputDebugStringA("LOW");
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		OutputDebugStringA("MEDIUM");
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		OutputDebugStringA("HIGH");
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		OutputDebugStringA("NOTIFICATION");
		break;
	default:
		OutputDebugStringA("????");
		break;
	}
	OutputDebugStringA(":");
	char buffer[512];
	sprintf_s(buffer, "%d", id);
	OutputDebugStringA(buffer);
	OutputDebugStringA("] ");
	OutputDebugStringA(message);
	OutputDebugStringA("\n");
}

bool CompileShader(GLint &shaderId_, const char* shaderCode, GLenum shaderType)
{
	shaderId_ = glCreateShader(shaderType);
	glShaderSource(shaderId_, 1, (const GLchar *const*)&shaderCode, nullptr);
	glCompileShader(shaderId_);

	GLint success = 0;
	glGetShaderiv(shaderId_, GL_COMPILE_STATUS, &success);

	{
		GLint maxLength = 0;
		glGetShaderiv(shaderId_, GL_INFO_LOG_LENGTH, &maxLength);

		if (maxLength > 0)
		{
			//The maxLength includes the NULL character
			GLchar *infoLog = new GLchar[maxLength];
			glGetShaderInfoLog(shaderId_, maxLength, &maxLength, &infoLog[0]);

			OutputDebugStringA(infoLog);
			OutputDebugStringA("\n");

			delete[] infoLog;
		}
	}
	return (success != GL_FALSE);
}
bool LinkShaders(GLint &programId_, GLint &vs, GLint &ps)
{
	programId_ = glCreateProgram();

	glAttachShader(programId_, vs);

	glAttachShader(programId_, ps);

	glLinkProgram(programId_);

	GLint success = 0;
	glGetProgramiv(programId_, GL_LINK_STATUS, &success);
	{
		GLint maxLength = 0;
		glGetProgramiv(programId_, GL_INFO_LOG_LENGTH, &maxLength);

		if (maxLength > 0)
		{
			//The maxLength includes the NULL character
			GLchar *infoLog = new GLchar[maxLength];
			glGetProgramInfoLog(programId_, maxLength, &maxLength, &infoLog[0]);

			OutputDebugStringA(infoLog);
			OutputDebugStringA("\n");

			delete[] infoLog;
		}
	}
	return (success != GL_FALSE);
}

static const char* PassThrouVS = "#version 420\n layout(location = 0) in  vec3 in_Position; layout(location = 1) in  vec4 in_Color; out vec4 ex_Color; void main(void) { gl_Position = vec4(in_Position, 1.0); ex_Color = in_Color; }";
static const char* PassThrouPS = "#version 420\n in  vec4 ex_Color; layout(location = 0) out vec4 out_Color; void main(void) { out_Color = ex_Color; }";

static const char* NewVertexShader = "#version 420\n layout (std140, binding = 0) uniform InstanceData{mat4 modelMatrix; vec4 colorModifier;}; layout(location = 0) in vec3 in_Position; layout(location = 1) in vec4 in_Color; layout(location = 2) in vec2 in_TexCoord; out vec2 ex_TexCoord; out vec4 ex_Color; void main(void){gl_Position = modelMatrix * vec4(in_Position, 1.0); ex_Color = in_Color; ex_TexCoord = vec2(in_TexCoord.s, 1.0 - in_TexCoord.t);}";
static const char* NewFragmentShader = "#version 420\n layout(binding = 0) uniform sampler albedoTexture; in vec4 ex_Color; in vec2 ex_TexCoord; layout(location = 0) out vec4 out_Color; layout (std140, binding = 0) uniform InstanceData{ mat4 modelMatrix; vec4 colorModifier;} void main (void) {vec4 texColor = texture(albedoTexture, ex_TexCoord); out_Color = ex_Color * texColor * colorModifier;}";

GLint passThroughShader;

//CARREGAR IMATGE
int x, y, comp;
unsigned char *data;
GLuint pikaTexture = 100;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		s_WindowHandleToDeviceContext = GetDC(hWnd);

		PIXELFORMATDESCRIPTOR pfd =
		{
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
			PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
			32,                        //Colordepth of the framebuffer.
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			24,                        //Number of bits for the depthbuffer
			8,                        //Number of bits for the stencilbuffer
			0,                        //Number of Aux buffers in the framebuffer.
			PFD_MAIN_PLANE,
			0,
			0, 0, 0
		};


		int letWindowsChooseThisPixelFormat;
		letWindowsChooseThisPixelFormat = ChoosePixelFormat(s_WindowHandleToDeviceContext, &pfd);
		SetPixelFormat(s_WindowHandleToDeviceContext, letWindowsChooseThisPixelFormat, &pfd);

		HGLRC tmpContext = wglCreateContext(s_WindowHandleToDeviceContext);
		wglMakeCurrent(s_WindowHandleToDeviceContext, tmpContext);

		// init glew
		GLenum err = glewInit();
		if (err != GLEW_OK)
		{
			assert(false); // TODO
		}

		int attribs[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
			WGL_CONTEXT_MINOR_VERSION_ARB, 1,
			WGL_CONTEXT_FLAGS_ARB,
			0 //WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB
#if _DEBUG
			| WGL_CONTEXT_DEBUG_BIT_ARB
#endif
			, 0
		};

		s_OpenGLRenderingContext = wglCreateContextAttribsARB(s_WindowHandleToDeviceContext, 0, attribs);

		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(tmpContext);

		wglMakeCurrent(s_WindowHandleToDeviceContext, s_OpenGLRenderingContext);

		//POR SI TENEMOS ERROR Y NO SABEMOS QUÉ PASA
		if (GLEW_ARB_debug_output)
		{
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallbackARB(openGLDebugCallback, nullptr);
			GLuint unusedIds = 0;
			glDebugMessageControl(GL_DONT_CARE,
				GL_DONT_CARE,
				GL_DONT_CARE,
				0,
				&unusedIds,
				true);
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

#ifndef _DEBUG
		ToggleFullscreen(hWnd);
#endif

		//Inicializar Shaders
		if (!dataInitialized)
		{
			GLint vs = 0, ps = 0;
			if (CompileShader(vs, PassThrouVS, GL_VERTEX_SHADER) && CompileShader(ps, PassThrouPS, GL_FRAGMENT_SHADER) && LinkShaders(passThroughId, vs, ps))
			{
				dataInitialized = true;
			}

			if (vs > 0)
				glDeleteShader(vs);
			if (ps > 0)
				glDeleteShader(ps);
		}
		/*glBegin(GL_TRIANGLES);
		{
		glVertexAttrib3f(1, 1, 0, 0);
		glVertexAttrib3f(0, -1, -1, 0);
		glVertexAttrib3f(1, 0, 1, 0);
		glVertexAttrib3f(0, +1, -1, 0);
		glVertexAttrib3f(1, 0, 0, 1);
		glVertexAttrib3f(0, 0, 1, 0);
		}
		glEnd();
		*/

		uint16_t idxs[6] = {
			0,1,3,
			0,3,2
		};

		//cosas de texura
		//CARREGAR IMATGE

		glGenTextures(1, &pikaTexture);
		glBindTexture(GL_TEXTURE_2D, pikaTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, pikaTexture);

		//pinto triangulos
		glDrawElements(GL_TRIANGLES, rendererData.quadVAO.numIndices, GL_UNSIGNED_SHORT, 0);

		//time init
		QueryPerformanceFrequency(&PerfCountFrequencyResult);
		l_PerfCountFrequency = PerfCountFrequencyResult.QuadPart;

		l_TicksPerFrame = l_PerfCountFrequency / 30;

		QueryPerformanceCounter(&l_LastFrameTime);
	}
	break;
	case WM_DESTROY:
		wglDeleteContext(s_OpenGLRenderingContext);
		s_OpenGLRenderingContext = nullptr;
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			return 0;
		}
		break;
	}
	case WM_SIZE:
	{
		screenWidth = LOWORD(lParam);  // Macro to get the low-order word.
		screenHeight = HIWORD(lParam); // Macro to get the high-order word.
		break;
	}
	case WM_ACTIVATE:
	{
		windowActive = wParam != WA_INACTIVE;
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int __stdcall WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPSTR lpCmdLine, __in int nShowCmd)
{
	GameAAA::Input input = {};

	GameAAA::GameData *gameData = GameAAA::CreateGameData();
	GameAAA::InitializeBalls_Game(gameData);

	// load window stuff
	MSG msg = { 0 };
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszClassName = L"Hello World";
	wc.style = CS_OWNDC;
	if (!RegisterClass(&wc))
		return 1;
	HWND hWnd = CreateWindowW(wc.lpszClassName, L"Hello World", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 640, 480, 0, 0, hInstance, 0);

	bool quit = false;
	do
	{
		MSG msg = {};
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			bool processed = false;

			if (!processed)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			if (msg.message == WM_KEYDOWN)
			{
				keyboard[msg.wParam & 255] = true;
				processed = true;
				if (msg.wParam == VK_ESCAPE)
				{
					//PostQuitMessage(0);
					quit = true;
				}
			}
			else if (msg.message == WM_KEYUP)
			{
				keyboard[msg.wParam & 255] = false;
				processed = true;
			}
		}

		input.aim = 0;
		input.force = 0;
		input.shoot = 0;

		if (keyboard['A'])
		{
			input.aim = 1;
		}
		if (keyboard['D'])
		{
			input.aim = -1;
		}
		if (keyboard['S'])
		{
			input.force = -1;
		}
		if (keyboard['W'])
		{
			input.force = 1;
		}
		if (keyboard['X'])
		{
			input.shoot = 1;
		}

		// time stuff
		LARGE_INTEGER l_CurrentTime;
		QueryPerformanceCounter(&l_CurrentTime);

		if (l_LastFrameTime.QuadPart + l_TicksPerFrame > l_CurrentTime.QuadPart)
		{
			int64_t ticksToSleep = l_LastFrameTime.QuadPart + l_TicksPerFrame - l_CurrentTime.QuadPart;
			int64_t msToSleep = 1000 * ticksToSleep / l_PerfCountFrequency;
			if (msToSleep > 0)
			{
				Sleep((DWORD)msToSleep);
			}
			continue;
		}
		while (l_LastFrameTime.QuadPart + l_TicksPerFrame <= l_CurrentTime.QuadPart)
		{
			l_LastFrameTime.QuadPart += l_TicksPerFrame;
		}

		double dt = (double)l_TicksPerFrame / (double)l_PerfCountFrequency;
		input.dt = (float)dt;
		//UPDATE GAME STUFF ///////////////////////////////////////////////////////////////////////////
		
		GameAAA::RenderCommands renderCommands = GameAAA::Update_Game(gameData, input);

		//RENDER STUFF ///////////////////////////////////////////////////////////////////////////
		glClearColor(0, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glColor3f(1.0f, 1.0f, 1.0f);

		RenderWindows(renderCommands);
		//Game
		
		/*
		//UI
		//force
		float helperYvalue = -1.0;
		float heightForEachRect = 2.0 / gm.forceRects;
		float leftVertexX = -0.9;
		float rightVertexX = -0.8;
		for (int i = 0; i < gm.currentForceRects; i++)
		{
			float yvalueForBottomVertexOfRect = helperYvalue;
			helperYvalue += heightForEachRect;
			float yvalueForTopVertexOfRect = helperYvalue;

			glRectf(leftVertexX, yvalueForTopVertexOfRect, rightVertexX, yvalueForBottomVertexOfRect);
		}
		*/
		SwapBuffers(s_WindowHandleToDeviceContext);
	} while (!quit);

	GameAAA::DestroyGameData(gameData);
	system("pause");
	OutputDebugStringW(L"HelloWorld");

	return 0;
}
