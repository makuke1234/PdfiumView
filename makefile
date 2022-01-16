CPPC=g++
MACROS=-D UNICODE -D _UNICODE
CFlags=-std=c++20 -Wall -Wextra -Wpedantic -I./pdfium/include $(MACROS)
RelFlags=-O3 -Wl,--strip-all,--build-id=none,--gc-sections -fno-ident -D NDEBUG -mwindows
DebFlags=-g -D _DEBUG
Lib=-L./pdfium/x86/lib -lpdfium.dll -lcomctl32 -lgdi32 -lcomdlg32

BINDIR=bin
OBJDIR=obj
TARGETNAME=PdfiumView

default: release

rel: release
deb: debug

release: objdir objfiles.r
debug: objdir objfiles.d

bulkr: objdir bulkobj.r
bulkd: objdir bulkobj.d

bulkobj.r: sourcebulk.o resource.o
	$(CPPC) $(addprefix $(OBJDIR)/, $^) -o $(BINDIR)/$(TARGETNAME).exe $(RelFlags) $(Lib)
bulkobj.d: sourcebulk.d.o resource.d.o
	$(CPPC) $(addprefix $(OBJDIR)/, $^) -o $(BINDIR)/$(TARGETNAME).d.exe $(DebFlags) $(Lib)

objfiles.r: main.o resource.o main_window.o common.o errors.o lib.o tabs.o opendialog.o
	$(CPPC) $(addprefix $(OBJDIR)/, $^) -o $(BINDIR)/$(TARGETNAME).exe $(RelFlags) $(Lib)
objfiles.d: main.d.o resource.d.o main_window.d.o common.d.o errors.d.o lib.d.o tabs.d.o opendialog.d.o
	$(CPPC) $(addprefix $(OBJDIR)/, $^) -o $(BINDIR)/$(TARGETNAME).d.exe $(DebFlags) $(Lib)

resource.o: resource.rc
	windres -i $^ -o $(OBJDIR)/$@ $(MACROS) -D FILE_NAME='\"$(TARGETNAME).exe\"'
resource.d.o: resource.rc
	windres -i $^ -o $(OBJDIR)/$@ $(MACROS) -D FILE_NAME='\"$(TARGETNAME).d.exe\"'

%.o: %.cpp
	$(CPPC) -c $^ -o $(OBJDIR)/$@ $(CFlags) $(RelFlags)
%.d.o: %.cpp
	$(CPPC) -c $^ -o $(OBJDIR)/$@ $(CFlags) $(DebFlags)

objdir:
	IF NOT EXIST $(OBJDIR) (mkdir $(OBJDIR))

clean:
	IF EXIST $(OBJDIR) (rd $(OBJDIR) /s /q)