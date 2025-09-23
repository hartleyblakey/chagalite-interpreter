.PHONY: all clean

CXXFLAGS := -std=c++20 -O0 -Wall -Wextra -g -I "../src"

BUILD = build

OBJECTS = $(BUILD)/tokenize.o $(BUILD)/comments.o $(BUILD)/cst.o

LIB_SOURCE = ../src

all: $(BUILD)/cst

$(BUILD):
	mkdir -p $@

$(BUILD)/%.o: $(LIB_SOURCE)/%.cpp $(LIB_SOURCE)/%.hpp | $(BUILD)
	g++ -o $@ $< -c $(CXXFLAGS)

$(BUILD)/cst: $(OBJECTS) main.cpp | $(BUILD)
	g++ -o $@ main.cpp $(BUILD)/*.o $(CXXFLAGS)

clean:
	rm -rf $(BUILD)
	rm -rf output