
#include <stdlib.h>
#include <Wire.h>

const int NUM_CONSOLES = 3;
const int NUM_INSTR = 9;
const int BUF_SIZE = 80;
const int MAX_STATES = 5;
const byte CHANGE_INSTR_NAME = 1;
const byte GET_INSTR_STATE = 2;
const byte CHANGE_CONSOLE_CMD = 3;

const char* names[] = {"first", "second", "third", "fourth", "fifth", "sixth", "seventh", "eighth", "ninth"};
const char* commands[] = {"set first to %s",
                          "turn second to %s",
                          "set third to the position %s",
                          "make sure fourth is %s",
                          "fifth is the way to %s",
                          "turn the sixth dial to %s",
                          "%s the seventh",
                          "greet eighth with %s",
                          "turn ninth to %s position"};
const char* targetNames [][MAX_STATES] = 
  {{"up", "down"},
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
    byte target;
    const char* instrumentName;
    const char* commandMessage;
    const char** stateNames;

  public:
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

    void setName(String s) {
      Wire.beginTransmission(busAddr);
      Wire.write(CHANGE_INSTR_NAME);
      Wire.write(instrumentName);
      Wire.endTransmission();
    }

    byte getTarget() { 
      return target;
    }

    void setTarget(byte newTarget) {
      target = newTarget;
    }

    byte getStateCount(){
      return stateCount;
    }

    String makeMessage(byte target) {
      char buf[BUF_SIZE];
      snprintf(buf, BUF_SIZE, commandMessage, stateNames[target]);
      String result = buf;
      return result;
    }


    Instrument(byte addr, int stateNum, const char* instName,
              const char* constructMessage, const char** sNames) {
      busAddr = addr;
      stateCount = stateNum;
      instrumentName = instName;
      commandMessage = constructMessage;
      stateNames = sNames;
    }
};

Instrument instruments[NUM_INSTR] =
  {
     Instrument(0x10, 2, names[0], commands[0], targetNames[0]),
     Instrument(0x11, 2, names[1], commands[1], targetNames[1]),
     Instrument(0x12, 2, names[2], commands[2], targetNames[2]),
     Instrument(0x13, 5, names[3], commands[3], targetNames[3]),
     Instrument(0x14, 3, names[4], commands[4], targetNames[4]),
     Instrument(0x15, 2, names[5], commands[5], targetNames[5]),
     Instrument(0x16, 2, names[6], commands[6], targetNames[6]),
     Instrument(0x17, 2, names[7], commands[7], targetNames[7]),
     Instrument(0x18, 3, names[8], commands[8], targetNames[8])
   };

class Console {
  private:
    byte busAddr;
    byte currentInstrument;
    byte instrumentGoal;

  public:
    void setCommand(byte instr, byte goal) {
      String message;
      char buf[BUF_SIZE];
      instruments[instr].setTarget(goal);
      currentInstrument = instr;
      instrumentGoal = goal;
      message = instruments[instr].makeMessage(goal);
      Wire.beginTransmission(busAddr);
      Wire.write(CHANGE_CONSOLE_CMD);
      Wire.write(message.c_str());
      Wire.endTransmission();
      return;
    }

    int currInstrument() {
      return currentInstrument;
    }

    explicit Console(byte addr) : busAddr(addr) {}
};

Console consoles[NUM_CONSOLES] = {
  Console(0x1),
  Console(0x2),
  Console(0x3)
};

void setup() {
  Wire.begin();
}

void loop() {
  for (int i = 0; i < NUM_CONSOLES; i++) {
    Console& current = consoles[i];
    Instrument& instr = instruments[current.currInstrument()];
    if (instr.getState() == instr.getTarget()) {
      //TODO: make sure next state is not contradicting current pending states
      byte nextInstr = rand() % NUM_INSTR;
      byte nextGoal = rand() % instr.getStateCount();
      current.setCommand(nextInstr, nextGoal);
    }
  }
  delay(10);
}

