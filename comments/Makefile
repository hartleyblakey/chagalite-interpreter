.PHONY: all clean

CXXFLAGS := -std=c++17 -O0 -Wall -Wextra -g -I "../src"

BUILD = build

OBJECTS = $(BUILD)/comments.o

LIB_SOURCE = ../src

all: $(BUILD)/comments

$(BUILD):
	mkdir -p $@

$(BUILD)/%.o: $(LIB_SOURCE)/%.cpp $(LIB_SOURCE)/%.hpp | $(BUILD)
	g++ -o $@ $< -c $(CXXFLAGS)

$(BUILD)/comments: $(OBJECTS) main.cpp | $(BUILD)
	g++ -o $@ main.cpp $(BUILD)/*.o $(CXXFLAGS)

clean:
	rm -rf $(BUILD)
	rm -rf output
