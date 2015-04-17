#include "TestBackupHelper.h"

#include "BackupHelper.h"

#include <libutils/FileUtils.h>

CPPUNIT_TEST_SUITE_REGISTRATION ( TestBackupHelper );

class TestBackup: public OPI::BackupInterface
{
public:
	TestBackup() {}

	virtual bool MountLocal(const string &configpath)
	{
		return true;
	}

	virtual bool MountRemote(const string &configpath)
	{
		return true;
	}

	virtual list<string> GetLocalBackups()
	{
		return {"a1","b1","c1"};
	}


	virtual list<string> GetRemoteBackups()
	{
#if 0
		list<string> ret, res = Utils::File::Glob("/tmp/backup/*");
		for( const auto& r: res)
		{

			if( Utils::File::GetFileName( r ) != "lost+found" )
			{
				ret.push_back( r );
				cout << r << endl;
			}
		}
		return res;
#else
		return {"a2","b2","c2"};
#endif
	}

	virtual void UmountLocal() {}
	virtual void UmountRemote() {}

	virtual bool RestoreBackup(const string &pathtobackup)
	{
		return true;
	}

	virtual ~TestBackup() {}
};


void TestBackupHelper::setUp()
{
}

void TestBackupHelper::tearDown()
{
}

void TestBackupHelper::Test()
{

	CPPUNIT_ASSERT_NO_THROW( OPI::BackupHelper("Test") );
	string cfg;
	{
		OPI::BackupHelper bh("Test", OPI::BackupInterfacePtr( new TestBackup ));
		bh.CreateConfig();
		cfg = bh.GetConfigFile();
		//cout << "CFG " << cfg << endl;
		CPPUNIT_ASSERT( Utils::File::FileExists( cfg ));

		CPPUNIT_ASSERT( bh.MountLocal( ) );
		CPPUNIT_ASSERT( bh.MountRemote( ) );

		CPPUNIT_ASSERT( bh.GetLocalBackups().size() > 0 );
		CPPUNIT_ASSERT( bh.GetRemoteBackups().size() > 0 );

		CPPUNIT_ASSERT( bh.RestoreBackup( "bla bla") );
	}
	CPPUNIT_ASSERT( ! Utils::File::FileExists( cfg ) );

	// Check shared ptr as well
	{
		OPI::BackupHelperPtr bhp( new OPI::BackupHelper("test", OPI::BackupInterfacePtr( new TestBackup)));

		CPPUNIT_ASSERT( bhp->MountLocal( ) );
		CPPUNIT_ASSERT( bhp->MountRemote( ) );

		CPPUNIT_ASSERT( bhp->GetLocalBackups().size() > 0 );
		CPPUNIT_ASSERT( bhp->GetRemoteBackups().size() > 0 );

		CPPUNIT_ASSERT( bhp->RestoreBackup( "bla bla") );
	}

}
