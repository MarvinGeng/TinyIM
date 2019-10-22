/**
 * @file ThreadPool.h
 * @author DennisMi (https://www.dennisthink.com/)
 * @brief 
 * @version 0.1
 * @date 2019-08-04
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef _DENNIS_THINK_THREAD_POOL_H_
#define _DENNIS_THINK_THREAD_POOL_H_
#include <vector>

class CThreadPoolTask
{
public:
	CThreadPoolTask();
	virtual ~CThreadPoolTask();

public:
	virtual BOOL IsRunning();
	virtual void SetRunning(BOOL bRunning);
	virtual int Run();
	virtual int Stop();						//锟斤拷锟斤拷锟斤拷锟斤拷锟叫碉拷锟斤拷锟斤拷锟斤拷锟斤拷慕涌冢锟斤拷锟斤拷锟绞碉拷郑锟�
	virtual void TaskFinish();				//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷丫锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷峁┮伙拷锟斤拷锟斤拷锟斤拷锟皆达拷幕锟斤拷锟�

private:
	BOOL m_bRunning;
};

struct TP_THREAD_INFO		// 锟竭筹拷锟斤拷息锟结构
{
	HANDLE hThread;
	LPARAM lParam;
};

class CThreadPool
{
public:
	CThreadPool();
	~CThreadPool();

public:
	BOOL Init(int nThreadNums);
	void UnInit();
	BOOL AddTask(CThreadPoolTask* lpTask);
	BOOL InsertTask(int nIndex, CThreadPoolTask* lpTask);
	void RemoveTask(CThreadPoolTask* lpTask);
	void RemoveAllTask();

private:
	static UINT APIENTRY ThreadProc(LPVOID lpParam);	// 锟竭程猴拷锟斤拷

private:
	std::vector<TP_THREAD_INFO*> m_arrThreadInfo;		// 锟竭筹拷锟斤拷息锟斤拷锟斤拷
	std::vector<CThreadPoolTask*> m_arrTask;			// 锟斤拷锟斤拷锟斤拷息锟斤拷锟斤拷
	
	CRITICAL_SECTION m_csThreadInfo;					// 锟竭筹拷锟斤拷息锟劫斤拷锟斤拷
	CRITICAL_SECTION m_csTask;							// 锟斤拷锟斤拷锟斤拷息锟劫斤拷锟斤拷

	HANDLE m_hSemaphore_Task;							// 锟斤拷锟斤拷通知锟脚猴拷锟斤拷
	HANDLE m_hEvent_Exit;								// 锟剿筹拷锟竭筹拷通知锟铰硷拷锟斤拷锟�
	HANDLE m_hEvent_ExitAll;							// 锟斤拷锟斤拷锟竭程撅拷锟剿筹拷锟斤拷锟酵ㄖ�锟铰硷拷锟斤拷锟�

	long m_lThreadNums;									// 锟竭筹拷锟斤拷锟斤拷
	long m_lRunningThreadNums;							// 锟斤拷锟节癸拷锟斤拷锟叫碉拷锟竭程硷拷锟斤拷
};

#endif