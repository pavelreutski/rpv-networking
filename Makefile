.PHONY: all clean debug release tests

.EXPORT_ALL_VARIABLES:
MAKEFLAGS = --no-print-directory

all: clean debug release tests

debug:
	cmake --preset debug
	cmake --build --preset debug

release:
	cmake --preset release
	cmake --build --preset release

tests:
	cmake --preset tests
	cmake --build --preset tests
	ctest --preset tests

run-echo-server-debug: debug
	cmake --build --preset debug --target run-echo-server

run-echo-server-release: release
	cmake --build --preset release --target run-echo-server

run-http-server-debug: debug
	cmake --build --preset debug --target run-http-server

run-http-server-release: release
	cmake --build --preset release --target run-http-server

clean:
	rm -rf build