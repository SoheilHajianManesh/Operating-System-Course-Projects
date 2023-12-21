CXX = g++
CXXFLAGS = -std=c++11

SRC_DIR = src
SRC_BUILD = build
OBJ_DIR = $(SRC_BUILD)/obj
INC_DIR = $(SRC_DIR)/include

OUT_MAIN = ImageFilters.out

VPATH = $(SRC_DIR)


$(OUT_MAIN): $(OBJ_DIR)/main.o $(OBJ_DIR)/bmp.o $(OBJ_DIR)/filter.o
	$(CXX) $(CXXFLAGS) $^ -o $@



$(OBJ_DIR)/main.o: $(SRC_DIR)/main.cpp $(INC_DIR)/bmp.hpp $(INC_DIR)/filter.hpp $(INC_DIR)/utils.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/bmp.o: $(SRC_DIR)/bmp.cpp $(INC_DIR)/bmp.hpp $(INC_DIR)/utils.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/filter.o: $(SRC_DIR)/filter.cpp $(INC_DIR)/filter.hpp $(INC_DIR)/bmp.hpp $(INC_DIR)/utils.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)



.PHONY: all clean

clean:
	rm -f $(OBJ_DIR)/*.o ./*.out ./*.bmp