#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "ReferenceCounted.h"

enum E_MESSAGE_TYPE {
	EMT_INSERT,
	EMT_ERASE,
	EMT_UPDATE,
	EMT_COUNT
};

class Message : public ReferenceCounted
{
public:
	E_MESSAGE_TYPE type;
	void* user_data;

	Message(E_MESSAGE_TYPE type, void* user_data)
	: type(type), user_data(user_data)
	{
	}
};

#endif // MESSAGE_H_
