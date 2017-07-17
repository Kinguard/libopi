#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include "LedControl.h"
#include <libutils/ClassTools.h>

namespace OPI {

class Notification: public Utils::NoCopy
{
public:

	Notification();

	enum Notice {
		Error,
		Waiting,
		Completed
	};

	void Notify(enum Notice notice, const string& info);

	virtual ~Notification();
private:
	LedControl leds;
};

extern Notification notification;

}


#endif // NOTIFICATION_H
