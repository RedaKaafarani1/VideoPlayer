CXX := g++

CXXFLAGS := -g -std=c++20 -Wall -Wextra -O0
CXXFLAGS += -Isrc
CXXFLAGS += $(shell pkg-config --cflags libavformat libavcodec libavutil libswscale libswresample raylib)

LDFLAGS :=

LIBS :=
LIBS += $(shell pkg-config --libs libavformat libavcodec libavutil libswscale libswresample raylib)

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

