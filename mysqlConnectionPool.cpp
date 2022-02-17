#include "mysqlConnectionPool.hpp"
#include "Connector.hpp"

size_t MySQLConnectionPool::num_connection_max_ {};

std::string MySQLConnectionPool::host_;
uint16_t MySQLConnectionPool::port_;
std::string MySQLConnectionPool::user_;
std::string MySQLConnectionPool::password_;

std::once_flag MySQLConnectionPool::init_flag_;

std::shared_ptr<MySQLConnectionPool> MySQLConnectionPool::singleton_instance_ { nullptr };

void MySQLConnectionPool::initSingletonInsance()
{
    singleton_instance_.reset(new MySQLConnectionPool());
    singleton_instance_->init_connection();
}

std::shared_ptr<MySQLConnectionPool> MySQLConnectionPool::getInstance()
{
    std::call_once(init_flag_, MySQLConnectionPool::initSingletonInsance);
    return singleton_instance_;
}

void MySQLConnectionPool::initConnectionPool(std::string_view host, uint16_t port,
    std::string_view user, std::string_view password, size_t max_connection)
{
    host_ = host;
    port_ = port;
    user_ = user;
    password_ = password;
    num_connection_max_ = max_connection;
}

void MySQLConnectionPool::connDeletor(sql::Connection* conn)
{
    conn->close();
    delete conn;
}

MySQLConnectionPool::MySQLConnectionPool()
    : driver { sql::mysql::get_mysql_driver_instance() }
    , curr_num_conn_ { static_cast<size_t>(num_connection_max_ / 2) }
{
}

std::unique_ptr<sql::Connection> MySQLConnectionPool::getConnector()
{
    std::unique_lock<std::mutex> lock(mtx_);

    if (!conns_.empty()) {
        ConnectionPtr conn = std::move(conns_.back());
        conns_.pop_back();
        return std::make_unique<Connector>(shared_from_this(), std::move(conn));
    } else if (curr_num_conn_ < num_connection_max_) {
        std::string host = host_ + ":" + std::to_string(port_);
        ConnectionPtr conn { driver->connect(host.c_str(), user_.c_str(), password_.c_str()), connDeletor };
        curr_num_conn_++;
        return std::make_unique<Connector>(shared_from_this(), std::move(conn));
    }

    cond_.wait(lock, [this] { return !this->conns_.empty(); });
    ConnectionPtr conn = std::move(conns_.back());
    conns_.pop_back();
    return std::make_unique<Connector>(shared_from_this(), std::move(conn));
}

void MySQLConnectionPool::returnTheConnection(ConnectionPtr conn)
{
    std::lock_guard<std::mutex> lock(mtx_);
    conns_.push_back(std::move(conn));
    cond_.notify_one();
}

void MySQLConnectionPool::init_connection()
{
    std::string host = host_ + ":" + std::to_string(port_);
    for (int i = 0; i < num_connection_max_ / 2; i++) {
        conns_.emplace_back(driver->connect(host, user_.c_str(), password_.c_str()), connDeletor);
    }
}