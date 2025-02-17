#include <spdlog/spdlog.h>
#include <mysqlx/xdevapi.h>
#include <iostream>

int main() {

    try {
        mysqlx::Session sess("localhost", 33060, "root", "12345678");
        auto schema = sess.getSchema("demo");

        if (auto table = schema.getTable("avatar"); table.existsInDatabase()) {
            // table.insert("pid", "index", "expired_time", "activated", "in_used")
            //         .values(112002, 1, 21143224, true, true)
            //         .execute();

            auto result = table.select().where("pid = 112002").execute();

            if (auto row = result.fetchOne(); !row.isNull()) {
                std::cout << row[0] << row[1] << row[2] << row[3] << row[4] << std::endl;
            }
        }

    } catch (std::exception &e) {
        spdlog::error("{}", e.what());
    }

    return 0;
}