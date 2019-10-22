#ifndef _DENNIS_C_MYSQL_STRUCT_H_
#define _DENNIS_C_MYSQL_STRUCT_H_
#include <string>
#include "CommonDef.h"
const int MAX_USER_ID_LEN = 8;
//�û������������Ϣ��ֻ�����û���������
struct T_USER_BEAN
{
	std::string m_strF_USER_ID;//�û�ID��8���ֽڣ���ϵͳ����
    std::string m_strF_USER_NAME;//�û�����
    std::string m_strF_PASS_WORD;//�û�����
	std::string m_strF_NICK_NAME;//�û��ǳ�

	bool IsValid()  const {
		if (m_strF_USER_ID.length() > MAX_USER_ID_LEN ||
			m_strF_USER_ID.empty())
		{
			return false;
		}

		if (m_strF_USER_NAME.empty())
		{
			return false;
		}

		if (m_strF_PASS_WORD.empty())
		{
			return false;
		}

		if (m_strF_NICK_NAME.empty())
		{
			return false;
		}
		return true;
	}
};

//�û���������Ϣ������������
struct T_USER_INFO_BEAN
{
	std::string m_strF_USER_ID;//�û�ID��8���ֽڣ���ϵͳ����
	std::string m_strF_USER_NAME;//�û���
	std::string m_strF_ADDRESS;//��ַ
	std::string m_strF_BIRTH_DATE;//����
	std::string m_strF_EMAIL_ADDR;//����
	std::string m_strF_NICK_NAME;//�ǳ�
	std::string m_strF_SIGNATURE;//����ǩ��
	std::string m_strF_FACE_ID;//ͷ����
	CLIENT_ONLINE_TYPE m_eOnlineState;
};

//����������Ϣ
struct T_USER_CHAT_MSG 
{
	std::string m_strF_MSG_ID;//��ϢID
	CHAT_MSG_TYPE m_eChatMsgType;
	std::string m_strF_FROM_ID;//���ͷ�ID
	std::string m_strF_TO_ID;//���շ�ID
	std::string m_strF_MSG_CONTEXT;//��Ϣ����
	std::string m_strF_OTHER_INFO;
	std::string m_strF_READ_FLAG;//�Ѷ���ʶ(UNREAD  READ)
	std::string m_strF_CREATE_TIME;
	T_USER_CHAT_MSG() {
		m_eChatMsgType = CHAT_MSG_TYPE::E_CHAT_UNKNOWN_TYPE;
		m_strF_OTHER_INFO = "";
	}
};

struct T_GROUP_CHAT_MSG
{
	std::string m_strF_MSG_ID;//��ϢID
	CHAT_MSG_TYPE m_eChatMsgType;
	std::string m_strF_SENDER_ID;//������ID
	std::string m_strF_GROUP_ID;//Ⱥ��ID
	std::string m_strF_MSG_CONTEXT;//��Ϣ����
	std::string m_strF_OTHER_INFO;//��������Ϣ
	std::string m_strF_CREATE_TIME;
};

//��Ӻ�����Ϣ
struct T_ADD_FRIEND_MSG_BEAN {
	std::string m_strF_MSG_ID;//��ϢID
	std::string m_strF_USER_ID;//�����û����
	std::string m_strF_FRIEND_ID;//���ܷ��û���
	E_ADD_FRIEND_STATUS m_eF_ADD_FRIEND_STATUS;//���״̬
	E_FRIEND_OPTION m_eF_FRIEND_OPTION;//���ѡ��
};

struct T_USER_TEAM_BEAN {
	std::string m_strF_TEAM_ID;
	std::string m_strF_USER_NAME;
	std::string m_strF_TEAM_NAME;
};

struct T_GROUP_BEAN {
	std::string m_strF_GROUP_ID;
	std::string m_strF_GROUP_NAME;
};

struct T_GROUP_RELATION_BEAN {
	std::string m_strF_GROUP_ID;
	std::string m_strF_USER_ID;//USER_Name
	std::string m_strF_LAST_READ_MSG_ID;
	E_GROUP_MEMBER_ROLE m_eRole;
};
#endif