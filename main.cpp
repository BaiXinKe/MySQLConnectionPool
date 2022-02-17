#include "mysqlConnectionPool.hpp"
#include <atomic>
#include <chrono>
#include <iostream>
#include <mysql-cppconn-8/jdbc/cppconn/statement.h>
#include <type_traits>
#include <typeinfo>

std::atomic_int threadIndex_ { 0 };

void threadFunc(std::shared_ptr<MySQLConnectionPool> pool)
{

    auto instance = pool->getConnector();

    std::cout << "thread: " << threadIndex_.fetch_add(1) << ": ";

    std::cout << "connection address: " << std::addressof(instance) << "\n";

    for (int j = 0; j < 10; j++)
        for (int i = 0; i < 10; i++) {
            auto instance = pool->getConnector();
            std::unique_ptr<sql::Statement> stmt(instance->createStatement());
            stmt->execute("use chauncy;");
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("select * from foo;"));

            while (res->next()) {
                std::cout << res->getInt64("id") << " ";
                std::cout << res->getString("name") << " ";
                std::cout << res->getBoolean("hasFun") << std::endl;
            }
        }

    std::this_thread::sleep_for(std::chrono::milliseconds(threadIndex_.load()));
}

int main()
{

    MySQLConnectionPool::initConnectionPool("127.0.0.1", 3306, "chauncy", "password", 32);
    auto pool = MySQLConnectionPool::getInstance();

    std::vector<std::thread> threads_;

    for (int i = 0; i < std::thread::hardware_concurrency(); i++) {
        threads_.emplace_back(&threadFunc, pool);
    }

    for (int i = 0; i < std::thread::hardware_concurrency(); i++) {
        threads_[i].join();
    }

    return 0;
}
