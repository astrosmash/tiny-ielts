#include "Sensor.h"

int main(void) {
  Sensor *sensor = Sensor_Init();
  fprintf(stdout, "main: inited sensor address %p(%lu b long)\n", sensor, sizeof(*sensor));

  Sensor_SetUpdateFrequency(sensor, 2500000);
  ssize_t val = Sensor_GetValue(sensor);
  size_t updfreq = Sensor_GetUpdateFrequency(sensor);
  fprintf(stdout, "main: sensor %zd updfreq %zu\n", val, updfreq);

  Sensor_Destruct(sensor);

  return EXIT_SUCCESS;
}
