set(target_prefix cybertwin)
set(source_files
    main.cc
    CybertwinSim.cc)

set(header_files
    CybertwinSim.h)

build_exec(
    EXECNAME "cybertwin"
    EXECNAME_PREFIX target_prefix
    SOURCE_FILES ${source_files}
    HEADER_FILES ${header_files}
    LIBRARIES_TO_LINK "${ns3-libs}"
    EXECUTABLE_DIRECTORY_PATH "cybertwinexec/"
)


