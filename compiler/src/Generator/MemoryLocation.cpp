#include "MemoryLocation.h"

#include <Generator/GenerationException.h>

Cepheid::Gen::MemoryLocation::MemoryLocation(std::string_view location) : m_location(location) {
}

std::string Cepheid::Gen::MemoryLocation::asAsm(size_t size) const {
  switch (size) {
    case 1:
      return "BYTE " + m_location;
    case 2:
      return "WORD " + m_location;
    case 4:
      return "DWORD " + m_location;
    case 8:
      return m_location;
    default:
      throw GenerationException("Unexpected size");
  }
}
