/**
 * @file Thread.cpp
 * @author DennisMi (https://www.dennisthink.com/)
 * @brief 线程的实现类
 * @version 0.1
 * @date 2019-08-04
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include "stdafx.h"

#include <functional>

#include "Thread.h"

CThread::CThread()
{

}

CThread::~CThread()
{

}

void CThread::Start()
{
    if (!m_spThread)
        m_spThread.reset(new std::thread(std::bind(&CThread::ThreadProc, this)));
}

void CThread::ThreadProc()
{
    Run();
}

void CThread::Join()
{
    if (m_spThread && m_spThread->joinable())
        m_spThread->join();
}