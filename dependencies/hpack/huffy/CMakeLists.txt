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
	encode-table.c
	encode-table.h
)

add_custom_command(
	OUTPUT
	encode-table.c
	COMMAND
	${CMAKE_CURRENT_SOURCE_DIR}/make-etable.sh
		< ${CMAKE_CURRENT_SOURCE_DIR}/huffcodes.txt
		> encode-table.c
	MAIN_DEPENDENCY
	huffcodes.txt
	DEPENDS
	make-etable.sh
	VERBATIM
)

target_include_directories(huffy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# TODO: Add tests:
#  bitbuf-test
#  decode-test
#  encode-test
#  random-test
