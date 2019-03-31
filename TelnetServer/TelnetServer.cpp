
#include "stdafx.h"

#include "winsock2.h"
SOCKET registeredClients[64];
int numRegisteredClients = 0;

int main()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(9000);

	bind(listener, (SOCKADDR *)&addr, sizeof(addr));
	listen(listener, 5);

	SOCKET sockets[64];
	WSAEVENT events[64];
	int numEvents = 0;

	WSAEVENT newEvent = WSACreateEvent();
	WSAEventSelect(listener, newEvent, FD_ACCEPT);

	sockets[numEvents] = listener;
	events[numEvents] = newEvent;
	numEvents++;

	int ret;
	char buf[256];
	char fileBuf[256];
	while (true)
	{
		ret = WSAWaitForMultipleEvents(numEvents, events, FALSE, INFINITE, FALSE);
		if (ret == WSA_WAIT_FAILED)
			break;

		if (ret == WSA_WAIT_TIMEOUT)
		{
			printf("Timed Out...\n");
			continue;
		}

		int index = ret - WSA_WAIT_EVENT_0;
		for (int i = index; i < numEvents; i++)
		{
			ret = WSAWaitForMultipleEvents(1, &events[i], TRUE, 0, FALSE);
			if (ret == WSA_WAIT_FAILED)
				continue;
			if (ret == WSA_WAIT_TIMEOUT)
				continue;

			WSANETWORKEVENTS networkEvents;
			WSAEnumNetworkEvents(sockets[i], events[i], &networkEvents);
			WSAResetEvent(events[i]);

			if (networkEvents.lNetworkEvents & FD_ACCEPT)
			{
				if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
				{
					printf("FD_ACCEPT failed\n");
					continue;
				}

				if (numEvents == WSA_MAXIMUM_WAIT_EVENTS)
				{
					printf("Too many connection\n");
					continue;
				}

				SOCKET client = accept(sockets[i], NULL, NULL);

				newEvent = WSACreateEvent();
				WSAEventSelect(client, newEvent, FD_READ | FD_CLOSE);

				events[numEvents] = newEvent;
				sockets[numEvents] = client;
				numEvents++;

				printf("New client connected %d\n", client);

			}

			if (networkEvents.lNetworkEvents & FD_READ)
			{
				if (networkEvents.iErrorCode[FD_READ_BIT] != 0)
				{
					printf("FD_READ failed\n");
					continue;
				}
				bool connected = false;
				int numberInArr = 0;
				ret = recv(sockets[i], buf, sizeof(buf), 0);
				if (ret <= 0)
				{
					printf("FD_READ failed\n");
					continue;
				}

				buf[ret] = 0;
				int j = 0;
				for (; j < numRegisteredClients; j++)
					if (sockets[i] == registeredClients[j]) {
						connected = true;
						numberInArr = i;
					}
				char cmd[64];
				char id[64];
				char tmp[64];
		
				char sendBuf[512];
				char cmdBuf[256];
				char *errorMsg = "Loi cu phap. Hay nhap lai\n";

				char targetId[64];
				printf("Received: %s\n", buf);
				if (connected) {

					if (buf[ret - 1] == '\n')

						buf[ret - 1] = 0;

					sprintf(cmdBuf, "%s > c:\\test_server\\out.txt", buf);

					system(cmdBuf);

					FILE *f = fopen("c:\\test_server\\out.txt", "r");

					while (fgets(fileBuf, sizeof(fileBuf), f))

					{

						send(sockets[i], fileBuf, strlen(fileBuf), 0);

					}

					fclose(f);

				}

				else {

					int found = 0;

					char pass[64];

					char userName[64];

					FILE *f = fopen("data.txt", "r");

					while (fgets(fileBuf, sizeof(fileBuf), f))

					{

						if (strcmp(buf, fileBuf) == 0)

						{

							ret = sscanf(buf, "%s %s", userName, pass);

							found = 1;

							break;

						}

					}

					fclose(f);



					if (found == 1)

					{

						char *msg = "Dang nhap thanh cong. Hay nhap lenh.\n";

						send(sockets[i], msg, strlen(msg), 0);

						registeredClients[numRegisteredClients] = sockets[i];

						numRegisteredClients++;

						connected = true;

						break;
					}

					else
					{
						char *msg = "Dang nhap that bai. Hay thu lai.\n";

						send(sockets[i], msg, strlen(msg), 0);
					}
				}

			}


			if (networkEvents.lNetworkEvents & FD_CLOSE)
			{
				if (networkEvents.iErrorCode[FD_CLOSE_BIT] != 0)
				{
					printf("FD_CLOSE failed\n");
					continue;
				}

				closesocket(sockets[i]);
				// Xoa socket va event khoi mang

				printf("Client disconnected\n");
			}
		}
	}

	return 0;
}