MAKEFLAGS := --jobs=$(shell nproc)

SRCS    := $(shell find . -name "*.cpp" -print -or -name "*.c" ! -iname ".*" -print)
HEADERS := $(shell find . -name "*.hpp" -print -or -name "*.inl" -print)
COMMON_SRCS    := $(shell find ../../common -name "*.cpp" ! -name "stdlib.cpp" -or -name "*.c" ! -iname ".*")
COMMON_HEADERS := $(shell find ../../common -name "*.hpp" -or -name "*.inl")
OBJS        := $(SRCS:%=%.o)
COMMON_OBJS := $(COMMON_SRCS:%=%.o)

BUILD ?= debug

CFLAGS   := -Werror -Wall
CXXFLAGS := -Wall -fno-rtti -std=c++17 -Wno-invalid-offsetof -Wno-parentheses -fno-exceptions
LFLAGS   := -lm -lSDL2

ifeq ($(BUILD),debug)
	CFLAGS   := $(CFLAGS) -g -Og -D DEBUG
	CXXFLAGS := $(CXXFLAGS) -g -Og -D DEBUG

else ifeq ($(BUILD),release)
	CFLAGS   := $(CFLAGS) -O2
	CXXFLAGS := $(CXXFLAGS) -O2
	LFLAGS   := $(LFLAGS) -O2
endif

CC  := gcc -I ../../ $(CFLAGS)
CXX := g++ -I ../../ $(CXXFLAGS)
LD  := gcc

all: test

test: Makefile $(OBJS) $(COMMON_OBJS)
	@$(LD) -o $@ $(LFLAGS) $(OBJS) $(COMMON_OBJS) $(LFLAGS)

%.c.o: %.c Makefile $(HEADERS) $(COMMON_HEADERS)
	@echo "C    " $<
	@$(CC) -c $< -o $@

%.cpp.o: %.cpp Makefile $(HEADERS) $(COMMON_HEADERS)
	@echo "CPP  " $<
	@$(CXX) -c $< -o $@

.PHONY: clean
clean:
	@$(RM) -rf ../build
	@find . -type f -name '*.o' -delete
	@find ../../common -type f -name '*.o' -delete
