#ifndef SERVICEHELPER_H
#define SERVICEHELPER_H

namespace OPI {

namespace ServiceHelper {

bool Start(const string& service);

bool Stop(const string& service);

bool Reload(const string& service);

bool IsRunning(const string& service);

pid_t GetPid(const string& service);

} // End NS
} // End NS
#endif // SERVICEHELPER_H
