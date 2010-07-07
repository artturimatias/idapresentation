# enable this in order to build with webkit 
QTWEBKIT=0

IDIR= include
ODIR= src/obj
SDIR= src
OSG_LIBS= -losg -losgUtil -losgViewer -losgAnimation -ltinyxml -lcurl  
CFLAGS= -W -Wall -g -I$(IDIR) 
ifeq ($(QTWEBKIT),1)
    CPPFLAGS= -DQTWEBKIT=1
    CFLAGS+=`pkg-config QtWebKit --libs --cflags`
    OSG_LIBS+=-losgQt
endif
CCC= g++
OUT= viewer
includes = $(wildcard include/*.h)
OBJ = smilNet.o baseRegion.o smilRegion.o smilRegionText.o smilRegion3D.o smilRegionImage.o smilParser.o
OBJS = $(patsubst %,$(ODIR)/%,$(OBJ))


viewer: $(OBJS) $(ODIR)/viewerkoe.o
	$(CCC) $(CFLAGS) $(CPPFLAGS) $(SDIR)/viewerkoe.cpp -o $(OUT) $(OBJS)  $(OSG_LIBS)

test: $(OBJS) $(ODIR)/regiontester.o
	$(CCC) $(CFLAGS) $(SDIR)/regiontester.cpp -o regiontester $(OBJS)  $(OSG_LIBS)

browser: $(OBJS) $(ODIR)/browsertest.o
	$(CCC) $(CFLAGS) $(SDIR)/browsertest.cpp -o browsertest $(OBJS)  $(OSG_LIBS) 


$(ODIR)/%.o: $(SDIR)/%.cpp ${includes}
	$(CCC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS) 


clean:
	rm -f $(ODIR)/*.o
	rm -f *~
