// writequery.cpp
#include <stdlib.h>
#include "writequery.hpp"

#include "runtime.hpp"

#include <assert.h>

namespace brook
{
   write_query::write_query()
      : _query(0)
   {
   }

   write_query::write_query( IWriteQuery* inQuery )
      : _query(inQuery)
   {
   }

   write_query::write_query( const write_query& inQuery )
      : _query(0)
   {
      (*this) = inQuery;
   }

   write_query write_query::create()
   {
      brook::Runtime* runtime = brook::Runtime::GetInstance();
      return write_query( runtime->createWriteQuery() );
   }

   write_query& write_query::operator=( const write_query& inQuery )
   {
      IWriteQuery* newQuery = inQuery._query;
      IWriteQuery* oldQuery = _query;

      if( newQuery ) newQuery->acquireReference();
      if( oldQuery ) oldQuery->releaseReference();

      _query = newQuery;

      return *this;
   }

   void write_query::begin()
   {
      assert( _query != NULL );
      _query->begin();
   }

   void write_query::end()
   {
      assert( _query != NULL );
      _query->end();
   }

   bool write_query::poll()
   {
      assert( _query != NULL );
      return _query->poll();
   }

   void write_query::wait()
   {
      assert( _query != NULL );
      _query->wait();
   }

   size_t write_query::count()
   {
      assert( _query != NULL );
      return _query->count();
   }

   write_query::operator IWriteQuery*()
   {
      return _query;
   }
}

