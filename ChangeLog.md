# UDMlib - Unstructured Data Management library


## REVISION HISTORY

---
- 2017-07-07  Version 1.2.2
  - set(CMAKE_FIND_ROOT_PATH /opt/FJSVtclang/1.2.0) in Toolchain_K.cmake


---
- 2017-06-26  Version 1.2.1
  - trim package


---
- 2017-06-26  Version 1.2.0
  - Cmake version
  - ctest still fails for test number 11-13, 25, 28, 39-43, 53, 56


---
- 2015-12-19  Version 1.1.3
  - Modifications (header file loading order) for the Intel Compiler


---
- 2015-12-12  Version 1.1.2
  - additional modify of include point of mpi.h


---
- 2015-11-28  Version 1.1.1
  - Fix the build error when compiling with ICC
    - catastrophic error: #error directive: "SEEK_SET is #defined but must not be for the C++ binding of MPI. Include mpi.h before stdio.h"
  - --with-comp becomes essential


---
- 2015-08-14  Version 1.1.0
  - introduce BUILD_DIR


---
- 2015-08-04
  - modify read/write CGNS::MIXED Elements.


---
- 2015-05-18  Version 1.0.8
  - fix shift operator.
  - modify isnan function.


---
- 2015-04-20  Version 1.0.7
  - fix printf args.
  - edit INSTALL

---
- 2015-04-17  Version 1.0.6
  - first release package

---
- 2015-04-11  Version 1.0.1
  - change credit


---
- 2014-10-15  Version 1.0.0
  - initial release
