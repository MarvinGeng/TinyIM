/**
 * @file Thread.h
 * @author DennisMi (https://www.dennisthink.com/)
 * @brief 线程的封装类
 * @version 0.1
 * @date 2019-08-04
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#ifndef _DENNIS_THINK_THREAD_H_
#define _DENNIS_THINK_THREAD_H_

#include <thread>
#include <memory>

class CThread
{
public:
    CThread();
    virtual ~CThread();

    CThread(const CThread& rhs) = delete;
    CThread& operator=(const CThread& rhs) = delete;

    void Start();
    virtual void Stop() = 0;

    void Join();

protected:
    virtual void Run() = 0;

private:
    void ThreadProc();

protected:
    bool                            m_bStop{ false }; //是否已经停止

private:
    std::shared_ptr<std::thread>    m_spThread; //线程类
};

#endif