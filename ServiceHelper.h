#ifndef SERVICEHELPER_H
#define SERVICEHELPER_H

#include <string>

namespace OPI {

namespace ServiceHelper {

bool Start(const std::string& service);

bool Stop(const std::string& service);

bool Reload(const std::string& service);

bool IsRunning(const std::string& service);

pid_t GetPid(const std::string& service);

} // End NS
} // End NS
#endif // SERVICEHELPER_H
