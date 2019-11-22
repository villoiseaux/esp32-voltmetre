//ab3a43bd-8742-4aaf-9859-3c0d3162621a:5c:cf:7f:cb:31:16

#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
 
#define RATIO 2
// Addre, SDA, SCL
SSD1306  display(0x3c,18,19,GEOMETRY_128_32);
 
const int analogInPin = A3;  
int sensorValue = 0;       

#define GRAPH_WIDTH 80
#define GRAPHPOS 40
#define GRAPHHEIGH 32

byte buffer[GRAPH_WIDTH];
int vmax=0;
bool first=true;

#define ADC_MODE  0
#define VOLT_MODE 1
#define PERCENT_MODE 2
#define MAX_ACQ 4095

#define MAX_MODE 2

#define LONGCLICK 1500
#define ACQ_STEP 500

int displayMode=VOLT_MODE;

unsigned long doAcqutition (){
    vmax=0;
    unsigned long before=micros();
    for (int i=0; i<GRAPH_WIDTH; i++) {
      sensorValue = analogRead(analogInPin);
      buffer[i]=map(sensorValue,0,MAX_ACQ,0,GRAPHHEIGH);
      if (sensorValue>vmax) vmax=sensorValue;
      while ((micros()%ACQ_STEP)!=0) {;}
    }
    return (micros()-before);  
}


void setup() {
  display.init();
  display.flipScreenVertically();
  display.clear();
  display.setFont(ArialMT_Plain_16);
  unsigned long t=doAcqutition();
  display.drawString(0, 0, String(t/1000.0,3)+String(" ms"));
  display.drawString(0, 16, String(GRAPH_WIDTH)+String(" pts ")+String(GRAPH_WIDTH*10000.0/t,2)+String (" kHz"));
  display.display();  

  Serial.begin(115200);
  Serial.println();
  Serial.println("ready");
  pinMode(23,INPUT);
  pinMode(2,OUTPUT);
  digitalWrite(2,HIGH);
  delay (1500);
  unsigned long a,b;
}


void loop() {
  sensorValue = analogRead(analogInPin);
  display.clear();
  //display.drawHorizontalLine(GRAPHPOS+GRAPH_WIDTH+1,GRAPHHEIGH-map(sensorValue,0,MAX_ACQ,0,GRAPHHEIGH),8);
  display.drawRect(GRAPHPOS+GRAPH_WIDTH+1,0,7,GRAPHHEIGH);
  display.fillRect(GRAPHPOS+GRAPH_WIDTH+1,GRAPHHEIGH-map(sensorValue,0,MAX_ACQ,0,GRAPHHEIGH),7,map(sensorValue,0,MAX_ACQ,0,GRAPHHEIGH));
  display.setFont(ArialMT_Plain_10);
  switch (displayMode) {
    case (ADC_MODE)    : display.drawString(0, 20, String(sensorValue)); break;
    case (VOLT_MODE)   : display.drawString(0, 20, String(sensorValue/(float(MAX_ACQ)/3.29),2)+String(" V")); break;
    case (PERCENT_MODE):display.drawString(0, 20, String(sensorValue*100/(float(MAX_ACQ)),0)+String(" %"));  break;
  }
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, String(vmax/(float(MAX_ACQ)/3.29),1)+String(" V"));  
  
  if (digitalRead(23)==LOW) {
    unsigned long push=millis();
    delay (50);
    digitalWrite(2,LOW);
    while (digitalRead(23)==LOW) {
      if (millis()>push+LONGCLICK) {           
            display.clear();
            display.setFont(ArialMT_Plain_16);
            display.drawString(2,8, "CHANGE MODE");
            display.display();  
      }
      delay(1);
    }
    first=false;
    if (millis()<push+LONGCLICK) { // shrort click
       doAcqutition ();
    } else { // long click;
        displayMode++;
        if (displayMode>MAX_MODE) displayMode=0;
    }
    digitalWrite(2,HIGH);
  }
  display.drawRect(GRAPHPOS, 0, GRAPH_WIDTH, 32);
  if (first) {
    display.setFont(ArialMT_Plain_10);
    if (millis()%2000>1000){
      display.drawString(GRAPHPOS+6, 1, String("simple click"));
      display.drawString(GRAPHPOS+6, 12, String("acq. sample"));
    } else {
      display.drawString(GRAPHPOS+6, 1, String("long click"));
      display.drawString(GRAPHPOS+6, 12, String("change mode"));
    }
  } else {
    for (int i=0; i<GRAPH_WIDTH; i++) {
      //display.drawVerticalLine(GRAPHPOS+i,GRAPHHEIGH-buffer[i],buffer[i]);
      if (i>0) {
        display.drawLine(GRAPHPOS+i-1,GRAPHHEIGH-buffer[i-1],GRAPHPOS+i,GRAPHHEIGH-buffer[i]);
      }
      //display.setPixel(GRAPHPOS+i,GRAPHHEIGH-buffer[i]-2);
    }
  }
  display.display();  
  delay(1);
}
