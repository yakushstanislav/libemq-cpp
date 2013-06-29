CXX:=g++
CFLAGS=-std=c++11 -I. -O2 -Wall -W
LDFLAGS=-lpthread -lemq

PREFIX?=/usr/local
INCLUDE_PATH?=include/emq
INSTALL_INCLUDE_PATH=$(PREFIX)/$(INCLUDE_PATH)
INSTALL?=cp -a

EXAMPLES_DIR=examples
EXAMPLES=$(EXAMPLES_DIR)/simple $(EXAMPLES_DIR)/queue-subscribe $(EXAMPLES_DIR)/channel-subscribe

all: $(EXAMPLES)

$(EXAMPLES_DIR)/simple:
	$(CXX) -o $@ $(CFLAGS) $(EXAMPLES_DIR)/simple.cpp $(LDFLAGS)

$(EXAMPLES_DIR)/queue-subscribe:
	$(CXX) -o $@ $(CFLAGS) $(EXAMPLES_DIR)/queue-subscribe.cpp $(LDFLAGS)

$(EXAMPLES_DIR)/channel-subscribe:
	$(CXX) -o $@ $(CFLAGS) $(EXAMPLES_DIR)/channel-subscribe.cpp $(LDFLAGS)

install:
	mkdir -p $(INSTALL_INCLUDE_PATH)
	$(INSTALL) emq++.h $(INSTALL_INCLUDE_PATH)

clean:
	rm -rf $(EXAMPLES)
