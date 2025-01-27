#include "BaseCommand.h"

#include <utility>

IBaseCommand::IBaseCommand(UCommandObject param)
    :mParam(std::move(param)){
}
