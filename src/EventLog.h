#pragma once
#include <Arduino.h>

struct EventLogEntry {
  uint32_t timestampMs;
  const char* severity;
  const char* code;
};

class EventLog {
public:
  static const size_t CAPACITY = 16;
  
  void begin() {}
  void log(const char* severity, const char* code) {
    if (_count < CAPACITY) {
      _entries[_count] = {millis(), severity, code};
      _count++;
    } else {
      _entries[_head] = {millis(), severity, code};
      _head = (_head + 1) % CAPACITY;
    }
  }
  
  size_t count() const { return _count; }
  
  // Return entries in chronological order
  size_t getEntries(EventLogEntry* outBuffer, size_t maxCount) const {
    size_t copyCount = (_count > maxCount) ? maxCount : _count;
    if (_count < CAPACITY) {
      for (size_t i = 0; i < copyCount; i++) {
        outBuffer[i] = _entries[i];
      }
    } else {
      for (size_t i = 0; i < copyCount; i++) {
        outBuffer[i] = _entries[(_head + i) % CAPACITY];
      }
    }
    return copyCount;
  }
  
  // Singleton instance
  static EventLog& instance() {
    static EventLog inst;
    return inst;
  }

private:
  EventLogEntry _entries[CAPACITY];
  size_t _head = 0;
  size_t _count = 0;
};
