// dx9writemask.cpp
#include "dx9writemask.hpp"

namespace brook
{
   DX9WriteMask::DX9WriteMask( GPUContextDX9* inContext, int inY, int inX )
      : _referenceCount(1)
   {
      _context = inContext;
      _device = _context->getDevice();

      _extentX = (size_t) inX;
      _extentY = (size_t) inY;

      HRESULT result = _device->CreateDepthStencilSurface(
         _extentX, _extentY,
         D3DFMT_D24S8,
         D3DMULTISAMPLE_NONE,
         0,
         FALSE,
         &_depthStencilSurface,
         NULL );
      DX9AssertResult( result, "failed to create depth buffer in DX9WriteMask::DX9WriteMask" );

      result = _device->CreateRenderTarget(
         _extentX, _extentY,
         D3DFMT_A8R8G8B8,
         D3DMULTISAMPLE_NONE,
         0,
         FALSE,
         &_colorSurface,
         NULL );
      DX9AssertResult( result, "failed to create color buffer in DX9WriteMask::DX9WriteMask" );


      bind();
      clear();
      unbind();
   }

	DX9WriteMask::~DX9WriteMask()
   {
   }

   /* IWriteQuery interface */

   void DX9WriteMask::acquireReference()
   {
      _referenceCount++;
   }

   void DX9WriteMask::releaseReference()
   {
      if( --_referenceCount == 0 )
         delete this;
   }

   void DX9WriteMask::bind()
   {
      HRESULT result;

      result = _device->SetDepthStencilSurface( _depthStencilSurface );
      DX9AssertResult( result, "failed to set depth buffer in DX9WriteMask::bind" );
   }

   void DX9WriteMask::unbind()
   {
      HRESULT result;

      result = _device->SetDepthStencilSurface( NULL );
      DX9AssertResult( result, "failed to set depth buffer in DX9WriteMask::unbind" );
   }

   void DX9WriteMask::enableTest()
   {
      HRESULT result;

      result = _device->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
      DX9AssertResult( result, "failed to enable depth test in DX9WriteMask::enableTest" );

      result = _device->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESS );
      DX9AssertResult( result, "failed to set depth compare in DX9WriteMask::enableTest" );
   }

   void DX9WriteMask::disableTest()
   {
      HRESULT result;

      result = _device->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
      DX9AssertResult( result, "failed to disable depth test in DX9WriteMask::disableTest" );
   }

   void DX9WriteMask::enableSet()
   {
      HRESULT result;

      result = _device->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
      DX9AssertResult( result, "failed to enable depth set in DX9WriteMask::enableSet" );
   }

   void DX9WriteMask::disableSet()
   {
      HRESULT result;

      result = _device->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
      DX9AssertResult( result, "failed to disable depth set in DX9WriteMask::disableSet" );
   }

   void DX9WriteMask::clear()
   {
      HRESULT result;

      _context->beginScene();

      result = _device->SetRenderTarget( 0, _colorSurface );
      DX9AssertResult( result, "failed to set render target in DX9Writemask::clear" );

      result = _device->Clear( 0, 0, D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0, 1.0f, 0 );
      DX9AssertResult( result, "failed to clear depth buffer in DX9WriteMask::clear" );

      _context->endScene();
   }
}
