
#pragma once

namespace brook {

#define CHECK_GL() __check_gl(__LINE__, __FILE__)

  void __check_gl(int line, char *file);

}
