/*
   Rockblock Dev code v6
   This should be the last version of the dev code as of 06/27/18
   !!never uplink while partially finished downlinking, never downlink while partially finished uplinking!!

*/
#include <IridiumSBD.h>
#define IridiumSerial Serial3
#define DIAGNOSTICS false // Change this to see diagnostics
IridiumSBD modem(IridiumSerial);
//for pic donwlink, already exist in hybrid, do not copy it
uint16_t photosize2 = 1000; //declared just for testing, should be taken out
uint8_t a[5120]; //declared just for testing, should be taken out

uint16_t bytesleft; //declared, needs to be integrated into hybrid
uint16_t counter = 0; //starts as 0, no image has been segmented for downlink yet // needs to be integrated to hybrid
//boolean to know when we are sending the first segment of an image in a downlink -> needs to be created in hybrid
bool isFirst = true;
bool imgsent = false; // last msg sent on downlink was an image. Starts as false

int exampleData = 7471236;
uint8_t buf[340]; //340 is the max for rockblock
int openSpot = 0;
long int lastReceive = 0; //is this number appropriate for startup?
long int lastSend = 0;
bool DLConf = false; //should you be checking to confirm sucessful downlink. true = yes, false = no
int lastSR = 0; // how many transmissions have passed since the last special report
int downlinkPeriod = 900000; // how many millis to wait between each downlink
int uplinkPeriod = 900000; // how many millis to wait before each uplink
int SRFreq = 10; //how many regular downlinks between special reports
int SRTimeout = 300000; // how many millis will the rockblock attempt to downlink
int DLFailCounter = 0; //how many times has downlinking failed
int x;
int err;

void setup() {
  Serial.begin(74880);
  delay(2000);
  IridiumSerial.begin(19200);
  modem.setPowerProfile(IridiumSBD::DEFAULT_POWER_PROFILE);
  Serial.println("Starting modem...");
  err = modem.begin();
  if (err != ISBD_SUCCESS)
  {
    Serial.print("Begin failed: error ");
    Serial.println(err);
    if (err == ISBD_NO_MODEM_DETECTED)
      Serial.println("No modem detected: check wiring.");
    return;
  }
  for (int i = 0; i<256; i++){ //create a for testing
    a[i]=i;
  }
}

void loop() {
  delay(1000);
  Serial.print("DL conf:");
  Serial.println(DLConf);
//  Serial.println();
//  Serial.print("downlinkperiod: ");
//  Serial.println(downlinkPeriod);
//  Serial.print("lastSend: ");
//  Serial.println(lastSend);
//  Serial.print("millis: ");
//  Serial.println(millis());
//  Serial.print("dl in :");
//  Serial.println( (downlinkPeriod + lastSend) - millis());
//  Serial.println();


  //  Serial.print("recieve in: ");
  //  Serial.println((uplinkPeriod + lastReceive) - millis());

  if (Serial.available() > 0) {
    while (Serial.available() != 0) {
      Serial.read();
      x = ISBD_SUCCESS;
      Serial.print("x is =");
      Serial.println(x);      
    }
    lastReceive = millis() - downlinkPeriod;
    lastSend = millis() - uplinkPeriod;
    Serial.println("Got to second part of if");
  }
Serial.println("Left If");
  if ((lastSend + downlinkPeriod) <= (millis()) & !DLConf) { // check if we're due for a downlink, also dont run when we're still waiting for DL confirmation

    //Downlink();
  }
Serial.println("Crossed Downlink");
  if (lastReceive + uplinkPeriod <= millis()) {
    //    Uplink(); TODO
    lastReceive = millis();
    //sbd sendrecieve text
  }
  // every x minutes initiate a null downlink to update
  // if there are any incoming messages. if so call read function
  // or maybe don't? every time we downlink we'll know if there are
  // incoming messages

  if (DLConf) {
    DownlinkCheck();
  }
Serial.println("Crossed DownlinkCheck");
}

void byteread(int value) {// convert a 1 byte int into 1 uint
  uint8_t f = 0;
  f = value & 0xFF;
  buf[openSpot] = f; openSpot++;
}
void twobyteread (int value) {// convert a 2 byte int into 2 uints
  uint8_t f = 0;
  f = value & 0xFF;
  buf[openSpot + 1] = f;
  f = (value >> 8) & 0xFF;
  buf[openSpot] = f; openSpot = (openSpot + 2);
}
void threebyteread (int value) { // convert a 3 byte int into 3 uints
  uint8_t f1 = 0;
  uint8_t f2 = 0;
  uint8_t f3 = 0;
  f1 = value & 0xFF;
  buf[openSpot + 2] = f1;
  f2 = (value >> 8) & 0xFF;
  buf[openSpot + 1] = f2;
  f3 = (value >> 16) & 0xFF;
  buf[openSpot] = f3;
  openSpot = (openSpot + 3);
}
void printBits(uint8_t myByte) {
  for (uint8_t mask = 0x80; mask; mask >>= 1) {
    if (mask  & myByte)
      Serial.print('1');
    else
      Serial.print('0');
  }
}
void Downlink() {
  for (int i = 0; i <= 3 ; i++) { // temp data generation. Be sure everything feeding into this is constrained!
    threebyteread(exampleData);
    exampleData++;
  }

  lastSR++;

  if (lastSR == SRFreq) {
    //compile all the sr data
  }

  x = modem.sendSBDBinary(buf, (openSpot - 1)); // sends from 0 to openspot - 1 (all space in array used)
      imgsent = false;
  // for now assume that we could fit the regular transmission and the special report on the same downlink
  // clear buffer of all sent data. be sure that the message has been sucessfully transmitted.
  DLConf = true;
}


void DownlinkCheck() { // Should now be finished
  Serial.print("downlinkcheck: ");
  Serial.println(DLFailCounter);
  Serial.print("ISBD Sucess Status: ");
  Serial.println(ISBD_SUCCESS);
  Serial.print("X value: ");
  Serial.println(x);

  if (x == ISBD_SUCCESS && imgsent){ //imgsent is a boolean used to verify if the last msg sent was a segment of our image
    Serial.println("Last msg was an imag, and we have received confirmation it was successfully transmitted");
    imgsent = false;
        Serial.println("What we received: ");
    for (int i = 0; i<min(340,bytesleft); i++){
      Serial.print(buf[i]); Serial.print(",");
      if (i%320==0){
      Serial.println("");
      }
    }
    counter += min(340,bytesleft); //advances the beginning of the copy from array a to buf
    bytesleft -= min(340,bytesleft); //tells us how much of the buf will be busy with bytes from image //bytesleft, imgsent, and counter should only be changed by the checker, guaranteeing we are always sending fresh bytes at the right time
    Serial.print("bytesleft: "); Serial.println(bytesleft);
    Serial.print("counter: "); Serial.println(counter);

    
    if (bytesleft==0){
      Serial.print("bytesleft is equal to zero. Message sent. Setting counter = 0; isFirst = true;");
      counter = 0; //resets counter for next image
      isFirst = true; //resets boolean, since we will be strating again.
    }
  }

  if (x == ISBD_SUCCESS) { //Everything worked out, reset everything needed to go again
    Serial.println("Success!");
    openSpot = 0;
    lastSend = millis();
    DLConf = false;
    if (DLFailCounter >= 5) { // undo the delay increased caused by multiple failed downlinks
      downlinkPeriod = (downlinkPeriod / 5);
    }
    DLFailCounter = 0;
    exit; //what is this?
  }
  if ((lastSend + downlinkPeriod + SRTimeout) <= millis()) {  //sr timeout is the max wait after the supposed transmission
    openSpot = 0;
    lastSend = millis();
    DLConf = false;
    DLFailCounter++;

    if (DLFailCounter == 5) {
      downlinkPeriod = (downlinkPeriod * 5);
    }
    // device failed to send mesage. retry 5 times. automatically double-quadruple downlink peroid to conserve power. Report back how many unsucessful attempts
    // perhaps the auto deploy should rely on something consistent. IE the orbital period, how to make sure it deploys over North America! tricky tricky
    // be sure entering a different mode (power saving, ect) wont disrupt this process
  }

}

void Uplink() { // this is difficult to develop without a working rockblock
  Serial.println("top of uplink");
  String Output = "";
  //If your application is receive-only, call sendReceiveSBDText with a NULL outbound message parameter.
  //int sendReceiveSBDText(const char *message, uint8_t *rxBuffer, size_t &rxBufferSize);
  uint8_t rxBuffer[270]; //max size the rockblock can handle for uplink
  size_t rxBuffersize; //rockblock is told how long it is
//should we not have only receive here? or does the send warn the iridium network we want to receive?
  modem.sendReceiveSBDText(NULL, rxBuffer, rxBuffersize); //rxbuffer is the uplink data it is receiving. we are reserving space. rxbuffersize is the size of the incoming data
  delay(10); //maybe remove this
//why are we using Text, to change to binary?
  if (rxBuffer == 0) { //shouldn't here be rxbuffersize
    Serial.println("no incoming message");
    //no incoming message
    lastReceive = millis();
    exit;
  }


  Serial.print("incoming message of ");
  Serial.print(rxBuffersize);
  Serial.println(" bytes");
  for (int i = 0; i <= rxBuffersize; i++) {
    Output += char(rxBuffer[i]);
  }

  Serial.print("output:");
  Serial.println(Output); //test this output with sendreceive iridium script
}

void PicDownlink(){
  Serial.println("We've entered PicDownlink Function");
  //First we start with the segmentation of an image on array a[5120], which happens procedurally, according to the success of the transmission
  if (isFirst){ //for first segment, where we set up bytesleft
      Serial.println("It seems this is the first segment of our image");
  bytesleft = photosize2;
  isFirst = false; //set up complete
  }
  
  //for (uint16_t counter=0; counter<=photosize2; counter += min(340,bytesleft)){ //counter to go through the array, goes up to photosize2(name to change)
    //build correct buf with the correct spots on image array, run once per counter loop
    Serial.println("building correct buf for transmission");
    for (int i = 0; i<min(340,bytesleft); i++){
      buf[i] = a[counter+i];
    }
    Serial.print("How many spots of the buf we are using: ");  Serial.println(min(340,bytesleft));
//        x = modem.sendSBDBinary(buf, min(340,bytesleft)); // sends from 0 to openspot - 1 (all space in array used)
        // for now assume that we could fit the regular transmission and the special report on the same downlink
        // clear buffer of all sent data. be sure that the message has been sucessfully transmitted.
        imgsent = true; //last img sent is an image, needed for downlink check!
        DLConf = true; //this will call the checker indirectly
}


//void adjustSendReceiveTimeout(variable); //put in popcom
// SRTimeout = (variable * 1000)




