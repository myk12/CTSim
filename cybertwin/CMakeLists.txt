set(cybertwin_target_prefix "ctsim")
set(cybertwin_target_name "cybertwin")

set(cybertwin_source_files
    "simulator.cc"
)

set(cybertwin_heders
    "simulator.h"
)

build_exec(
    EXECNAME ${cybertwin_target_name}
    EXECNAME_PREFIX ${cybertwin_target_prefix}
    SOURCE_FILES ${cybertwin_source_files}
    LIBRARIES_TO_LINK "${ns3-libs}"
                        yaml-cpp
    EXECUTABLE_DIRECTORY_PATH "${cybertwin_target_prefix}/"
)
