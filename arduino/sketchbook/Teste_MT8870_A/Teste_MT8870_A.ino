void setup() {                
  int I;
  for (I = 2; I <= 12; I++;) {
    pinMode(I, OUTPUT);     
    switch (I) {
      case 1:
        //do something when var equals 1
        break;
      case 2:
        //do something when var equals 2
        break;
      default: 
        // if nothing else matches, do the default
        // default is optional
      break;
    }  
  }
}


void loop() {
 
}
