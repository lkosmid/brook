// dx9writequery.cpp
#include "dx9writequery.hpp"

namespace brook
{
   DX9WriteQuery::DX9WriteQuery( GPUContextDX9* inContext )
      : _referenceCount(1), _query(0), _ready(true), _count(0)
   {
      IDirect3DDevice9* device = inContext->getDevice();

      HRESULT result = device->CreateQuery( D3DQUERYTYPE_OCCLUSION, &_query );
      GPUAssert( !FAILED(result), "failed to allocate query in DX9WriteQuery::DX9WriteQuery" );
   }

	DX9WriteQuery::~DX9WriteQuery()
   {
      if( _query )
         _query->Release();
   }

   /* IWriteQuery interface */

   void DX9WriteQuery::acquireReference()
   {
      _referenceCount++;
   }

   void DX9WriteQuery::releaseReference()
   {
      if( --_referenceCount == 0 )
         delete this;
   }

   void DX9WriteQuery::begin()
   {
      _ready = false;
      _query->Issue( D3DISSUE_BEGIN );
   }

   void DX9WriteQuery::end()
   {
      _query->Issue( D3DISSUE_END );
   }

   bool DX9WriteQuery::poll()
   {
      if( _ready ) return true;

      DWORD queryResult = 0;

      HRESULT result = _query->GetData( (void*) &queryResult, (DWORD) sizeof(queryResult), D3DGETDATA_FLUSH );
      GPUAssert( !FAILED(result), "occlusion query failed in DX9WriteQuery::poll()" );

      if( result != S_FALSE )
      {
         _count = (size_t) queryResult;
         _ready = true;
         return true;
      }

      return false;
   }

   void DX9WriteQuery::wait()
   {
      // for DX9 occlusion queries, the
      // only option is to busy-wait
      while( !poll() )
         ;
   }

   size_t DX9WriteQuery::count()
   {
      if( !_ready ) wait();
      return _count;
   }
}
