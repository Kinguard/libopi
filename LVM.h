#ifndef LVM_H
#define LVM_H

#include <list>
#include <string>
#include <memory>

#include <lvm2app.h>
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

	void AddPhysicalVolume(PhysicalVolumePtr pv);
	void RemovePhysicalVolume(PhysicalVolumePtr pv);

	/**
	 * @brief CreateLogicalVolume
	 * @param name name of volume to create
	 * @param size size of volume, if zero use all available space in vg
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
	void RemoveLogicalVolume( LogicalVolumePtr vol);

	virtual ~VolumeGroup();
private:
	VolumeGroup(const string& name, LVM* lvm);
	VolumeGroup(const string& name, LVM* lvm, vg_t vg);

	string name;
	vg_t vghandle;
	LVM* lvm;
	friend class LogicalVolume;
	friend class LVM;
};

class PhysicalVolume
{
public:
	PhysicalVolume(const string& path);

	string Path();

	virtual ~PhysicalVolume();
private:
	string path;
};

class LogicalVolume
{
public:

	string Name();

	~LogicalVolume();
private:
	LogicalVolume(const string& name,VolumeGroup *volume, lv_t lv );
	string name;
	VolumeGroup* volume;
	lv_t lv;
	friend class VolumeGroup;
};

class LVM
{
public:
	LVM();

	list<PhysicalVolumePtr> ListUnusedPhysicalVolumes();
	PhysicalVolumePtr CreatePhysicalVolume(const string& devpath, uint64_t size=0);

	list<VolumeGroupPtr> ListVolumeGroups();
	VolumeGroupPtr GetVolumeGroup(const string& name);

	/* create new volumegroup, if pvs empty use all avaliable pvs */
	VolumeGroupPtr CreateVolumeGroup(const string& name, list<PhysicalVolumePtr> pvs = {});

	void RemoveVolumeGroup( VolumeGroupPtr vg);

	virtual ~LVM();
protected:
	lvm_t GetLVMHandle();
private:
	void checkLVM();
	lvm_t lvm;

	friend class VolumeGroup;
};



}


#endif // LVM_H
