# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 20)

# Absolute paths to source files
cmake_policy(SET CMP0076 NEW)

project ("shitty")

add_library(shitty)
target_include_directories(
	shitty
	INTERFACE
	.
)
target_include_directories(
	shitty
	PRIVATE
	dependencies
)
target_link_libraries(
	shitty
	PRIVATE
	fb64
	hpack
)

# Include sub-projects.
add_subdirectory ("dependencies/fb64")
add_subdirectory ("dependencies/hpack")
add_subdirectory ("http1")
add_subdirectory ("http2")

target_sources(shitty
	PRIVATE
	ClientStream.h
	Connection.cpp
	Connection.h
	ConnectionManager.cpp
	ConnectionManager.h
	Date.cpp
	Date.h
	Error.cpp
	Error.h
	EventReceiver.cpp
	EventReceiver.h
	Headers.cpp
	Headers.h
	Message.cpp
	Message.h
	MessageHandler.cpp
	MessageHandler.h
	Payload.h
	Payload-inl.h
	ProxyHandler.cpp
	ProxyHandler.h
	Request.cpp
	Request.h
	RequestHandler.cpp
	RequestHandler.h
	RequestHandlerFactory.h
	Response.cpp
	Response.h
	Route.cpp
	Route-inl.h
	Routes.cpp
	Routes.h
	safefd.h
	Server.cpp
	Server.h
	ServerStream.h
	# Obsolete?
	#settings.cpp
	#settings.h
	SignalReceiver.cpp
	SignalReceiver.h
	SignalSource.cpp
	SignalSource.h
	SimpleMessageHandler.cpp
	SimpleMessageHandler.h
	StaticResponder.cpp
	StaticResponder.h
	StatusStrings.cpp
	StatusStrings.h
	Stream.cpp
	Stream.h
	StreamBuf.cpp
	StreamBuf.h
	StringUtils.h
	StringUtils-inl.h
	SwitchTransport.cpp
	SwitchTransport.h
	UnhandledRequestHandler.cpp
	UnhandledRequestHandler.h
)

add_executable(hello-world
	HelloWorld.cpp
)
target_link_libraries(hello-world
	fb64
	shitty
)
add_executable(print-requests
	PrintRequestServer.cpp
)
target_link_libraries(print-requests
	fb64
	shitty
)
add_executable(proxy
	ProxyServer.cpp
)
target_link_libraries(proxy
	fb64
	shitty
)
