#include "Luks.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <libutils/Exceptions.h>
#include <libutils/FileUtils.h>

#include <libudev.h>

#include <iostream>
using namespace std;
using namespace Utils;

namespace OPI
{

Luks::Luks(const string &path): path(path), open(false)
{
	if( crypt_init( &this->cryptdevice, path.c_str() ) < 0 )
	{
		throw Utils::ErrnoException("Failed to init cryptsetup");
	}
}

bool Luks::isLuks(const string &device)
{
	struct udev *udev=udev_new();

	if ( !udev ) {
		return false;
	}

	struct udev_device *dev = udev_device_new_from_subsystem_sysname( udev, "block", device.c_str() );
	if( ! dev )
	{
		// Lets try device number instead
		try
		{
			string rname = File::RealPath(device);

			struct stat sbuf = {};
			if( stat(rname.c_str(), &sbuf) != 0 )
			{
				udev_unref(udev);
				return false;
			}

			dev = udev_device_new_from_devnum(udev, 'b', sbuf.st_rdev );

			if ( ! dev )
			{
				udev_unref(udev);
				return false;
			}
		}
		catch( Utils::ErrnoException& err )
		{
			udev_unref(udev);
			return false;
		}
	}

	const char* c_type = udev_device_get_property_value(dev, "ID_FS_TYPE");
	if( ! c_type )
	{
		udev_device_unref( dev );
		udev_unref(udev);
		return false;
	}

	string type(c_type);
	bool ret = type == "crypto_LUKS";

	udev_device_unref( dev );
	udev_unref(udev);

	return ret;
}

void Luks::Format(const string &password)
{
	struct crypt_params_luks1 params = {};

	params.hash = "sha1";
	params.data_alignment = 0;
	params.data_device = nullptr;

	int r = crypt_format(
				this->cryptdevice,
				CRYPT_LUKS1,
				"aes",
				"xts-plain64",
				nullptr,
				nullptr,
				256/8,
				&params
				);

	if( r < 0 )
	{
		throw Utils::ErrnoException("Failed to LUKS format device");
	}

	r = crypt_keyslot_add_by_volume_key(
				this->cryptdevice,
				CRYPT_ANY_SLOT,
				nullptr,
				0,
				password.c_str(),
				password.length()
				);
	if( r < 0 )
	{
		throw Utils::ErrnoException("Failed to add key to LUKS volume");
	}

}

bool Luks::Open(const string &name, const string &password, bool discard)
{

	int r = crypt_load(
				this->cryptdevice,
				CRYPT_LUKS1,
				nullptr
				);

	if( r < 0 )
	{
		throw Utils::ErrnoException("Failed to load context");
	}

	r = crypt_activate_by_passphrase(
				this->cryptdevice,
				name.c_str(),
				CRYPT_ANY_SLOT,
				password.c_str(),
				password.length(),
				discard?CRYPT_ACTIVATE_ALLOW_DISCARDS:0
				);

	if( r < 0 )
	{
		return false;
	}
	this->name = name;
	this->open = true;

	return true;
}

bool Luks::Active(const string &name)
{
	crypt_status_info info = crypt_status( this->cryptdevice, name.c_str() );

	return info == CRYPT_ACTIVE || info == CRYPT_BUSY;
}

void Luks::Close(const string& mname)
{
	string mappername;
	if( mname != "" )
	{
		mappername = mname;
	}
	else
	{
		mappername = this->name;
	}

	if( crypt_deactivate( this->cryptdevice, mappername.c_str() ) < 0 )
	{
		throw Utils::ErrnoException("Failed to deactivate device");
	}
	this->open = false;
}

Luks::~Luks()
{
	crypt_free( this->cryptdevice );
}

} // End NS
