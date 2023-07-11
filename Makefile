CXX = g++
CXXFLAGS = -Wall -pedantic -IUvcGrabber/include
LDFLAGS = -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_videoio -lopencv_highgui -lstdc++fs

SRC_DIR=UvcGrabber/src
OBJ_DIR=UvcGrabber/obj

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

TARGET = prog

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< $(LDFLAGS) -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
	
.PHONY: clean

$(shell mkdir -p $(OBJ_DIR))
