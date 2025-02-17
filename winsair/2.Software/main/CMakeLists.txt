set(COMPONENT_ADD_INCLUDEDIRS "${PROJECT_PATH}" "$ENV{IDF_PATH}/examples/common_components/protocol_examples_common/include/")
set(COMPONENT_SRCS
    main.c timer.c tcp_server.c  DAP_handle.c
     wifi_handle.c)
register_component()





