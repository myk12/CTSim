build_lib(
  LIBNAME topology-read
  SOURCE_FILES
    helper/topology-reader-helper.cc
    model/inet-topology-reader.cc
    model/orbis-topology-reader.cc
    model/rocketfuel-topology-reader.cc
    model/topology-reader.cc
    model/cybertwin-topology-reader.cc
  HEADER_FILES
    helper/topology-reader-helper.h
    model/inet-topology-reader.h
    model/orbis-topology-reader.h
    model/rocketfuel-topology-reader.h
    model/topology-reader.h
    model/cybertwin-topology-reader.h
  LIBRARIES_TO_LINK ${libnetwork}
                    yaml-cpp::yaml-cpp
  TEST_SOURCES test/rocketfuel-topology-reader-test-suite.cc
)

