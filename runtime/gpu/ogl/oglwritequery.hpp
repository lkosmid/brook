// oglwritequery.hpp
#pragma once

#include "oglfunc.hpp"
#include "oglcontext.hpp"
#include "../../writequery.hpp"

namespace brook {

   class OGLWriteQuery : public brook::IWriteQuery {
   public:
      OGLWriteQuery(void);
      ~OGLWriteQuery(void);

      /* IWriteQuery interface */

      void acquireReference() { ++_ref; };
      void releaseReference() { if (--_ref == 0) { delete this; } };

      void begin(void);
      void end(void);
      bool poll(void);
      void wait(void);
      size_t count(void);

   private:
      size_t _ref;

      GLuint _queryID;
      GLuint _numWritten;
      bool _done;
   };
}
