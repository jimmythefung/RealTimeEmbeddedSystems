/*
 * Pin 2 and 3 are interrupts with vector number 0 and 1, respectively
 * -Pin 2 used to toggle mode
 * -Pin 3 used to save sketch to buffer
 */
const int pin2Button = 2;     // the number of the pushbutton pin
const int pin3Button = 3;      // the number of the seven segment led pin

bool pin2ButtonReleased(){
  static int buttonReleased = 0;
  static int buttonPressed = 0;
  
  if( digitalRead(pin2Button)==0 ){
    if (buttonPressed == 0){
      buttonPressed = 1; 
    }
  }

  if( digitalRead(pin2Button)==1 ){
    if (buttonPressed==1){
      buttonReleased = 1; 
    }
  }

  if( buttonPressed==1 && buttonReleased==1){
    buttonPressed = 0;
    buttonReleased = 0;
    return true;
  }
  return false;
}

bool pin3ButtonReleased(){
  static int buttonReleased = 0;
  static int buttonPressed = 0;
  
  if( digitalRead(pin3Button)==0 ){
    if (buttonPressed == 0){
      buttonPressed = 1; 
    }
  }

  if( digitalRead(pin3Button)==1 ){
    if (buttonPressed==1){
      buttonReleased = 1; 
    }
  }

  if( buttonPressed==1 && buttonReleased==1){
    buttonPressed = 0;
    buttonReleased = 0;
    return true;
  }
  return false;
}