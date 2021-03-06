
#include <stdlib.h>
#include <Wire.h>

const int BUF_SIZE = 80;
const int MAX_STATES = 5;
const byte CHANGE_INSTR_NAME = 1;
const byte GET_INSTR_STATE = 2;
const byte CHANGE_CONSOLE_CMD = 3;
const byte START_INSTR_STATE = 4;
const byte START_PRESS_CHECK = 5;
const byte BUTTON_PRESSED = 1;

const byte WAITING = 1;
const byte GAMEPLAY = 2;
const byte RESET = 3;
const int TIMEOUT = 5000;

const char* const names[] = {"Toilet Plunger", "second", "third", "fourth", "fifth", "sixth", "seventh", "eighth", "ninth"};
const char* const commands[] = {"Plunge Toilets%s",
                          "turn second to %s",
                          "set third to the position %s",
                          "make sure fourth is %s",
                          "fifth is the way to %s",
                          "turn the sixth dial to %s",
                          "%s the seventh",
                          "greet eighth with %s",
                          "turn ninth to %s position"};
const char* const targetNames [][MAX_STATES] = 
  {{"", ""},
   {"high", "low"},
   {"on", "off"},
   {"1", "2", "3", "4", "5"},
   {"good", "better", "best"},
   {"arr", "lmao"},
   {"tehe", "rofl"},
   {"sup", "ayo bb"},
   {"red", "blue", "green"}};

class Instrument {
  private:
    byte busAddr;
    byte stateCount;
    bool pendingCommand;
    const char* instrumentName;
    const char* commandMessage;
    const char* const* stateNames;

  public:

    byte getBeginState() {
      Wire.beginTransmission(busAddr);
      Wire.write(START_INSTR_STATE);
      Wire.endTransmission();
      
      Wire.requestFrom(busAddr, 1);
      byte state;
      while (Wire.available()) {
         state = Wire.read();
      }
      return state;
    }
  
    byte getState() {
      Wire.beginTransmission(busAddr);
      Wire.write(GET_INSTR_STATE);
      Wire.endTransmission();
      
      Wire.requestFrom(busAddr, 1);
      byte state;
      while (Wire.available()) {
         state = Wire.read();
      }
      return state;
    }

    void setName() {
      Wire.beginTransmission(busAddr);
      Wire.write(CHANGE_INSTR_NAME);
      Wire.write(instrumentName);
      Wire.endTransmission();
    }

    void setPendingCommand(bool val) {
      pendingCommand = val;
    }

    bool hasPendingCommand() {
      return pendingCommand;
    }

    byte getStateCount() {
      return stateCount;
    }

    String makeMessage(byte target) {
      char buf[BUF_SIZE];
      snprintf(buf, BUF_SIZE, commandMessage, stateNames[target]);
      String result = buf;
      return result;
    }


    Instrument(byte addr, int stateNum, const char* instName,
              const char* constructMessage, const char* const* sNames) {
      busAddr = addr;
      stateCount = stateNum;
      instrumentName = instName;
      commandMessage = constructMessage;
      stateNames = sNames;
      pendingCommand = false;
    }
};

Instrument instruments[] =
  {
     Instrument(0x10, 2, names[0], commands[0], targetNames[0]),
     //Instrument(0x11, 2, names[1], commands[1], targetNames[1]),
     //Instrument(0x12, 2, names[2], commands[2], targetNames[2]),
     //Instrument(0x13, 5, names[3], commands[3], targetNames[3]),
     //Instrument(0x14, 3, names[4], commands[4], targetNames[4]),
     //Instrument(0x15, 2, names[5], commands[5], targetNames[5]),
     //Instrument(0x16, 2, names[6], commands[6], targetNames[6]),
     //Instrument(0x17, 2, names[7], commands[7], targetNames[7]),
     //Instrument(0x18, 3, names[8], commands[8], targetNames[8])
   };

const int NUM_INSTR = sizeof(instruments) / sizeof(Instrument);

class Console {
  private:
    byte busAddr;
    byte currentInstrument;
    byte instrumentGoal;
    bool isReady;

  public:
    void setCommand(byte instr, byte goal) {
      instruments[instr].setPendingCommand(true);
      currentInstrument = instr;
      instrumentGoal = goal;
      
      String message = instruments[instr].makeMessage(goal);
      Wire.beginTransmission(busAddr);
      Wire.write(CHANGE_CONSOLE_CMD);
      Wire.write(message.c_str());
      Wire.endTransmission();
    }

    bool checkIfReady() {
      if (isReady) { return true; }
      Wire.beginTransmission(busAddr);
      Wire.write(START_PRESS_CHECK);
      Wire.endTransmission();
      
      Wire.requestFrom(busAddr, 1);
      byte state;
      while (Wire.available()) {
         state = Wire.read();
      }
      isReady = (state == BUTTON_PRESSED);
      return isReady;
    }

    void setNotReady() {
      isReady = false;
    }

    void clearCommand() {
      instruments[currentInstrument].setPendingCommand(false);
    }

    bool isCompleted() {
      Instrument& instr = instruments[currentInstrument];
      bool result = instr.getState() == instrumentGoal;
      Serial.print("Instr state is ");
      Serial.print(instr.getState());
      Serial.print(" goal is ");
      Serial.println(instrumentGoal);
      if (result) {
        Serial.print("Command completed for console 0x");
        Serial.println(busAddr, HEX);
      }
      return result;
    }

    void genCommand() {
      Serial.print("Generating command for console 0x");
      Serial.println(busAddr, HEX);
      
      byte nextInstr;
      // Only select an instrument that does not have a pending command 
      do {
        nextInstr = random(NUM_INSTR);
      }
      while (instruments[nextInstr].hasPendingCommand());
      
      byte nextGoal;
      do {
        nextGoal = random(instruments[nextInstr].getStateCount());
      }
      while (nextGoal == instruments[nextInstr].getState());

      Serial.print("  Generating command for instr ");
      Serial.print(nextInstr);
      Serial.print(", goal ");
      Serial.println(nextGoal);
      setCommand(nextInstr, nextGoal);
    }

    explicit Console(byte addr) : busAddr(addr) {
      isReady = false;
    }
};

Console consoles[] = {
  Console(0x1),
  //Console(0x2),
  //Console(0x3)
};

const int NUM_CONSOLES = sizeof(consoles) / sizeof(Console);

byte state;
int score;
int lastTime;

void setup() {
  Serial.begin(9600); // for debugging
  Wire.begin();

  delay(3000);
  Serial.println("Initializing...");
  
  randomSeed(analogRead(0));
  for (int i = 0; i < NUM_INSTR; i++) {
    instruments[i].setName();
    delay(2000);
  }
  for (int i = 0; i < NUM_CONSOLES; i++) {
    consoles[i].genCommand();
    delay(2000);
  }
  score = 0;
  lastTime = millis();
  state = WAITING;
}

void loop() {
  if (state == WAITING) {
    bool goToNext = true;
    for (int i = 0; i < NUM_CONSOLES; i++) {
      goToNext = goToNext && consoles[i].checkIfReady();
    }
    if(goToNext) {
      state = GAMEPLAY;
    }
  }
  else if (state == GAMEPLAY) {
    if (millis() - lastTime > TIMEOUT) {
      state = RESET;
    } else {
      for (int i = 0; i < NUM_CONSOLES; i++) {
        Console& current = consoles[i];
        if (current.isCompleted()) {
          current.clearCommand();
          current.genCommand();
          int newTime = millis();
          score += TIMEOUT - (newTime - lastTime);
          lastTime = newTime;
        }
      }
    }
  }
  else if (state == RESET) {
    for (int i = 0; i < NUM_CONSOLES; i++) {
      consoles[i].setNotReady();
    }
    //TODO: reset all the instrument names
    state = WAITING;
  }
  delay(10);
}
