#include <QtGui>
#include "mset.h"

// for our timer function:
#include <time.h>
#include <inttypes.h>

// for debugging:
#include <iostream>
using std::cout;
using std::endl;


// hack: keep a variable to let us know if the application
// has been initialized so that we can draw the set on startup
bool drawnYet = false;


// In case you want to time your routines, the following function
// should give you a microsecond timer.
uint64_t clockGetTime_mu()
{
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000000LL + (uint64_t)ts.tv_nsec / 1000LL;
}

mandelbrotArea::mandelbrotArea(QWidget *parent) : QWidget(parent)
{
	// set our inital view to hold the entire mandelbrot set.
	// a 3x3 square with -2,-1.5 as the lower left should work.
	llCoord.real = -2;
	llCoord.imag = -1.5;
	windowWidth = 3;
	this->maxIterations = 200; // a decent default
	setAttribute(Qt::WA_StaticContents);
}

bool mandelbrotArea::saveImage(const QString &fileName, const char *fileFormat)
{
	QImage visibleImage = image;
	resizeImage(&visibleImage, size());
	visibleImage.save(fileName, fileFormat);
	return true;
}

void mandelbrotArea::refreshImage()
{
	// this is just an example.  by making this function a "slot" (see the
	// header file) we can easily connect it to events from other objects
	// just as an illustration, we'll draw the gradient the other way.
	QPainter qp(&image);
	QColor qc;
	QPen qpen;
	// get the dimensions of our image in terms of pixels:
	unsigned long iwidth = image.width();
	unsigned long iheight = image.height();
	double unit = 1.0 / iwidth; // on a scale of 0-1, how wide is a pixel?
	for (unsigned long i = 0; i < iwidth; i++) {
		qc.setRgbF(i*unit,sqrt(i*unit),i*unit); // set the color we want to draw.
		qpen.setColor(qc); // apply the color to the pen
		qp.setPen(qpen);   // set the painter to use that pen
		qp.drawLine(iwidth-i-1,0,iwidth-i-1,iheight); // draw a line of the specified color.
	}
	update(); // repaint screen contents
	return;
}

void mandelbrotArea::mouseDoubleClickEvent(QMouseEvent *event)
{
	// TODO: write the code for the zoom operation.  You can get
	// the mouse button from event->button() and check for
	// event->button() == Qt::LeftButton, etc. to figure out what
	// button is being pressed.

	double x = event -> x();
	double y = event -> x();
	double w = image.width();
	double h = image.width();
	double unit = windowWidth/ w;
	double unitY = windowWidth / h;

	if(event -> button() == Qt::LeftButton) {
		w /= 4;
		h /= 4;
		this -> maxIterations +=100;

		windowWidth /= 2;
	}
	else if(event -> button() == Qt::RightButton) {
		this -> maxIterations -= 100;
		windowWidth *= 2;
	}
	else {
		w /= 2;
		h /= 2;
	}
	llCoord.real += (y-w)*unit;
	llCoord.imag -= (y-h)*unitY;
	render();

}

void mandelbrotArea::mousePressEvent(QMouseEvent *event)
{
	// if you want your application to respond to mouse
	// clicks, implement this function.  You can figure out
	// which mouse button was pressed by examining event->button()
	// and you can get the position from event->pos()
}

void mandelbrotArea::mouseMoveEvent(QMouseEvent *event)
{
	// This will be called whenever the mouse moves across your
	// window.  Might be useful for dragging the image, should
	// you decide to implement it.
}

void mandelbrotArea::mouseReleaseEvent(QMouseEvent *event)
{
	// this would be useful to implement the drag operation.
}

void mandelbrotArea::paintEvent(QPaintEvent *event)
{
	// check: if blank, render first.
	if (!drawnYet) {
		drawnYet = true;
		this->render();
	}
	// otherwise, just repaint the portion of the screen that needs it.
	QPainter painter(this);
	QRect dirtyRect = event->rect();
	painter.drawImage(dirtyRect, image, dirtyRect);
}

void mandelbrotArea::resizeEvent(QResizeEvent *event)
{
	resizeImage(&image, QSize(width(), height()));
	update();
	QWidget::resizeEvent(event);
	return;
}

void mandelbrotArea::render()
{
	// TODO: write this function.
	// draw the portion of the mandelbrot set that lies in our window.
	// idea: loop through all the pixels, iterate the function figure out how
	// quickly the point is diverging, and color it based on the result

	// to give you some idea of what you need to do, I've drawn a nice green
	// gradient on the screen.  You will want to set the individual pixels
	// using the qp.drawPoint(x,y) function, instead of drawing a line.
	QPainter qp(&image);
	QColor qc;
	QPen qpen;
	// get the dimensions of our image in terms of pixels:
	unsigned long iwidth = image.width();
	unsigned long iheight = image.height();
	double unit = 1.0 / iwidth; // on a scale of 0-1, how wide is a pixel?
	for (unsigned long i = 0; i < iwidth; i++) {
		for (unsigned long j = 0; j < iheight; j++) {
		/*
		qc.setRgbF(i*unit,sqrt(i*unit),i*unit); // set the color we want to draw.
		qpen.setColor(qc); // apply the color to the pen
		qp.setPen(qpen);   // set the painter to use that pen
		qp.drawLine(i,0,i,iheight); // draw a line of the specified color.

		double x = ((double)i- (double)centerX)*unit*3;
		double y = ((double)j- (double)centerY)*unitY*3;
*/
			double x = llCoord.real+((double)i*unit);
			double y = llCoord.imag-((double)j*unit);
		complex c = complex(x,y);
		complex z;
		unsigned long iter;
		for(iter = 0; iter < maxIterations && z.real*z.real + z.imag*z.imag < 4.0;
		iter++) {
			z = z*z +c;
		}
		long double perc = (double)iter/(double)maxIterations;
		perc = (perc == 1) ? 0 : perc;
		qc.setRgbF(perc, sqrt(perc), perc);
		qpen.setColor(qc);
		qp.setPen(qpen);
		qp.drawPoint(i,j);
		}
	}
	update(); // repaint screen contents
	return;
}

void mandelbrotArea::resizeImage(QImage *image, const QSize &newSize)
{
	// your custom resize code would go here...
	if (image->size() == newSize)
		return;

	// for now, we will just expand our image with blank (white) space.
	QImage newImage(newSize, QImage::Format_RGB32);
	newImage.fill(qRgb(255, 255, 255));
	QPainter painter(&newImage);
	painter.drawImage(QPoint(0, 0), *image);
	*image = newImage;
}

unsigned long mandelbrotArea::getIterations()
{
	/* NOTE: this is used by MainWindow for the initial value to place
	 * in the iteration control dialog. */
	return this->maxIterations;
}

void mandelbrotArea::setIterations(unsigned long newMax)
{
	/* NOTE: this is called by MainWindow once the iterations
	 * dialog has been processed. */
	this->maxIterations = newMax;
}
