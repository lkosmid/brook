// logger.cpp
#include "logger.hpp"

#include <fstream>

namespace brook {
namespace internal {

  Logger::Logger()
    : stream(NULL), ownsStream(false), level(-1), prefix(NULL)
  {
    const char* levelVariable = getenv("BRT_LOG_LEVEL");
    
    path = getenv("BRT_LOG_PATH");
    prefix = getenv("BRT_LOG_PREFIX");

    if( levelVariable )
      level = atoi( levelVariable );
  }

  Logger::~Logger()
  {
    if( stream )
    {
      *stream << std::flush;
      if( ownsStream )
        delete stream;
    }
  }

  Logger& Logger::getInstance()
  {
    static Logger sResult;
    return sResult;
  }

  bool Logger::isEnabled( int inLevel ) {
    return getInstance().level >= inLevel;
  }
  
  std::ostream& Logger::getStream()
  {
    Logger& instance = getInstance();

    if( !instance.stream )
    {
      if( instance.path && strlen(instance.path) != 0 )
      {
        instance.ownsStream = true;
        instance.stream = new std::ofstream( instance.path );
      }
      else
        instance.stream = &(std::cerr);
    }

    std::ostream& result = *instance.stream;
    if( instance.prefix )
      result << instance.prefix;
    return result;
  }

  void Logger::setLevel( int inLevel ) {
    getInstance().level = inLevel;
  }

  void Logger::setStream( std::ostream& inStream, bool inAssumeOwnership )
  {
    Logger& instance = getInstance();
    
    if( instance.stream == &inStream )
    {
      instance.ownsStream = inAssumeOwnership;
      return;
    }

    if( instance.stream && instance.ownsStream )
      delete instance.stream;

    instance.stream = &inStream;
    instance.ownsStream = inAssumeOwnership;
  }

}}
