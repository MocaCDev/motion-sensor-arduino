// include the library code:
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to

const int rs = 12, en = 10, d4 = 9, d5 = 8, d6 = 7, d7 = 6;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define rows        16
#define cols        2

/* Different amounts of time for `detect_for`.
 * Max is a minute.
 * */
enum times {
  second = 1000,
  tsecond = 10000, // tsecond - ten seconds
  thsecond = 30000, // thsecond - thirty seconds
  ffsecond = 45000, // ffsecond - fourty five seconds
  minute = 60000
};

static int tot = 0;
/*
   * hpf_m - Has Printed For Motion
   * hpf_nm - Has Printed For No Motion
   * */
static bool hpf_m = false;
static bool hpf_nm = false;

/* The "total amount" of motion detected in one sitting */
static uint8_t total_motion = 0;

/* For `detect_for`. This array will store the total amount of motion detections that happened within X amount of miliseconds.
 * mdarr - motion detection array
 * */
static bool mdarr[100];

class MotionSensor {
  public:

  /* ptdm - Pin To Detect Motion */
  uint8_t ptdm;

  /* Keep track on whether or not we have printed */
  bool has_printed = false;

  /* dr_bps - Data Rates in Bits Per Second */
  MotionSensor() {}

  /*
   * set_ptdm_and_bps: setup the (p)in (t)o (d)etect (m)otion and the data rate in (b)its (p)er (s)econd
   *  input:
   *    uint8_t ptdm: pin # that will send high signal if there is motion, or low signal if there is no motion
   *    uint16_t: data rate in BPS (Bits Per Second)
   *  return: nothing
   * */
  void set_ptdm_and_bps(uint8_t ptdm, uint16_t bps) {
    ptdm = ptdm;

    Serial.begin(bps);
    pinMode(ptdm, INPUT);

    Serial.flush();
  }

  /*
   * setup_lcd: init interface to LCD screen (specified width/height of the display)
   *  input: none
   *  return: nothing
   * */
  void setup_lcd() {
    lcd.begin(rows, cols);
  }

  /*
   * detect: read "status" from ptdm (check if the signal is high or low). If ptdm is high, print "MOTION" to LCD screen else print "NO MOTION".
   *  input:
   *    uint8_t ptdm: pin to detect motion (ptdm). Whatever pin is hooked to the `output` of the HC-SR501 PIR MOTION SENSOR.
   *    bool dtmm: detect too much motion (dtmm). True to detect too much else false.
   *    uint8_t motion_count: This is the amount of motion detected all at once. If the total motion number reaches `motion_count` it will display "TM MOTION".
   *  return: nothing
   * */
  void detect(uint8_t ptdm, bool dtmm, uint8_t motion_count) {
    /* Read the status from the `output` of the HC-SR501 PIR MOTION SENSOR. If high, will return `true` else `false`. */
    bool motion = digitalRead(ptdm);

    if(motion) {

      /* Does the user want to detect too much motion (dtmm)? */
      if(dtmm) {

        /* If so, increment the total amount of motion happening in one sitting. */
        total_motion++;

        /* If the total amount of motion is >= the total amount of motion the user wants to lookout for,
         * prompt "TM Motion" on the LCD 1602.
         * Clear the LCD 1602 display screen, set the cursor and display the message.
         * Set hpf_m and hpf_nm to false since we are now displaying a message for too much motion.
         * */
        if(total_motion >= motion_count) {
          hpf_m = false;
          hpf_nm = false;

          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("TM Motion");

          /* Skip everything else. Go straight to the end of the function. */
          goto out;
        }
      }

      /* 
       * Even if the user wants to detect too much motion (dtmm), if `total_motion` is not >= `motion_count` we will end up here.
       * If the program hasn't printed for motion (hpf_m (has printed for motion) is false), then go ahead set `hpf_m`
       * to true (since we are now printing for motion), set `hpf_nm` to false (since we are no longer printing for no motion),
       * clear the LCD 1602 display, set the cursor and display "Motion".
       * */
      if(!(hpf_m))
      {
        hpf_m = true;
        hpf_nm = false;

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Motion");
      }
    } else {

      /* If motion has stopped, then set `total_motion` to zero since any sort of continous motion would not be happening. */
      total_motion = 0;

      /*
       * If the program hasn't printed for no motion (hpf_nm (has printed for no motion) is false), then go ahead and set `hpf_nm`
       * to true (since we are now printing for no motion), set `hpf_m` to false (since we are no longer printing for motion),
       * clear the LCD 1602 display, set the cursor and display "No Motion".
       * */
      if(!(hpf_nm))
      {
        hpf_m = false;
        hpf_nm = true;

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("No Motion");
      }
    }

    /* End of function. Delay for 1000 miliseconds. */
    out:
    delay(1000);
  }

  /* Testing function for testing things. */
  void test_detect(uint8_t ptdm, bool dtmm, uint8_t motion_count) {
    bool motion = digitalRead(ptdm);

    if(motion) {

      /* Does the user want to detect too much motion (dtmm)? */
      if(dtmm) {

        /* If so, increment. */
        total_motion++;

        if(total_motion >= motion_count) {
          Serial.println("Too much motion");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("TM Motion");
          goto out;
        }
      }

      if(!(hpf_m))
      {
        hpf_m = true;
        hpf_nm = false;
        Serial.println("Motion");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Motion");
      }
    } else {
      total_motion = 0;
      if(!(hpf_nm))
      {
        hpf_m = false;
        hpf_nm = true;
        Serial.println("No Motion");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("No Motion");
      }
    }

    //lcd.blink();
    out:
    delay(1000);
  }

  void detect_for(uint8_t ptdm, enum times time, bool dtmm, uint8_t motion_count) {
    /* ta - time accumulated. */
    uint8_t ta = 0;

    while((ta * 1000) < time) {
      Serial.println("Here");
      ta++;
      delay(second);
    }
    delay(second);
    exit(0);
  }

  ~MotionSensor() {}
};

static MotionSensor ms;

void setup() {
  ms.set_ptdm_and_bps(2, 9600);
  ms.setup_lcd();
}

void loop() {

  ms.detect_for(2, 0x2710, false, 0);

}
