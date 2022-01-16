export CPLUS_INCLUDE_PATH=./pdfium/include
export LIBRARY_PATH=./pdfium/x86/lib

BIN=bin
OBJ=obj
SRC=src
TARGET=PdfiumView

CXX=g++
MACROS=-D UNICODE -D _UNICODE
CXXDEFFLAGS=-std=c++20 -Wall -Wextra -Wpedantic -Wconversion $(MACROS) -m32
RelFlags=-O3 -Wl,--strip-all,--build-id=none,--gc-sections -fno-ident -D NDEBUG -mwindows
DebFlags=-g -O0 -D _DEBUG
LIB=-lpdfium.dll -lcomctl32 -lgdi32 -lcomdlg32 -municode


SRCFILES=$(wildcard $(SRC)/*.cpp)
SRCBULKFILES=$(wildcard $(SRC)/*.cxx)
RSCFILES=$(wildcard $(SRC)/*.rc)

RELOBJFILES=$(SRCFILES:%.cpp=%.cpp.o)
RELOBJFILES+=$(RSCFILES:%.rc=%.rc.o)
RELOBJFILES:=$(RELOBJFILES:$(SRC)/%=$(OBJ)/%)

DEBOBJFILES=$(SRCFILES:%.cpp=%.cpp.d.o)
DEBOBJFILES+=$(RSCFILES:%.rc=%.rc.d.o)
DEBOBJFILES:=$(DEBOBJFILES:$(SRC)/%=$(OBJ)/%)


RELOBJFILESBULK=$(SRCBULKFILES:%.cxx=%.cxx.o)
RELOBJFILESBULK+=$(RSCFILES:%.rc=%.rc.o)
RELOBJFILESBULK:=$(RELOBJFILESBULK:$(SRC)/%=$(OBJ)/%)

DEBOBJFILESBULK=$(SRCBULKFILES:%.cxx=%.cxx.d.o)
DEBOBJFILESBULK+=$(RSCFILES:%.rc=%.rc.d.o)
DEBOBJFILESBULK:=$(DEBOBJFILESBULK:$(SRC)/%=$(OBJ)/%)

default: release

rel: release
deb: debug

bulkr: $(RELOBJFILESBULK)
	$(CXX) $^ -o $(BIN)/$(TARGET).exe $(RelFlags) $(LIB)
bulkd: $(DEBOBJFILESBULK)
	$(CXX) $^ -o $(BIN)/deb$(TARGET).exe $(DebFlags) $(LIB)

release: $(RELOBJFILES)
	$(CXX) $^ -o $(BIN)/$(TARGET).exe $(RelFlags) $(LIB)
debug: $(DEBOBJFILES)
	$(CXX) $^ -o $(BIN)/deb$(TARGET).exe $(DebFlags) $(LIB)


$(OBJ)/%.rc.o: $(SRC)/%.rc $(OBJ)
	windres -i $< -o $@ $(MACROS) -D FILE_NAME='\"$(TARGET).exe\"'
$(OBJ)/%.rc.d.o: $(SRC)/%.rc $(OBJ)
	windres -i $< -o $@ $(MACROS) -D FILE_NAME='\"deb$(TARGET).exe\"'

$(OBJ)/%.o: $(SRC)/% $(OBJ)
	$(CXX) -c $< -o $@ $(CXXDEFFLAGS) $(RelFlags)
$(OBJ)/%.d.o: $(SRC)/% $(OBJ)
	$(CXX) -c $< -o $@ $(CXXDEFFLAGS) $(DebFlags)

$(OBJ):
	mkdir $(OBJ)

$(BIN):
	mkdir $(BIN)

clean:
	rm -r -f $(OBJ)
	rm -f $(BIN)/*.exe
