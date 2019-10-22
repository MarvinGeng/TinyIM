#ifndef _DENNIS_THINK_UI_COMMON_STRUCT_H_
#define _DENNIS_THINK_UI_COMMON_STRUCT_H_


struct BuddyInfo
{
    UINT	uAccountID;
    TCHAR	szAccountName[32];
    TCHAR	szMarkName[32];
    long	nTeamIndex;
    long	nGroupCount;		//群个数
    UINT	GroupID[64];		//群号ID
};

//好友分组信息结构体
struct BuddyTeamInfo
{
    long	nTeamIndex;      //分组索引
    TCHAR	szTeamName[16];  //分组名称
};

#define MAX_LAST_MSG_TEXT_LENGTH 18
#define MAX_RECENT_NAME_LENGTH	 32

struct RecentInfo
{
	E_UI_CHAT_MSG_TYPE	nType;		//消息类型（好友聊天信息、群组、讨论组）
    UINT	uUserID;
    UINT	uFaceID;	//头像类型
    UINT64  uMsgTime;
    TCHAR   szName[MAX_RECENT_NAME_LENGTH];
    TCHAR   szLastMsgText[MAX_LAST_MSG_TEXT_LENGTH];
};

//记录当前添加好友的状态信息
struct AddFriendInfo_ST
{
	E_UI_OPERATE_FRIEND nCmd;				//操作类型（Apply/Agree/Refuse）
	UINT	uAccountID;			//账户ID
	TCHAR	szAccountName[32];	//账户名
	TCHAR	szNickName[32];		//昵称
};



struct UserBasicInfo_ST
{
	UINT  uAccountID;
	char  szAccountName[32];
	char  szNickName[64];
    char  szMarkName[64];
    char  szSignature[256];
    char  szAddress[51225];
	UINT  uFaceID;
    int   nStatus;              //在线状态
    int   clientType;           //客户端类型
    char  customFace[64];
    int   nGender;
    int   nBirthday;
    char  szPhoneNumber[64];
    char  szMail[64];
};

struct UserExtendInfo_ST
{
	UINT	uAccountID;
	UINT	uGender;
	UINT	uShengXiao;
	UINT	uConstel;
	UINT    uBlood;
	UINT	uBirthday;

	char	szCountry[32];			// 国家
	char	szProvince[32];			// 省份
	char	szCity[32];				// 城市
	char	szPhone[32];			// 电话
	char	szMobile[32];			// 手机
	char	szOccupation[32];		// 职业
	char	szCollege[32];			// 毕业院校
	char	szEmail[48];			// 邮箱
	char	szHomepage[64];			// 个人主页
	char	szSignature[256];		// 个性签名
	char	szPersonal[512];		// 个人说明	
	char    szCustomFace[MAX_PATH];	// 自定义头像名
};

struct FileProgress
{
	TCHAR szDestPath[MAX_PATH];
	long  nPercent;				//百分比，0～100之间(值为-1时，为获取文件md5值)
	long  nVerificationPercent;	//获取md5值进度（0～100）
};

#endif