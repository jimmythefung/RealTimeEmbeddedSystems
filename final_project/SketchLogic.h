/*
 * 8x8 LED and Driver parameters
 */
int DIN = 5;   // DIN pin of MAX7219 module
int CLK = 4;   // CLK pin of MAX7219 module
int CS = 6;    // CS pin of MAX7219 module
int maxInUse = 1;
MaxMatrix m(DIN, CS, CLK, maxInUse); 

// sketch parameters
int frameSize = 0;
char frameBuffer[10][10] = {0};
char sketch[8][8] = {0};
int mode = 0;
const int modeLEDPin = 13;

char reduceCharArr(char a[]){
  char c = 0;
  for(int i=0; i<8; i++){
    if ( a[i]==1 ){
      c = c | (1 << 7-i);
    }
  }
  return c;
}

void rotateSketchBy90(char a[8][8]){
  char t[8][8] = {0};
  // construct rotateSketchBy90; first row is last column
  for(int i=0; i<8; i++){
    for(int j=0; j<8; j++){
      t[j][7-i] = a[i][j];
    }
  }

  // copy the rotateSketchBy90 back to input arr
  for(int i=0; i<8; i++){
    for(int j=0; j<8; j++){
      a[i][j] = t[i][j];
    }
  }

}

void saveSketchToBuffer(char sketch[8][8], char bufferCell[10]){
  rotateSketchBy90(sketch);

  // first 2 rows are static dimensions 8x8
  bufferCell[0] = 8;
  bufferCell[1] = 8;

  // write each row to frame
  for(int i=0; i<8; i++){
    bufferCell[i+2] = reduceCharArr( sketch[i] );
  }
}


void printSketch(char arr[8][8]){
  
  for (int i=0; i<8; i++){
    for (int j=0; j<8; j++){
      if (arr[i][j]==1){
        Serial.print(1);
      }else{
        Serial.print(0);
      }
      Serial.print(" ");
    }
    Serial.print("\n");
  }
}

void updateCursorPosition(int &c, int &r){
  // Right
  if( movedRight() ){
    Serial.println("Right (+X)");
    if(c<7){c++;}
  }

  // Left
  if( movedLeft() ){
    Serial.println("Left (-X)");
    if(c>0){c--;}
  }

  // Up
  if( movedUp() ){
    Serial.println("Down (-Y)");
    if(r<7){r++;}
  }

  // Down
  if( movedDown() ){
    Serial.println("Up (+Y)");
    if(r>0){r--;}
  }
}

void mode_toggle(){
  if(pin2ButtonReleased()){
    // toggle mode and LED
    mode ^= 1;
    digitalWrite(modeLEDPin, mode);
  }
}

void saveSketch(){
  if(pin3ButtonReleased()){
    if (frameSize < 9){
      frameSize++;
      saveSketchToBuffer(sketch, frameBuffer[frameSize-1]);
      // clear sketch screen
      memset(sketch, 0, sizeof(sketch) );
    }
    Serial.print("Frame size: ");
    Serial.println(frameSize);
  }
}
