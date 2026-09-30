CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread

app: main.cpp threadfuncs.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

run: app
	./app

clean:
	rm -f app output.log trace.log

.PHONY: run clean
