// dx9writequery.hpp
#pragma once

#include "dx9base.hpp"
#include "../../writequery.hpp"

namespace brook {

   class DX9WriteQuery :
      public brook::IWriteQuery
   {
   public:
      DX9WriteQuery( GPUContextDX9* inContext );
	   ~DX9WriteQuery();

      /* IWriteQuery interface */

      void acquireReference();
      void releaseReference();

      void begin();
      void end();
      bool poll();
      void wait();
      size_t count();

   private:
      size_t _referenceCount;
      IDirect3DQuery9* _query;
      bool _ready;
      size_t _count;
   };
}