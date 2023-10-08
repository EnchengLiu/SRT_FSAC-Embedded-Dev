#include "readport.h"

int main()
{
	WZSerialPort w;
	//这里是选择端口号，其他波特率信息可在头文件修改，或者在下面重新赋值。
	if (w.open("COM3"))
	{
		cout << "打开成功" << endl;
		cout << "在这里我发送：恭喜发财" << endl;

		w.send("恭喜发财");
		//w.close();
	}
	else
	{
		cout << "打开失败" << endl;
	}
	
	while (true)
	{	
		//w.receive();
		cout << "receive: " << w.receive() << endl;
		//w.revcmsg.clear();


	}
	return 0;
}
