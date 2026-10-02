#include <Adafruit_NeoPixel.h>

#define PIN            6
#define NUMPIXELS      64

struct RGB {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Animation Settings
const unsigned int  COUNTDOWN_MS  = 10000;
const long          FADE_DURATION = 1500; // Interpolation Dauer der Farbe, wenn das Flugzeug abhebt
const int           PLANE_LENGTH  = 3;
const int           START_PIXEL   = 2;
const float         BRIGHTNESS    = 0.5;
const bool          BG_LIGHT      = true;

// Takeoff Simulation Params
const float         LIFTOFF_SPEED_KMH   = 350.0;
const float         LIFTOFF_DISTANCE_M  = 1700.0;
const float         RUNWAY_LENGTH_M     = 3152;

const float         LIFTOFF_SPEED_MS    = LIFTOFF_SPEED_KMH / 3.6;
const unsigned long LIFTOFF_TIME        = (2 * LIFTOFF_DISTANCE_M / LIFTOFF_SPEED_MS) * 1000.0;
const float         ACCELERATION        = LIFTOFF_SPEED_MS / (LIFTOFF_TIME / 1000.0);

// others
unsigned long waveStartTime = 0;

RGB applyBrightness(RGB color) {
  RGB out;
  out.r = (uint8_t)(color.r * BRIGHTNESS);
  out.g = (uint8_t)(color.g * BRIGHTNESS);
  out.b = (uint8_t)(color.b * BRIGHTNESS);

  return out;
}

RGB breathEffect(RGB color, long t, float cycleTime, int minBrightness) {
  // Sinus-Atmung 0..1
  float breath = (sin(t / cycleTime * TWO_PI) + 1.0) * 0.5;

  // Mindesthelligkeit erzwingen
  uint8_t totalBrightness = minBrightness + breath * (255 - minBrightness);
  float factor = totalBrightness / 255.0f;

  RGB out;
  out.r = (uint8_t)(color.r * factor);
  out.g = (uint8_t)(color.g * factor);
  out.b = (uint8_t)(color.b * factor);

  return out;
}

RGB getCountdownColor(long takeoffRollTime) {
  RGB c;

  float t = ((float)takeoffRollTime / COUNTDOWN_MS) + 1;
  if (t < 0.0) t = 0.0;
  if (t > 1.0) t = 1.0;

  if (t < 0.5) {
    // Rot → Gelb
    float p = t / 0.5;
    c.r = 255;
    c.g = (uint8_t)(255 * p);
    c.b = 0;
  } else {
    // Gelb → Grün
    float p = (t - 0.5) / 0.5;
    c.r = (uint8_t)(255 * (1.0 - p));
    c.g = 255;
    c.b = 0;
  }

  return c;
}

RGB getPlaneColor(long t) {
  RGB ground = {0, 255, 100};
  RGB air    = {100, 100, 255};

  RGB out;

  if (t <= LIFTOFF_TIME) {
    return ground;
  }

  float p = float(t - LIFTOFF_TIME) / float(FADE_DURATION);
  if (p > 1.0f) p = 1.0f; // Maximum = 1

  // Lineare Interpolation
  out.r = ground.r * (1.0f - p) + air.r * p;
  out.g = ground.g * (1.0f - p) + air.g * p;
  out.b = ground.b * (1.0f - p) + air.b * p;

  return out;
}

int getPlanePos(long t_ms) {
  float t_s = t_ms / 1000.0;
  float y = 0.5 * ACCELERATION * pow(t_s, 2);

  // normalise y
  y = y / RUNWAY_LENGTH_M;

  int pixelIdx = ((NUMPIXELS - START_PIXEL) * y) + START_PIXEL;
  return pixelIdx;
}

int getPlaneSpeed(long t_ms) {
  float t_s = t_ms / 1000.0;
  float y = ACCELERATION * t_s;

  // normalise y
  y = y / LIFTOFF_SPEED_MS;

  int trailLength = PLANE_LENGTH * y;
  if (trailLength < 1) trailLength = 1;
  return trailLength;
}

void displayPlaneLEDs(RGB color, int headIdx, int length, bool fade) {
  RGB c;

  for (int i=0; i<length; i++) {
    int pixelIdx = headIdx - i;
    if (pixelIdx >= 0 && pixelIdx < NUMPIXELS) {
      if (fade) {
        float b = pow((length - i) / (float)length, 3.0); // Kubischer Fade-Effekt
        c.r = color.r * b;
        c.g = color.g * b;
        c.b = color.b * b;
      } else {
        c = color;
      }
      pixels.setPixelColor(pixelIdx, pixels.Color(c.r, c.g, c.b));
    }
  }
}

void setup() {
  pixels.begin();
  pixels.clear();
  pixels.show();
  waveStartTime = millis();
}

void loop() {
  unsigned long now = millis();
  unsigned long elapsed = now - waveStartTime;

  long takeoffRollTime = elapsed - COUNTDOWN_MS;

  // background
  for (int i=0; i<NUMPIXELS; i++) {
    RGB backgroundColor;

    backgroundColor.r = 7;
    backgroundColor.g = 3;
    backgroundColor.b = 1;

    if (BG_LIGHT) {
      pixels.setPixelColor(i, pixels.Color(backgroundColor.r, backgroundColor.g, backgroundColor.b));
    } else {
      pixels.clear();
    }
  }

  if (takeoffRollTime < 0) {
    // Animation Teil 1: Countdown zum Takeoff
    
    RGB color;

    color = getCountdownColor(takeoffRollTime);
    color = breathEffect(color, elapsed, COUNTDOWN_MS/2, 0);
    color = applyBrightness(color);

    displayPlaneLEDs(color, START_PIXEL, 3, true);
  } else {
    // Animation Teil 2: TakeoffRoll

    RGB color;
    color = getPlaneColor(takeoffRollTime);
    color = applyBrightness(color);

    int pos;
    pos = getPlanePos(takeoffRollTime);

    // int len = getPlaneSpeed(takeoffRollTime);

    displayPlaneLEDs(color, pos, PLANE_LENGTH, true);

    // Animation beenden und wiederholen
    if (pos - PLANE_LENGTH > NUMPIXELS) {
      waveStartTime = now;
      elapsed = 0;
    }
  }

  pixels.show();
  delay(10);
}