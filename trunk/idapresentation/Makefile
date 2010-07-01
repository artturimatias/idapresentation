IDIR= include
ODIR= src/obj
SDIR= src
OSG_LIBS= -losg -losgUtil -losgViewer -losgAnimation -ltinyxml -lcurl  
CFLAGS= -W -Wall -g -I$(IDIR) `pkg-config QtWebKit --libs --cflags`
CCC= g++
OUT= viewer
includes = $(wildcard include/*.h)
OBJ = smilNet.o baseRegion.o smilRegion.o smilRegionText.o smilRegion3D.o smilRegionImage.o smilParser.o
OBJS = $(patsubst %,$(ODIR)/%,$(OBJ))


viewer: $(OBJS) $(ODIR)/viewerkoe.o
	$(CCC) $(CFLAGS) $(SDIR)/viewerkoe.cpp -o $(OUT) $(OBJS)  $(OSG_LIBS)

test: $(OBJS) $(ODIR)/regiontester.o
	$(CCC) $(CFLAGS) $(SDIR)/regiontester.cpp -o regiontester $(OBJS)  $(OSG_LIBS)

$(ODIR)/%.o: $(SDIR)/%.cpp ${includes}
	$(CCC) -c -o $@ $< $(CFLAGS)



clean:
	rm -f $(ODIR)/*.o
	rm -f *~
