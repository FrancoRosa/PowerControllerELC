#define pwmOut PA8
#define ledOut PC13

#define frcIn  PA0
#define status PC14

const int ticSpeed = 40; //us
const int ticConstant = 12500;

const int maxDuty = 36000;
const int minDuty = 0;

const int perTarget   =  208;   //60Hz
const int perMinAbs   =  192;   //65Hz
const int perMaxAbs   =  227;   //55Hz2014q1/bin/../lib/gcc/arm-none-eabi/4.8.3/../../../../arm-none-eabi/lib/armv7-m\libnosys.a(sbrk.o): In function `_sbrk':


#define printsize 100
char printbuffer[printsize];

volatile int pwmDuty = 0;
volatile int pwmValue = 0;
volatile int perActual = 0;
volatile int perCounter = 0;
volatile int displayCounter=0;
volatile int realfrecuency=0;
int realduty=0;
volatile char c;
volatile bool flagDisplay=false;

volatile int failCounter = 0;
volatile int controlCounter = 0;
volatile int actionCounter=0;

volatile int error = 0;
volatile int errorConst = 1;

void setup()
{
    Serial1.begin(115200);
    pinMode(pwmOut, PWM);
    pinMode(ledOut, OUTPUT);
    pinMode(status, OUTPUT);
    attachInterrupt(frcIn, interruptFunction, FALLING);
    HardwareTimer timer1 = HardwareTimer(1);
    timer1.setPrescaleFactor(1);
    timer1.setPeriod(250); //500uS - > 2KHz
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
}


void loop()
{

    if (flagDisplay){
        flagDisplay = false;
        realduty = pwmValue / 36;
        if(perActual != 0) {
            realfrecuency = 125000 / perActual;
        }
        printStringSerial(realduty,realfrecuency);
        workingOk();
    }
}

void workingOk(){
    if (perActual>perMinAbs && perActual<perMaxAbs) {
            failCounter = 0;
            controlCounter++;
            digitalWrite(status, HIGH);
        }
    else {
            controlCounter = 0;
            failCounter++;
    }
            
    if (failCounter>=20){failCounter=20;digitalWrite(status, LOW);}
    if (controlCounter>=20){controlCounter=20;digitalWrite(status, HIGH);}

}

void printStringSerial(int realduty, int realfrecuency){
    sprintf(printbuffer, "%03d, %03d\r\n", realduty, realfrecuency);
    for(int i = 0; i<printsize; i++ ){
        if (printbuffer[i] != 0){
            Serial1.print(char(printbuffer[i]));
        }
        else{
                break;
        }
    }
}

void controlFunction(){
    /*
    Ej:
    error = 300 - 200; running slow, e>0
    error = 100 - 200; running fast, e<0
    error = 200 - 200;
    */

    error = perActual - perTarget;
    if (error > 0){
        pwmDuty = pwmDuty - (error * errorConst);
        if(pwmDuty<minDuty) pwmDuty = minDuty;
    }

    if (error < 0){
        pwmDuty = pwmDuty + (error * errorConst);
        if(pwmDuty>maxDuty) pwmDuty = maxDuty;
    }

    pwmValue = pwmDuty;
    pwmWrite(pwmOut,pwmDuty);

}
void interruptFunction() {
    perActual = perCounter;
    perCounter = 0;
    
    /*
    meanbuffer[cycCounter]=perActual;
    cycCounter++;
    
    if (cycCounter>=meansize) cycCounter=0;

    for (int i=0;i<meansize;i++){
        perSum = perSum+meanbuffer[i];
    }
    perAvrg = perSum/meansize;
    perSum = 0;
    */
    //controlFunction();
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
    if(actionCounter>312){ 
        actionCounter=0;
        controlFunction();
    }
}

