#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#include "CommonMsg.h"
#include "CMsgPersistentUtil.h"
#include "SnowFlake.h"
static CMsgPersistentUtil util("1234");
static SnowFlake g_snowFlake(4, 4);
static std::string MsgId() {
	return std::to_string(g_snowFlake.nextId());
}

static std::string UserId(){
	std::string strMsgId = MsgId();
	if (strMsgId.length() > 8) {
		return strMsgId.substr(strMsgId.length() - 8, 8);
	}
	else {
		return "12345678";
	}
}

TEST_CASE("FriendChatRecvTextMsgInsert") {
	CHECK(util.InitDataBase());
	FriendChatRecvTxtReqMsg reqMsg;
	reqMsg.m_strMsgId = MsgId();
	reqMsg.m_chatMsg.m_strChatMsgId = MsgId();
	reqMsg.m_chatMsg.m_strSenderId = UserId();
	reqMsg.m_chatMsg.m_strReceiverId = UserId();
	reqMsg.m_chatMsg.m_strMsgTime = "2019-01-01 10:10:10";
	{
		ChatMsgElemVec elemVec;
		{
			{
				ChatMsgElem elem;
				elem.m_eType = CHAT_MSG_TYPE::E_CHAT_EMOJI_TYPE;
				elem.m_nFaceId = 1;

				elemVec.push_back(elem);
			}

			{
				ChatMsgElem elem;
				elem.m_eType = CHAT_MSG_TYPE::E_CHAT_IMAGE_TYPE;
				elem.m_strImageName = "C:\\Windows\\Program File\\Test.jpg";

				elemVec.push_back(elem);
			}

			{
				ChatMsgElem elem;
				elem.m_eType = CHAT_MSG_TYPE::E_CHAT_TEXT_TYPE;
				elem.m_strContext = "I'am a \"Famous| boy,I am Neo";

				elemVec.push_back(elem);
			}
		}
		reqMsg.m_chatMsg.m_strContext = MsgElemVec(elemVec);
	}
	
	CHECK(util.Save_FriendChatSendTxtRspMsg(reqMsg.m_chatMsg));
}

TEST_CASE("GroupChatRecvTextMsgInsert") {
	CHECK(util.InitDataBase());
	RecvGroupTextMsgReqMsg reqMsg;
	reqMsg.m_strMsgId = "123456768";
	reqMsg.m_strGroupId = "11111111";
	reqMsg.m_strSenderId = "22222222";
	reqMsg.m_strContext = "TEXT";
	CHECK(util.Save_RecvGroupTextMsgReqMsg(reqMsg));
	RecvGroupTextMsgReqMsg recvMsg;
	CHECK(util.Get_RecvGroupTextMsgReqMsg(recvMsg));
	std::cout << recvMsg.ToString() << std::endl;
	CHECK(util.Update_RecvGroupTextMsgReqMsg(recvMsg));
    
}