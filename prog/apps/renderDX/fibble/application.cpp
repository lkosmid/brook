// application.cpp

#include "core.hpp"
#include "application.hpp"

using namespace fibble;

Application::Application() {
}

Application::~Application() {
}

int Application::runMessageLoop()
{
	MSG message;
  while( true )
  {
	  while( ::PeekMessage( &message, NULL, 0, 0, PM_NOREMOVE ) )
	  {

		  int result = ::GetMessage( &message, NULL, 0, 0 );

		  if( result == -1 || result == 0 )
        return result;

		  ::TranslateMessage( &message );
		  ::DispatchMessage( &message );
	  }

    handleIdle();
  }
}
