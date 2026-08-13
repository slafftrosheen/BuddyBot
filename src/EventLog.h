#pragma once
#include <Arduino.h>

struct EventLogEntry {
  uint32_t timestampMs = 0;
  const char* severity = "INFO";
  const char* code = "unknown";
  const char* component = "system";
  uint32_t correlationId = 0;
};

class EventLog {
public:
  static const size_t CAPACITY = 16;
  
  void begin() {}
  void log(
    const char* severity,
    const char* code,
    const char* component = "system",
    uint32_t correlationId = 0
  ) {
    lock();
    if (_count < CAPACITY) {
      _entries[_count] = {millis(), severity, code, component, correlationId};
      _count++;
    } else {
      _entries[_head] = {millis(), severity, code, component, correlationId};
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
