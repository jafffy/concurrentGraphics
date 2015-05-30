#ifndef MESSAGE_H_
#define MESSAGE_H_

enum E_MESSAGE_TYPE {
	EMT_INSERT,
	EMT_ERASE,
	EMT_COUNT
};

class Message
{
public:
	E_MESSAGE_TYPE type;
	void* user_data;
};

#endif // MESSAGE_H_