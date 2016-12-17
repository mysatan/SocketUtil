#include "BNSocketUtil.h"

int BNSocketUtil::socketHandle=0;
LabelTTF*  BNSocketUtil::msgLabel;
bool BNSocketUtil::connectFlag=false;

void BNSocketUtil::connect(const char* ip, unsigned short port)
{
	if(!connectFlag)
	{
		msgLabel->setString("\u8fde\u63a5\u4e2d");
		new std::thread(&BNSocketUtil::threadConnectTask,ip,port);
	}
}

void BNSocketUtil::sendInt(int si)
{
	 send(socketHandle,&si,4,0);
}

void BNSocketUtil::sendFloat(float sf)
{
	send(socketHandle,&sf,4,0);
}

void BNSocketUtil::sendStr(const char* str,int len)
{
	sendInt(len);
	send(socketHandle,str,len,0);
}

char* BNSocketUtil::receiveBytes(int len)
{
	char* result=new char[len];

	int status=recv(socketHandle, result, len, 0);

	while(status!=len)
	{
		int index=status;
		char b[len-status];
		int count=recv(socketHandle, b, len-status, 0);
		CCLOG("receiveBytes receive count %d",count);
		status=status+count;
		if(count!=0)
		{
			for(int i=0;i<count;i++)
			{
				result[index+i]=b[i];
				CCLOG("index %d %d",index+i,i);
			}
		}
	}

	return result;
}

int BNSocketUtil::receiveInt()
{
	char* a=receiveBytes(4);
	int ri;
	CCLOG("%d %d %d %d",a[0],a[1],a[2],a[3]);

	memset(&ri, 0, sizeof(ri));
	memcpy((char*)(&ri), a,4);
    delete a;
	return ri;
}

float BNSocketUtil::receiveFloat()
{
	char* a=receiveBytes(4);
	float ri;
	CCLOG("%d %d %d %d",a[0],a[1],a[2],a[3]);
	memset(&ri, 0, sizeof(ri));
	memcpy((char*)(&ri), a,4);
	delete a;
	return ri;
}

char* BNSocketUtil::receiveStr()
{
	int len=receiveInt();
	CCLOG("strlen %d",len);
	char* a=receiveBytes(len);
	char* result=new char[len+1];
	memset(result, 0, len+1);
	memcpy(result, a,len);
	CCLOG("receiveStr receive  %s",result);
	return result;
}

void  BNSocketUtil::closeConnect()
{
	if(connectFlag)
	{
		::close(socketHandle);
		connectFlag=false;
	}
}

void* BNSocketUtil::threadConnectTask(const char* ip, unsigned short port)
{
	CCLOG("===Connect===");
	struct sockaddr_in sa;
	struct hostent* hp;
	hp = gethostbyname(ip);
	if(!hp)
	{
		 CCLOG("hp error!");
	     return 0;
	}
	memset(&sa, 0, sizeof(sa));
	memcpy((char*)&sa.sin_addr, hp->h_addr, hp->h_length);
	sa.sin_family = hp->h_addrtype;
	sa.sin_port = htons(port);
	socketHandle = socket(sa.sin_family, SOCK_STREAM, 0);
	if(socketHandle < 0)
	{
		CCLOG( "failed to create socket");
		return 0;
	}
	if(::connect(socketHandle, (sockaddr*)&sa, sizeof(sa)) < 0)
	{
		CCLOG("failed to connect socket");
	    ::close(socketHandle);
	    return 0;
	}
	CCLOG("Client connect OK ！ IP: %s:%d ",ip,port);
	msgLabel->setString("\u8fde\u63a5\u6210\u529f");
	connectFlag=true;
	new std::thread(&BNSocketUtil::threadReceiveTask);
}

void* BNSocketUtil::threadReceiveTask()
{
	while(connectFlag)
	{
		CCLOG("---------MSG--------");
		//每一轮先接收标志位整数   0-表示数据为整数  1-表示数据为浮点数 2-表示字符串
		int ri=receiveInt();
		CCLOG("flag=%d",ri);
		if(ri==0)
		{
			int data=receiveInt();
			CCLOG("data=%d",data);
			msgLabel->setString(String::createWithFormat("%d",data)->getCString());
		}
		else if(ri==1)
		{
			float data=receiveFloat();
			CCLOG("data=%f",data);
			msgLabel->setString(String::createWithFormat("%f",data)->getCString());
		}
		else if(ri=2)
		{
			char* data=receiveStr();
			CCLOG("data=%s",data);
			msgLabel->setString(data);
		}
	}
	CCLOG("---------Dis--------");
}
