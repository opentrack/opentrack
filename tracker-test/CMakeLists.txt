set(SDK_TEST_TRACKER FALSE CACHE BOOL "sine wave test tracker")
if(SDK_TEST_TRACKER)
    otr_module(tracker-test)
endif()
