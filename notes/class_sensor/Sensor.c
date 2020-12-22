// Sensor ctor & dtor
#define Sensor_Init (*Sensor_Construct)

Sensor *Sensor_Construct(void) {
  Sensor *s = (Sensor *)malloc(sizeof(Sensor));
  assert(s);
  _Sensor_SetValue(s, -1);
  fprintf(stdout, "class Sensor: allocated new object on %p\n", s);
  return s;
};

void Sensor_Destruct(Sensor * const s) {
  assert(s);
  fprintf(stdout, "class Sensor: freed new object on %p\n", s);
  free(s);
};

// Public methods
// -> = dereference by addr
ssize_t Sensor_GetValue(Sensor const * const s){
  assert(s);
  assert(s->value);
  return s->value;
};
ssize_t Sensor_GetFilterFrequency(Sensor const * const s){
  assert(s);
  assert(s->filterFrequency);
  return s->filterFrequency;
};
ssize_t Sensor_GetUpdateFrequency(Sensor const * const s){
  assert(s);
  assert(s->updateFrequency);
  return s->updateFrequency;
};

void Sensor_SetFilterFrequency(Sensor * const s, size_t filterFrequency){
  assert(s);
  s->filterFrequency = filterFrequency;
};
void Sensor_SetUpdateFrequency(Sensor * const s, size_t updateFrequency){
  assert(s);
  s->updateFrequency = updateFrequency;
};

// Private methods
static void _Sensor_SetName(Sensor * const s, char *name){
  assert(s);
  assert(name);
  strncpy(s->name, name, strlen(name));
};
static void _Sensor_SetValue(Sensor * const s, ssize_t value){
  assert(s);
  s->value = value;
};
