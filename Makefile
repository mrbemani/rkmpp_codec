# make file of cv_mpp

CXX = g++
CXXFLAGS = -std=c++11 -I/usr/local/include -I/usr/local/include/opencv4
LDFLAGS = -L/usr/local/lib
LDLIBS = -lopencv_core -lopencv_videoio -lopencv_imgproc -lrockchip_mpp #  -lmpp_base -lmpp_codec -lmpp_rc 

TARGET = cv_mpp
SRC = cv_mpp.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

deps:
	@echo "Checking for OpenCV..."
	@pkg-config --modversion opencv4 || (echo "Installing OpenCV..." && sudo apt-get install -y libopencv-dev)
	@echo "Checking for Rockchip MPP..."
	@ldconfig -p | grep libmpp.so || (echo "Fetching and installing Rockchip MPP..." && \
		git clone https://github.com/rockchip-linux/mpp.git && \
		cd mpp && \
		cmake -DRKPLATFORM=ON -DHAVE_DRM=ON && make && sudo make install)

clean:
	rm -f $(TARGET)

.PHONY: all deps clean
