#ifndef WINDOW_H
#define WINDOW_H

#include <qwt_thermo.h>
#include <qwt_knob.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qpushbutton.h>
#include <qfile.h>
#include <qstring.h>
#include <qwt_text_label.h>
#include <stdio.h>

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

    ~Window(); // Deconstructor

	void timerEvent( QTimerEvent * );

public slots:

    void Record();
    void Filter();

// internal variables for the window class
private:
    void CalculateBPM();


    QwtPlot      plot;
    QwtPlotCurve curve;
    QPushButton  button, button2;
    QString filename = "data.txt";
    QFile file;
    QwtTextLabel label;
    QwtTextLabel heartRate;


	// layout elements from Qt itself http://qt-project.org/doc/qt-4.8/classes.html
    QVBoxLayout  vLayout;  // vertical layout
    QHBoxLayout  hLayout;  // horizontal layout

    static const int plotDataSize =400;
    static const int samplesForBPM = 2500; //20 sec of samples

	// data arrays for the plot
	double xData[plotDataSize];
	double yData[plotDataSize];
    double samples[2*samplesForBPM];

    int flagRecord = 0;
	int count;
    double inVal;
    double converted;
    double average = 0;
    int flagFilter = 0;
    int sampleCount = 0;

    ADCreader *reader;
    Iir::Butterworth::BandStop<3> notch;
};

#endif // WINDOW_H
