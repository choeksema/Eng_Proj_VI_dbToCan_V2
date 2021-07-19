CC=g++
EXE=web_sub
#Clear to turn off warnings
WARN=-Wall

SDIR=src
SDIR2=../src

IDIR=include
IDIR2=../include
CFLAGS=-I$(IDIR)
CFLAGS2=-I$(IDIR2)

# Likely needs to change per platform
LDIR=-L/usr/lib/arm-linux-gnueabihf#x86_64-linux-gnu
LIBS=-lmysqlcppconn -lpcan

DEPS= $(wildcard $(IDIR)/*.h)
DEPS2= $(wildcard $(IDIR2)/*.h)
#$(info	Header files: $(DEPS) $(DEPS2))

SRCS=$(wildcard $(SDIR)/*.cpp)
SRCS2=$(wildcard $(SDIR2)/*.cpp)
#$(info	Source files: $(SRCS) $(SRCS2))

ODIR=obj
ODIR2=../obj
OBJ=$(patsubst $(SDIR)/%.cpp, $(ODIR)/%.o, $(SRCS))
OBJ2=$(patsubst $(SDIR2)/%.cpp, $(ODIR2)/%.o, $(SRCS2))
#$(info	Obj files: $(OBJ))

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CC) $(WARN) -c -o $@ $< $(CFLAGS)

$(ODIR2)/%.o: $(SDIR2)/%.cpp $(DEPS2)
	$(CC) $(WARN) -c -o $@ $< $(CFLAGS2)

$(EXE): $(OBJ) $(OBJ2)
	$(CC) $(WARN) -o $@ $^ $(CFLAGS) $(CFLAGS2) $(LDIR) $(LIBS)

.PHONY: clean # don't mess up any files named "clean"

clean:
	rm -f $(ODIR)/*.o $(ODIR2)/*.o
