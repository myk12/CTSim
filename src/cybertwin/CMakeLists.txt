#find_external_library(
#  DEPENDENCY_NAME openssl
#  LIBRARY_NAME libcrypto.so
#)

build_lib(
    LIBNAME cybertwin
    SOURCE_FILES
        helper/cybertwin-helper.cc
        model/apps/cybertwin-app.cc
        model/cybertwin-manager.cc
        model/cybertwin.cc
        model/apps/end-host-bulk-send.cc
        model/apps/download-server.cc
        model/apps/download-client.cc
        model/cybertwin-node.cc
        model/networks/cybertwin-name-resolution-service.cc
        model/networks/multipath-data-transfer-protocol.cc
        model/cybertwin-header.cc
        model/cybertwin-common.cc
        model/cybertwin-tag.cc
        model/cybertwin-client.cc
        model/cybertwin-packet-tags.cc
        model/cybertwin-app-helper.cc
        model/cybertwin-app-download-server.cc
        model/cybertwin-app-download-client.cc
        model/cybertwin-endhost-daemon.cc
        
    HEADER_FILES
        helper/cybertwin-helper.h
        model/apps/cybertwin-app.h
        model/cybertwin-manager.h
        model/cybertwin.h
        model/apps/end-host-bulk-send.h
        model/apps/download-server.h
        model/apps/download-client.h
        model/cybertwin-node.h
        model/networks/cybertwin-name-resolution-service.h
        model/networks/multipath-data-transfer-protocol.h
        model/cybertwin-header.h
        model/cybertwin-common.h
        model/cybertwin-tag.h
        model/cybertwin-client.h
        model/cybertwin-packet-tags.h
        model/cybertwin-app-helper.h
        model/cybertwin-app-download-server.h
        model/cybertwin-app-download-client.h
        model/cybertwin-endhost-daemon.h
    LIBRARIES_TO_LINK ${libcore}
                        ${libapplications}
                        ${libinternet}
                        "crypto.so"

    TEST_SOURCES test/cybertwin-test-suite.cc
                 ${examples_as_tests_sources}
)