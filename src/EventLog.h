#pragma once
#include <Arduino.h>

struct EventLogEntry {
  uint32_t timestampMs;
  const char* severity;
  const char* code;
  const char* component;
  char intentId[37];
};

class EventLog {
public:
  static const size_t CAPACITY = 16;
  
  void begin() {}
  void log(
    const char* severity,
    const char* code,
    const char* component = "system",
    const char* intentId = nullptr
  ) {
    lock();
    if (_count < CAPACITY) {
      _entries[_count].timestampMs = millis();
      _entries[_count].severity = severity;
      _entries[_count].code = code;
      _entries[_count].component = component;
      if (intentId) {
          strncpy(_entries[_count].intentId, intentId, sizeof(_entries[_count].intentId) - 1);
          _entries[_count].intentId[sizeof(_entries[_count].intentId) - 1] = '\0';
      } else {
          _entries[_count].intentId[0] = '\0';
      }
      _count++;
    } else {
      _entries[_head].timestampMs = millis();
      _entries[_head].severity = severity;
      _entries[_head].code = code;
      _entries[_head].component = component;
      if (intentId) {
          strncpy(_entries[_head].intentId, intentId, sizeof(_entries[_head].intentId) - 1);
          _entries[_head].intentId[sizeof(_entries[_head].intentId) - 1] = '\0';
      } else {
          _entries[_head].intentId[0] = '\0';
      }
      _head = (_head + 1) % CAPACITY;
    }
    unlock();
  }
  
  size_t count() const {
    lock();
    const size_t value = _count;
    unlock();
    return value;
  }
  
  // Return entries in chronological order
  size_t getEntries(EventLogEntry* outBuffer, size_t maxCount) const {
    if (!outBuffer || maxCount == 0) {
      return 0;
    }

    lock();
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
    unlock();
    return copyCount;
  }
  
  // Singleton instance
  static EventLog& instance() {
    static EventLog inst;
    return inst;
  }

private:
  void lock() const {
#if defined(ARDUINO_ARCH_ESP32)
    portENTER_CRITICAL(&_mutex);
#endif
  }

  void unlock() const {
#if defined(ARDUINO_ARCH_ESP32)
    portEXIT_CRITICAL(&_mutex);
#endif
  }

  EventLogEntry _entries[CAPACITY];
  size_t _head = 0;
  size_t _count = 0;
#if defined(ARDUINO_ARCH_ESP32)
  mutable portMUX_TYPE _mutex = portMUX_INITIALIZER_UNLOCKED;
#endif
};
