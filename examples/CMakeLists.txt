add_executable(readfile readfile.c)
target_link_libraries(readfile cbor)

add_executable(create_items create_items.c)
target_link_libraries(create_items cbor)

add_executable(streaming_parser streaming_parser.c)
target_link_libraries(streaming_parser cbor)

add_executable(streaming_array streaming_array.c)
target_link_libraries(streaming_array cbor)

add_executable(sort sort.c)
target_link_libraries(sort cbor)

add_executable(hello hello.c)
target_link_libraries(hello cbor)

add_executable(cbor_sequence cbor_sequence.c)
target_link_libraries(cbor_sequence cbor)

add_executable(crash_course crash_course.c)
target_link_libraries(crash_course cbor)

find_package(CJSON)

if(CJSON_FOUND)
  add_executable(cjson2cbor cjson2cbor.c)
  target_include_directories(cjson2cbor PUBLIC ${CJSON_INCLUDE_DIRS})
  target_link_libraries(cjson2cbor cbor ${CJSON_LIBRARY})

  add_executable(cbor2cjson cbor2cjson.c)
  target_include_directories(cbor2cjson PUBLIC ${CJSON_INCLUDE_DIRS})
  target_link_libraries(cbor2cjson cbor ${CJSON_LIBRARY})
endif()

file(COPY data DESTINATION .)
