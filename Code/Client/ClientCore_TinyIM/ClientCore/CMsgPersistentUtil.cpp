/**
 * @file CMsgPersistentUtil.cpp
 * @author DennisMi (https://www.dennisthink.com/)
 * @brief 消息持久化实现
 * @version 0.1
 * @date 2019-09-12
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include "CMsgPersistentUtil.h"
#include "fmt/format.h"
#include <time.h>
/**
 * @brief 初始化数据库
 * 
 * @return true 
 * @return false 
 */

std::shared_ptr<spdlog::logger> CMsgPersistentUtil::ms_logger=nullptr;
bool CMsgPersistentUtil::InitDataBase()
{
	if (nullptr == ms_logger)
	{
		ms_logger = spdlog::default_logger();
	}
	m_pDb = new SQLite::Database("TinyDemo.db",SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
	if (m_pDb->tableExists("T_FRIEND_CHAT_MSG"))
	{
		LOG_INFO(ms_logger, "TABLE T_FRIEND_CHAT_MSG is Exist [ {} {} ]", __FILE__, __LINE__);
	}
	else
	{
		std::string strSQL = R"(CREATE TABLE T_FRIEND_CHAT_MSG(
		F_MSG_ID TEXT,
		F_MSG_TYPE TEXT,
        F_FROM_ID TEXT,
		F_TO_ID TEXT,
		F_MSG_CONTEXT TEXT,
        F_OTHER_INFO TEXT,
		F_READ_FLAG TEXT,
		F_CREATE_TIME TEXT))";
		m_pFriendChatCreate = new SQLite::Statement(*m_pDb, strSQL);
		m_pFriendChatCreate->executeStep();
		// Reset the query to be able to use it again later
		m_pFriendChatCreate->reset();
	}


	if (m_pDb->tableExists("T_GROUP_CHAT_MSG"))
	{
		LOG_INFO(ms_logger, "TABLE T_GROUP_CHAT_MSG is Exist [ {} {} ]", __FILE__, __LINE__);
	}
	else
	{
		std::string strSQL = R"(CREATE TABLE T_GROUP_CHAT_MSG(
		F_MSG_ID TEXT,
		F_MSG_TYPE TEXT,
        F_USER_ID TEXT,
		F_GROUP_ID TEXT,
		F_MSG_CONTEXT TEXT,
        F_OTHER_INFO TEXT,
		F_READ_FLAG TEXT,
		F_CREATE_TIME TEXT))";
		m_pGroupChatCreate = new SQLite::Statement(*m_pDb, strSQL);
		m_pGroupChatCreate->executeStep();
		// Reset the query to be able to use it again later
		m_pGroupChatCreate->reset();
	}

	if(nullptr == m_pGroupChatSelect)
	{
		std::string strSQL = R"(SELECT * FROM T_GROUP_CHAT_MSG WHERE F_READ_FLAG='UN_READ')";

		m_pGroupChatSelect = new SQLite::Statement(*m_pDb, strSQL);
	}

	if (nullptr == m_pGroupChatUpdate)
	{
		std::string strSqlTemplate = R"(UPDATE T_GROUP_CHAT_MSG SET F_READ_FLAG='READ' WHERE F_MSG_ID=?)";
		m_pGroupChatUpdate = new SQLite::Statement(*m_pDb, strSqlTemplate);
	}

	if (nullptr == m_pGroupChatInsert)
	{
		std::string strSQLTemplate = R"(INSERT INTO T_GROUP_CHAT_MSG VALUES (?,?,?,?,?,?,?,?) )";
		m_pGroupChatInsert = new SQLite::Statement(*m_pDb, strSQLTemplate);
	}
	return false;
}


/**
 * @brief 保存添加好友的请求
 * 
 * @param msg 
 * @return true 
 * @return false 
 */
bool CMsgPersistentUtil::Save_AddFriendRecvReqMsg(const AddFriendRecvReqMsg& msg)
{
	auto item = m_AddFriendRecvReqMsgMap.find(msg.m_strMsgId);
	if (item != m_AddFriendRecvReqMsgMap.end())
	{
		return true;
	}
	else
	{
		m_AddFriendRecvReqMsgMap.insert(std::pair<std::string, AddFriendRecvReqMsg>(msg.m_strMsgId,msg));
		return true;
	}

}

/**
 * @brief 获取好友接收请求消息
 * 
 * @param msg 
 * @return true 
 * @return false 
 */
bool CMsgPersistentUtil::Get_AddFriendRecvReqMsg(AddFriendRecvReqMsg& msg)
{
	auto item = m_AddFriendRecvReqMsgMap.find(msg.m_strMsgId);
	if (item != m_AddFriendRecvReqMsgMap.end())
	{
		msg = item->second;
		return true;
	}
	else
	{
		if (!m_AddFriendRecvReqMsgMap.empty())
		{
			msg = m_AddFriendRecvReqMsgMap.begin()->second;
			return true;
		}
	}
	return false;
}

/**
 * @brief 删除添加好友消息
 * 
 * @param msg 
 * @return true 
 * @return false 
 */
bool CMsgPersistentUtil::Remove_AddFriendRecvReqMsg(const AddFriendRecvReqMsg& msg)
{
	m_AddFriendRecvReqMsgMap.erase(msg.m_strMsgId);
	return true;
}

/**
 * @brief 保存添加好友通知请求消息
 * 
 * @param msg 
 * @return true 
 * @return false 
 */
bool CMsgPersistentUtil::Save_AddFriendNotifyReqMsg(const AddFriendNotifyReqMsg& msg)
{
	auto item = m_AddFriendNotifyReqMsgMap.find(msg.m_strMsgId);
	if (item != m_AddFriendNotifyReqMsgMap.end())
	{
		return true;
	}
	else
	{
		m_AddFriendNotifyReqMsgMap.insert(std::pair<std::string, AddFriendNotifyReqMsg>(msg.m_strMsgId, msg));
	}
	return false;
}

/**
 * @brief 获取添加好友请求消息
 * 
 * @param msg 
 * @return true 
 * @return false 
 */
bool CMsgPersistentUtil::Get_AddFriendNotifyReqMsg(AddFriendNotifyReqMsg& msg)
{
	auto item = m_AddFriendNotifyReqMsgMap.find(msg.m_strMsgId);
	if (item != m_AddFriendNotifyReqMsgMap.end())
	{
		msg = item->second;
		return true;
	}
	else
	{
		if (!m_AddFriendNotifyReqMsgMap.empty())
		{
			msg = m_AddFriendNotifyReqMsgMap.begin()->second;
			return true;
		}
	}
	return false;
}

/**
 * @brief 移除添加好友请求消息
 * 
 * @param msg 
 * @return true 
 * @return false 
 */
bool CMsgPersistentUtil::Remove_AddFriendNotifyReqMsg(const AddFriendNotifyReqMsg& msg)
{
	m_AddFriendNotifyReqMsgMap.erase(msg.m_strMsgId);
	return true;
}

/**
 * @brief 保存接收到的好友文本消息
 * 
 * @param msg 
 * @return true 
 * @return false 
 */
bool CMsgPersistentUtil::Save_FriendChatRecvTxtReqMsg(const FriendChatRecvTxtReqMsg& msg)
{
	std::string strSQLTemplate = R"(INSERT INTO T_FRIEND_CHAT_MSG VALUES('{0}','{1}','{2}','{3}','{4}','{5}','UNREAD','{6}'))";
	std::string strSql = fmt::format(strSQLTemplate,
		msg.m_strMsgId,
		ChatType(CHAT_MSG_TYPE::E_CHAT_TEXT_TYPE),
		msg.m_strFromId,
		msg.m_strToId,
		msg.m_strContext,
		msg.m_fontInfo.ToString(),
		std::to_string(time(nullptr)));
	SQLite::Statement*   m_pInsertQuery = new SQLite::Statement(*m_pDb, strSql);
	bool bResult = m_pInsertQuery->executeStep();
	// Reset the query to be able to use it again later
	m_pInsertQuery->reset();

	return bResult;
}

/**
 * @brief 获取好友聊天消息
 * 
 * @param msg 
 * @return true 
 * @return false 
 */
bool CMsgPersistentUtil::Get_FriendChatRecvTxtReqMsg(FriendChatRecvTxtReqMsg& msg)
{
	std::string strSQL = R"(SELECT * FROM T_FRIEND_CHAT_MSG WHERE F_READ_FLAG='UNREAD';)";

	SQLite::Statement*   m_pInsertQuery = new SQLite::Statement(*m_pDb, strSQL);
	bool bResult = m_pInsertQuery->executeStep();
	if (bResult)
	{
		msg.m_strMsgId = m_pInsertQuery->getColumn(0).getString();
		msg.m_strFromId = m_pInsertQuery->getColumn(2).getString();
		msg.m_strToId = m_pInsertQuery->getColumn(3).getString();
		msg.m_strContext = m_pInsertQuery->getColumn(4).getString();
	}
	else
	{

	}
	// Reset the query to be able to use it again later
	m_pInsertQuery->reset();
	/*auto item = m_FriendChatRecvTxtReqMsgMap.find(msg.m_strMsgId);
	if (item != m_FriendChatRecvTxtReqMsgMap.end())
	{
		return true;
	}
	else
	{
		m_FriendChatRecvTxtReqMsgMap.insert(std::pair<std::string, FriendChatRecvTxtReqMsg>(msg.m_strMsgId, msg));
	}*/
	return bResult;
}

/**
 * @brief 更新好友聊天的文本消息
 * 
 * @param msg 
 * @return true 
 * @return false 
 */
bool CMsgPersistentUtil::Update_FriendChatRecvTxtReqMsg(const FriendChatRecvTxtReqMsg& msg)
{
	std::string strSqlTemplate = R"(UPDATE T_FRIEND_CHAT_MSG SET F_READ_FLAG='READ' WHERE F_MSG_ID='{0}';)";

	std::string strSql = fmt::format(strSqlTemplate,msg.m_strMsgId);
	SQLite::Statement*   m_pInsertQuery = new SQLite::Statement(*m_pDb, strSql);
	bool bResult = m_pInsertQuery->executeStep();
	return bResult;
}


//RecvGroupTextMsgReqMsg
/**
 * @brief 保存群组聊天的文本消息
 * 
 * @param msg 
 * @return true 
 * @return false 
 */
bool CMsgPersistentUtil::Save_RecvGroupTextMsgReqMsg(const RecvGroupTextMsgReqMsg& msg)
{
	if (nullptr != m_pGroupChatInsert)
	{
		SQLite::bind(*m_pGroupChatInsert,
			msg.m_strMsgId,
			ChatType(CHAT_MSG_TYPE::E_CHAT_TEXT_TYPE),
			msg.m_strSenderId,
			msg.m_strGroupId,
			msg.m_strContext,
			"OTHER_INFO",
			"UN_READ",
			std::to_string(time(nullptr)));
		int nNb = m_pGroupChatInsert->exec();
		m_pGroupChatInsert->reset();
		if (nNb != 0)
		{
			return true;
		}
		else
		{
			std::string strMsg = m_pGroupChatInsert->getErrorMsg();
			LOG_ERR(ms_logger, "{} [{} {}]", strMsg, __FILENAME__, __LINE__);
			// Reset the query to be able to use it again later

			return false;
		}
	}
	else
	{
		return false;
	}

}

/**
 * @brief 获取收到的群聊文本消息
 * 
 * @param msg 
 * @return true 
 * @return false 
 */
bool CMsgPersistentUtil::Get_RecvGroupTextMsgReqMsg(RecvGroupTextMsgReqMsg& msg)
{
	bool bResult = m_pGroupChatSelect->executeStep();
	if (bResult)
	{
		msg.m_strMsgId = m_pGroupChatSelect->getColumn(0).getString();
		msg.m_strGroupId = m_pGroupChatSelect->getColumn(2).getString();
		msg.m_strSenderId = m_pGroupChatSelect->getColumn(3).getString();
		msg.m_strContext = m_pGroupChatSelect->getColumn(4).getString();
	}
	if (!bResult)
	{
		std::string strMsg = m_pGroupChatSelect->getErrorMsg();
		LOG_ERR(ms_logger, "{} [{} {}]", strMsg, __FILENAME__, __LINE__);
	}
	// Reset the query to be able to use it again later
	m_pGroupChatSelect->reset();
	return bResult;
}

/**
 * @brief 更新收到的群聊文本消息
 * 
 * @param msg 
 * @return true 
 * @return false 
 */
bool CMsgPersistentUtil::Update_RecvGroupTextMsgReqMsg(const RecvGroupTextMsgReqMsg& msg)
{
	SQLite::bind(*m_pGroupChatUpdate, msg.m_strMsgId);
	int nResult = m_pGroupChatUpdate->exec();
	m_pGroupChatUpdate->reset();
	return nResult != 0;
}


bool CMsgPersistentUtil::Save_FriendRecvFileMsgReqMsg(const FriendRecvFileMsgReqMsg& msg)
{
	m_FriendRecvFileMsgReqMsgMap.insert(std::pair<std::string,FriendRecvFileMsgReqMsg>(msg.m_strToId,msg));
	return true;
}

bool CMsgPersistentUtil::Get_FriendRecvFileMsgReqMsg(FriendRecvFileMsgReqMsg& msg)
{
	if (!m_FriendRecvFileMsgReqMsgMap.empty())
	{
		msg = m_FriendRecvFileMsgReqMsgMap.begin()->second;
	}
	return true;
}

bool CMsgPersistentUtil::Update_FriendRecvFileMsgReqMsg(const FriendRecvFileMsgReqMsg& msg)
{
	if (!m_FriendRecvFileMsgReqMsgMap.empty())
	{
		m_FriendRecvFileMsgReqMsgMap.erase(m_FriendRecvFileMsgReqMsgMap.begin());
	}
	return true;
}


bool CMsgPersistentUtil::Save_FriendNotifyFileMsgReqMsg(const FriendNotifyFileMsgReqMsg& msg)
{
	m_FriendNotifyFileMsgReqMsgMap.insert({msg.m_strToId, msg});
	return false;
}

bool CMsgPersistentUtil::Get_FriendNotifyFileMsgReqMsg(FriendNotifyFileMsgReqMsg& msg)
{
	if (m_FriendNotifyFileMsgReqMsgMap.empty())
	{
		return false;
	}
	msg = (m_FriendNotifyFileMsgReqMsgMap.begin()->second);
	return true;
}

bool CMsgPersistentUtil::Update_FriendNotifyFileMsgReqMsg(const FriendNotifyFileMsgReqMsg& msg)
{
	if (m_FriendNotifyFileMsgReqMsgMap.empty())
	{
		return false;
	}
	m_FriendNotifyFileMsgReqMsgMap.erase(m_FriendNotifyFileMsgReqMsgMap.begin());
	return true;
}