#pragma once
#include "Types.h"

const ExpressionSpec& expressionSpec(ExpressionId id);
const char* moodName(Mood mood);
const char* expressionName(ExpressionId expression);

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