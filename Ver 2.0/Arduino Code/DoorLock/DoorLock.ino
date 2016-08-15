#include <AddicoreRFID.h>
#include <SPI.h>
#include "LowPower.h"
 
#define    uchar    unsigned char
#define    uint    unsigned int
#define MAX_LEN 16

AddicoreRFID myRFID; 
 
/////////////////////////////////////////////////////////////////////
//set the pins
/////////////////////////////////////////////////////////////////////
const int chipSelectPin = 10;
const int resetPin = 9;
const int button = 2;
const int power_Relay = 7;
const int door_Relay = 8; 

void setup() {                
  Serial.begin(9600);                        // 시리얼 모니터 출력 설정
  SPI.begin();

  pinMode(chipSelectPin,OUTPUT);              // Set digital pin 10 as OUTPUT to connect it to the RFID /ENABLE pin 
  pinMode(resetPin,OUTPUT);                     // Set digital pin 10 , Not Reset and Power-down
  pinMode(power_Relay,OUTPUT);
  pinMode(door_Relay,OUTPUT);
  pinMode(button,INPUT_PULLUP);
  
  digitalWrite(chipSelectPin, LOW);         // Activate the RFID reader
  digitalWrite(resetPin, HIGH);

  myRFID.AddicoreRFID_Init();  
}

void loop() {
  attachInterrupt(0, wakeUp, LOW);  
  
  //Serial.println("sleep");
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); //set Sleep mode

  detachInterrupt(0); 
  //Serial.println("wake up");    
  checking();
  
}

void wakeUp(){}

void checking() //when wake up, this function will working
{
    int button_state = 0;
    int count = 0; 
    int k,check_nfc=0;

    digitalWrite(power_Relay, HIGH);
    count=15;
    delay(200);        
   
    while(count!=0){
        check_nfc=0;
        button_state = digitalRead(button);
        count--;

        if(button_state==0){ // this is safe Function. 
            delay(100);
            for(k=0;k<35;k++){    
                button_state = digitalRead(button);
                if(button_state==0){
                    delay(200);
                    check_nfc++;
                }
                else
                  break;
            }
            
            if(check_nfc>30){ when you push Button for 30 seconds, door lock is unlock
                digitalWrite(door_Relay, HIGH);
                delay(500);
                digitalWrite(door_Relay, LOW);
                count=0;
            }
                    
        }else{               
            uchar status;
            uchar str[MAX_LEN];
            str[1] = 0x4400;
            String mynum = "";

            //Find tags, return tag type
            status = myRFID.AddicoreRFID_Request(PICC_REQIDL, str); 
            if (status == MI_OK)
            {
                if(str[0] == 100 && str[1] == 11)                    
                {                  
                digitalWrite(door_Relay, HIGH);
                delay(500);
                digitalWrite(door_Relay, LOW);
                }
            }
            
            status = myRFID.AddicoreRFID_Anticoll(str);
            if (status == MI_OK)//if attached card is registered card, doorlock will working
            {         
                Serial.println(str[0]);            
                Serial.println(str[1]);            
                if(str[0] == 5 && str[1] == 167)                    
                {                  
                    digitalWrite(door_Relay, HIGH);
                    delay(500);
                    digitalWrite(door_Relay, LOW);
                } else if(str[0] == 115 && str[1] == 5 ) {            
                    digitalWrite(door_Relay, HIGH);
                    delay(500);
                    digitalWrite(door_Relay, LOW);
                }
                else if(str[0] == 213 && str[1] == 143  ) {            
                    digitalWrite(door_Relay, HIGH);
                    delay(500);
                    digitalWrite(door_Relay, LOW);
                }
                delay(1000);
                count=0;
            }
            myRFID.AddicoreRFID_Halt();      
        }
        
        delay(100);
    }
    digitalWrite(power_Relay, LOW);
}
