#include <Wire.h>

//These values are the readings of the whitepoint of the led. Take the back of vinyl sticker and put it againts face of sensor
#define LED_RED 7540.0f
#define LED_GREEN 14470.0f
#define LED_BLUE 7270.0f

//Calculate the balancing factors
#define BAL_RED (LED_GREEN/LED_RED)
#define BAL_GREEN (LED_GREEN/LED_GREEN) 
#define BAL_BLUE (LED_GREEN/LED_BLUE)

#define I2C_ADDR 0x52

#define RED 0
#define GREEN 1
#define BLUE 2
#define YELLOW 3
#define A_BAD_COLOR 4

#define NUM_REV 1

uint8_t readBuff[9];
uint16_t ir=0;
uint16_t red=0;
uint16_t green=0;
uint16_t blue=0;

void setup() {
  Wire.begin();
  Serial.begin(115200);
  i2cWrite(0x00,0b0110);  //enable light sensor and activate rgb mode
  i2cWrite(0x04,0b01000000); //set to 16 bit resolution for 25ms response time and set measurement rate to 25ms

  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
}

void loop() {
  
 Serial.println(GetNewColor());
 //for (int i = 1; i <= (NUM_REV * 8); i++) GetNewColor();
 //Serial.println("Revolutions completed");
 
}

void i2cWrite(uint8_t reg, uint8_t val){
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

void i2cRead(uint8_t reg,uint8_t *val,uint16_t len){
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(I2C_ADDR, len);
    for(uint8_t i=0;i<len;i++){
      val[i]=Wire.read();
    }
}

int GetColor() {
i2cRead(0x0A,readBuff,12);

  ir=(readBuff[1]<<8)|readBuff[0];
  green=(readBuff[4]<<8)|readBuff[3];
  blue=(readBuff[7]<<8)|readBuff[6];
  red=(readBuff[10]<<8)|readBuff[9];

  red*=BAL_RED;
  green*=BAL_GREEN;
  blue*=BAL_BLUE;
  
#ifdef UNDEF
  Serial.print(ir);
  Serial.print(" ");
  Serial.print(red);
  Serial.print(" ");
  Serial.print(green);
  Serial.print(" ");
  Serial.print(blue);
  Serial.println(" ");
#endif

  //Normalize the readings to brightest channel then apply log scale to better discern the colors.
  float maxV=max(blue,max(red,green));
  red=255*pow(red/maxV,5);
  green=255*pow(green/maxV,5);
  blue=255*pow(blue/maxV,5);

  if(blue>254){ //max blue?
    red=0;
    green=0;
    blue=255;
  }
  else if(green>254){ //max green?
    red=0;
    green=255;
    blue=0;
  }
  else if(red>254){   //Max red occurs when it's red or yellow
    if(green<25){     //Check green channel, if it's below threadhold then it's not yellow
      red=255;
      green=0;
      blue=0;
    }
    else{             //red is max and green is significant portion so we can assume it's yellow (though white will also trigger this since we don't check blue channel)
      red=255;
      green=150;
      blue=0;
    }
  }
  
  #ifdef UNDEF
  Serial.print("red = ");
  Serial.print(red);
  Serial.print(" green = ");
  Serial.print(green);
  Serial.print(" blue = ");
  Serial.println(blue);
  #endif
  
  delay(25);

if (green == 0 && blue == 0) return RED;
else if (red == 0 && blue == 0) return GREEN;
else if (green == 0 && red == 0) return BLUE;
else if (blue == 0) return YELLOW;
else return A_BAD_COLOR;
}

int CurrentColor = RED;
int LastColor = A_BAD_COLOR;
int GetNewColor() {

  while(CurrentColor == LastColor) CurrentColor = GetColor();
  LastColor = CurrentColor;
  
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(CurrentColor + 2, HIGH);
  
  return CurrentColor;
  
}
