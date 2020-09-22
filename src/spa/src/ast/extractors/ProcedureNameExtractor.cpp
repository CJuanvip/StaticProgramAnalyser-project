#include <ProcedureNameExtractor.h>
#include <ASTToString.h>
namespace ast{
  using namespace common;
  void transferProcedureName(const ProcDef& procedure, pkb::PKBTableManager& mgr) {
    auto& namesTable = mgr.getNamesTable();
    auto& nameTypesTable = mgr.getNameTypesTable();
    std::string procedureName = procedure.name;
    const auto& nameIds = namesTable.filterRight(procedureName);
    if (nameIds.empty()) {
      auto nameId = namesTable.count() + 1;
      namesTable.insert(nameId, procedureName);
      nameTypesTable.insert(nameId, EntityId::Procedure);
    } else {
      if (nameIds.size() != 1) {
        throw std::runtime_error("Each name_string should have only 1 name_id.");
      }

      auto& nameId = *nameIds.begin();
      const auto& types = nameTypesTable.filterLeft(nameId);
      if (types.find(EntityId::Procedure) == types.end()) {
        throw std::runtime_error("Should not have a procedure name with non-procedure type");
      } else {
        // Duplicate Procedure Name
        throw SemanticError(ERROR_MESSAGE_DUPLICATE_PROC_NAME);
      }
    }
  }
}