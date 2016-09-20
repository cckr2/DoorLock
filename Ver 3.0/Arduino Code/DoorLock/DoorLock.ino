#include "LowPower.h"
#define wake_result 15
#define firmware_result 19
#define tag_result 25

const unsigned char wake[24]={
  0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03, 0xfd, 0xd4, 0x14, 0x01, 0x17, 0x00};//wake up NFC module
const unsigned char firmware[9]={
  0x00, 0x00, 0xFF, 0x02, 0xFE, 0xD4, 0x02, 0x2A, 0x00};//
const unsigned char tag[11]={
  0x00, 0x00, 0xFF, 0x04, 0xFC, 0xD4, 0x4A, 0x01, 0x00, 0xE1, 0x00};//detecting tag command
const unsigned char std_ACK[25] = {
  0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x0C, \
0xF4, 0xD5, 0x4B, 0x01, 0x01, 0x00, 0x04, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x4b, 0x00};
unsigned char old_id[5];
unsigned char cur_id[5];

unsigned char receive_ACK[25];//Command receiving buffer
const int button = 2;
const int power_Relay = 7;
const int door_Relay = 8; 
void setup()
{
  Serial.begin(115200);   // open serial with PC
  delay(100);
  
  pinMode(power_Relay,OUTPUT);
  pinMode(door_Relay,OUTPUT);
  pinMode(button,INPUT_PULLUP);


}

void loop()
{
  attachInterrupt(0, wakeUp, LOW);  
  
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); //set Sleep mode

  detachInterrupt(0);    
  checking();

}

void UART_Send_Byte(unsigned char command_data)
{//send byte to device
  Serial.write(command_data);
  Serial.flush();// complete the transmission of outgoing serial data
}

void wake_card(void)
{//send wake[] to device
  unsigned char i;
  for(i=0;i<24;i++) //send command
    UART_Send_Byte(wake[i]);
}

void firmware_version(void)
{//send firmware[] to device
  unsigned char i;
  for(i=0;i<9;i++) //send command
    UART_Send_Byte(firmware[i]);
}

void send_tag(void)
{//send tag[] to device
  unsigned char i;
  for(i=0;i<11;i++) //send command
    UART_Send_Byte(tag[i]);
}

void read_ACK(unsigned char temp)
{//read ACK into reveive_ACK[]
  //Serial.println("read the result");
  unsigned char i;
  for(i=0;i<temp;i++) {
    receive_ACK[i]= Serial.read();
  }
}

void display(unsigned char tem)
{
  for (int i=0;i<tem;i++)
  {
    if (receive_ACK[i] < 16) // to make 2 digits
      Serial.print("0");
    Serial.print(receive_ACK[i], HEX);
    if (i<(tem-1)) Serial.print(" ");
  }
  Serial.println();
}

void copy_id (void)
{//save old id
  int i;
  for (i=0 ; i<5 ; i++) {
    old_id[i] = cur_id[i];
  }
}

// read tag id from tag values
void read_id (void) {
  int ai, ci;
  for (ci=0, ai=19; ci<5; ci++,ai++) {
    cur_id[ci] = receive_ACK[ai];
  }
}

//return true if find id is old
char same_id (void)
{
  int ai, oi;
  for (oi=0,ai=19; oi<5; oi++,ai++) {
    if (old_id[oi] != receive_ACK[ai])
      return 0;
  }
  return 1;
}

// true if tag is FF FF FF FF FF
char no_detection(void) {
  int i;
  for (i=0 ; i<sizeof(cur_id) ; i++) {
    if (cur_id[i] != 255)  // FF
      return 0;
  }
  return 1;
}

void print_id() {
  for (int i=0 ; i<sizeof(cur_id) ; i++) {
    if (cur_id[i] < 16) // to make 2 digits
      Serial.print("0");
    Serial.print(cur_id[i], HEX);
    Serial.print("(");
    Serial.print(cur_id[i]);
    Serial.print(")");
    if (i<(sizeof(cur_id)-1)) Serial.print(" ");
  }
  Serial.println();
}

int check_id(int count) {
  for (int i=0 ; i<5 ; i++) {
      if (cur_id[i] < 16) // to make 2 digits
          Serial.print("0");
      Serial.print(cur_id[i]);
  }
  if(cur_id[0] == 115 && cur_id[1] == 5 && cur_id[2] == 85)                    
  {                  
      digitalWrite(door_Relay, HIGH);
      delay(500);
      digitalWrite(door_Relay, LOW);
      return 0;
  }else if(cur_id[0] == 7 && cur_id[1] == 78 && cur_id[2] == 123)                    
  {                  
      digitalWrite(door_Relay, HIGH);
      delay(500);
      digitalWrite(door_Relay, LOW);
      return 0;
  }else if(cur_id[0] == 5 && cur_id[1] == 167 && cur_id[2] == 165)                    
  {                  
      digitalWrite(door_Relay, HIGH);
      delay(500);
      digitalWrite(door_Relay, LOW);
      return 0;
  }else if(cur_id[0] == 238 && cur_id[1] == 49 && cur_id[2] == 26)                    
  {                  
      digitalWrite(door_Relay, HIGH);
      delay(500);
      digitalWrite(door_Relay, LOW);
      return 0;
  }  
  return count;
}


void NFC_init(){
    Serial.println("wake card");
    delay(100);  // give delay before sending command bytes
    wake_card();
    delay(100);
    read_ACK(wake_result);
    delay(100);
    display(wake_result);
    
    firmware_version();
    delay(100);
    read_ACK(firmware_result);
    delay(100);
    display(firmware_result);
}

void wakeUp(){}
void checking() //when wake up, this function will working
{
    int button_state = 0;
    int count = 0; 
    int k,check_nfc=0;

    digitalWrite(power_Relay, HIGH);
    count=15;
    delay(100);        
    NFC_init();
    delay(100);
    while(count!=0){
        check_nfc=0;
        button_state = digitalRead(button);
        count--;

        if(button_state==0){ // this is safe Function. 
            delay(100);
            for(k=0;k<25;k++){    
                button_state = digitalRead(button);
                if(button_state==0){
                    delay(200);
                    check_nfc++;
                }
                else
                  break;
            }
            
            if(check_nfc>20){
                digitalWrite(door_Relay, HIGH);
                delay(500);
                digitalWrite(door_Relay, LOW);
                count=0;
            }
                    
        }else{   
            send_tag();
            read_ACK(tag_result);
            read_id();
            if (!no_detection()) {
                //print_id();
                count = check_id(count);
            }
        }
        delay(100);
    }
    digitalWrite(power_Relay, LOW);
}

