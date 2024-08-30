//
// Created by Seres67 on 25/08/2024.
//

#ifndef SESSION_HPP
#define SESSION_HPP

#include <chrono>

typedef struct
{
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
} Session;
void load_start_session();
void save_session();
void save_session_sync();
void pull_session();
void check_session();

#endif // SESSION_HPP
