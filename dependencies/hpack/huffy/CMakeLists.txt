add_library(
	huffy
)

target_sources(
	huffy
	PRIVATE
	bitbuf.c
	bitbuf.h
	decode.c
	decode.h
	encode.c
	encode.h
)

target_include_directories(
	huffy
	PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}
)

# Must be a target not a command so that huffy (library) can depend on it.
add_custom_target(
	encode-table
	BYPRODUCTS
	encode-table.c
	COMMAND
	${CMAKE_CURRENT_SOURCE_DIR}/make-etable.sh
		< ${CMAKE_CURRENT_SOURCE_DIR}/huffcodes.txt
		> encode-table.c
	DEPENDS
	make-etable.sh
	huffcodes.txt
	VERBATIM
)

add_dependencies(huffy encode-table)

# TODO: Add tests:
#  bitbuf-test
#  decode-test
#  encode-test
#  random-test
