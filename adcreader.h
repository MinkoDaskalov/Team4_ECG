#ifndef ADCREADER
#define ADCREADER

#include <QThread>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "gz_clk.h"
#include "gpio-sysfs.h"
#include <assert.h>

class ADCreader : public QThread
{
public:
    ADCreader();
    // ring buffer functions
    int hasSample();
    int getSample();

	void quit();
	void run();

    void writeReset(int fd);
    void writeReg(int fd, uint8_t v);
    uint8_t readReg(int fd);
    int readData(int fd);

private:
    bool running;
    void pabort(const char *s);

    // file descriptor on the SPI interface
    int fd;

    // file descriptor on the interrupt pin
    int sysfs_fd;

    // data collected
    int *samples;

    // pointer to ringbuffer
    int *pIn;
    int *pOut;
    uint8_t mode;
    uint8_t bits;
    uint32_t speed;
    uint16_t delay;
    int drdy_GPIO;


};

#endif
