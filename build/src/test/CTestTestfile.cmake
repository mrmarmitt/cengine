# CMake generated Testfile for 
# Source directory: C:/Users/mrmar/Documents/projetos_de_estudo/c++/cengine/src/test
# Build directory: C:/Users/mrmar/Documents/projetos_de_estudo/c++/cengine/build/src/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(cengine_tests "C:/Users/mrmar/Documents/projetos_de_estudo/c++/cengine/build/src/test/Debug/cengine_tests.exe")
  set_tests_properties(cengine_tests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/mrmar/Documents/projetos_de_estudo/c++/cengine/src/test/CMakeLists.txt;58;add_test;C:/Users/mrmar/Documents/projetos_de_estudo/c++/cengine/src/test/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(cengine_tests "C:/Users/mrmar/Documents/projetos_de_estudo/c++/cengine/build/src/test/Release/cengine_tests.exe")
  set_tests_properties(cengine_tests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/mrmar/Documents/projetos_de_estudo/c++/cengine/src/test/CMakeLists.txt;58;add_test;C:/Users/mrmar/Documents/projetos_de_estudo/c++/cengine/src/test/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(cengine_tests "C:/Users/mrmar/Documents/projetos_de_estudo/c++/cengine/build/src/test/MinSizeRel/cengine_tests.exe")
  set_tests_properties(cengine_tests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/mrmar/Documents/projetos_de_estudo/c++/cengine/src/test/CMakeLists.txt;58;add_test;C:/Users/mrmar/Documents/projetos_de_estudo/c++/cengine/src/test/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(cengine_tests "C:/Users/mrmar/Documents/projetos_de_estudo/c++/cengine/build/src/test/RelWithDebInfo/cengine_tests.exe")
  set_tests_properties(cengine_tests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/mrmar/Documents/projetos_de_estudo/c++/cengine/src/test/CMakeLists.txt;58;add_test;C:/Users/mrmar/Documents/projetos_de_estudo/c++/cengine/src/test/CMakeLists.txt;0;")
else()
  add_test(cengine_tests NOT_AVAILABLE)
endif()
subdirs("../../_deps/googletest-build")
