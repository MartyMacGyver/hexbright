#ifndef STAMPEDE_H
#define STAMPEDE_H

#define OFF_MODE 0
#define ON_MODE 1
#define AIRPLANE_MODE 2
#define LOW_POWER 3

class hexbright;

class Stampede {
    public:
      Stampede(hexbright *hb);
      void doTick();
      void doUpdate();
    
    private:
      void doStrobe();
      void scaleBrightness();
      void changeMode(const int newMode);
      void displayBatteryLow();
      void displayCharge();
      void displayPowerSave();
      int pollStates();
      
      int currentBrightness;
      int onTime;
      int lowPowerTime;
      bool scaleBackwards;
      int mode;
      bool changingBrightness;
      bool strobing;
      bool powerSaveEnabled;
      int tapCount;
      int lastTap;
      hexbright *hbLib;
};

#endif
