//Remote Recepcion Controller
//

#define displaySize 20
#define remoteSize 20
#define repeaterSize 20
#define led PC13

char displayBuffer[displaySize];
volatile int dbuffer=0;

char remoteBuffer[remoteSize];
volatile int rbuffer=0;

char repeaterBuffer[repeaterSize];
volatile int cbuffer=0;

int frecuency = 591;
int duty = 791;

const char onCommand[] =  "on";
const char offCommand[] =  "off";

volatile bool onFlag = false;
volatile bool offFlag = false;
volatile bool displayFlag = false;
volatile bool repeaterFlag = false;
volatile bool remoteCommandFlag = false;
volatile int ndata = 0;

void setup(){
    pinMode(led, OUTPUT);
    Serial.begin(115200); //For debugging purpouses
    Serial1.begin(9600); //Connects to wifirepeater RF (reads voltages from battery and panels)
    //Serial2.begin(9600);   //Connects to RF transeiver (reads commands for relay control on/off)
    Serial3.begin(9600);   //Connects to display
    Serial.print("USB - Debug\n\r");
    Serial1.print("UART1 - RF WifiRepeater\n\r");
    //Serial2.print("UART2 - RF PowerHouse\n\r");
    Serial3.print("UART3 - Display\n\r");}

void loop(){
    //if (Serial2.available()) {
    //Serial.write(Serial2.read());
    //processingRemoteData(Serial2.read());
    //}

    if (Serial3.available()) {
    //Serial.write(Serial3.read());
    processingRemoteData(Serial3.read());
    }

    if (Serial1.available()) {
    //Serial.write(Serial1.read());
    processingRemoteData(Serial1.read());
    }

    if(displayFlag){
        displayFlag=false;
        controllerToDisplay();
        printStringRF();
    }

    if(remoteCommandFlag){
            if (onFlag){ 
                Serial1.println("on");
                onFlag=false;
            }
            if (offFlag){
                Serial1.println("off");
                offFlag=false;
            }
            remoteCommandFlag=false;

    }

    if(repeaterFlag){
        repeaterFlag=false;
        controllerToDisplay();
    }
}

void controllerToDisplay(){
    sprintf(displayBuffer, "t3.txt=\"%c%c.%c\"\xFF\xFF\xFF", 
        remoteBuffer[0],remoteBuffer[1],remoteBuffer[2]);
    printStringDisplay();
    delay(100);
    sprintf(displayBuffer, "t4.txt=\"%c%c.%c\"\xFF\xFF\xFF", 
        remoteBuffer[5],remoteBuffer[6],remoteBuffer[7]);
    printStringDisplay();
    delay(100);

}

void repeaterToDisplay(){
    //12.3, 13.6, 1
    sprintf(displayBuffer, "t3w.txt=\"%c%c.%c\"\xFF\xFF\xFF", 
        repeaterBuffer[0],repeaterBuffer[1],repeaterBuffer[3]);
    printStringDisplay();
    sprintf(displayBuffer, "t4w.txt=\"%c%c.%c\"\xFF\xFF\xFF", 
        repeaterBuffer[6],repeaterBuffer[7],repeaterBuffer[9]);
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
    for(int i = 0; i<repeaterSize; i++ ){
        if (repeaterBuffer[i] != 0){
            Serial2.print(char(repeaterBuffer[i]));
        }
        else{
                break;
        }
    }
}
void processingRemoteData(char c){
    Serial.print(c);
    remoteBuffer[rbuffer] = c;
    rbuffer++;
    if ((c == '\n')||(c == '\r')) {
        if (rbuffer>2){
            comparator();
            remoteCommandFlag=true;

        }
        if (rbuffer>5) displayFlag=true;
        rbuffer=0;
    }
    if (rbuffer>=remoteSize) rbuffer=0;
    
}
//12.3, 13.6, 1


void processingWifiRepeaterData(char c){
    Serial.print(c);
    repeaterBuffer[cbuffer] = c;
    cbuffer++;
    if ((c == '\n')||(c == '\r')) {
        if (cbuffer>10){
            repeaterFlag=true;
            cbuffer=0;
        }
    }
    if (cbuffer>=repeaterSize) cbuffer=0;
    
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