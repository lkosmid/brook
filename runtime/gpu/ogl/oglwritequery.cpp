// oglwritequery.cpp
#include "oglwritequery.hpp"
#include "oglcheckgl.hpp"

namespace brook
{
   OGLWriteQuery::OGLWriteQuery()
   {
      _ref = 1;

      glGenQueriesARB(1, &_queryID);
      CHECK_GL();
   }


   OGLWriteQuery::~OGLWriteQuery(void)
   {
      glDeleteQueriesARB(1, &_queryID);
      _queryID = 0;
   }


   void OGLWriteQuery::begin(void)
   {
      _done = false;
      glBeginQueryARB(GL_SAMPLES_PASSED_ARB, _queryID);
   }


   void OGLWriteQuery::end(void)
   {
      glEndQueryARB(GL_SAMPLES_PASSED_ARB);
   }


   bool OGLWriteQuery::poll(void)
   {
      GLint done;

      glGetQueryObjectivARB(_queryID, GL_QUERY_RESULT_AVAILABLE_ARB, &done);
      return (done != GL_FALSE);
   }


   void OGLWriteQuery::wait(void)
   {
      /*
       * I don't think there is an explicit wait. So we'll just drain the
       * query completely.
       */
      count();
   }


   size_t OGLWriteQuery::count(void)
   {
      if (!_done) {
         glGetQueryObjectuivARB(_queryID, GL_QUERY_RESULT_ARB, &_numWritten);
         _done = true;
      }
      return _numWritten;
   }
}
