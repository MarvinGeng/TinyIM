
/**
 * @file CMsgPersistentUtil.h
 * @author DennisMi (https://www.dennisthink.com/)
 * @brief 客户端消息持久化的类
 * @version 0.1
 * @date 2019-09-12
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef _DENNIS_THINK_C_MSG_PERSISTENT_UTIL_H_
#define _DENNIS_THINK_C_MSG_PERSISTENT_UTIL_H_
#include <memory>
#include "CommonMsg.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "SQLiteCpp/VariadicBind.h"
#include "Log.h"

class CMsgPersistentUtil: public std::enable_shared_from_this<CMsgPersistentUtil>
{
public:
	bool InitDataBase();

	//AddFriendRecvReqMsg
	bool Save_AddFriendRecvReqMsg(const AddFriendRecvReqMsg& msg);

	bool Get_AddFriendRecvReqMsg(AddFriendRecvReqMsg& msg);

	bool Remove_AddFriendRecvReqMsg(const AddFriendRecvReqMsg& msg);

	//AddFriendNotifyReqMsg
	bool Save_AddFriendNotifyReqMsg(const AddFriendNotifyReqMsg& msg);

	bool Get_AddFriendNotifyReqMsg(AddFriendNotifyReqMsg& msg);

	bool Remove_AddFriendNotifyReqMsg(const AddFriendNotifyReqMsg& msg);

	//FriendChatRecvTxtReqMsg
	bool Save_FriendChatRecvTxtReqMsg(const FriendChatRecvTxtReqMsg& msg);

	bool Get_FriendChatRecvTxtReqMsg(FriendChatRecvTxtReqMsg& msg);

	bool Update_FriendChatRecvTxtReqMsg(const FriendChatRecvTxtReqMsg& msg);

	//FriendRecvFileMsgReqMsg
	bool Save_FriendRecvFileMsgReqMsg(const FriendRecvFileMsgReqMsg& msg);

	bool Get_FriendRecvFileMsgReqMsg(FriendRecvFileMsgReqMsg& msg);

	bool Update_FriendRecvFileMsgReqMsg(const FriendRecvFileMsgReqMsg& msg);


	//FriendNotifyFileMsgReqMsg
	bool Save_FriendNotifyFileMsgReqMsg(const FriendNotifyFileMsgReqMsg& msg);

	bool Get_FriendNotifyFileMsgReqMsg(FriendNotifyFileMsgReqMsg& msg);

	bool Update_FriendNotifyFileMsgReqMsg(const FriendNotifyFileMsgReqMsg& msg);

	//RecvGroupTextMsgReqMsg
	bool Save_RecvGroupTextMsgReqMsg(const RecvGroupTextMsgReqMsg& msg);

	bool Get_RecvGroupTextMsgReqMsg(RecvGroupTextMsgReqMsg& msg);

	bool Update_RecvGroupTextMsgReqMsg(const RecvGroupTextMsgReqMsg& msg);

	static std::shared_ptr<spdlog::logger> ms_logger;

private:
	std::map<std::string, AddFriendRecvReqMsg> m_AddFriendRecvReqMsgMap;
	std::map<std::string, AddFriendNotifyReqMsg> m_AddFriendNotifyReqMsgMap;
	std::map<std::string, FriendChatRecvTxtReqMsg> m_FriendChatRecvTxtReqMsgMap;
	std::map<std::string, RecvGroupTextMsgReqMsg> m_RecvGroupTextMsgReqMsgMap;
	std::map<std::string, FriendRecvFileMsgReqMsg> m_FriendRecvFileMsgReqMsgMap;
	std::map<std::string, FriendNotifyFileMsgReqMsg> m_FriendNotifyFileMsgReqMsgMap;

	
	SQLite::Database*    m_pDb;    ///< Database connection
	//FriendChat Msg
	SQLite::Statement*   m_pFriendChatCreate; ///< Database prepared SQL query

	//GroupChatMsg Msg
	SQLite::Statement*   m_pGroupChatCreate; ///< Database prepared SQL query

	SQLite::Statement*   m_pGroupChatInsert; ///< Database prepared SQL query

	SQLite::Statement*   m_pGroupChatSelect; ///< Database prepared SQL query

	SQLite::Statement*   m_pGroupChatUpdate; ///< Database prepared SQL query

};
using CMsgPersistentUtil_SHARED_PTR = std::shared_ptr<CMsgPersistentUtil>;
#endif