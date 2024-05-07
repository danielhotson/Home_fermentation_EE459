enum STATE {
  INITIAL,
  BREW,
};

/*****  Functions  *****/

/*
        Checks inputs from the temperature sensor and real-time clock
*/
void CheckInputs(void);

/*
  Format updated Temperature and Elapsed Time readings and put them on the
  display in the form: Temp: TTT.T HEATING (if relay is on) Time Elapsed:
    DD:HH:MM:SS

*/
void UpdateScreen(void);
void UpdateOutputs(void);

/*
        Initialize all of the components (LCD, RTC, Temperature Sensor)
*/
void Init(void);