set(LIB_SRC
        FileUtil.cpp
        LogFile.cpp
        AsyncLogging.cpp
        LogStream.cpp
        CountDownLatch.cpp
        Thread.cpp
        Logging.cpp
        )

add_library(libserver_base ${LIB_SRC})

target_link_libraries(libserver_base pthread)

add_subdirectory(tests)