// gleswritequery.hpp
#pragma once

#include "glesfunc.hpp"
#include "glescontext.hpp"
#include "../../writequery.hpp"

//Queries are not supported in GLES 2, only in GLES 3

namespace brook {

   class GLESWriteQuery : public brook::IWriteQuery {
   public:
      GLESWriteQuery(void);
      ~GLESWriteQuery(void);

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
