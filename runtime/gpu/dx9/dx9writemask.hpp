// dx9writemask.hpp
#pragma once

#include "dx9base.hpp"
#include "../../writemask.hpp"

namespace brook {

   class DX9WriteMask :
      public brook::IWriteMask
   {
   public:
      DX9WriteMask( GPUContextDX9* inContext, int inY, int inX );
	   ~DX9WriteMask();

      /* IWriteMask interface */

      void acquireReference();
      void releaseReference();

      void bind();
      void unbind();
      void enableTest();
      void disableTest();
      void enableSet();
      void disableSet();
      void clear();

   private:
      size_t _referenceCount;
   
      GPUContextDX9* _context;
      IDirect3DDevice9* _device;
      size_t _extentX, _extentY;

      IDirect3DSurface9* _depthStencilSurface;
      IDirect3DSurface9* _colorSurface;
  };
}