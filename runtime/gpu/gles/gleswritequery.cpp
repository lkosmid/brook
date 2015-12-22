// gleswritequery.cpp
#include "gleswritequery.hpp"
#include "glescheckgl.hpp"

namespace brook
{
   GLESWriteQuery::GLESWriteQuery()
   {
      _ref = 1;
#ifdef GLES3
      glGenQueries(1, &_queryID);
      CHECK_GL();
#endif
   }


   GLESWriteQuery::~GLESWriteQuery(void)
   {
#ifdef GLES3
      glDeleteQueries(1, &_queryID);
#endif
      _queryID = 0;
   }


   void GLESWriteQuery::begin(void)
   {
      _done = false;
#ifdef GLES3
      glBeginQuery(GL_SAMPLES_PASSED, _queryID);
#endif
   }


   void GLESWriteQuery::end(void)
   {
#ifdef GLES3
      glEndQuery(GL_SAMPLES_PASSED);
#endif
   }


   bool GLESWriteQuery::poll(void)
   {
      GLint done;

#ifdef GLES3
      glGetQueryObjectiv(_queryID, GL_QUERY_RESULT_AVAILABLE, &done);
      return (done != GL_FALSE);
#else
      return GL_TRUE;
#endif
   }


   void GLESWriteQuery::wait(void)
   {
      /*
       * I don't think there is an explicit wait. So we'll just drain the
       * query completely.
       */
      count();
   }


   size_t GLESWriteQuery::count(void)
   {
#ifdef GLES3
      if (!_done) {
         glGetQueryObjectuiv(_queryID, GL_QUERY_RESULT, &_numWritten);
         _done = true;
      }
      return _numWritten;
#else
	return 0;
#endif
   }
}
