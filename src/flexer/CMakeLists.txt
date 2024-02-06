SET(NAME flexer)
add_library(${NAME} INTERFACE)
target_include_directories(${NAME} INTERFACE include/ )

add_executable(client src/client.cc)
target_link_libraries(client PUBLIC flexer stdc++fs commandLineParser)
