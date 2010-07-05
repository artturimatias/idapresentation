/* OpenSceneGraph example, osgcompositeviewer.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <iostream>

#include <osg/Notify>
#include <osg/io_utils>

#include <osg/ArgumentParser>
#include <osgDB/WriteFile>
#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgWidget/Browser>


#include <QtWebKit/QWebSettings>
#include <QtWebKit/QtWebKit>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsView>
#include <QtGui/QApplication>
#include <QtGui/QPainter>
#include <QtGui/QtEvents>

#include <osgQt/QGraphicsViewAdapter>
#include <osgQt/QWebViewImage>


#include "smilParser.h"
// Thread that runs the viewer's frame loop as we can't run Qt in the background...
class qtThread : public OpenThreads::Thread
{
    public:

        qtThread(osgViewer::ViewerBase* viewerBase, bool doQApplicationExit):
            _viewerBase(viewerBase),
            _doQApplicationExit(doQApplicationExit) {}

        ~qtThread()
        {
            cancel();
            while(isRunning())
            {
                OpenThreads::Thread::YieldCurrentThread();
            }
        }

        int cancel()
        {
            _viewerBase->setDone(true);
            return 0;
        }

        void run()
        {
            int result = _viewerBase->run();

            if (_doQApplicationExit) QApplication::exit(result);
        }

        osg::ref_ptr<osgViewer::ViewerBase> _viewerBase;
        bool _doQApplicationExit;
};


int main(int argc, char **argv)
{
    // Qt requires that we construct the global QApplication before creating any widgets.
    QApplication app(argc, argv);

    osg::ref_ptr<SmilRegionImage>  reg = new SmilRegionImage(0,0,20,20,-4,"reg1");
    reg->setAlpha(1.0f);
    reg->loadFile("http://opendimension.org/ida");

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer(arguments);
    viewer->setSceneData(reg->getTransform(osg::Vec2(600,600)));
    viewer->setCameraManipulator(new osgGA::TrackballManipulator());
    viewer->addEventHandler(new osgViewer::StatsHandler);

    // start viewer thread
    qtThread qtsaie(viewer.get(), true);
    qtsaie.startThread();
    return QApplication::exec();

        // run the frame loop, interleaving Qt and the main OSG frame loop
      //  while(!viewer->done())
      //  {
            // process Qt events - this handles both events and paints the browser image
      //      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

        //    viewer->frame();
       // }

    //     QApplication::exit();
}
