// window.cpp

#include "core.hpp"
#include "window.hpp"

using namespace fibble;

const char* Window::kWindowClassName = "Fibble_WindowClass";

Window::Window( int inWidth, int inHeight )
{
	registerWindowClass();
	windowHandle = ::CreateWindow( kWindowClassName, "GL Test", WS_OVERLAPPEDWINDOW,
		-1, -1, inWidth, inHeight, ::GetDesktopWindow(), NULL, GetModuleHandle(NULL), this );
}

Window::~Window()
{
	::DestroyWindow( windowHandle );
}

Window* Window::create()
{
	Window* result = new Window();
	return result;
}

void Window::show() {
	::ShowWindow( windowHandle, SW_SHOW );
}

void Window::hide() {
	::ShowWindow( windowHandle, SW_HIDE );
}

void Window::handleEvents()
{
	MSG message;

	while( ::PeekMessage( &message, NULL, 0, 0, PM_NOREMOVE ) )
	{

		int result = ::GetMessage( &message, NULL, 0, 0 );

		if( result == -1 )
		{
//			setResult(-1);
			return;
		}

		if( result == 0 )
		{
//			setResult(0);
			return;
		}

		::TranslateMessage( &message );
		::DispatchMessage( &message );
	}
}

LRESULT Window::handleMessage( UINT inMessage, WPARAM wParam, LPARAM lParam )
{
	switch( inMessage )
	{
	case WM_DESTROY:
		::PostQuitMessage( 0 );
		return 0;
  case WM_LBUTTONDOWN:
    handleMouseDown( (int)LOWORD(lParam), (int)HIWORD(lParam) );
    return 0;
  case WM_LBUTTONUP:
    handleMouseUp( (int)LOWORD(lParam), (int)HIWORD(lParam) );
    return 0;
  case WM_MOUSEMOVE:
    handleMouseMove( (int)LOWORD(lParam), (int)HIWORD(lParam) );
    return 0;
	}

	return ::DefWindowProc( windowHandle, inMessage, wParam, lParam );
}

void Window::finalize()
{
	if( windowHandle == NULL ) return;

	// TIM: TODO: handle destruction...
}

void Window::registerWindowClass()
{
	static bool sInitialized = false;
	if( sInitialized ) return;
	sInitialized = true;

	WNDCLASSEX classDesc;
	ZeroMemory( &classDesc, sizeof(classDesc) );
	classDesc.cbSize = sizeof(classDesc);

	classDesc.style = CS_CLASSDC;
	classDesc.lpfnWndProc = (WNDPROC)(Window::windowCallback);
	classDesc.hInstance = GetModuleHandle(NULL);
	classDesc.lpszClassName = kWindowClassName;

	RegisterClassEx( &classDesc );
}

LRESULT WINAPI Window::windowCallback( HWND inWindowHandle, UINT inMessage, WPARAM wParam, LPARAM lParam )
{
	Window* window = (Window*)GetWindowLong( inWindowHandle, GWL_USERDATA );

	if( inMessage == WM_CREATE )
	{
		CREATESTRUCT* creationInfo = (CREATESTRUCT*)(lParam);
		window = (Window*)creationInfo->lpCreateParams;
		SetWindowLong( inWindowHandle, GWL_USERDATA, (long)window );
	}

	if( window == NULL )
		return DefWindowProc( inWindowHandle, inMessage, wParam, lParam );

	LRESULT result = window->handleMessage( inMessage, wParam, lParam );

	if( inMessage == WM_DESTROY )
		window->finalize();

	return result;
}
