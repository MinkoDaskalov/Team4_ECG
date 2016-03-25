#include "adcreader.h"
#include <QDebug>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define MAX_SAMPLES 65536

ADCreader::ADCreader()
{
  int ret = 0;

  // set up ringbuffer
  samples = new int[MAX_SAMPLES];
  // pointer for incoming data
  pIn = samples;
  // pointer for outgoing data
  pOut = samples;

  // SPI constants
  static const char *device = "/dev/spidev0.0";
  mode = SPI_CPHA | SPI_CPOL;
  bits = 8;
  speed = 50000;
  delay = 10;
  drdy_GPIO = 22;

  // open SPI device
  fd = open(device, O_RDWR);
  if (fd < 0)
    pabort("can't open device");

  /*
   * spi mode
   */
  ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
  if (ret == -1)
    pabort("can't set spi mode");

  ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
  if (ret == -1)
    pabort("can't get spi mode");

  /*
   * bits per word
   */
  ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
  if (ret == -1)
    pabort("can't set bits per word");

  ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
  if (ret == -1)
    pabort("can't get bits per word");

  /*
   * max speed hz
   */
  ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
  if (ret == -1)
    pabort("can't set max speed hz");

  ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
  if (ret == -1)
    pabort("can't get max speed hz");

  fprintf(stderr, "spi mode: %d\n", mode);
  fprintf(stderr, "bits per word: %d\n", bits);
  fprintf(stderr, "max speed: %d Hz (%d KHz)\n", speed, speed/1000);

  // enable master clock for the AD
  // divisor results in roughly 4.9MHz
  // this also inits the general purpose IO
  gz_clock_ena(GZ_CLK_5MHz,5);

  // enables sysfs entry for the GPIO pin
  gpio_export(drdy_GPIO);
  // set to input
  gpio_set_dir(drdy_GPIO,0);
  // set interrupt detection to falling edge
  gpio_set_edge(drdy_GPIO,"falling");
  // get a file descriptor for the GPIO pin
  sysfs_fd = gpio_fd_open(drdy_GPIO);

  // resets the AD7705 so that it expects a write to the communication register
  writeReset(fd);
  writeReg(fd,0x00);

  // tell the AD7705 that the next write will be to the clock register
  writeReg(fd,0x20);
  // write 00001110 : CLOCKDIV=1,CLK=1,expects 4.9152MHz input clock
  writeReg(fd,0x0E);

  // tell the AD7705 that the next write will be the setup register
  writeReg(fd,0x10);
  // intiates a self calibration and then after that starts converting
  writeReg(fd,0x44);
}

void ADCreader::writeReset(int fd)
{
  int ret;
  uint8_t tx1[5] = {0xff,0xff,0xff,0xff,0xff};
  uint8_t rx1[5] = {0};
  struct spi_ioc_transfer tr;

  memset(&tr,0,sizeof(struct spi_ioc_transfer));
  tr.tx_buf = (unsigned long)tx1;
  tr.rx_buf = (unsigned long)rx1;
  tr.len = sizeof(tx1);

  ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
  if (ret < 1) {
   printf("\nerr=%d when trying to reset. \n",ret);
   pabort("Can't send spi message");
  }
}

void ADCreader::writeReg(int fd, uint8_t v)
{
  int ret;
  uint8_t tx1[1];
  tx1[0] = v;
  uint8_t rx1[1] = {0};
  struct spi_ioc_transfer tr;

  memset(&tr,0,sizeof(struct spi_ioc_transfer));
  tr.tx_buf = (unsigned long)tx1;
  tr.rx_buf = (unsigned long)rx1;
  tr.len = sizeof(tx1);

  ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
  if (ret < 1)
    pabort("can't send spi message");
}

uint8_t ADCreader::readReg(int fd)
{
    int ret;
    uint8_t tx1[1];
    tx1[0] = 0;
    uint8_t rx1[1] = {0};
    struct spi_ioc_transfer tr;

    memset(&tr,0,sizeof(struct spi_ioc_transfer));
    tr.tx_buf = (unsigned long)tx1;
    tr.rx_buf = (unsigned long)rx1;
    tr.len = sizeof(tx1);

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1)
      pabort("can't send spi message");

    return rx1[0];
}

int ADCreader::readData(int fd)
{
    int ret;
    uint8_t tx1[2] = {0,0};
    uint8_t rx1[2] = {0,0};
    struct spi_ioc_transfer tr;

    memset(&tr,0,sizeof(struct spi_ioc_transfer));
    tr.tx_buf = (unsigned long)tx1;
    tr.rx_buf = (unsigned long)rx1;
    tr.len = sizeof(tx1);

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1){
      printf("\n can't send spi message, ret = %d\n",ret);
      exit(1);
      }

    return (rx1[0]<<8)|(rx1[1]);
}

void ADCreader::run()
{
    running = true;

    while (running) {

        //ask for a read from the comms register
        writeReg(fd,0x08);

        int x = readReg(fd);

        //check for the data ready bit to be low - /DRDY is glitchy on the board! use this instead
        if (!(x & 0x80)){

        // tell the AD7705 to read the data register (16 bits)
        writeReg(fd,0x38);
        // read the data register by performing two 8 bit reads
        int value = readData(fd)-0x8000;


        *pIn = value;
        if (pIn == (&samples[MAX_SAMPLES-1]))
          pIn = samples;
        else
          pIn++;
        }
    }

    close(fd);
    gpio_fd_close(sysfs_fd);
}

int ADCreader::getSample()
{
  assert(pOut!=pIn);
  int value = *pOut;
  if (pOut == (&samples[MAX_SAMPLES-1]))
    pOut = samples;
  else
    pOut++;
  return value;
}


int ADCreader::hasSample()
{
  return (pOut!=pIn);
}


void ADCreader::quit()
{
    running = false;
    exit(0);
}

void ADCreader::pabort(const char *s)
{
    perror(s);
    abort();
}



