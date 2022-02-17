#ifndef _MYSQL_CONNECTION_POOL__
#define _MYSQL_CONNECTION_POOL__

#include <boost/noncopyable.hpp>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <mysql-cppconn-8/jdbc/mysql_driver.h>
#include <string_view>
#include <thread>
#include <vector>

class MySQLConnectionPool : public boost::noncopyable,
                            public std::enable_shared_from_this<MySQLConnectionPool> {
private:
    static void connDeletor(sql::Connection* conn);

public:
    static constexpr size_t DEFAULT_INIT_CONN_SIZE { 24 };
    using ConnectionPtr = std::unique_ptr<sql::Connection, decltype(MySQLConnectionPool::connDeletor)*>;

    std::unique_ptr<sql::Connection> getConnector();
    void returnTheConnection(ConnectionPtr conn);

    static void initConnectionPool(std::string_view host, uint16_t port,
        std::string_view user, std::string_view password, size_t max_connection = DEFAULT_INIT_CONN_SIZE);

    static std::shared_ptr<MySQLConnectionPool> getInstance();

private:
    static void initSingletonInsance();
    void init_connection();

private:
    explicit MySQLConnectionPool();
    static size_t num_connection_max_;

    std::mutex mtx_;
    std::condition_variable cond_;

    sql::mysql::MySQL_Driver* driver;
    std::vector<ConnectionPtr> conns_;
    size_t curr_num_conn_;

    static std::string host_;
    static uint16_t port_;
    static std::string user_;
    static std::string password_;

    static std::shared_ptr<MySQLConnectionPool> singleton_instance_;
    static std::once_flag init_flag_;
};

#endif