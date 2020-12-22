#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


// class Sensor
#define maxSensorNameLength 255

typedef struct {
  char name[maxSensorNameLength + 1];
  ssize_t value;
  size_t filterFrequency;
  size_t updateFrequency;
} Sensor;

// Sensor ctor & dtor
Sensor *Sensor_Construct(void);
void Sensor_Destruct(Sensor * const s);

// Public methods
ssize_t Sensor_GetValue(Sensor const * const s);
ssize_t Sensor_GetFilterFrequency(Sensor const * const s);
ssize_t Sensor_GetUpdateFrequency(Sensor const * const s);

void Sensor_SetFilterFrequency(Sensor * const s, size_t filterFrequency);
void Sensor_SetUpdateFrequency(Sensor * const s, size_t updateFrequency);

// Private methods
static void _Sensor_SetName(Sensor * const s, char *name);
static void _Sensor_SetValue(Sensor * const s, ssize_t value);

// Definition
#include "Sensor.c"
