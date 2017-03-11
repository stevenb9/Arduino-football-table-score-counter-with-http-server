//storage variables
#include <SPI.h>
#include <Ethernet.h>

//Init LDRReading vars
int LDRReading = 999;
int LDRReading2 = 999;

//Set IP/MAC Settings for Ethernet Shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1,177);

//Init variable for points
int points_1 = 0;
int points_2 = 0;

//Set pins for LDR sensors
int LDR_Pin2 = A0; //analog pin 0
int LDR_Pin = A1; //analog pin 0

//Toggle helper variable for LED on interrupt
boolean toggle1 = 0;

//Init new EthernetServer on port 80
EthernetServer server(80);


//Start Arduino's Setup
void setup(){
  //config Ledpin as output
  pinMode(13, OUTPUT);
  
  //Start serial output
  Serial.begin(9600);
  
  //start the Ethernet connection
  Ethernet.begin(mac, ip);
  
  //Start server
  server.begin();
  
  //Print server Address in terminal
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  
  //Stop interrupts & delays (needed to configure the interrupt timer)
  cli();

  //set timer1 interrupt
  TCCR1A = 0;  //set entire TCCR1A register to 0
  TCCR1B = 0;  //same for TCCR1B
  TCNT1  = 0;  //initialize counter value to 0
  
  // set compare match register for 1hz increments
  OCR1A = 200;  //Dont lower value, will randomly freeze arduino (min 90)
  
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10); 
  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
}


//Interrupt function (will be used when arduino is sending data to client)
ISR(TIMER1_COMPA_vect){
 //Read LDR values
 LDRReading = analogRead(LDR_Pin);
 LDRReading2 = analogRead(LDR_Pin2); 
 
 //Print values to serial
 Serial.println(LDRReading);
 Serial.print(LDRReading2);
 
  //Show when the interrupt is called, used for debug if arduino has frozen
  if (toggle1){
    digitalWrite(13,HIGH);
    toggle1 = 0;
  }
  else{
    digitalWrite(13,LOW);
    toggle1 = 1;
  }
}
  


//Arduino's main loop
void loop(){
  // listen for incoming client requests
  EthernetClient client = server.available();
  
  //If a client is requesting the scoreboard
  if (client) {
     //Allow interrupts, this will make sure there is cointinious reading of die LDR sensors
     sei();

    //an http request ends with a blank line
    boolean currentLineIsBlank = true;
    
    //as long as the client is connected
    while (client.connected()) {
      //And available
      if (client.available()) {
        //Read the client request
        char c = client.read();
        
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == 'n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          
          //HTML
          client.println("<html>");
          
          //Background Color/Image
          //client.print("<body background='IMAGEURL'>");
          client.print("<body bgcolor='GREEN'>");
          
          //Div Holding the score
          client.print("<div align='center'><p style='font-size: 450%;'> Digital Football </p><p style='font-size: 1000%;'><font color = 'BLUE'>");
          client.print(points_1);        
          client.print("</font> - <font color = 'RED'>");
          client.print(points_2);
          client.print("</font></p></div></html>");
          
          //Break when done sending your Score
          break;
        }
        if (c == 'n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != 'r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    
    //give the web browser time to receive the data
     delay(40);
     
    // close the connection:
    client.stop();
    
    //Stop interrupts, main loop will take over again checking the LDR's as fast as possible
    cli();
  }
  
  //Read LDR's values
  LDRReading = analogRead(LDR_Pin);
  LDRReading2 = analogRead(LDR_Pin2);

  //Test if any of the LDR's has been "triggered"
  if(LDRReading < 800){
    //Add point   
    points_1++;
    
    //Allow interrupts to use the delay function
    sei();
  
    //Delay after a ball has passed to make sure your ball only gets count once
    delay(150);
        
    //Stop interrupts, main loop will take over again   
    cli();
  }
  
  //Test LDR 2
  if(LDRReading2 < 800){
    //add point
    points_2++;
    
    //Allow interrupts to use the delay function
    sei();
  
    //Delay after a ball has passed to make sure your ball only gets count once
    delay(150);
        
    //Stop interrupts, main loop will take over again   
    cli();
  }
}

