CXX = g++ -std=c++14
CC = $(CXX)
CXXFLAGS = -g -O0 -Wall -I/usr/lib/llvm-4.0/include/ `pkg-config gtkmm-3.0 --cflags`
LDLIBS = `pkg-config gtkmm-3.0 --libs` -L/usr/lib/llvm-4.0/lib/ -lclang
OBJECTS = graph.o node.o drawingarea_zoom_drag.o graph_layout_algorithms.o view_filters.o
COMP = $(CXX) $(CXXFLAGS) $^ $(LDLIBS) -o $@

all : $(OBJECTS) get_call_graph main

clean : all
	rm *.o

main : main.o $(OBJECTS)
	$(COMP)

graph : graph.o node.o

get_call_graph : get_call_graph.o
	$(COMP)
