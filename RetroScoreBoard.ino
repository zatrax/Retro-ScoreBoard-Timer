/*
  This is a retro ScoreBoard using a TM1638 eval board containing
  8x7-segments displays, 8x3 colors LEDs and 8x push buttons.

  Buttons (from left to right):
    0: Decrease Player 1 Score
    1: Increase Player 1 Score
    2: Decrease Player 2 Score
    3: Increase Player 2 Score
    4: 
    5:
    6:
    7:

  Display (show one after the other):
   "Soul Calibur 2" (scrolling)
    Flashing:
      (Player 1)(<p1Score>)
      (Player 2)(<p2Score>)
*/

#include <TM1638.h>
#include <SD.h>
#include <ctype.h>

// define a module on data pin 8, clock pin 9 and strobe pin 7
TM1638 m(8, 9, 7);
const int chipSelect = 4;
File myFile;
unsigned long p1Score = 0;
unsigned long p2Score = 0;
const int flashRate = 150;
const int nbTimesName = 3;
int i = 0;
int j = 0;
int Mode = 0;
const int maxMode = 3;
bool Released = false;
byte butt[8];
int Phase = 0;
unsigned long StartTime = 0;
unsigned long TimerStartTime = 0;
bool NewPhase = true;
bool NewMode = true;
unsigned int ContinueTimer = 1;
unsigned int PhaseDelay = 0;
unsigned int CycleCounter = 0;
unsigned long ShowTime = 0;
unsigned long StopTime = 0;
unsigned long CurrentTime = (48.0 - (3.0 + 33.0/60.0)) * 3600000;

void setup() {
  // Initialise SD card
  pinMode(10, OUTPUT);
  SD.begin(chipSelect);
  ReadScores();
}

void loop() {
  switch(Mode)
  {
    // Player 1 & Player 2 Scores
    case 0: 
      DisplayScores();
      break;
    // Timer Up
    case 1:
      TimerUp();
      break;
    case 2:
      Countdown();
      break;
  }
  CheckButtons();
}

// Continue Timer Values:
// 0: Show 0000
// 1: Counting
// 2: Stop Counting
// 3: Reset Counter, hold at 0
void TimerUp()
{
  if(NewMode){
    NewMode = false;
    TimerStartTime = millis();
		ContinueTimer = 1;
  }

	unsigned long cntTime = millis()-TimerStartTime;

	if(ContinueTimer == 3){			// Reset
		StopTime = 0;
		cntTime = 0;
		TimerStartTime = 0;
		ContinueTimer = 2;
	}
	if(ContinueTimer == 2){			// Stop
		if(StopTime == 0 && TimerStartTime > 0)
				StopTime = cntTime;
		cntTime = StopTime;
	}
	else if(ContinueTimer == 1 && StopTime > 0){		// Resume
		// adjust TimerStartTime to continue from StopTime
		TimerStartTime = millis() - StopTime;
		cntTime = millis() - TimerStartTime;
		StopTime = 0;
	}
	else if(ContinueTimer == 0)
		cntTime = 0;
		
	unsigned long mm = (cntTime/10)%100;
	unsigned long SS = (cntTime/1000)%60;
  
	unsigned long minutes = (cntTime/1000 - SS)/60;
	unsigned long MM = minutes%60;
	unsigned long hours = (minutes - MM)/60;
	unsigned long HH = hours%60;
	unsigned long showTime = HH*1000000 + MM*10000 + SS*100 + mm;

	m.setDisplayToDecNumber(showTime, 84);
}

void Countdown()
{
  if(NewMode){
    NewMode = false;
    StartTime = millis();
  }
  unsigned long cntTime = millis()-StartTime;
  cntTime = CurrentTime - cntTime; 
  unsigned long mm = (cntTime/10)%100;
  unsigned long SS = (cntTime/1000)%60;
  
  unsigned long minutes = (cntTime/1000 - SS)/60;
  unsigned long MM = minutes%60;
  unsigned long hours = (minutes - MM)/60;
  unsigned long HH = hours%60;
  m.setDisplayToDecNumber(HH*1000000 + MM*10000 + SS*100 + mm, 84);
}

void DisplayScores()
{
  const char* str = "     SOUL CALIbur 2     ";
  const int sz = 24;
 
  
  switch(Phase)
    {
      // Display "Soul Calibur 2"
      // j: show counter
      // i: char position
      case 0:         
       if(NewPhase){
         InitNewPhase(120);
         m.clearDisplay();
       }
       else if(j<1){
         if(i<sz){
           if(DelayDone()) i++;
           m.setDisplayToString(str + i);
         }
         else{
           i = 0;
           j++;
         }
       }
       else{
         NewPhase = true;
         Phase = 10;
       }
       break;
      
      //----------> Display Scores Phases
      // Show Player1 Name
      case 10:
        if(NewPhase)  InitNewPhase(300);
        else if(j<6){
          if(DelayDone()){ 
            j++;
            i += 2;
            if(i >= 6)  i=0;
            m.clearDisplay();
          }
          else
            m.setDisplayToString("PLAyEr 1", 0, i);     // replace with player name
        }
        else{
         NewPhase = true;
         Phase++;
        }
        break; 
        
      // Show Player1 Score
      case 11:
        if(NewPhase)  InitNewPhase(300);
        else if(j<6){
          if(DelayDone()) j++;
          if(j%2 == 1)  m.setDisplayToDecNumber(p1Score,0);
          else  m.clearDisplay();
        }
        else if(j ==6){
          PhaseDelay = 650;
          if(DelayDone()) j++;
        }
        else if(j == 7){
          PhaseDelay = 3000;
          if(DelayDone()) j++;
          m.setDisplayToDecNumber(p1Score,0);
        }
        else{
         NewPhase = true;
         Phase++;
        }
        break;
        
      // Show Player 2 name
      case 12:
         if(NewPhase){
           InitNewPhase(300);
           m.clearDisplay();
          }
        else if(j<6){
          if(DelayDone()){ 
            j++;
            i += 2;
            if(i >= 6)  i=0;
            m.clearDisplay();
          }
          else
            m.setDisplayToString("PLAyEr 2", 0, i);   // Replace with player name
        }
        else{
         NewPhase = true;
         Phase++;
        }
        break; 
        
      // Show Player 2 Score
      case 13:
        if(NewPhase)  InitNewPhase(300);
        else if(j<6){
          if(DelayDone()) j++;
          if(j%2 == 1)  m.setDisplayToDecNumber(p2Score,0);
          else  m.clearDisplay();
        }
        else if(j ==6){
          PhaseDelay = 650;
          if(DelayDone()) j++;
        }
        else if(j == 7){
          PhaseDelay = 3000;
          if(DelayDone()) j++;
          m.setDisplayToDecNumber(p2Score,0);
        }
        else{
         NewPhase = true;
         Phase = 0;
        }
        break;
        
      // <------------------------------------

      // Display Editing mode Scores Player 1
      case 20:
        if(NewPhase)  InitNewPhase(2000);
        if(!DelayDone()){
          m.setDisplayToDecNumber(p1Score, 0, false);
					m.setDisplayToString("Player 1",0, 0);        
					delay(30);
				}
        else{
         SaveScores();
         NewPhase = true;
         Phase = 0;
        }
        break;
        
      // Display Editing mode Scores Ett
      case 21:
        if(NewPhase)  InitNewPhase(2000);
        if(!DelayDone()){
          m.setDisplayToDecNumber(p2Score, 0, false);
					m.setDisplayToString("Player 2",0, 1);
					delay(30);
        }
        else{
         SaveScores();
         NewPhase = true;
         Phase = 0;
        }
        break;
    }
}

// Function that returns true if invalid File
bool NoFileOrEmpty(char* fileName){
	if(!SD.exists(fileName))
		return true;
	myFile = SD.open(fileName);
	if(myFile.available() < 10){
		myFile.close();
		return true;
	}
	return false;
} 

// Function to Read Scores from SD card
void ReadScores()
{
	//Create Scores if unexistant
	if(NoFileOrEmpty("scores.txt")){
		p2Score = 10;
		p1Score = 50;
		return;
	}
	 // Open the settings file for reading:
	myFile = SD.open("scores.txt");
  char character;
  String description = "";
  String value = "";
  boolean valid = true;
    // read from the file until there's nothing else in it:
    while (myFile.available()) 
		{
			character = myFile.read();
      if(character == '/'){
        // Comment - ignore this line
        while(character != '\n'){
          character = myFile.read();
        };
      } 
			else if(isalnum(character)){  // Add a character to the description
        description.concat(character);
      } 
			else if(character =='='){  // start checking the value for possible results
				// First going to trim out all trailing white spaces
				do {
					character = myFile.read();
				} while(character == ' ');
        if(description == "p1Score" || description == "p2Score") {
          value = "";
          while(character != '\n'){
            if(isdigit(character)){
              value.concat(character);
            } 
						else if(character != '\n' && character != '\r') /*&& character != 13)*/{
              // Use of invalid values
              valid = false;
            }
					if(myFile.available())
						character = myFile.read();
          };
          if (valid){
            char charBuf[12];
            value.toCharArray(charBuf,value.length()+1);
            // Convert chars to integer
						if(description == "p1Score")
							p1Score = atol(charBuf);
						else if(description == "p2Score")
							p2Score = atol(charBuf);
          }
        }
				else { // unknown parameter
          while(character != '\n')
          character = myFile.read();
        }
        description = "";
      } 
			else {
        // Ignore this character (could be space, tab, newline, carriage return or something else)
      }
    }
    // close the file:
    myFile.close();
}

// Function to Save Scores to the SD card
void SaveScores()
{
  m.clearDisplay();
	
  delay(2000);
  SD.remove("/scores.txt");
  myFile = SD.open("scores.txt", FILE_WRITE);
  // if the file opened okay, write to it:
  if (myFile) {
		myFile.println("p1Score = " + String(p1Score) + "\np2Score = " + String(p2Score) + "\n");
  }
	myFile.close();
}

// Function that initialises a new phase
void InitNewPhase(unsigned int phaseDelay)
{
  i = 0;
  j = 0;
  NewPhase = false;
  StartTime = millis();
  PhaseDelay = phaseDelay;
}

// Function that returns true if the delay is done.
// Sets the new startTime if true
bool DelayDone()
{
  if((millis() - StartTime) > PhaseDelay){
    StartTime = millis();
    return true;
  }
  return false;
}

//Function that handles buttons events
void CheckButtons()
{
  byte buttons = m.getButtons();
  *butt = buttons;
  switch (*butt) {
    case 0:
      Released = true;
      break;
    case 1: // Left button
      Mode = 0;
      Phase = 20;
      NewPhase = true;
      if(Released && p1Score > 0)
        p1Score--;
      Released = false;
      break;
    case 2:
      Mode = 0;
      Phase = 20;
      NewPhase = true;
      if(Released)
        p1Score++;
      Released = false;
      break;
    case 4:
      Mode = 0;
      Phase = 21;
      NewPhase = true;
      if(Released && p2Score > 0)
        p2Score--;
      Released = false;
      break;
    case 8:
      Mode = 0;
      Phase = 21;
      NewPhase = true;
      if(Released)
        p2Score++;
      Released = false;
      break;
    case 16:
      break;
    case 32:
      break;
    case 64:    
      Mode = 2;
      //Phase = 0;
      NewPhase = true;
      break;
    case 128: // Right Button
			if(CycleCounter > 20){
				ContinueTimer = 3;
				CycleCounter = 0;
			}
			else if(CycleCounter >= 0 && !Released) 
				CycleCounter++;
			
			else if(Released && TimerStartTime == 0)
				NewMode = true;
			else if(Released && ContinueTimer == 1 && Mode == 1)
				ContinueTimer = 2;
			else if(Released && ContinueTimer == 2 && Mode == 1)
				ContinueTimer = 1;

      Mode = 1;
			if(Released)	
				CycleCounter = 0;
			Released = false;
      break;
  }
    delay(20);
}
