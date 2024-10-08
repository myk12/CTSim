build_lib_example(
    NAME cybertwin-example
    SOURCE_FILES cybertwin-example.cc
    LIBRARIES_TO_LINK ${libcybertwin}
                    ${libpoint-to-point}
)

build_lib_example(
    NAME cybertwin-multipath-transfer
    SOURCE_FILES cybertwin-multipath-datatransfer.cc
    LIBRARIES_TO_LINK ${libcybertwin}
                    ${libpoint-to-point}
                    ${libapplications}
                    ${libinternet}
                    ${libnetwork}
                    ${libcsma}
)

build_lib_example(
    NAME cybertwin-name-resolution-service
    SOURCE_FILES cybertwin-name-resolution-service.cc
    LIBRARIES_TO_LINK ${libcybertwin}
                    ${libpoint-to-point}
                    ${libapplications}
                    ${libinternet}
                    ${libnetwork}
                    ${libcsma}
)

build_lib_example(
    NAME cybertwin-json
    SOURCE_FILES cybertwin-json.cc
    LIBRARIES_TO_LINK ${libcybertwin}
                    ${libpoint-to-point}
                    ${libapplications}
                    ${libinternet}
                    ${libnetwork}
                    ${libcsma}
)