#include <EntityId.h>
namespace common {
  std::ostream& operator<<(std::ostream& os, const EntityId& entityId)
  {
    os << static_cast<unsigned int>(entityId);
    return os; 
  }
}