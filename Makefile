CXX = g++
CXXFLAGS = -std=c++11 -Wall 
TARGET = candlestick_sma

all: $(TARGET)

$(TARGET): candlestick_sma.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(TARGET) *.csv

.PHONY: all clean