#include "Notification.h"

#include <libutils/Logger.h>
#include <iostream>

#include "SysInfo.h"

using namespace std;
using namespace Utils;
namespace OPI
{

Notification notification;

Notification::Notification()
{
}

void Notification::Notify(Notification::Notice notice, const string &info)
{
	switch (sysinfo.Type()) {
	case SysInfo::TypePC:
	{
		switch ( notice ) {
		case Notification::Completed:
			logg << Logger::Notice << "Completed: [" << info << "]" << lend;
			break;
		case Notification::Waiting:
			logg << Logger::Notice << "Waiting: [" << info << "]" << lend;
			break;
		case Notification::Error:
			logg << Logger::Error << "Error: [" << info << "]" << lend;
			break;
		default:
			break;
		}
	}
		break;
	case SysInfo::TypeOpi:
	{
		switch ( notice ) {
		case Notification::Completed:
            // all green leds on
            this->leds.SetTrigger("usr0", "none");
            this->leds.Brightness("usr0", true);

            this->leds.SetTrigger("usr1", "none");
            this->leds.Brightness("usr1", true);

            this->leds.SetTrigger("usr2", "none");
            this->leds.Brightness("usr2", true);

            // red led off
            this->leds.SetTrigger("usr3", "none");
            this->leds.Brightness("usr3", false);

			break;

		case Notification::Waiting:
            // first and second green leds on
            this->leds.SetTrigger("usr0", "none");
            this->leds.Brightness("usr0", true);

            this->leds.SetTrigger("usr1", "none");
            this->leds.Brightness("usr1", true);

            // heartbeat on third led
            this->leds.SetTrigger("usr2", "heartbeat");
            this->leds.Brightness("usr2", true);

            // red led off
            this->leds.SetTrigger("usr3", "none");
            this->leds.Brightness("usr3", false);

            break;

        case Notification::Error:
            // first and second green leds on
            this->leds.SetTrigger("usr0", "none");
            this->leds.Brightness("usr0", true);

            this->leds.SetTrigger("usr1", "none");
            this->leds.Brightness("usr1", true);

            // third led off
            this->leds.SetTrigger("usr2", "none");
            this->leds.Brightness("usr2", false);

            // red led heartbeat
            this->leds.SetTrigger("usr3", "heartbeat");

            break;
		default:
			break;
		}
	}
		break;
	case SysInfo::TypeArmada:
	{
		switch ( notice ) {
        // "heartbeat" trigger can have LED in either state, when changing to "none"
        // make sure to set the value also.

		case Notification::Completed:
            this->leds.SetTrigger("blue","none");
            this->leds.Brightness("blue", false);  // make sure led is turned off.

            this->leds.SetTrigger("red","none");
            this->leds.Brightness("red", false);  // make sure led is turned off.

            this->leds.SetTrigger("green","none");
            this->leds.Brightness("green", true);  // make sure led is turned on.
            break;
		case Notification::Waiting:
            this->leds.SetTrigger("blue","none");
            this->leds.Brightness("blue", false);  // make sure led is turned off.

            this->leds.SetTrigger("red","none");
            this->leds.Brightness("red", false);  // make sure led is turned off.

            this->leds.SetTrigger("green","heartbeat");
            break;
		case Notification::Error:
            this->leds.SetTrigger("blue","none");
            this->leds.Brightness("blue", false);  // make sure led is turned off.

            this->leds.SetTrigger("red","none");
            this->leds.Brightness("red", true);  // make sure led is turned on.

            this->leds.SetTrigger("green","none");
            this->leds.Brightness("green", false);  // make sure led is turned off.
            break;
		default:
			break;
		}
	}
		break;

	default:
		logg << Logger::Notice << "Unimplemented system type in Notification subsytem" << lend;
		break;
	}


}


Notification::~Notification()
{

}

}


