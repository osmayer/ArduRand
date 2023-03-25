/*
 * LCD related code, Modified from library provided by the manufacture. 
 */
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>
#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 8   // can be a digital pin
#define XP 9   // can be a digital pin

#define TS_MINX 130
#define TS_MAXX 905

#define TS_MINY 75
#define TS_MAXY 930
/*
#define TS_MINX 125
#define TS_MAXX 913

#define TS_MINY 118
#define TS_MAXY 833*/
// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
// optional
#define LCD_RESET A4

// Assign human-readable names to some common 16-bit color values:
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF


#define MINPRESSURE 275
#define MAXPRESSURE 1000


#define BOXSIZE 40
#define PENRADIUS 2
#define NUMINTS 10
int oldcolor, currentcolor;

#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;


#define MAX_SIZE 1024

byte random_values[MAX_SIZE];
unsigned int generated_values[MAX_SIZE/2];
unsigned int filled_bins = 0;
boolean printed = false;
byte intSize = 0;
unsigned int arrayPosition = 0;
/*
 * 0=Main Screen
 * 1=Integer
 * 2=Coin Flip
 * 3=Dice Roll
 * 4=Raw Output
 */
byte screen = 0;
byte subscreen = 0;

/*
 * Sets up the generator. Initializing the simulator. 
 */
void setup() {
  
  tft.reset();

  uint16_t identifier = tft.readID();
  tft.begin(identifier);
  tft.setRotation(1);
  tft.setTextSize(2.5);
  pinMode(13, OUTPUT);
  Serial.begin(74880);
  populateArray();
  drawMainScreen();
  
  Serial.println("Hello, World");
    

  // put your setup code here, to run once:

}

/*
 * Gets random numbers from the analog circuit and feeds them into an array containing the data.
 */
void populateArray(){
  Serial.println("CALLED!");
  displayProgress(0);
  delay(400);
  tft.drawRect(20, 100, 440, 100, WHITE);
  unsigned int i=0;
  for(; i<sizeof(random_values); i++){
    
    random_values[i] = analogRead(A15) >> 2; 
    //Serial.println(random_values[i]);
    if(i%50==0) displayProgress(i);
    
   }
   displayProgress(i);
   generateNumbers();
}

/*
 * Displays the progress of the generator as it first starts
 * @param i The current value of the simulation, which will be used to display the data. 
 */
void displayProgress(unsigned int i){
  tft.setCursor(20, 70);
  //tft.fillRect(0,0, 480, 320, BLACK);
  
  tft.fillRect(0, 0, 480, 99, BLACK);
  tft.fillRect(20, 100, ((double)i/(double)sizeof(random_values))*440, 100, GREEN);
  tft.print("Loading: ");
  tft.print(i);
  tft.print(" ");
  tft.print(((double)i/(double)sizeof(random_values))*100);
  tft.print("%");
  delay(50);
  
}

/*
 * Goes through the simulation.
 * This is required to make the program interactive and allow for user input. 
 */
void loop() {
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);
  
  // if sharing pins, you'll need to fix the directions of the touchscreen pins
  //pinMode(XP, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  //Make sure that this isn't noise
  if(p.z> MINPRESSURE && p.z < MAXPRESSURE){
    //Serial.println(p.x);
    //Serial.print(p.y);
    if (p.z != -1) {
          p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
          p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
          Serial.print("("); Serial.print(p.x); Serial.print(", ");
          Serial.print(p.y); Serial.print(", ");
          Serial.print(p.z); Serial.println(") ");
    }
    //Serial.println(p);
    //Check if click was in the top row
    if(screen == 0){

      if(p.y >= 20 && p.y <= 150){
        if(p.x >= 20 && p.x <= 230){
          //Button 1; 
          Serial.println("Button 1!");
          screen = 1;
          subscreen = 1;
          createIntSelecter();
          displayIntSlider(20);
         } else if(p.x >= 250 && p.x <= 460){
          //Button 2; 
          Serial.println("Button 2!");
          screen = 2;
          
          displayCoinFlip();
         }
         //Check if it was in the bottom row  
      } else if(p.y >= 170 && p.y <= 300){
        if(p.x >= 20 && p.x <= 230){
          //Button 3; 
          Serial.println("Button 3!");
          screen = 3;
          displayDiceRoll();
         } else if(p.x >= 250 && p.x <= 460){
          //Button 4; 
          Serial.println("Button 4!");
          screen = 4;
          displayRawData();
         }
      }
      } else if(screen == 1){
        if(subscreen == 1){
            if(p.y >= 20 && p.y <= 70){
              if(p.x >= 20 && p.x <= 120){
                screen = 0;  
                subscreen = 0;
                drawMainScreen();
              }  else if(p.x >= 360 && p.x <= 460){
                subscreen = 2;  
                displayIntegers();
              }
            }
            if(p.y >= 140 && p.y <= 280 && p.x >= 20 && p.x<=460){
              displayIntSlider(p.x);
             }
          } else if(subscreen == 2){
            if(p.x >= 20 && p.x<=120 && p.y >= 250 && p.y <= 300) {
              screen = 0; 
              subscreen = 0;  
              drawMainScreen();
            }
          }

        //byte integerValue = 16;
        //displayIntSlider(integerValue);
      } else if(screen == 2){
          if(p.x >= 20 && p.x<=120 && p.y >= 250 && p.y <= 300) {
              screen = 0; 
              subscreen = 0;  
              drawMainScreen();
          }  
      } else if(screen == 3){
        if(p.x >= 20 && p.x<=120 && p.y >= 250 && p.y <= 300) {
              screen = 0; 
              subscreen = 0;  
              drawMainScreen();
          }    
      } else if(screen == 4){
         if(p.x >= 20 && p.x<=120 && p.y >= 250 && p.y <= 300) {
              screen = 0; 
              subscreen = 0;  
              drawMainScreen();
          }   
      }
      }
  // put your main code here, to run repeatedly:

}

//Creating Functions 
/*
 * Displays the selector for integers. Not interactive but allows for inital setup. 
 */
void createIntSelecter(){
  cover();
  tft.drawRect(19, 139, 441, 142, WHITE);
  tft.fillRect(20, 20, 100, 50, BLUE);  
  tft.fillRect(360, 20, 100, 50, BLUE);  
  tft.setCursor(22,38);
  tft.println("Home");
  tft.setCursor(364, 38);
  tft.println("Continue");
  
}

//Drawing Functions
/*
 * Draws the main screen on the TFt display. Contains all of the rectangles but is not interactive. 
 */
void drawMainScreen(){
    tft.fillScreen(BLACK);
    tft.setTextColor(WHITE);

    //tft.getBounds("Integer", x, y, &x1, &y1, &w, &h);
    //Serial.println(x1, y, w, h);
    tft.fillRect(20, 20, 210, 130 , BLUE );
    tft.drawRect(20, 20, 210, 130, WHITE);
    tft.setCursor(75, 75);
    tft.println("Integer");              
    tft.fillRect(250, 20, 210,130,RED);
    tft.drawRect(250, 20, 210,130, WHITE);
    tft.fillRect(20, 170, 210, 130 , BLUE );
    tft.drawRect(20, 170, 210, 130 , WHITE );
    tft.fillRect(250, 170, 210,130,RED);
    tft.drawRect(250, 170, 210,130,WHITE);
    
    tft.setCursor(300, 75);
    tft.println("Dice Roll");
    tft.setCursor(75, 225);
    tft.println("Coin Toss");
    tft.setCursor(300, 225);
    tft.println("Raw Data");
    tft.setTextColor(WHITE);
    tft.setCursor(270, 302);
    //tft.print("Total Bytes: ");
    //tft.print(sizeof(generated_values) - arrayPosition);
    tft.println();
    
}

/*
 * Displays the slider. Not interactive but is used to redraw the bar.
 * @param xposition Where the slider must be displayed on the screen. 
 */
void displayIntSlider(int xposition){
  tft.fillRect(20,140,439, 139, BLACK);
  tft.fillRect(20, 300, 460, 100, BLACK);
  

  //int xposition = (int)(current*27.5);
  tft.setCursor(20,300);
  //tft.setColor(255, 255, 255);
  tft.print("Number of bits to be generated: ");
  tft.println(map(xposition,1, 460,1,17));
  
  intSize = map(xposition,1, 460,1,17);
  //tft.println((xposition-1));
  
  //tft.setColor(xposition, 0, 0);
  tft.fillRect(20, 140, (xposition-20), 140, GREEN);
  //tft.setColor(0, 0, 0);
  //tft.fillRect((xposition+5), 140, 309, 140, BLACK);
  tft.fillRect(xposition,140,(3),140, WHITE); // Positioner
}

/*
 * Displays a series of integer values of a certain number of bits, based on the selection of the user. 
 * Not interactive
 */
void displayIntegers(){
  
  cover();
  tft.setCursor(0, 0);
  tft.setTextSize(3);
  tft.print("Generated "); 
  tft.print(intSize);
  tft.print(" Bit Integers");
  tft.println();
  tft.setTextSize(2.5);
  Serial.println();
  //Serial.println(sizeof(values));
  for(int i=0; i<NUMINTS; i++){
      Serial.println(generated_values[arrayPosition]);
      tft.println(bitExtracted(generated_values[arrayPosition], intSize, 1));
      arrayPosition++; 
  }
  tft.fillRect(20, 250, 100, 50, BLUE);
  tft.setCursor(22, 265);
  tft.println("Home");  
}

/*
 * Displays coin flips, based on the % value of several different data points from the array. 
 */
void displayCoinFlip(){
  cover();
  tft.setCursor(0, 0);
  tft.setTextSize(3);
  tft.println("Generated Die Rolls");
  tft.setTextSize(2.5);
  for(int i = 1; i<=NUMINTS; i++){
    unsigned int current = generated_values[arrayPosition];
    bool found = false;
    for(byte j = 2; j<7; j++){
      if(current % j == 0){
          tft.print("Roll ");
          tft.print(i);
          tft.print(": ");
          tft.println(j);
          found = true;
          break;
        }
       
    }  
    if(!found){
      tft.print("Roll ");
      tft.print(i);
      tft.print(": ");
      tft.println(1);
    }
    arrayPosition++;
  }
  tft.fillRect(20, 250, 100, 50, BLUE);
  tft.setCursor(22, 265);
  tft.println("Home"); 
}

/*
 * Displays interactive elements and the values of the dice rolls based on whether or not the values are even or odd. 
 */
void displayDiceRoll(){
  cover();
  tft.setCursor(0, 0);
  tft.setTextSize(3);
  tft.println("Generated Coin Tosses");
  tft.setTextSize(2.5);
  for(int i = 1; i<=NUMINTS; i++){
    unsigned int current = generated_values[arrayPosition];
    
    tft.print("Flip ");
    tft.print(i);
    tft.print(": ");
    tft.println((current % 2 == 0) ? "Heads" : "Tails");
    
    arrayPosition++;
  }
  tft.fillRect(20, 250, 100, 50, BLUE);
  tft.setCursor(22, 265);
  tft.println("Home"); 
  
}

/*
 * Prints out the button rectangles and the list of values from the array. 
 */
void displayRawData(){
  cover();
  tft.setCursor(0, 0);
  tft.setTextSize(3);
  tft.println("Random Value Output");
  tft.setTextSize(1.7);  
  for(int i=0; i<20; i++){
    for(int j=0; j<10; j++){
        tft.print(generated_values[arrayPosition]);
        Serial.println(generated_values[arrayPosition]);
        tft.print(" ");
        arrayPosition++;
     }
     tft.println();  
     Serial.println(arrayPosition);
  }
  tft.setTextSize(2.5);
  tft.fillRect(20, 250, 100, 50, BLUE);
  tft.setCursor(22, 265);
  tft.println("Home"); 
}

void cover(){
  tft.fillRect(0,0,480, 320, BLACK);
}
//Generating Functions

/*
 * Populates the generated_values array with the random values from the analog circuit.
 */
void generateNumbers(){
  int count = 0;
    Serial.println();
    Serial.println();
    Serial.println("generateNumbers() size:");
    Serial.println(sizeof(generated_values));
    Serial.println(sizeof(random_values));
    Serial.println();
    Serial.println();
  for(unsigned int i = 0; i<MAX_SIZE; i+=2){
    //Serial.println(count);
    bool a[8];
    bool b[8];
    for (int j = 0;  j < 8;  ++j)
       a[j] =  0 != (random_values[i] & (1 << j));
    for (int j = 0;  j < 8;  ++j)
       b[j] =  0 != (random_values[i+1] & (1 << j));
    //Serial.println("B Size:");
    //Serial.println(sizeof(b));
    bool combined[sizeof(a) + sizeof(b)];
    unsigned int ac = 0;
    unsigned int bc = 0;
    //Serial.println(sizeof(combined));
    for(unsigned int j = 0; j< sizeof(combined); j++){
      if(j%2==0){
        combined[j] = a[ac];
        ac++; 
      } else{
        combined[j] = b[bc];
        bc++;  
      }
    }
    unsigned int finalValue = 0;
    //Serial.println("Size:");
    //Serial.println(sizeof(combined));
    
    for (int k=0; k<sizeof(combined); k++) {
      //Serial.println("In loop!");
      if (combined[k] == true) {
          finalValue |= (1<<k);
          //Serial.println(finalValue);
       }
      
    }
    //Serial.println(finalValue);
    generated_values[count] = finalValue;
    Serial.println(generated_values[count]);
    Serial.println(i);
    count++;

  }  


}


/*
 * This function is from https://www.geeksforgeeks.org/extract-k-bits-given-position-number/
 * It works by going from the begining of a number of a certain point, k, and and extracting all of the bits from the range. 
 */
int bitExtracted(int number, int k, int p)
{
    return (((1 << k) - 1) & (number >> (p - 1)));
}

/*
 * Converts a byte into a list a booleans.
 * @param number the number to be converted into booleans.
 * @return A list of booleans represnting the number.
 */
bool *getBoolArray(byte number){
    bool b[8];
    
    for (int j = 0;  j < 8;  ++j)
       b [j] =  0 != (number & (1 << j));
    Serial.println("Initial Size:");
    for(int j=0; j<sizeof(b); j++) Serial.println(b[j]);
    return b;
}
