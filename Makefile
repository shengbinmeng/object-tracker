SRCS := tracker.cpp

OPENCV_PC_PATH=${PWD}/opencv/build/local/lib/pkgconfig

CXXFLAGS := `export PKG_CONFIG_PATH=${OPENCV_PC_PATH}; pkg-config --cflags opencv`
LDFLAGS  := `export PKG_CONFIG_PATH=${OPENCV_PC_PATH}; pkg-config --static --libs opencv`
LDFLAGS  += -lstdc++

OBJS := $(SRCS:%.cpp=%.o)

track: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f track $(OBJS)
