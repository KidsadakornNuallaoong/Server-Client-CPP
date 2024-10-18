.SILENT:
.PHONY: build run clean

GXX=g++
CXXFLAGS=-Wall -g
 
name=client

inputfile=$(name).cpp
outdir=dist
outfile=$(name)

ifeq ($(OS),Windows_NT)
LDFLAGS=-lws2_32 -lcurl
build:
ifeq (,$(wildcard $(outdir)))
	mkdir $(outdir)
endif
	$(GXX) $(CXXFLAGS) $(inputfile) -o $(outdir)/$(outfile) $(LDFLAGS)
run: build
	$(outdir)/$(outfile)
	$(MAKE) --no-print-directory clean
clean:
	del $(outfile).exe *.exe 2> nul
	rmdir /s /q $(outdir) 2>nul || echo Directory not found
else
build:
ifeq (,$(wildcard $(outdir)))
	mkdir $(outdir)
endif
	$(GXX) $(CXXFLAGS) $(inputfile) -o ./$(outdir)/$(outfile) $(LDFLAGS)
run: build
	./$(outdir)/$(outfile)
	$(MAKE) --no-print-directory clean
clean:
	rm -f $(outfile) *.exe
	rm -rf $(outdir)
endif