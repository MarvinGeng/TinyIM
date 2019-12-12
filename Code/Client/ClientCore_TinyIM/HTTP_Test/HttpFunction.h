#ifndef _DENNIS_THINK_HTTP_FUNCTION_H_
#define _DENNIS_THINK_HTTP_FUNCTION_H_
#include "CommonMsg.h"
UserRegisterRspMsg HttpUserRegister(const UserRegisterReqMsg& reqMsg);
UserUnRegisterRspMsg HttpUserUnRegister(const UserUnRegisterReqMsg& reqMsg);
#endif