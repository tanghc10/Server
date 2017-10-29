#include "stdafx.h"
#include "Message_handle.h"
#include "protocol.h"
#include "cJSON.h"


Message_handle::Message_handle()
{
}


Message_handle::~Message_handle()
{
}

typedef int(*CLIENT_PROC)(void);
typedef struct
{
	char cmd;
	CLIENT_PROC pfn;
}MSG_PROC;

int client_login(void)
{
	return 0;
}

static MSG_PROC Procs[] =
{
	{ LOG_IN ,			client_login}
};

int Message_handle::MessageCommand()
{
	//CClientView* pView = (CClientView*)((CMainFrame*)AfxGetApp()->m_pMainWnd)->GetActiveView();
	//pView->m_listData.AddString(CString(buf));
	int i = 0;
	int cmd = 0;
	CLIENT_PROC pfn = NULL;
	cJSON *json_root = NULL;

	if (!buf)
	{
		return FALSE;
	}
	
	json_root = cJSON_Parse(buf);
	if (!json_root)
	{
		return FALSE;
	}
	cmd = cJSON_GetObjectItem(json_root, "c")->valueint;
	for (; i < sizeof(Procs) / sizeof(MSG_PROC); i++)
	{
		if (Procs[i].cmd == cmd)
		{
			pfn = Procs->pfn;
			if (pfn)
			{
				pfn();
				cJSON_Delete(json_root);
				return TRUE;
			}
			else
			{
				cJSON_Delete(json_root);
				return FALSE;
			}
		}
	}

	cJSON_Delete(json_root);
	return FALSE;
	return 0;
}