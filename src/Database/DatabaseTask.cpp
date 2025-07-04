#include "DatabaseTask.h"

#include <utility>

UDBTrans::UDBTrans(ATransaction functor)
    : functor_(std::move(functor)) {
}

void UDBTrans::Execute(mysqlx::Schema &schema) noexcept {
    try {
        schema.getSession().startTransaction();
        std::invoke(functor_, schema);
        schema.getSession().commit();
    } catch (const std::exception &e) {
        SPDLOG_ERROR("{} - Database Error: {}", __FUNCTION__, e.what());
    }
}
