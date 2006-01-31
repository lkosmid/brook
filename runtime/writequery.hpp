// writequery.hpp
#ifndef __BROOK_RUNTIME_WRITEQUERY_HPP__
#define __BROOK_RUNTIME_WRITEQUERY_HPP__

namespace brook
{
   class IWriteQuery
   {
   public:
       virtual ~IWriteQuery() {}
      virtual void acquireReference() = 0;
      virtual void releaseReference() = 0;

      virtual void begin() = 0;
      virtual void end() = 0;
      virtual bool poll() = 0;
      virtual void wait() = 0;
      virtual size_t count() = 0;
   };
}

#endif
