// window.hpp
#pragma once

#include "base.hpp"

namespace fibble {

  class Window
  {
  public:
	  static Window* create();
	  ~Window();

	  void show();
	  void hide();

    HWND getWindowHandle() { return windowHandle; }
    Context* getContext() { return context; }

    void handleEvents();

  protected:
	  Window( int inWidth = 0, int inHeight = 0 );
	  void finalize();

    virtual void handleMouseDown( int x, int y ) {}
    virtual void handleMouseUp( int x, int y ) {}
    virtual void handleMouseMove( int x, int y ) {}

    virtual LRESULT handleMessage( UINT inMessage, WPARAM wParam, LPARAM lParam );

  private:
	  HWND windowHandle;
    Context* context;

	  static const char* kWindowClassName;
	  static void registerWindowClass();
	  static LRESULT WINAPI windowCallback( HWND inWindowHandle, UINT inMessage, WPARAM wParam, LPARAM lParam );
  };
}