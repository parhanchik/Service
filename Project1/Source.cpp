#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>

SERVICE_STATUS serviceStatus;
SERVICE_STATUS_HANDLE serviceStatusHandle;
LPTSTR servicePath;
LPTSTR serviceName = (LPTSTR)"Bsit";

int addLogMessage(int code, const char* str)
{
	FILE* log = fopen("C:\\Users\\1\\Desktop\\log.txt", "a+");
	if (log)
	{
		fprintf(log, "[code: %u] %s\n", code, str);
		fclose(log);
		return 0;
	}
}

int InstallService() {
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!hSCManager) {
		addLogMessage(0, "Error: Can't open Service Control Manager");
		return -1;
	}

	SC_HANDLE hService = CreateService(
		hSCManager,
		serviceName,
		serviceName,
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		servicePath,
		NULL, NULL, NULL, NULL, NULL
	);
	if (!hService) {
		int err = GetLastError();
		switch (err) {
		case ERROR_ACCESS_DENIED:
			addLogMessage(1,"Error: ERROR_ACCESS_DENIED");
			break;
		case ERROR_CIRCULAR_DEPENDENCY:
			addLogMessage(1, "Error: ERROR_CIRCULAR_DEPENDENCY");
			break;
		case ERROR_DUPLICATE_SERVICE_NAME:
			addLogMessage(1, "Error: ERROR_DUPLICATE_SERVICE_NAME");
			break;
		case ERROR_INVALID_HANDLE:
			addLogMessage(1, "Error: ERROR_INVALID_HANDLE");
			break;
		case ERROR_INVALID_NAME:
			addLogMessage(1, "Error: ERROR_INVALID_NAME");
			break;
		case ERROR_INVALID_PARAMETER:
			addLogMessage(1, "Error: ERROR_INVALID_PARAMETER");
			break;
		case ERROR_INVALID_SERVICE_ACCOUNT:
			addLogMessage(1, "Error: ERROR_INVALID_SERVICE_ACCOUNT");
			break;
		case ERROR_SERVICE_EXISTS:
			addLogMessage(1, "Error: ERROR_SERVICE_EXISTS");
			break;
		default:
			addLogMessage(1, "Error: Undefined");
		}
		CloseServiceHandle(hSCManager);
		return -1;
	}
	CloseServiceHandle(hService);

	CloseServiceHandle(hSCManager);
	addLogMessage(1, "Success install service!");
	return 0;
}


int RemoveService()
{
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCManager)
	{
		addLogMessage(2, "Error: Can't open Service Control Manager");
		return -1;
	}
	SC_HANDLE hService = OpenService(hSCManager, serviceName, SERVICE_STOP | DELETE);
	if (!hService)
	{
		addLogMessage(2, "Error: Can't remove service");
		CloseServiceHandle(hSCManager);
		return -1;
	}

	DeleteService(hService);
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
	addLogMessage(2 ,"Success remove service!");
	return 0;
}

int StartSService()
{
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	SC_HANDLE hService = OpenService(hSCManager, serviceName, SERVICE_START);
	
	if (!StartService(hService, 0, NULL)) {
		CloseServiceHandle(hSCManager);
		addLogMessage(6, "Error: Can't start service");
		return -1;
	}

	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
	return 0;
}

void ControlHandler(DWORD request)
{
	switch (request)
	{
	case SERVICE_CONTROL_STOP:
		addLogMessage(4, "Stopped.");

		serviceStatus.dwWin32ExitCode = 0;
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(serviceStatusHandle, &serviceStatus);
		return;


	case SERVICE_CONTROL_SHUTDOWN:
		addLogMessage(4, "Shutdown.");

		serviceStatus.dwWin32ExitCode = 0;
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(serviceStatusHandle, &serviceStatus);
		return;

	default:
		break;
	}

	SetServiceStatus(serviceStatusHandle, &serviceStatus);

	return;
}

void ServiceMain(int argc, char** argv) {

	serviceStatusHandle = RegisterServiceCtrlHandler(serviceName, (LPHANDLER_FUNCTION)ControlHandler);
	if (!serviceStatusHandle)
	{
		addLogMessage(5, "Error: Registering ServiceControl");
		return;
	}
	serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	serviceStatus.dwCurrentState = SERVICE_START_PENDING;
	serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	serviceStatus.dwWin32ExitCode = 0;
	serviceStatus.dwServiceSpecificExitCode = 0;
	serviceStatus.dwCheckPoint = 0;
	serviceStatus.dwWaitHint = 0;
	serviceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(serviceStatusHandle, &serviceStatus);
	while (serviceStatus.dwCurrentState == SERVICE_RUNNING)
	{
		FILE *f = fopen("C:\\Users\\1\\Desktop\\config.txt", "r");
		if (f == NULL)
		{
			addLogMessage(5, "Error: config.txt");
			serviceStatus.dwCurrentState = SERVICE_STOPPED;
			serviceStatus.dwWin32ExitCode = -1;
			SetServiceStatus(serviceStatusHandle, &serviceStatus);
			return;
		}
		else
		{
			char temp[128], command[1024];
			sprintf(command, "\"C:\\Program Files (x86)\\7-Zip\\7z.exe\"  a -tzip ");
			while (fgets(temp, sizeof(temp), f))
				sprintf(command + strlen(command) -1, " %s", temp);
			sprintf(command + strlen(command), "\n\0");
			system(command);
		}
	}
	addLogMessage(5, "Success!");
	return;
}
void main(int argc, char* argv[])
{
	servicePath = LPTSTR(argv[0]);

	if (argc - 1 == 0)
	{
		SERVICE_TABLE_ENTRY ServiceTable[2];
		ServiceTable[0].lpServiceName = serviceName;
		ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;
		ServiceTable[1].lpServiceName = NULL;
		ServiceTable[1].lpServiceProc = NULL;

		if (!StartServiceCtrlDispatcher(ServiceTable))
		{
			addLogMessage(6, "Error: StartServiceCtrlDispatcher");
		}
	}
	else if (strcmp(argv[argc - 1], "install") == 0)
	{
		InstallService();
	}
	else if (strcmp(argv[argc - 1], "remove") == 0)
	{
		RemoveService();
	}
	else if (strcmp(argv[argc - 1], "start") == 0)
	{
		StartSService();
	}
	else if (strcmp(argv[argc - 1], "stop") == 0)
	{
			char command[32];
			sprintf(command, "net stop %s", serviceName);
			system(command);
	}
}
