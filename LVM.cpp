#include "LVM.h"

#include <libutils/Process.h>
#include <libutils/String.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <sstream>
#include <utility>

using namespace Utils;
using json = nlohmann::json;

namespace OPI
{

LVM::LVM() = default;

list<PhysicalVolumePtr> LVM::ListUnusedPhysicalVolumes()
{
	list<PhysicalVolumePtr> pvs = this->ListPhysicalVolumes();

	auto last = remove_if(pvs.begin(), pvs.end(), [](const PhysicalVolumePtr& item){ return item->inUse(); }  );

	return list<PhysicalVolumePtr>(pvs.begin(), last);
}

list<PhysicalVolumePtr> LVM::ListPhysicalVolumes()
{
	list<PhysicalVolumePtr> res;
	bool result = false;
	string ret;

	tie(result, ret) = Process::Exec("/sbin/pvs --reportformat json");

	if( ! result )
	{
		throw std::runtime_error("LVM: Failed to retrieve pvs");
	}

	json v;
	try
	{
		v = json::parse(ret);
	}
	catch (json::parse_error& err)
	{

		throw std::runtime_error("LVM: Failed to parse reply when retrieving pvs"s+err.what());
	}

	if( ! v["report"][0]["pv"].is_array() )
	{
		throw std::runtime_error("LVM: Unable to retrieve pvs from reply");
	}

	json pvs = v["report"][0]["pv"];
	for (size_t i=0; i < pvs.size(); i++)
	{
		json p = pvs[static_cast<int>(i)];
		PhysicalVolumePtr pv(new PhysicalVolume(p["pv_name"].get<string>(), p["vg_name"].get<string>()));
		res.push_back(pv);
	}

	return res;
}

PhysicalVolumePtr LVM::CreatePhysicalVolume(const string &devpath, uint64_t size)
{
	bool result = false;
	stringstream cmd;

	cmd << "/sbin/pvcreate -y " << devpath;

	if( size > 0 )
	{
		cmd << " --setphysicalvolumesize " << size;
	}

	tie(result, ignore) = Process::Exec(cmd.str());

	if( ! result )
	{
		throw std::runtime_error("LVM: Failed to create pv");
	}

	return  PhysicalVolumePtr(new PhysicalVolume(devpath));
}

void LVM::RemovePhysicalVolume(const PhysicalVolumePtr& pv)
{
	stringstream cmd;
	bool result = false;

	cmd << "/sbin/pvremove -y " << pv->Path();

	tie(result, ignore) = Process::Exec(cmd.str());

	if( ! result )
	{
		throw std::runtime_error("LVM: Failed to remove pv");
	}
}

list<VolumeGroupPtr> LVM::ListVolumeGroups()
{
	list<VolumeGroupPtr> res;
	bool result = false;
	string ret;

	tie(result, ret) = Process::Exec("/sbin/vgs --reportformat json");

	if( ! result )
	{
		throw std::runtime_error("LVM: Failed to retrieve vgs");
	}


	json v;
	try
	{
		v = json::parse(ret);
	}
	catch (json::parse_error& err)
	{
		throw std::runtime_error("LVM: Failed to parse reply when retrieving vgs: "s+err.what());
	}

	if( ! v["report"][0]["vg"].is_array() )
	{
		throw std::runtime_error("LVM: Unable to retrieve vgs from reply");
	}

	json vgs = v["report"][0]["vg"];
	for (size_t i=0; i < vgs.size(); i++)
	{
		json v = vgs[static_cast<int>(i)];
		VolumeGroupPtr pv(new VolumeGroup(v["vg_name"].get<string>(), this));
		res.push_back(pv);
	}

	return res;
}

VolumeGroupPtr LVM::GetVolumeGroup(const string &name)
{
	auto vgs = this->ListVolumeGroups();

	auto it=find_if(vgs.begin(), vgs.end(), [name](const VolumeGroupPtr& vg)
	{
		return vg->Name() == name;
	});

	if( it == vgs.end() )
	{
		return nullptr;
	}
	return *it;
}

VolumeGroupPtr LVM::CreateVolumeGroup(const string &name, list<PhysicalVolumePtr> pvs)
{

	if( pvs.size() == 0)
	{
		pvs = this->ListUnusedPhysicalVolumes();
	}

	if( pvs.size() == 0 )
	{
		throw std::runtime_error("LVM: Create volumegroup without any physical volumes available");
	}

	stringstream cmd;
	bool result;

	cmd << "/sbin/vgcreate -y " << name << " ";
	for(const auto& pv:pvs)
	{
		cmd << " " << pv->Path();
	}

	tie(result, ignore) = Process::Exec(cmd.str());

	if( ! result )
	{
		throw std::runtime_error("LVM: Failed to create volume group");
	}

	VolumeGroupPtr vg(new VolumeGroup(name, this) );

	return vg;
}

void LVM::RemoveVolumeGroup(const VolumeGroupPtr& vg)
{
	stringstream cmd;
	bool result;

	cmd << "/sbin/vgremove -y " << vg->Name();

	tie(result, ignore) = Process::Exec(cmd.str());

	if( ! result )
	{
		throw std::runtime_error("LVM: Failed to remove vg");
	}
}

LVM::~LVM() = default;

PhysicalVolume::PhysicalVolume(string path, const string &volumegroup)
	:path(std::move(path)), volumegroup(volumegroup)
{

}

bool PhysicalVolume::inUse()
{
	return this->volumegroup!="";
}

string PhysicalVolume::VolumeGroup()
{
	return this->volumegroup;
}

string PhysicalVolume::Path()
{
	return this->path;
}

PhysicalVolume::~PhysicalVolume() = default;

/*
 *
 * Volume group
 *
 */

VolumeGroup::VolumeGroup(string name, LVM *lvm)
	:name(std::move(name)), lvm(lvm)
{

}

string VolumeGroup::Name()
{
	return this->name;
}

list<PhysicalVolumePtr> VolumeGroup::ListPhysicalVolumes()
{
	auto pvs = this->lvm->ListPhysicalVolumes();
	auto last = remove_if(pvs.begin(),pvs.end(),
						  [this](const PhysicalVolumePtr& pv)
							{
								return pv->VolumeGroup()!=this->name;
							}
						  );

	return list<PhysicalVolumePtr>(pvs.begin(), last);
}

void VolumeGroup::AddPhysicalVolume(const PhysicalVolumePtr& pv)
{
	stringstream cmd;
	bool result;

	cmd << "/sbin/vgextend " << this->Name() << " " << pv->Path();

	tie(result, ignore) = Process::Exec(cmd.str());

	if( ! result )
	{
		throw std::runtime_error("LVM: Failed to add pv to vg");
	}
}

void VolumeGroup::RemovePhysicalVolume(const PhysicalVolumePtr& pv)
{
	stringstream cmd;
	bool result;

	cmd << "/sbin/vgreduce " << this->Name() << " " << pv->Path();

	tie(result, ignore) = Process::Exec(cmd.str());

	if( ! result )
	{
		throw std::runtime_error("LVM: Failed to add pv to vg");
	}
}

list<LogicalVolumePtr> VolumeGroup::GetLogicalVolumes()
{
	list<LogicalVolumePtr> res;

	bool result;
	string ret;

	tie(result, ret) = Process::Exec("/sbin/lvs --reportformat json");

	if( ! result )
	{
		throw std::runtime_error("LVM: Failed to retrieve lvs");
	}

	json v;
	try
	{
		v = json::parse(ret);
	}
	catch (json::parse_error& err)
	{
		throw std::runtime_error("LVM: Failed to parse reply when retrieving lvs: "s+err.what());
	}

	if( ! v["report"][0]["lv"].is_array() )
	{
		throw std::runtime_error("LVM: Unable to retrieve lvs from reply");
	}

	json lvs = v["report"][0]["lv"];
	for (size_t i=0; i < lvs.size(); i++)
	{
		json v = lvs[static_cast<int>(i)];
		if( v["vg_name"].get<string>() == this->Name() )
		{
			LogicalVolumePtr lv(new LogicalVolume(v["lv_name"].get<string>(), this));
			res.push_back(lv);
		}
	}

	return res;
}

LogicalVolumePtr VolumeGroup::CreateLogicalVolume(const string &name, uint64_t size)
{
	stringstream cmd;
	bool result;

	cmd << "/sbin/lvcreate -y --type linear ";
	if( size == 0 )
	{
		cmd << " -l 100%VG ";
	}
	else
	{
		cmd << " -l " << size << " ";
	}

	cmd << "-n " << name << " " << this->Name();

	tie(result, ignore) = Process::Exec(cmd.str());

	if( ! result )
	{
		throw std::runtime_error("LVM: Failed to create lv");
	}

	return LogicalVolumePtr(new LogicalVolume(name,this));
}

LogicalVolumePtr VolumeGroup::GetLogicalVolume(const string &name)
{
	auto lvs = this->GetLogicalVolumes();
	for(auto lv:lvs)
	{
		if( lv->Name() == name )
		{
			return lv;
		}
	}
	return nullptr;
}

void VolumeGroup::RemoveLogicalVolume(const LogicalVolumePtr& vol)
{
	stringstream cmd;
	bool result;

	cmd << "/sbin/lvremove -y " << this->Name()<<"/"<<vol->Name();

	tie(result, ignore) = Process::Exec(cmd.str());

	if( ! result )
	{
		throw std::runtime_error("LVM: Failed to create lv");
	}
}

VolumeGroup::~VolumeGroup() = default;

/*
 * Logical volume implementation
 */

string LogicalVolume::Name()
{
	return this->name;
}

string LogicalVolume::VolumeName()
{
	return this->volume->Name();
}

LogicalVolume::~LogicalVolume() = default;

LogicalVolume::LogicalVolume(string name, VolumeGroup *volume)
	:name(std::move(name)), volume(volume)
{

}


}

