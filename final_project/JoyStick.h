/*
 * JoyStick parameters
 */
const int VRxPin= 0;      //VRx pin connected to arduino pin A0
const int VRyPin= 1;      //VRy pin connected to arduino pin A1
const int SwButtonPin= 8; //SW pin connected to arduino pin D8

int readJoyStick_X(){
  return (uint32_t)analogRead(VRxPin)*100/1024;
}
int readJoyStick_Y(){
  return (uint32_t)analogRead(VRyPin)*100/1024;
}
int readJoyStick_SW(){
  return digitalRead(SwButtonPin);
}


const int potPin=2;
int readPot(){
  return (uint32_t)analogRead(potPin)*100/1024;
}


bool joystick_button_is_released(){
  static int buttonReleased = 0;
  static int buttonPressed = 0;
  if( digitalRead(SwButtonPin)==0 ){
    if (buttonPressed == 0){
      buttonPressed = 1; 
    }
  }

  if( digitalRead(SwButtonPin)==1 ){
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

bool movedRight(){
  static int risingEdge = 0;
  static int fallingEdge = 0;

  // Detect rising edge
  if (readJoyStick_X() > 70){
    if(risingEdge==0){
      risingEdge = 1;
    }
  }

  // Detect falling edge
  if (readJoyStick_X() < 70){
    if(risingEdge==1){
      fallingEdge = 1;
    }
  }

  if (risingEdge==1 && fallingEdge==1){
    risingEdge=0;
    fallingEdge=0;
    return true;
  }else{
    return false;
  }
}

bool movedLeft(){
  static int risingEdge = 0;
  static int fallingEdge = 0;

  // Detect rising edge
  if (readJoyStick_X() < 30){
    if(risingEdge==0){
      risingEdge = 1;
    }
  }

  // Detect falling edge
  if (readJoyStick_X() > 30){
    if(risingEdge==1){
      fallingEdge = 1;
    }
  }

  if (risingEdge==1 && fallingEdge==1){
    risingEdge=0;
    fallingEdge=0;
    return true;
  }else{
    return false;
  }
}

bool movedUp(){
  static int risingEdge = 0;
  static int fallingEdge = 0;

  // Detect rising edge
  if (readJoyStick_Y() > 70){
    if(risingEdge==0){
      risingEdge = 1;
    }
  }

  // Detect falling edge
  if (readJoyStick_Y() < 70){
    if(risingEdge==1){
      fallingEdge = 1;
    }
  }

  if (risingEdge==1 && fallingEdge==1){
    risingEdge=0;
    fallingEdge=0;
    return true;
  }else{
    return false;
  }
}

bool movedDown(){
  static int risingEdge = 0;
  static int fallingEdge = 0;

  // Detect rising edge
  if (readJoyStick_Y() < 30){
    if(risingEdge==0){
      risingEdge = 1;
    }
  }

  // Detect falling edge
  if (readJoyStick_Y() > 30){
    if(risingEdge==1){
      fallingEdge = 1;
    }
  }

  if (risingEdge==1 && fallingEdge==1){
    risingEdge=0;
    fallingEdge=0;
    return true;
  }else{
    return false;
  }
}