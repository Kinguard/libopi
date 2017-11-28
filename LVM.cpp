#include "LVM.h"

using namespace OPI;

#include <iostream>

using namespace std;

list<PhysicalVolumePtr> LVM::ListUnusedPhysicalVolumes()
{

	this->checkLVM();

	list<PhysicalVolumePtr> ret;

	struct dm_list *pvs;
	pvs = lvm_list_pvs( this->lvm );

	struct lvm_pv_list *pvlist;
	dm_list_iterate_items( pvlist, pvs )
	{
		ret.push_back( PhysicalVolumePtr(
						   new PhysicalVolume( lvm_pv_get_name( pvlist->pv ) )
						   ));
	}

	if( lvm_list_pvs_free( pvs ) != 0 )
	{
		throw std::runtime_error( lvm_errmsg( this->lvm ));
	}

	return ret;
}

PhysicalVolumePtr LVM::CreatePhysicalVolume(const string &devpath, uint64_t size)
{
	PhysicalVolumePtr ret(new PhysicalVolume(devpath) );

	if( lvm_pv_create( this->lvm, devpath.c_str(), size) != 0 )
	{
		throw std::runtime_error( lvm_errmsg( this->lvm ));
	}

	return ret;
}

void LVM::RemovePhysicalVolume(PhysicalVolumePtr pv)
{
	if( lvm_pv_remove( this->lvm, pv->Path().c_str() ) != 0 )
	{
		throw std::runtime_error( lvm_errmsg( this->lvm ));
	}
}

list<VolumeGroupPtr> LVM::ListVolumeGroups()
{
	list<VolumeGroupPtr> ret;

	struct dm_list *vgnames;
	struct lvm_str_list *slist;

	vgnames = lvm_list_vg_names( this->lvm );
	dm_list_iterate_items( slist, vgnames )
	{
		ret.push_back(
					VolumeGroupPtr( new VolumeGroup(slist->str, this ) )
					);
	}

	return ret;
}

VolumeGroupPtr LVM::GetVolumeGroup(const string &name)
{
	return VolumeGroupPtr( new VolumeGroup(name, this) );
}

VolumeGroupPtr LVM::CreateVolumeGroup(const string &name, list<PhysicalVolumePtr> pvs)
{
	if( pvs.size() == 0 )
	{
		pvs = this->ListUnusedPhysicalVolumes();
	}

	vg_t vg = lvm_vg_create( this->lvm, name.c_str() );

	VolumeGroupPtr volgroup = VolumeGroupPtr( new VolumeGroup(name, this, vg) );

	for( auto& pv: pvs)
	{
		volgroup->AddPhysicalVolume(pv);
	}


	if( lvm_vg_write( vg ) != 0 )
	{
		throw std::runtime_error( lvm_errmsg( this->lvm ) );
	}

	return volgroup;
}

void LVM::RemoveVolumeGroup(VolumeGroupPtr vg)
{
	if( lvm_vg_remove( vg->vghandle ) != 0 )
	{
		throw std::runtime_error( lvm_errmsg( this->lvm ) );
	}


	if( lvm_vg_write( vg->vghandle ) != 0 )
	{
		throw std::runtime_error( lvm_errmsg( this->lvm ) );
	}

}

LVM::LVM()
{
	this->lvm = lvm_init(nullptr);

	this->checkLVM();
}

LVM::~LVM()
{
	if( this->lvm != nullptr )
	{
		lvm_quit( this->lvm );
	}
}

lvm_t LVM::GetLVMHandle()
{
	return this->lvm;
}

void LVM::checkLVM()
{
	if( this->lvm == nullptr )
	{
		throw std::runtime_error("No LVM context available");
	}
}

PhysicalVolume::PhysicalVolume(const string &path): path(path)
{

}

string PhysicalVolume::Path()
{
	return this->path;
}

PhysicalVolume::~PhysicalVolume()
{

}

VolumeGroup::VolumeGroup(const string &name, LVM *lvm): name(name),lvm(lvm)
{
	this->vghandle = lvm_vg_open( this->lvm->GetLVMHandle(), this->name.c_str(), "w",0);

	if( this->vghandle == nullptr )
	{
		throw std::runtime_error( lvm_errmsg( this->lvm->GetLVMHandle() ) );
	}
}

VolumeGroup::VolumeGroup(const string &name, LVM *lvm, vg_t vg): name(name), vghandle(vg), lvm(lvm)
{
	if( this->vghandle == nullptr )
	{
		throw std::runtime_error( lvm_errmsg( this->lvm->GetLVMHandle() ) );
	}
}

string VolumeGroup::Name()
{
	return this->name;
}

list<PhysicalVolumePtr> VolumeGroup::ListPhysicalVolumes()
{
	list<PhysicalVolumePtr> ret;

	struct dm_list *pvnames;
	pvnames = lvm_vg_list_pvs( this->vghandle);

	struct lvm_pv_list *lvlist;

	dm_list_iterate_items(lvlist, pvnames )
	{
		ret.push_back( PhysicalVolumePtr(new PhysicalVolume( lvm_pv_get_name(lvlist->pv) ) ));
	}

#if 0
	if( lvm_list_pvs_free( pvnames ) != 0 )
	{
		throw std::runtime_error( lvm_errmsg( this->lvm->GetLVMHandle() ) );
	}
#endif

	return ret;
}

void VolumeGroup::AddPhysicalVolume(PhysicalVolumePtr pv)
{
	if( lvm_vg_extend( this->vghandle, pv->Path().c_str() ) != 0 )
	{
		throw std::runtime_error( lvm_errmsg( this->lvm->GetLVMHandle() ) );
	}

	if( lvm_vg_write( this->vghandle) != 0 )
	{
		throw std::runtime_error( lvm_errmsg( this->lvm->GetLVMHandle() ) );
	}

}

void VolumeGroup::RemovePhysicalVolume(PhysicalVolumePtr pv)
{
	if( lvm_vg_reduce( this->vghandle, pv->Path().c_str() )  != 0 )
	{
		throw std::runtime_error( lvm_errmsg( this->lvm->GetLVMHandle() ) );
	}

	if( lvm_vg_write( this->vghandle) != 0 )
	{
		throw std::runtime_error( lvm_errmsg( this->lvm->GetLVMHandle() ) );
	}
}

list<LogicalVolumePtr> VolumeGroup::GetLogicalVolumes()
{
	list<LogicalVolumePtr> lvs;
	struct dm_list *list;
	list = lvm_vg_list_lvs( this->vghandle);

	struct lvm_lv_list *lvitem;

	dm_list_iterate_items(lvitem, list )
	{
		lv_t lv = lvitem->lv;
		lvs.push_back( LogicalVolumePtr( new LogicalVolume( lvm_lv_get_name(lv) , this, lv) ) );
	}

	return lvs;
}

LogicalVolumePtr VolumeGroup::CreateLogicalVolume(const string &name, uint64_t size)
{

	if( size == 0 )
	{
		size = lvm_vg_get_free_extent_count( this->vghandle ) * lvm_vg_get_extent_size( this->vghandle);
	}

	cout << "Create lv using " << size << " extents"<< endl;

	lv_t lv = lvm_vg_create_lv_linear( this->vghandle, name.c_str(), size );

	if( lv == nullptr )
	{
		throw std::runtime_error( lvm_errmsg( this->lvm->GetLVMHandle() ) );
	}
	return LogicalVolumePtr( new LogicalVolume(name, this, lv) );
}

LogicalVolumePtr VolumeGroup::GetLogicalVolume(const string &name)
{
	lv_t lv = lvm_lv_from_name( this->vghandle, name.c_str() );

	if( lv == nullptr )
	{
		throw std::runtime_error( lvm_errmsg( this->lvm->GetLVMHandle() ) );
	}

	return LogicalVolumePtr( new LogicalVolume(name, this, lv) );
}

void VolumeGroup::RemoveLogicalVolume(LogicalVolumePtr vol)
{
	if( lvm_vg_remove_lv( vol->lv ) != 0 )
	{
		throw std::runtime_error( lvm_errmsg( this->lvm->GetLVMHandle() ) );
	}
}

VolumeGroup::~VolumeGroup()
{
	if( lvm_vg_close( this->vghandle ) != 0 )
	{
		cout << "Close failed"<< endl;
		// Not throwing any exception
		// TOdo:: How to handle?
		//throw std::runtime_error( lvm_errmsg( this->lvm->GetLVMHandle() ) );
	}
}

LogicalVolume::LogicalVolume(const string &name, VolumeGroup *volume, lv_t lv):name(name), volume(volume), lv(lv)
{

}

string LogicalVolume::Name()
{
	return this->name;
}

LogicalVolume::~LogicalVolume()
{

}
