#ifndef _DENNIS_THINK_C_IM_ROBOT_H_
#define _DENNIS_THINK_C_IM_ROBOT_H_
#include <string>
#include <vector>
class CIMRobot
{
public:
	CIMRobot();
	virtual ~CIMRobot();
	bool loadConfig(const std::string strCfg);
	void Run();
private:
	void UserLogin();
	void UserLogout();

	//Friend Begin
	void GetFriendList();
	void SendMsg();
	void SendFaceMsg();
	void SendSingleImageMsg();
	void GetRecvMsg();

	void GetFriendHistroyFirst();
	void GetFriendHistoryLast();
	void GetFriendHistoryPrev();
	void GetFriendHistoryNext();

	void AddFriendTeam();
	void RemoveFriendTeam();
	void MoveFriendToTeam();
	//Friend End

	//Group Begin
	void GetGroupList();
	void SendGroupMsg();
	void GetRecvGroupMsg();


	void GetGroupHistroyFirst();
	void GetGroupHistoryLast();
	void GetGroupHistoryPrev();
	void GetGroupHistoryNext();

	void SearchChatMsg();
	//Group End
	//Load From Client
	std::string m_strHttpIp;
	int m_nHttpPort;
	std::string m_strUserName;
	std::string m_strPassWord;

	//Http Data
	std::string m_strUserId;
	std::vector<std::string> m_strFriendVec;
	std::vector<std::string> m_strGroupVec;
};
#endif