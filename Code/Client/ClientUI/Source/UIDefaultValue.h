﻿
/**
 * @brief 对界面中使用的一些固定值进行定义
 * 
 */

#ifndef _DENNIS_THINK_UI_DEFAULT_VALUE_H_
#define _DENNIS_THINK_UI_DEFAULT_VALUE_H_

#define CHAT_BG_IMAGE_NAME			_T("BuddyChatDlgBg.png")
#define CHAT_EXPAND_BG_IMAGE_NAME   _T("BuddyChatDlgExpandBg.png")
//
const int USER_NAME_FONT_SIZE_DEFAULT = 9;
const int USER_CONTENT_MIN_FONT_SIZE = 12;
const int USER_CONTENT_MAX_FONT_SIZE = 48;

const long CHAT_DLG_WIDTH = 587;//聊天对话框宽度
const long CHAT_DLG_HEIGHT = 535;//聊天对话框的高度
const long CHAT_DLG_EXPAND_WIDTH = 960;//聊天对话框扩展宽度
const long RIGHT_CHAT_WINDOW_WIDTH = CHAT_DLG_EXPAND_WIDTH - CHAT_DLG_WIDTH;//历史信息控件的宽度


#define CHAT_BG_IMAGE_NAME			_T("BuddyChatDlgBg.png")
#define CHAT_EXPAND_BG_IMAGE_NAME   _T("BuddyChatDlgExpandBg.png")

const long GROUP_CHAT_DLG_WIDTH = 670/*587*/;//群组聊天对话框的宽度
const long GROUP_CHAT_DLG_HEIGHT = 600;//群组聊天对话框的高度
//Dennis Begin
const long GROUP_DLG_MIN_HEIGHT = 600; //群聊窗口最小高度
const long GROUP_DLG_MIN_WIDTH = 600;  //群聊窗口最小宽度
const long GROUP_DLG_TITLE_HEIGHT = 70;//群聊天标题栏高度
const long GROUP_DLG_OUT_BORDER_WIDTH = 3;//外边框宽度3像素
const long GROUP_DLG_IN_BORDER_WIDTH = 1;//内部边框一个像素
const long GROUP_DLG_MEMBER_LIST_WIDTH = 180;//群聊窗口成员
const long GROUP_DLG_MIN_EDIT_WIDTH = 420;//编辑区域最小宽度
const long GROUP_DLG_TOOL_BAR_HEIGHT = 36;//工具栏高度
const long GROUP_DLG_NOTICE_HEIGHT = 180; //公告的高度
const double GROUP_DLG_RECV_EDIT_PERCENT = 0.7; //左侧区域接收部分占比
const long GROUP_DLG_STATIC_TEXT_HEIGHT = 20;
//Dennis End
const long GROUP_MEMBER_LIST_WIDTH = 200;//群组成员列表控件宽度
const long GROUP_CHAT_DLG_EXTEND_WIDTH = 1043;//扩展历史消息后的宽度
const long GROUP_DLG_MSG_LOG_WIDTH = 373;//群组历史消息宽度


//Team
#define DEFAULT_TEAM_ID "10000000"
#endif