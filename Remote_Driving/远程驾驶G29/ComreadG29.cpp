#include <Windows.h>
#include <thread>
#include <stdio.h>
#include <iostream>
#include <winsock.h>
#include <stdlib.h>
#include <string.h>

#include "LogitechSteeringWheelLib.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "LogitechSteeringWheelLib.lib")
#pragma comment( linker, "/subsystem:\"console\" /entry:\"WinMainCRTStartup\"")

using namespace std;



void initialization() {
	//初始化套接字库
	WORD w_req = MAKEWORD(2, 2);//版本号
	WSADATA wsadata;
	int err;
	err = WSAStartup(w_req, &wsadata);
	if (err != 0) {
		cout << "初始化套接字库失败！" << endl;
	}
	else {
		//cout << "初始化套接字库成功！" << endl;
	}
	//检测版本号
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
		cout << "套接字库版本号不符！" << endl;
		WSACleanup();
	}
	else {
		//cout << "套接字库版本正确！" << endl;
	}
	//填充服务端地址信息

}

void SendMessage(int x, int y, int z, int bb)
{
	//定义长度变量
	int send_len = 0;
	int recv_len = 0;
	//定义发送缓冲区和接受缓冲区
	char send_buf[100];
	//char recv_buf[100];
	//定义服务端套接字，接受请求套接字
	SOCKET s_server;
	//服务端地址客户端地址
	SOCKADDR_IN server_addr;
	initialization();
	//填充服务端信息
	server_addr.sin_family = AF_INET;
	//server_addr.sin_addr.S_un.S_addr = inet_addr("119.45.138.172");
	server_addr.sin_addr.S_un.S_addr = inet_addr("81.68.251.140");
	server_addr.sin_port = htons(6666);
	//创建套接字
	s_server = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(s_server, (SOCKADDR*)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		cout << "服务器连接失败！" << endl;
		WSACleanup();
	}

	else {
		cout << "服务器连接成功！" << endl;
	}


	//发送,接收数据

		//cout << "请输入发送信息:";
		//cin >> send_buf;
	sprintf_s(send_buf, "%X%X%X%X", x + 1081344, y + 1081344, z + 1081344, bb + 1);
	send_len = send(s_server, send_buf, sizeof(send_buf), 0);
	if (send_len < 0) {

		cout << "发送失败！" << endl;

	}
	printf("send success x = %d,y = %d,z = %d,dangwei =%d\n", x + 42768, y + 42768, z + 42768, bb + 1);
	Sleep(100);
	//memset(recv_buf, 0, sizeof(recv_buf));
	/*recv_len = recv(s_server, recv_buf, 100, 0);
	if (recv_len < 0) {
		cout << "接受失败！" << endl;
	}
	else {
		cout << "服务端信息:" << recv_buf << endl;
	}*/


	//关闭套接字
	closesocket(s_server);
	//释放DLL资源
	WSACleanup();

}


int driver[5];
int driverdata = 0;
int changeddata = 0;
#define BUF_SIZE 4096

HANDLE H_Mutex = NULL;

HANDLE H_Event = NULL;


int sendthere(int aaa)

{

	//步骤1：创建共享文件句柄

	HANDLE shared_file = CreateFileMapping(

		INVALID_HANDLE_VALUE,//物理文件句柄

		NULL,  //默认安全级别

		PAGE_READWRITE,      //PAGE_READWRITE表示可读可写，PAGE_READONLY表示只读，PAGE_WRITECOPY表示只写

		0,  //高位文件大小

		BUF_SIZE,  //低位文件大小

		L"ShareMemory"  //共享内存名称

	);

	if (shared_file == NULL)

	{

		cout << "Could not create file mapping object..." << endl;

		return 1;

	}

	//步骤2：映射缓存区视图，得到指向共享内存的指针

	LPVOID lpBUF = MapViewOfFile(

		shared_file, //已创建的文件映射对象句柄

		FILE_MAP_ALL_ACCESS,//访问模式:可读写

		0, //文件偏移的高32位

		0, //文件偏移的低32位

		BUF_SIZE //映射视图的大小

	);

	if (lpBUF == NULL)

	{

		cout << "Could not create file mapping object..." << endl;

		CloseHandle(shared_file);

		return 1;

	}

	H_Mutex = CreateMutex(NULL, FALSE, L"sm_mutex");

	H_Event = CreateEvent(NULL, FALSE, FALSE, L"sm_event");



	//步骤3：操作共享内存

	char Buffer[97];



	_itoa_s(driverdata, Buffer, 10);
	//cout << "A proccess:Please input the content to the process B" << endl;

	//	cin.getline(Buffer, 97);
   // Buffer = int2str(aaa);
	Sleep(20);
	cout << "Buffer: " << Buffer << endl;

	WaitForSingleObject(H_Mutex, INFINITE); //使用互斥体加锁

	memcpy(lpBUF, Buffer, strlen(Buffer) + 1);

	ReleaseMutex(H_Mutex); //放锁

	SetEvent(H_Event);





	CloseHandle(H_Mutex);

	CloseHandle(H_Event);

	//步骤4：解除映射和关闭句柄

	UnmapViewOfFile(lpBUF);

	CloseHandle(shared_file);

	return 0;

}




int* WheelInit(HWND hwnd)
{

	//LogiSteeringInitialize(true);

	if (LogiSteeringInitialize(true))
	{

		printf("init secusssssss\n");
		//printf("%d\n", hwnd);
		int gearShifter = 0;
		//sendthere(1);
		//printf("aaaaaaaaaa\n");
		while (LogiUpdate() && LogiIsConnected(0))
		{
			printf("update seccessed\n");
			LogiPlaySpringForce(0, 0, 90, 1);//产生弹簧力
			LogiPlayDamperForce(0, 5);//产生阻尼力
			LogiPlaySurfaceEffect(0, 0, 5, 1000);//路面响应效果
			//printf("connect secuss\n");
			Sleep(20);
			DIJOYSTATE2* wheel = LogiGetState(0);

			//输出角度，油门，刹车信息

			//printf("Angle = %d  Accelerator = %d  Brake = %d\n", wheel->lX, wheel->lY,wheel->lRz);

			//printf(wheel->rgdwPOV)
			//std::cout << wheel->rglSlider[0] << endl;

							//挡位 
			if (LogiButtonIsPressed(0, 13) || LogiButtonIsPressed(0, 14) || LogiButtonIsPressed(0, 15) || LogiButtonReleased(0, 15) || LogiButtonReleased(0, 14) || LogiButtonReleased(0, 13))
			{


				//挡位 2（P) 挡
				if (LogiButtonTriggered(0, 13))
				{
					gearShifter = 2;
				}
				else if (LogiButtonIsPressed(0, 13))
				{
					// if (gearShifter.OnPressed != null) gearShifter.OnPressed(2);
				}
				else if (LogiButtonReleased(0, 13))
				{
					// if (gearShifter.OnReleased != null) gearShifter.OnReleased(2);
				}

				//挡位 3（D） 挡
				if (LogiButtonTriggered(0, 14))
				{
					gearShifter = 1;
				}
				else if (LogiButtonIsPressed(0, 14))
				{
					//if (gearShifter.OnPressed != null) gearShifter.OnPressed(3);
				}
				else if (LogiButtonReleased(0, 14))
				{
					//if (gearShifter.OnReleased != null) gearShifter.OnReleased(3);
				}

				//挡位 4（R） 挡
				if (LogiButtonTriggered(0, 15))
				{
					gearShifter = -1;
				}
				else if (LogiButtonIsPressed(0, 15))
				{
					// if (gearShifter.OnPressed != null) gearShifter.OnPressed(4);
				}
				else if (LogiButtonReleased(0, 15))
				{
					//if (gearShifter.OnReleased != null) gearShifter.OnReleased(4);
				}


				//N档            
				{
					if (LogiButtonReleased(0, 13)
						|| LogiButtonReleased(0, 14)
						|| LogiButtonReleased(0, 15)
						)
					{
						gearShifter = 0;
					}
				}
			}

			//SendMessage(wheel->lX, wheel->lY, wheel->lRz,wheel->rglSlider[0], gearShifter);
			//cout << wheel->lX<< wheel->lY<< wheel->lRz<< wheel->rglSlider[0]<< gearShifter << endl;

			driver[0] = wheel->lX;
			driver[1] = wheel->lY;
			driver[2] = wheel->lRz;
			driver[3] = wheel->rglSlider[0];
			driver[4] = gearShifter;
			changeddata = 0;
			switch (gearShifter)
			{
			case -1:
				changeddata = changeddata + 30000000;
				break;
			case 1:
				changeddata = changeddata + 10000000;
				break;
			case 2:
				changeddata = changeddata + 20000000;
				break;
			default:
				break;
			}
			changeddata = changeddata - ((driver[1] - 32767) * 22 / 65535) * 10000;
			if (driver[0] > 0) {
				changeddata = changeddata + 1000;
			}
			changeddata = changeddata + abs(driver[0]) * 450 / 32767;
			driverdata = changeddata;
			printf("Angle = %d  Accelerator = %d  Brake = %d  离合 = %d  档位 = %d\n", driver[0], driver[1], driver[2], driver[3], driver[4]);
			SendMessage(driver[0], driver[1], driver[2], driver[4]);
			//printf("Angle = %d  Accelerator = %d  Brake = %d  离合 = %d  档位 = %d\n", driver[0], driver[1], driver[2], driver[3], driver[4]);
			printf("changeddata = %d\n", changeddata);
			sendthere(111);
			printf("update finished\n");
		}

	}
	else
	{
		printf("init faild");
	}
	return driver;
}


// 必须要进行前导声明  
LRESULT CALLBACK WindowProc(
	_In_  HWND hwnd,
	_In_  UINT uMsg,
	_In_  WPARAM wParam,
	_In_  LPARAM lParam
);

int CALLBACK WinMain(
	_In_  HINSTANCE hInstance,
	_In_  HINSTANCE hPrevInstance,
	_In_  LPSTR lpCmdLine,
	_In_  int nCmdShow
)
{
	// 类名  
	TCHAR cls_Name[] = L"My Class";
	// 设计窗口类  
	WNDCLASS wc = { sizeof(WNDCLASS) };
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = cls_Name;
	wc.hInstance = hInstance;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	// 注册窗口类  
	RegisterClass(&wc);

	// 创建窗口
	HWND hwnd = CreateWindow(
		cls_Name,           //类名，要和刚才注册的一致  
		L"方向盘Demo",          //窗口标题文字  
		WS_OVERLAPPEDWINDOW,        //窗口外观样式  
		38,             //窗口相对于父级的X坐标  
		20,             //窗口相对于父级的Y坐标  
		500,                //窗口的宽度  
		500,                //窗口的高度  
		NULL,               //没有父窗口，为NULL  
		NULL,               //没有菜单，为NULL  
		hInstance,          //当前应用程序的实例句柄  
		NULL);              //没有附加数据，为NULL  
	if (hwnd == NULL)                //检查窗口是否创建成功  
		return 0;

	// 显示窗口  
	ShowWindow(hwnd, SW_SHOW);

	// 更新窗口  
	UpdateWindow(hwnd);

	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); //设置窗口最前端
	thread GetWheelData(WheelInit, hwnd);
	GetWheelData.detach();






	//MessageBox(0, "调用了WinMain函数", "测试：", 0);


	// 消息循环  
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

// 在WinMain后实现  
LRESULT CALLBACK WindowProc(
	_In_  HWND hwnd,
	_In_  UINT uMsg,
	_In_  WPARAM wParam,
	_In_  LPARAM lParam
)
{

	switch (uMsg)
	{
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	case WM_PAINT:
	{
		/*PAINTSTRUCT     ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		int length;
		TCHAR buff[1024];
		length = wsprintf(buff, TEXT("the angle is : %d"), x);
		TextOut(hdc, 20, 20, buff, length);
		EndPaint(hwnd, &ps);
		break;*/

	}
	default:
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}






