CXX = g++
CXXFLAGS = -Wall -pedantic -IFrameGrabber/include
LDFLAGS = -lopencv_core -lopencv_imgproc -lopencv_imgcodecs

SRC_DIR=FrameGrabber/src
OBJ_DIR=FrameGrabber/obj

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

TARGET = prog

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< $(LDFLAGS) -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	
.PHONY: clean

$(shell mkdir -p $(OBJ_DIR))
