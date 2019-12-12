#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#include "HttpFunction.h"

TEST_CASE("UserRegister") {
	std::string strUserRegiser = R"({"MsgId": "12241864922023337984", "NickName": "TestUser2", "Password": "TestUser2", "UserName": "TestUser2"})";
	UserRegisterReqMsg reqMsg;
	reqMsg.FromString(strUserRegiser);
	UserRegisterRspMsg rspMsg = HttpUserRegister(reqMsg);
	CHECK_EQ(rspMsg.m_eErrCode, ERROR_CODE_TYPE::E_CODE_SUCCEED);

	std::string strUnRegister = R"( {"MsgId": "12241864940176285696", "Password": "TestUser2", "UserName": "TestUser2", "VerifyCode": ""})";
	UserUnRegisterReqMsg unReqMsg;
	unReqMsg.FromString(strUnRegister);
	UserUnRegisterRspMsg unRspMsg = HttpUserUnRegister(unReqMsg);
	CHECK_EQ(rspMsg.m_eErrCode, ERROR_CODE_TYPE::E_CODE_SUCCEED);
}



