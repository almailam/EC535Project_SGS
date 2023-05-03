// BU EC535 Project - Smart Gardening System (SGS)
// Abdulaziz AlMailam & Samuel Gulinello

// INCLUDE DIRECTIVES

// QT
#include "mainwindow.h"
#include <QLabel>
#include <QTimer>
#include <QThread>
#include <QPushButton>
#include <QProcess>

// LibC/Linux
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

using namespace std;



// Program definitions
#define ADDRESS_BME 0x76
#define ADDRESS_ADS 0x48

char *filename = "/dev/i2c-2"; // I2C Device bus connected to the sensors
int file;

typedef signed int BME280_S32_t;
typedef unsigned int BME280_U32_t;
typedef long long signed int BME280_S64_t;

// Global sensor value inputs
BME280_S32_t t_fine, temp_global = 22.5, humidity_global = 40;
int water_global = 0;

// QT text label for temperature, humidity, and water level
QLabel *temp_humidity_water_label;



// ADS1115 I2C ADC SENSOR

// Helper for ADS1115 to map and convert raw readings
int ads_map(int value) {
    int result = (value + 42) * 100 / 152;
    if (result < 0) {
        result = 0;
    } else if (result > 100) {
        result = 100;
    }
    return result;
}

// Handler for ADS1115 I2C ADC Interactions
int ADS1115_read() {
    /* Create I2C Bus*/
    if ((file = open(filename, O_RDWR)) < 0) {
        /* ERROR HANDLING: you can check errno to see what went wrong */
        perror("Failed to open the i2c bus");
        exit(1);
    }

    /* Get ADS1115 Device*/
    if (ioctl(file, I2C_SLAVE, ADDRESS_ADS) < 0) {
        printf("Failed to acquire bus access and/or talk to slave.\n");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        exit(1);
    }

    /* The following 6 lines are borrowed from https://github.com/ControlEverythingCommunity/ADS1115/blob/master/C/ADS1115.c*/
    // Select configuration register(0x01)
    // AINP = AIN0 and AINN = AIN1, +/- 2.048V
    // Continuous conversion mode, 128 SPS(0x84, 0x83)
    char config[3] = {0};
    config[0] = 0x01;
    config[1] = 0x84;
    config[2] = 0x83;
    write(file, config, 3);

    // Read Data from ADS1115
    char reg[1] = {0x00};
    write(file, reg, 1);
    char data[2]={0};
    if(read(file, data, 2) != 2) {
        printf("Error : Input/Output error \n");
        exit(1);
    }

    // Convert the data
    int raw_adc = ((data[0] << 8) + data[1]);
    if (raw_adc > 32767) raw_adc -= 65535;

    close(file);

    water_global = ads_map(raw_adc / 100);

    return water_global;
}



// BME280 TEMPERATURE/HUMIDITY/PRESSURE I2C SENSOR

// Handler for BME280 Temp/Humidity I2C Sensor Interactions
void bme280_(BME280_S32_t *temp, BME280_S32_t *humidity) {
    /* Create I2C Bus*/
    if ((file = open(filename, O_RDWR)) < 0) {
        /* ERROR HANDLING: you can check errno to see what went wrong */
        perror("Failed to open the i2c bus");
        exit(1);
    }

    /* Get BME280 Device*/
    if (ioctl(file, I2C_SLAVE, ADDRESS_BME) < 0) {
        printf("Failed to acquire bus access and/or talk to slave.\n");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        exit(1);
    }

    // Read 24 bytes of data from register(0x88)
    // Calibration Data
    char reg[1] = {0x88};
    write(file, reg, 1);
    char data[24] = {0};
    if(read(file, data, 24) != 24) {
        printf("Error : Input/Output error \n");
        exit(1);
    }

    /* Format data from calibration register */
    int dig_T1 = (data[0] + (data[1] << 8));
    int dig_T2 = (data[2] + (data[3] << 8));
    int dig_T3 = (data[4] + (data[5] << 8));
    int dig_P1 = (data[6] + (data[7] << 8));
    int dig_P2 = (data[8] + (data[9] << 8));
    int dig_P3 = (data[10] + (data[11] << 8));
    int dig_P4 = (data[12] + (data[13] << 8));
    int dig_P5 = (data[14] + (data[15] << 8));
    int dig_P6 = (data[16] + (data[17] << 8));
    int dig_P7 = (data[18] + (data[19] << 8));
    int dig_P8 = (data[20] + (data[21] << 8));
    int dig_P9 = (data[22] + (data[23] << 8));

    /* Get Humidity Data */
    reg[0] = 0xA1;
    write(file, reg, 1);
    char hdata[8] = {0};
    read(file, hdata, 1);
    int dig_H1 = hdata[0];

    reg[0] = 0xE1;
    hdata[0] = 0b0000;
    write(file, reg, 1);
    read(file, hdata, 7);

    /* Format Humidity Data */
    int dig_H2 = (hdata[0] + (hdata[1] * 256));
    int dig_H3 = hdata[2] & 0xFF ;
    int dig_H4 = (hdata[3] * 16 + (hdata[4] & 0xF));
    int dig_H5 = (hdata[4] / 16) + (hdata[5] * 16);
    int dig_H6 = hdata[6];

    /* The following 28 lines of code is borrowed from https://github.com/ControlEverythingCommunity/BME280/blob/master/C/BME280.c*/
    // Select control humidity register(0xF2)
    // Humidity over sampling rate = 1(0x01)
    char config[2] = {0};
    config[0] = 0xF2;
    config[1] = 0x01;
    write(file, config, 2);
    // Select control measurement register(0xF4)
    // Normal mode, temp and pressure over sampling rate = 1(0x27)
    config[0] = 0xF4;
    config[1] = 0x27;
    write(file, config, 2);
    // Select config register(0xF5)
    // Stand_by time = 1000 ms(0xA0)
    config[0] = 0xF5;
    config[1] = 0xA0;
    write(file, config, 2);

    // Read 8 bytes of data from register(0xF7)
    // pressure msb1, pressure msb, pressure lsb, temp msb1, temp msb, temp lsb, humidity lsb, humidity msb
    reg[0] = 0xF7;
    write(file, reg, 1);
    read(file, data, 8);

    // Convert pressure and temperature data to 19-bits
    long adc_p = ((long)(data[0] * 65536 + ((long)(data[1] * 256) + (long)(data[2] & 0xF0)))) / 16;
    long adc_t = ((long)(data[3] * 65536 + ((long)(data[4] * 256) + (long)(data[5] & 0xF0)))) / 16;
    // Convert the humidity data
    long adc_h = (data[6] * 256 + data[7]);

    // Convert raw data into desired values - Code borrowed from https://www.mouser.com/datasheet/2/783/BST-BME280-DS002-1509607.pdf
    BME280_S32_t var1, var2;
    var1 = ((((adc_t >> 3) - ((BME280_S32_t) dig_T1 << 1))) * ((BME280_S32_t) dig_T2)) >> 11;
    var2 = (((((adc_t >> 4) - ((BME280_S32_t) dig_T1)) * ((adc_t >> 4) - ((BME280_S32_t) dig_T1))) >> 12) *
            ((BME280_S32_t) dig_T3)) >> 14;
    t_fine = var1 + var2;
    *temp = ((t_fine * 5 + 128) >> 8) / 100;

    BME280_S64_t var3, var4, p;
    var3 = (BME280_S64_t)t_fine - (BME280_S64_t)128000;
    var4 = var3 * var3 * (BME280_S64_t) dig_P6;
    var4 = var4 + ((var3 * (BME280_S64_t) dig_P5) << 17);
    var4 = var4 + (((BME280_S64_t) dig_P4) << 35);
    var3 = ((var3 * var3 * (BME280_S64_t) dig_P3) >> 8) + ((var3 * (BME280_S64_t) dig_P2) << 12);
    var3 = (((((BME280_S64_t) 1) << 47) + var3))*((BME280_S64_t) dig_P1) >> 33;

    p = 1048576 - adc_p;
    p = (((p << 31) - var4)*3125) / var3;
    var3 = (((BME280_S64_t) dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var4 = (((BME280_S64_t) dig_P8) * p) >> 19;
    p = ((p + var3 + var4) >> 8) + (((BME280_S64_t) dig_P7) << 4);
    p = (BME280_U32_t) p;

    BME280_S32_t v_x1_u32r;
    v_x1_u32r = t_fine - (BME280_S32_t)76800;
    v_x1_u32r = (((((adc_h << 14) - (((BME280_S32_t) dig_H4) << 20) - (((BME280_S32_t) dig_H5) * v_x1_u32r)) +
                   ((BME280_S32_t) 16384)) >> 15) * (((((((v_x1_u32r * ((BME280_S32_t) dig_H6)) >> 10) * (((v_x1_u32r *
                                                                           ((BME280_S32_t) dig_H3)) >> 11) + ((BME280_S32_t) 32768))) >> 10) + ((BME280_S32_t) 2097152)) *
                         ((BME280_S32_t) dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((BME280_S32_t) dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > (BME280_S32_t)419430400 ? (BME280_S32_t)419430400 : v_x1_u32r);
    *humidity = ((BME280_U32_t) (v_x1_u32r >> 12)) / 1024;

    close(file);
}

// Updates and returns the (global) temperature value from the sensor
BME280_S32_t get_temp() {
    BME280_S32_t temp;
    BME280_S32_t humidity;

    bme280_(&temp, &humidity);

    temp_global = temp;
    return temp;
}

// Updates and returns the (global) humidity value from the sensor
BME280_S32_t get_humidity() {
    BME280_S32_t temp;
    BME280_S32_t humidity;

    bme280_(&temp, &humidity);

    humidity_global = humidity;
    return humidity;
}



// SIGNAL HANDLING //

// Run the pumpToggle output control script to enable the pump output if the on-screen pump button is pressed
void MainWindow::pump_button_pressed_response() { QProcess process; process.startDetached("/bin/sh", QStringList()<< "/root/project/pumpToggle.sh"); }

// Run the pumpToggle output control script to disable the pump output if the on-screen pump button is released
void MainWindow::pump_button_release_response() { QProcess process; process.startDetached("/bin/sh", QStringList()<< "/root/project/pumpToggle.sh"); }

// Run the LEDToggle output control script to enable the LED output if the on-screen LED button is pressed
void MainWindow::LED_button_pressed_response() { QProcess process; process.startDetached("/bin/sh", QStringList()<< "/root/project/LEDToggle.sh"); }

// Run the LEDToggle output control script to disable the LED output if the on-screen LED button is released
void MainWindow::LED_button_release_response() { QProcess process; process.startDetached("/bin/sh", QStringList()<< "/root/project/LEDToggle.sh"); }



// TIMER FUNCTIONALITY

// Refreshes the display after reading new temperature, humidity, and water level values from the sensors
void MainWindow::updateDisplay() {
    temp_global = get_temp();
    humidity_global = get_humidity();
    water_global = ADS1115_read();
    string th_disp_string = "Temperature: " + std::to_string(temp_global) + "C\nHumidity: " + std::to_string(humidity_global) + "%\nWater Level: " + std::to_string(water_global) + "%";
    temp_humidity_water_label -> setText(QString::fromStdString(th_disp_string));
}



// MAINWINDOW SYSTEM FUNCTION

// MainWindow function to drive all system functionality
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {

    // INITIALIZE SENSOR DATA DISPLAY
    temp_humidity_water_label = new QLabel(this);
    string th_disp_string = "Temperature: " + std::to_string(temp_global) + "C\nHumidity: " + std::to_string(humidity_global) + "%\nWater Level: " + std::to_string(water_global) + "%";
    temp_humidity_water_label -> setGeometry(QRect(100, 30, 90, 120));
    temp_humidity_water_label -> setFixedWidth(500);


    // OUTPUT CONTROL
    QPushButton *pump_button = new QPushButton(this); // Pump is connected to GPIO67
    QPushButton *LED_button = new QPushButton(this);  // Pump is connected to GPIO66

    pump_button -> setText(tr("Pump"));
    LED_button -> setText(tr("LED"));

    // Initialize outputs to off (0)
    MainWindow::pump_button_release_response();
    MainWindow::LED_button_release_response();

    // Connect button signals to their respective slots to control the output behavior
    connect(pump_button, SIGNAL(pressed()), this, SLOT(pump_button_pressed_response()));
    connect(pump_button, SIGNAL(released()), this, SLOT(pump_button_release_response()));
    connect(LED_button, SIGNAL(pressed()), this, SLOT(LED_button_pressed_response()));
    connect(LED_button, SIGNAL(released()), this, SLOT(LED_button_release_response()));

    // Update the position of the pump and LED buttons
    pump_button -> setGeometry(QRect(110, 150, 70, 70));
    LED_button -> setGeometry(QRect(230, 150, 70, 70));


    // START LOOP TIMER
    QTimer *timer = new QTimer(this);
    timer->setInterval(250); // Timer fires every 250ms so the screen refreshes 4x every second
    connect(timer, SIGNAL(timeout()), this, SLOT(updateDisplay()));
    timer->start();
}

// mainwindow destructor
MainWindow::~MainWindow()
{

}

