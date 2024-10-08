#find_external_library(
#  DEPENDENCY_NAME openssl
#  LIBRARY_NAME libcrypto.so
#)

build_lib(
    LIBNAME cybertwin
    SOURCE_FILES
        helper/cybertwin-helper.cc
        model/apps/cybertwin-app.cc
        model/apps/cybertwin-manager.cc
        model/apps/cybertwin.cc
        model/apps/end-host-initd.cc
        model/apps/end-host-bulk-send.cc
        model/apps/download-server.cc
        model/apps/download-client.cc
        model/devices/cybertwin-node.cc
        model/networks/cybertwin-name-resolution-service.cc
        model/networks/multipath-data-transfer-protocol.cc
        model/cybertwin-header.cc
        model/cybertwin-common.cc
        model/cybertwin-tag.cc
        model/cybertwin-client.cc
        model/cybertwin-packet-tags.cc
        
    HEADER_FILES
        helper/cybertwin-helper.h
        model/apps/cybertwin-app.h
        model/apps/cybertwin-manager.h
        model/apps/cybertwin.h
        model/apps/end-host-initd.h
        model/apps/end-host-bulk-send.h
        model/apps/download-server.h
        model/apps/download-client.h
        model/devices/cybertwin-node.cc
        model/devices/cybertwin-node.h
        model/networks/cybertwin-name-resolution-service.h
        model/networks/multipath-data-transfer-protocol.h
        model/cybertwin-header.h
        model/cybertwin-common.h
        model/cybertwin-tag.h
        model/cybertwin-client.h
        model/cybertwin-packet-tags.h
    LIBRARIES_TO_LINK ${libcore}
                        ${libapplications}
                        ${libinternet}
                        "crypto.so"

    TEST_SOURCES test/cybertwin-test-suite.cc
                 ${examples_as_tests_sources}
)