#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define pwmOut PA8
#define ledOut PC13
#define ledFrec  PB11
#define ledPower PB10
#define butnPower PA6

#define ledGreenFrec PA7
#define ledRedLevel PB0


#define frcIn  PA0
#define status PC14

#define OLED_RESET 9
Adafruit_SSD1306 display(OLED_RESET);

const int ticSpeed = 40; //us
const int ticConstant = 12500;

const int maxDuty = 36000;
const int minDuty = 0;

const int perTarget   =  208;   //60Hz
const int perMinAbs   =  198;   //70Hz
const int perMaxAbs   =  220;   //50Hz2014q1/bin/../lib/gcc/arm-none-eabi/4.8.3/../../../../arm-none-eabi/lib/armv7-m\libnosys.a(sbrk.o): In function `_sbrk':


#define printsize 100
#define meansize 5
char printbuffer[printsize];
int meanbuffer[meansize];

volatile int pwmDuty = 0;
volatile int perActual = 0;
volatile int perCounter = 0;
volatile int displayCounter=0;
volatile int realfrecuency=0;
volatile int realduty=0;
volatile char c;
volatile bool flagDisplay=false;
volatile bool flagAction=false;

volatile int error = 0;
volatile float errorConst = 2;
volatile int failCounter = 0;
volatile int controlCounter = 0;
volatile int actionCounter=0;
volatile int cycCounter=0;
volatile int perAvrg=0;
volatile int perSum=0;
volatile int modeCount =0;
volatile int modeCounter =0;

void setup()
{
    pwmWrite(pwmOut,0);
    Serial1.begin(115200);
    pinMode(pwmOut, PWM);
    pinMode(butnPower, INPUT_PULLUP);
    pinMode(ledFrec, OUTPUT);
    pinMode(ledPower, OUTPUT);
    pinMode(ledOut, OUTPUT);
    pinMode(status, OUTPUT);
    pinMode(ledRedLevel, OUTPUT);
    pinMode(ledGreenFrec, OUTPUT);

    
    //display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    //display.clearDisplay();
    //display.setTextColor(WHITE);
    //display.setTextSize(4);
    //display.print("PCB");

    attachInterrupt(frcIn, interruptFunction, FALLING);
    HardwareTimer timer1 = HardwareTimer(1);
    timer1.setPrescaleFactor(1);
    timer1.setPeriod(500); //500uS - > 2KHz
    timer_dev *t = TIMER1; //refers t to Timer 8 memory location, how to read back?
    timer_reg_map r = t->regs;

    HardwareTimer timer2 = HardwareTimer(2);
    timer2.pause();
    timer2.setPeriod(ticSpeed);
    timer2.setChannel1Mode(TIMER_OUTPUT_COMPARE);
    timer2.setCompare(TIMER_CH1, 1);  // Interrupt 1 count after each update
    timer2.attachCompare1Interrupt(timerFunction);
    timer2.refresh();
    timer2.resume();
    for (int i=0; i<meansize;i++) {
        meanbuffer[i]=10000;
    }
}


void loop()
{

    
    if (flagDisplay){
        flagDisplay = false;
        realduty = pwmDuty / 36;
        if(perActual != 0) {
            realfrecuency = 125000 / perActual;
        }
        sprintf(printbuffer, "%03d, %03d\r\n", realfrecuency, realduty);
        printStringSerial();
        if (perActual>perMaxAbs) digitalWrite(ledGreenFrec,!digitalRead(ledGreenFrec));
        else digitalWrite(ledGreenFrec,HIGH);


        modeCounter++;
        digitalWrite(ledRedLevel,!digitalRead(ledRedLevel));
        if (modeCounter>2*modeCount) digitalWrite(ledRedLevel, LOW);
        if (modeCounter>10) modeCounter=0;

    }

    if (!digitalRead(butnPower)){
        delay(100);
        if(!digitalRead(butnPower)) {
            modeCount++;
            if (modeCount>3) modeCount=0;
        }
        switch (modeCount) {
            case 0:
                pwmDuty=0;
                break;
            case 1:
                pwmDuty=maxDuty/8 ;
                break;
            case 2:
                pwmDuty=maxDuty/4 ;
                break;
            case 3:
                pwmDuty=maxDuty/2;
                break;
            default:
                pwmDuty=0;
                break;
        }
    }

    
    pwmWrite(pwmOut,pwmDuty);

}

void printStringSerial(){
    for(int i = 0; i<printsize; i++ ){
        if (printbuffer[i] != 0){
            Serial1.print(char(printbuffer[i]));
        }
        else{
                break;
        }
    }
}

void interruptFunction() {
    perActual = perCounter;
    perCounter = 0;
    
    meanbuffer[cycCounter]=perActual;
    cycCounter++;
    
    if (cycCounter>=meansize) cycCounter=0;

    for (int i=0;i<meansize;i++){
        perSum = perSum+meanbuffer[i];
    }
    perAvrg = perSum/meansize;
    perSum = 0;

}

void timerFunction(){
    perCounter++;
    if(perCounter > 10000) {
        perActual = 10000;
        perCounter = 0;
    }

    displayCounter++;
    if(displayCounter > 2500) {
        displayCounter = 0;
        flagDisplay = true;
    }

    actionCounter++;
    if(actionCounter>1250){
        actionCounter=0;
        flagAction=true;
    }
}

