// writemask.cpp
#include "writemask.hpp"

#include "runtime.hpp"

#include <assert.h>

namespace brook
{
   write_mask::write_mask()
      : _mask(0)
   {
   }

   write_mask::write_mask( IWriteMask* inMask )
      : _mask(inMask)
   {
   }

   write_mask::write_mask( const write_mask& inMask )
      : _mask(0)
   {
      (*this) = inMask;
   }

   write_mask write_mask::create( int inY, int inX )
   {
      brook::Runtime* runtime = brook::Runtime::GetInstance();
      return write_mask( runtime->createWriteMask( inY, inX ) );
   }

   write_mask& write_mask::operator=( const write_mask& inMask )
   {
      IWriteMask* newMask = inMask._mask;
      IWriteMask* oldMask = _mask;

      if( newMask ) newMask->acquireReference();
      if( oldMask ) oldMask->releaseReference();

      _mask = newMask;

      return *this;
   }

   void write_mask::bind()
   {
      assert( _mask != NULL );
      _mask->bind();
   }

   void write_mask::unbind()
   {
      assert( _mask != NULL );
      _mask->unbind();
   }

   void write_mask::enableTest()
   {
      assert( _mask != NULL );
      _mask->enableTest();
   }

   void write_mask::disableTest()
   {
      assert( _mask != NULL );
      _mask->disableTest();
   }

   void write_mask::enableSet()
   {
      assert( _mask != NULL );
      _mask->enableSet();
   }

   void write_mask::disableSet()
   {
      assert( _mask != NULL );
      _mask->disableSet();
   }

   void write_mask::clear()
   {
      assert( _mask != NULL );
      _mask->clear();
   }
}

