#ifndef LVM_H
#define LVM_H

#include <list>
#include <string>
#include <memory>

#include <stdint.h>

using namespace std;

namespace OPI {


class LogicalVolume;
typedef shared_ptr<LogicalVolume> LogicalVolumePtr;

class PhysicalVolume;
typedef shared_ptr<PhysicalVolume> PhysicalVolumePtr;

class VolumeGroup;
typedef shared_ptr<VolumeGroup> VolumeGroupPtr;

class LVM;
typedef shared_ptr<LVM> LVMPtr;

class VolumeGroup
{
public:

	VolumeGroup() = delete;
	VolumeGroup( const VolumeGroup&) = delete;
	VolumeGroup& operator=( const VolumeGroup&) = delete;

	string Name();

	list<PhysicalVolumePtr> ListPhysicalVolumes();

	void AddPhysicalVolume(const PhysicalVolumePtr& pv);
	void RemovePhysicalVolume(const PhysicalVolumePtr& pv);

	/**
	 * @brief GetLogicalVolumes of VG
	 * @return list with all logical volumes of VG
	 */
	list<LogicalVolumePtr> GetLogicalVolumes();

	/**
	 * @brief CreateLogicalVolume
	 * @param name name of volume to create
	 * @param size size of volume in bytes, if zero use all available space in vg
	 * @return New shared ponter to the new Logical volume
	 */
	LogicalVolumePtr CreateLogicalVolume(const string& name, uint64_t size=0);

	/**
	 * @brief GetLogicalVolume
	 * @param name Logical volume to retrieve from VG
	 * @return Shared pointer to logical volume object
	 */
	LogicalVolumePtr GetLogicalVolume( const string& name);

	/**
	 * @brief RemoveLogicalVolume
	 * @param vol Logical volume to remove from volume group
	 */
	void RemoveLogicalVolume( const LogicalVolumePtr& vol);

	virtual ~VolumeGroup();
private:
	VolumeGroup(string  name, LVM* lvm);

	string name;
	LVM* lvm;
	friend class LogicalVolume;
	friend class LVM;
};

class LogicalVolume
{
public:

	string Name();
	string VolumeName();

	~LogicalVolume();
private:
	LogicalVolume(string  name,VolumeGroup *volume );
	string name;
	VolumeGroup* volume;
	friend class VolumeGroup;
};

class PhysicalVolume
{
public:
	PhysicalVolume(string  path, const string& volumegroup="");

	bool inUse();
	string VolumeGroup();

	string Path();

	virtual ~PhysicalVolume();
private:
	string path;
	string volumegroup;
};

class LVM
{
public:
	LVM();

	list<PhysicalVolumePtr> ListUnusedPhysicalVolumes();
	list<PhysicalVolumePtr> ListPhysicalVolumes();
	PhysicalVolumePtr CreatePhysicalVolume(const string& devpath, uint64_t size=0);
	void RemovePhysicalVolume( const PhysicalVolumePtr& pv);

	list<VolumeGroupPtr> ListVolumeGroups();
	VolumeGroupPtr GetVolumeGroup(const string& name);

	/* create new volumegroup, if pvs empty use all avaliable pvs */
	VolumeGroupPtr CreateVolumeGroup(const string& name, list<PhysicalVolumePtr> pvs = {});

	void RemoveVolumeGroup( const VolumeGroupPtr& vg);

	virtual ~LVM();
protected:

	friend class VolumeGroup;
};



}


#endif // LVM_H
