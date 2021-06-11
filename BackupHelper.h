#ifndef BACKUPHELPER_H
#define BACKUPHELPER_H

#include <memory>
#include <string>
#include <list>
using namespace std;


//TODO: Generalize this and include auth to make self contained
// and possible to add alternative backup solutions

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
	virtual bool RestoreBackup( const string& pathtobackup, const string& destprefix ) = 0;
	virtual  ~BackupInterface();
};

typedef shared_ptr<BackupInterface> BackupInterfacePtr;

class OPIBackup: public BackupInterface
{
public:
	OPIBackup();

	bool MountLocal(const string& configpath) override;
	bool MountRemote(const string& configpath) override;
	list<string> GetLocalBackups() override;
	list<string> GetRemoteBackups() override;
	void UmountLocal() override;
	void UmountRemote() override;
	bool RestoreBackup( const string& pathtobackup, const string& destprefix ) override;
	virtual ~OPIBackup();
};


class BackupHelper
{
public:
	BackupHelper(string  pwd, BackupInterfacePtr iface = BackupInterfacePtr( new OPIBackup ));

	void SetPassword(const string& pwd);

	bool MountLocal();
	bool MountRemote();
	list<string> GetLocalBackups();
	list<string> GetRemoteBackups();

	void UmountLocal();
	void UmountRemote();

	bool RestoreBackup( const string& path, const string& destprefix = "");

	void CreateConfig();

	string GetConfigFile();

	~BackupHelper();
private:
	bool localmounted;
	bool remotemounted;
	bool cfgcreated;
	BackupInterfacePtr iface;
	string pwd;
	char tmpfilename[128];
};

typedef shared_ptr<BackupHelper> BackupHelperPtr;

}

#endif // BACKUPHELPER_H
