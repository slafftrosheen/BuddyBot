#pragma once
#include "Types.h"

class PersonaManager {
public:
  void begin();
  void next();
  void set(PersonaId id);

  const PersonaProfile& current() const;
  PersonaId id() const;

  const char* messageFor(Mood mood) const;
  uint16_t moodColor(Mood mood) const;

private:
  PersonaId _id = PersonaId::PIXEL;
};