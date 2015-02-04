#ifndef BACKUPHELPER_H
#define BACKUPHELPER_H

#include <memory>
#include <string>
#include <list>
using namespace std;

namespace OPI {


class BackupInterface
{
public:
	virtual bool MountLocal(const string& configpath) = 0;
	virtual bool MountRemote(const string& configpath) = 0;
	virtual list<string> GetLocalBackups() = 0;
	virtual list<string> GetRemoteBackups() = 0;
	virtual void UmountLocal() = 0;
	virtual void UmountRemote() = 0;
};

typedef shared_ptr<BackupInterface> BackupInterfacePtr;

class OPIBackup: public BackupInterface
{
public:
	OPIBackup();

	virtual bool MountLocal(const string& configpath);
	virtual bool MountRemote(const string& configpath);
	virtual list<string> GetLocalBackups();
	virtual list<string> GetRemoteBackups();
	virtual void UmountLocal();
	virtual void UmountRemote();

	virtual ~OPIBackup();
};


class BackupHelper
{
public:
	BackupHelper(const string& pwd, BackupInterfacePtr iface = BackupInterfacePtr( new OPIBackup ));

	bool MountLocal(const string& configpath);
	bool MountRemote(const string& configpath);
	list<string> GetLocalBackups();
	list<string> GetRemoteBackups();

	void UmountLocal();
	void UmountRemote();

	void CreateConfig();

	string GetConfigFile();

	~BackupHelper();
private:
	BackupInterfacePtr iface;
	string pwd;
	char tmpfilename[128];
};

typedef shared_ptr<BackupHelper> BackupHelperPtr;

}

#endif // BACKUPHELPER_H
