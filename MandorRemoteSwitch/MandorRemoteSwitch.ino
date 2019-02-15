//Remote PowerHouse

// Code for data streammer remote relay control and display management
//
//Read UART from powerMeter (rs485)
//Read UART from Controller (rs485)
//show frecuency and load on screen 
//
#define displaySize 20
#define remoteSize 10
#define controllerSize 20
#define led PC13
#define onRelay  PB13
#define offRelay PB12

char displayBuffer[displaySize];
volatile int dbuffer=0;

char remoteBuffer[remoteSize];
volatile int rbuffer=0;

char controllerBuffer[controllerSize];
volatile int cbuffer=0;

int frecuency = 591;
int duty = 791;

const char onCommand[] =  "on";
const char offCommand[] =  "off";

volatile bool onFlag = false;
volatile bool offFlag = false;
volatile bool displayFlag = false;
volatile bool delayFlag = false;
volatile int ndata = 0;

void setup(){
    pinMode(led, OUTPUT);
    pinMode(onRelay, OUTPUT);
    pinMode(offRelay, OUTPUT);
    digitalWrite(onRelay, HIGH);
    digitalWrite(offRelay, HIGH);
    Serial.begin(115200); //For debugging purpouses
    Serial1.begin(115200); //Connects controller (reads frecuency and duty ej: 589, 1000)
    Serial2.begin(9600);   //Connects to RF transeiver (reads commands for relay control on/off)
    Serial3.begin(9600);   //Connects to display
    Serial.print("USB - Debug\n\r");
    Serial1.print("UART1 - Controller\n\r");
    Serial2.print("UART2 - RF\n\r");
    Serial3.print("UART3 - Display\n\r");}

void loop(){
    if (Serial2.available()) {
    processingRemoteData(Serial2.read());
    }

    if (Serial3.available()) {
    processingRemoteData(Serial3.read());
    }

    if (Serial1.available()) {
    processingControllerData(Serial1.read());
    }

    if(displayFlag){
        displayFlag=false;
        controllerToDisplay();
        printStringRF();
    }

    if (delayFlag){
        delay(100);
        digitalWrite(onRelay, HIGH);
        digitalWrite(offRelay, HIGH);
        delayFlag=false;}
}

void controllerToDisplay(){
    sprintf(displayBuffer, "t3.txt=\"%c%c.%c\"\xFF\xFF\xFF", 
        controllerBuffer[0],controllerBuffer[1],controllerBuffer[2]);
    printStringDisplay();
    sprintf(displayBuffer, "t4.txt=\"%c%c.%c\"\xFF\xFF\xFF", 
        controllerBuffer[5],controllerBuffer[6],controllerBuffer[7]);
    printStringDisplay();
}
void refreshDisplay(int frec, int duty){
    valueRefreshFrec(frecuency); delay(100);
    valueRefreshDuty(duty); delay(100);

}

void valueRefreshFrec(int frec){
    sprintf(displayBuffer, "t3.txt=\"%d.%d\"\xFF\xFF\xFF", frec/10,frec%10);
    printStringDisplay();
}

void valueRefreshDuty(int duty){
    sprintf(displayBuffer, "t4.txt=\"%d.%d\"\xFF\xFF\xFF", duty/10,duty%10);
    printStringDisplay();
}

void printStringDisplay(){
    for(int i = 0; i<displaySize; i++ ){
        if (displayBuffer[i] != 0){
            Serial3.print(char(displayBuffer[i]));
        }
        else{
                break;
        }
    }
}

void printStringRF(){
    for(int i = 0; i<controllerSize; i++ ){
        if (controllerBuffer[i] != 0){
            Serial2.print(char(controllerBuffer[i]));
        }
        else{
                break;
        }
    }
}
void processingRemoteData(char c){
    remoteBuffer[rbuffer] = c;
    rbuffer++;
    if ((c == '\n')||(c == '\r')) {
        if (rbuffer>2){
            comparator();
            if (onFlag){ 
                digitalWrite(onRelay, LOW);
                onFlag=false;
                delayFlag=true;
            }
            if (offFlag){
                digitalWrite(offRelay, LOW);
                offFlag=false;
                delayFlag=true;
            }
        }
        rbuffer=0;
    }
    if (rbuffer>=remoteSize) rbuffer=0;
    
}

void processingControllerData(char c){
    controllerBuffer[cbuffer] = c;
    cbuffer++;
    if ((c == '\n')||(c == '\r')) {
        if (cbuffer>8){
        ndata++;
        if (ndata>=10){
            displayFlag=true;
            ndata=0;}
        }
        cbuffer=0;
    }
    if (cbuffer>=remoteSize) cbuffer=0;
    
}

void comparator(void){
    Serial.print("bufferZise:");
    Serial.println(rbuffer);
    for (int i=0; i< rbuffer; i++){
        Serial.print(i);
        Serial.print(" ");
        Serial.print(remoteBuffer[i]);
    Serial.println(" ");
    }
    if (memcmp(remoteBuffer, onCommand,  2)  == 0) {onFlag = true; Serial.println(">> onFlag");}
    if (memcmp(remoteBuffer, offCommand, 2)  == 0) {offFlag = true; Serial.println(">> offFlag");}

}