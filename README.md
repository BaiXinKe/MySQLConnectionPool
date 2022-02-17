# A mysql connection pool use modern C++

MySQL connection pooling, implemented using modern C++, implements automatic reclamation of connections through the RAII mechanism and decorator pattern.  
Current problem: Condition variable signal loss that can occur when too many threads are started.

使用现代 C++实现的 MySQL 连接池，通过 RAII 机制和装饰器模式实现了对连接的自动回收。
目前存在的问题：当启动过多的线程时可能发生的条件变量信号丢失。
