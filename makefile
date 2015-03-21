TARGET = server
STANDARD = -Wall -D_REENTRANT -D_THREAD_SAFE
RELEASE = -O2 $(STANDARD)
DEBUG = -g $(STANDARD)

# uncomment for a particular build cause I am not sure how to
# automate this from the command line
FLAGS = $(RELEASE)
#FLAGS = $(DEBUG)
#CXX = /usr/local/bin/g++

OBJS = main.o \
       server.o 

# build the target
$(TARGET): $(OBJS)
	$(CXX) $(FLAGS) -o $(TARGET) $(OBJS) -lpthread

# build the objects
.cpp.o:
	$(CXX) $(FLAGS) $*.cpp -c -o $@

depends: 
	rm -rf dependencies
	$(CXX) $(FLAGS) -M *.cpp >> dependencies

clean: 
	@if [ -f $(TARGET) ]; then rm $(TARGET); fi
	@for file in $(OBJS); do \
		if [ -f $$file ]; then \
			rm $$file; \
		fi; \
	done

# dependencies
