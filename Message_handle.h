#pragma once
class Message_handle
{
public:
	char *buf;
public:
	Message_handle();
	~Message_handle();
public:
	virtual int MessageCommand();
};

