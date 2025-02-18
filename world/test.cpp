#include <spdlog/spdlog.h>
#include <mysqlx/xdevapi.h>
#include <iostream>

int main() {

    try {
        mysqlx::Session sess("localhost", 33060, "root", "12345678");
        auto schema = sess.getSchema("demo");

        if (auto table = schema.getTable("appearance"); table.existsInDatabase()) {
            table.insert("pid", "avatar", "avatar_frame", "update_time")
                    .values(100032, 1, 1, 1700003220)
                    .execute();

            auto result = table.select().where("pid = 100032").execute();

            if (auto row = result.fetchOne(); !row.isNull()) {
                std::cout << row[0] << row[1] << row[2] << row[3] << std::endl;
            }
        }

    } catch (std::exception &e) {
        spdlog::error("{}", e.what());
    }

    return 0;
}