#ifndef WINDOW_H
#define WINDOW_H

#include <qwt_thermo.h>
#include <qwt_knob.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qpushbutton.h>
#include <qfile.h>
#include <qstring.h>

#include <QBoxLayout>
#include "adcreader.h"
#include "Iir.h"


// class definition 'Window'
class Window : public QWidget
{
	// must include the Q_OBJECT macro for for the Qt signals/slots framework to work with this class
	Q_OBJECT

public:
	Window(); // default constructor - called when a Window is declared without arguments

	void timerEvent( QTimerEvent * );
    ADCreader *reader;
    Iir::Butterworth::LowPass<3> lp;

public slots:

    void Record();
    void Filter();

// internal variables for the window class
private:
	QwtPlot      plot;
	QwtPlotCurve curve;
    QPushButton  button, button2;
    QString filename = "data.txt";
    QFile file;



	// layout elements from Qt itself http://qt-project.org/doc/qt-4.8/classes.html
	QVBoxLayout  vLayout;  // vertical layout
	QHBoxLayout  hLayout;  // horizontal layout

    static const int plotDataSize =400;

	// data arrays for the plot
	double xData[plotDataSize];
	double yData[plotDataSize];

    int flag = 0;
	int count;
    double inVal;
    double converted;
    int flag2 = 0;

};

#endif // WINDOW_H
