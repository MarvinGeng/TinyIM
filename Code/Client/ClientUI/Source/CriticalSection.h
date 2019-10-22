/**
 * @brief 自动临界区，暂时没有使用
 * 
 */

#ifndef _DENNIS_THINK_C_AUTO_CRITICAL_SECTION_H_ 
#define _DENNIS_THINK_C_AUTO_CRITICAL_SECTION_H_
class CAutoCriticalSection
{
public:
	CAutoCriticalSection(CRITICAL_SECTION& cs);
	~CAutoCriticalSection();

	void Lock();
	void Unlock();

private:
	CRITICAL_SECTION&	 m_cs;	
};
#endif