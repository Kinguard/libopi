#ifndef SERVICEHELPER_H
#define SERVICEHELPER_H

#include <string>

namespace OPI {

namespace ServiceHelper {

/**
 * @brief Start, start system service
 * @param service, service to start
 * @return true upon success
 */
bool Start(const std::string& service);

/**
 * @brief Stop, stop system service
 * @param service, service to stop
 * @return true upon success
 */
bool Stop(const std::string& service);

/**
 * @brief Enable, enable system service
 * @param service, service to enable
 * @return true upon success
 */
bool Enable(const std::string& service);

/**
 * @brief Disable, disable system service
 * @param service, service to disable
 * @return true upon success
 */
bool Disable(const std::string& service);

/**
 * @brief Reload, reload system service
 * @param service, service to reload
 * @return true upon success
 */
bool Reload(const std::string& service);

/**
 * @brief Restart, restart system service
 * @param service, service to restart
 * @return true upon success
 */
bool Restart(const std::string& service);

/**
 * @brief IsRunning check if service is running
 * @param service, service to check
 * @return true if service is running
 */
bool IsRunning(const std::string& service);

/**
 * @brief IsEnabled, check if service is enabled
 * @param service, service to check
 * @return true if enabled
 */
bool IsEnabled(const std::string& service);

/**
 * @brief IsAvailable, check if service is enabled/installed on system
 * @param service, service to check
 * @return true if available
 */
bool IsAvailable(const std::string& service);

/**
 * @brief GetPid, return pid of service if possible
 * @param service, service to check
 * @return pid > 0 if pid determined 0 if not
 */
pid_t GetPid(const std::string& service);

} // End NS
} // End NS
#endif // SERVICEHELPER_H
