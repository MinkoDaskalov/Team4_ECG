#include "window.h"
#include <cmath>  // for sine stuff
#include <assert.h>

Window::Window() : plot( QString("Heart Rate Monitor") ), count(0) // <-- 'c++ initialisation list' - google it!
{

    button.setText("START RECORD");
    connect(&button, SIGNAL(released()), SLOT(Record()));

    button2.setText("FILTER ON");
    connect(&button2, SIGNAL(released()), SLOT(Filter()));

    label.setText("Real Time Embedded Programming M - Team 4 ECG");
    label.show();  // Works without it as well

    heartRate.setText("Heart Rate: -- bpm");
    heartRate.show();

	// set up the initial plot data
	for( int index=0; index<plotDataSize; ++index )
	{
		xData[index] = index;
        yData[index] = 0;
	}


	// make a plot curve from the data and attach it to the plot
    curve.setSamples(xData, yData, plotDataSize);
    curve.attach(&plot);

    plot.replot();
    plot.show();

    hLayout.addWidget(&button);
    hLayout.addWidget(&button2);
    hLayout.addWidget(&heartRate);

    vLayout.addWidget(&plot);
    vLayout.addLayout(&hLayout);
    vLayout.addWidget(&label);
    setLayout(&vLayout);



    reader = new ADCreader();
    reader->start();

    lp.setup (3, 250, 45); //order,sampling rate, Fc
    lp.reset ();
}


Window::~Window()
{
    // tells the thread to no longer run its endless loop
    reader->quit();
    // wait until the run method has terminated
    reader->wait();
    //	delete adcreader;
}


void Window::timerEvent( QTimerEvent * )
{
    while(reader->hasSample()){


            //Filter or not
            if (flagFilter == 1)
                inVal = lp.filter(reader->getSample());
            else
                inVal = reader->getSample();

            memmove( yData, yData+1, (plotDataSize-1) * sizeof(double) );
            converted = (inVal/65536)*3.3;
            yData[plotDataSize-1] = converted;
            curve.setSamples(xData, yData, plotDataSize);
            ++count;
            ++sampleCount;

            if (sampleCount <= 2500){
                average = ((average*(sampleCount-1))+converted)/sampleCount;
                samples[sampleCount] = converted;
            }

            else
                CalculateBPM();

            //Save to text file if button pressed
            if (flagRecord == 1){
                QTextStream out(&file);
                out<<count<<" "<<converted<<endl;

            }
    }
        plot.replot();
}


void Window::Record(){
    if (flagRecord==0){
        button.setText("STOP RECORD");
        flagRecord = 1;
        file.setFileName(filename);
        file.open(QIODevice::WriteOnly|QIODevice::Text);
        count = 0;
    }
    else{
        button.setText("START RECORD");
        flagRecord = 0;
        file.close();
    }
}


void Window::Filter(){
    if (flagFilter == 0){
        button2.setText("FILTER OFF");
        flagFilter = 1;
    }
    else{
        button2.setText("FILTER ON");
        flagFilter = 0;
    }

}

void Window::CalculateBPM(){

    timeNew = clock()/CLOCKS_PER_SEC;
    int flagBeat = 0;
    int beats = 0;
    double threshold  = average-0.04; // Experimental

    for(int i = 1; i<=2500; i++){
        if (samples[i]<threshold){
            if (flagBeat == 0){
                ++beats;
                flagBeat =1;
            }
        }
        else{
            if (flagBeat == 1)
                flagBeat = 0;
        }
    }
    int elapsedTime = timeNew-timeOld;
    int bpm = beats*60/elapsedTime;

    if (bpm>=60 && bpm <=120)
        heartRate.setText("Heart Rate:"+QString::number(bpm)+" bpm");
    else
        heartRate.setText("Heart Rate: -- bpm");

    sampleCount = 0;
    average = 0;
    timeOld = timeNew;
}
