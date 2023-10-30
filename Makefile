# make file of cv_mpp

CXX = g++

CXXFLAGS1 = -std=c++11 -I/usr/include -I/usr/local/include -I/usr/include/opencv4 -I./mpp -I./mpp/inc -I./mpp/mpp/base/inc -I./mpp/mpp/codec/inc -I./mpp/mpp/inc -I./mpp/osal/inc
LDFLAGS1 = -L/usr/local/lib -L/usr/lib
LDLIBS1 = -lopencv_core -lopencv_videoio -lopencv_imgproc -lopencv_imgcodecs -lrockchip_mpp #  -lmpp_base -lmpp_codec -lmpp_rc 

CXXFLAGS2 = -std=c++11 -I/usr/include -I/usr/local/include -I/usr/include/opencv4
LDFLAGS2 = -L/usr/local/lib -L/usr/lib
LDLIBS2 = -lopencv_core -lopencv_videoio -lopencv_imgproc -lopencv_imgcodecs

TARGET1 = cv_mpp 
SRC1 = cv_mpp.cpp
TARGET2 = cv2jpg
SRC2 = cv2jpg.cpp

all: $(TARGET1) $(TARGET2)

cv_mpp: $(TARGET1)

cv2jpg: $(TARGET2)

$(TARGET1): $(SRC1)
	$(CXX) $(CXXFLAGS1) $(LDFLAGS1) -o $@ $^ $(LDLIBS1)

$(TARGET2): $(SRC2)
	$(CXX) $(CXXFLAGS2) $(LDFLAGS2) -o $@ $^ $(LDLIBS2)


deps:
	@echo "Checking for OpenCV..."
	@pkg-config --modversion opencv4 || (echo "Installing OpenCV..." && sudo apt-get install -y libopencv-dev)
	@echo "Checking for Rockchip MPP..."
	@ldconfig -p | grep libmpp.so || (echo "Fetching and installing Rockchip MPP..." && \
		git clone https://github.com/rockchip-linux/mpp.git && \
		cd mpp && \
		cmake -DRKPLATFORM=ON -DHAVE_DRM=ON && make && sudo make install)

clean:
	rm -f $(TARGET1) $(TARGET2)

.PHONY: all deps clean
