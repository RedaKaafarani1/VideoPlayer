CXX := g++

CXXFLAGS := -g -std=c++20 -Wall -Wextra -O0
CXXFLAGS += -Isrc
CXXFLAGS += $(shell pkg-config --cflags libavformat libavcodec libavutil libswscale libswresample)

RAYLIB_DIR := raylib
CXXFLAGS += -I$(RAYLIB_DIR)/include

LDFLAGS :=
LDFLAGS += -Lraylib/lib -Wl,-rpath,'$$ORIGIN/raylib/lib'


LIBS :=
LIBS += $(shell pkg-config --libs libavformat libavcodec libavutil libswscale libswresample)
LIBS += -lraylib -lm -ldl -lpthread -lX11 -lGL

SRC_DIR   := src
BUILD_DIR := build
TARGET    := app

SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean

