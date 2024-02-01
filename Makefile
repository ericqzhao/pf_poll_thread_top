#CXX = clang++
CXXFLAGS = -O2 -std=c++11 -I.. -Wall -Wextra -pthread

PREFIX = /usr/local
#PREFIX = $(shell brew --prefix)

OPENSSL_DIR = $(PREFIX)/opt/openssl@3
OPENSSL_SUPPORT = -DCPPHTTPLIB_OPENSSL_SUPPORT -I$(OPENSSL_DIR)/include -L$(OPENSSL_DIR)/lib -lssl -lcrypto

ifneq ($(OS), Windows_NT)
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Darwin)
		OPENSSL_SUPPORT += -framework CoreFoundation -framework Security
	endif
endif

ZLIB_SUPPORT = -DCPPHTTPLIB_ZLIB_SUPPORT -lz

BROTLI_DIR = $(PREFIX)/opt/brotli
BROTLI_SUPPORT = -DCPPHTTPLIB_BROTLI_SUPPORT -I$(BROTLI_DIR)/include -L$(BROTLI_DIR)/lib -lbrotlicommon -lbrotlienc -lbrotlidec

all: server client hello simplecli simplesvr upload redirect ssesvr ssecli benchmark issue


client : client.cc ../httplib.h Makefile
	$(CXX) -o pf_top $(CXXFLAGS) pf_thread_top.cc  $(ZLIB_SUPPORT) -I./nlohmann/ 


pem:
	openssl genrsa 2048 > key.pem
	openssl req -new -key key.pem | openssl x509 -days 3650 -req -signkey key.pem > cert.pem

clean:
	rm server client hello simplecli simplesvr upload redirect ssesvr ssecli benchmark *.pem
