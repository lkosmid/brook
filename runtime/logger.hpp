// logger.hpp
#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include <iostream>

#define BROOK_LOGGER_ENABLED

#ifdef BROOK_LOGGER_ENABLED

#define BROOK_LOG( __level ) \
  if( ::brook::internal::Logger::isEnabled( __level ) ) \
    ::brook::internal::Logger::getStream()

#else

#define BROOK_LOG( __level ) \
  if( 0 ) ::std::cout

#endif

namespace brook {
namespace internal {

  class Logger
  {
  public:
    static bool isEnabled( int inLevel );
    static std::ostream& getStream();

    static void setLevel( int inLevel );
    static void setStream( std::ostream& inStream, bool inAssumeOwnership = false );

  private:
    Logger();
    static Logger& getInstance();

    const char* prefix;
    const char* path;
    std::ostream* stream;
    bool ownsStream;
    int level;
#ifdef _MSC_VER
#if _MSC_VER <= 1200
  public: //bug with destructor protection. Fails to compile with private dest
#endif
#endif
    ~Logger();
  };

}}

#endif
