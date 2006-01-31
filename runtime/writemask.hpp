// writemask.hpp
#ifndef __BROOK_RUNTIME_WRITEMASK_HPP__
#define __BROOK_RUNTIME_WRITEMASK_HPP__

namespace brook
{
   class IWriteMask
   {
   public:
       virtual ~IWriteMask() {}
      virtual void acquireReference() = 0;
      virtual void releaseReference() = 0;

      virtual void bind() = 0;
      virtual void unbind() = 0;
      virtual void enableTest() = 0;
      virtual void disableTest() = 0;
      virtual void enableSet() = 0;
      virtual void disableSet() = 0;
      virtual void clear() = 0;
   };
}

#endif
